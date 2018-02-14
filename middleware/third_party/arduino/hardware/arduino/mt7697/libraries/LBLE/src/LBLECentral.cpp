#include <Arduino.h>
#include "LBLECentral.h"
#include <stdlib.h>
#include <vector>
#include <algorithm>

extern "C" {
#include "utility/ard_ble.h"
#include "utility/ard_bt_company_id.h"
#include "utility/ard_bt_service_id.h"
#include <bt_gattc.h>
}

//////////////////////////////////////////////////////////////////////////////////
// Singlelton object definition
//////////////////////////////////////////////////////////////////////////////////
LBLECentralClass LBLECentral;

//////////////////////////////////////////////////////////////////////////////////
// LBLEAdvertisements
//////////////////////////////////////////////////////////////////////////////////
LBLEAdvertisements::LBLEAdvertisements(const bt_gap_le_advertising_report_ind_t& adv):
    adv_data(adv)
{

}

uint32_t LBLEAdvertisements::getAdvDataWithTypeFromPayload(uint8_t type, uint8_t* dstBuf, uint32_t bufLen, const bt_gap_le_advertising_report_ind_t& payload)
{
    uint32_t ad_data_len = 0;
    uint8_t ad_data_type = 0;

    for (uint32_t cursor = 0; cursor < payload.data_length; cursor += (ad_data_len + 1))
    {
        ad_data_len = payload.data[cursor];

        /* Error handling for data length over 30 bytes. */
        if (ad_data_len > 0x1F)
        {
            return 0;
        }

        ad_data_type = payload.data[cursor+1];

        if (ad_data_type == type)
        {
            // found, copy the data
            // note that ad_data_len INCLUDES the ad_data_type bytes.
            const uint32_t output_len = (ad_data_len - 1);

            if(dstBuf && bufLen >= output_len)
            {
                memcpy(dstBuf, payload.data + cursor +2, output_len);
            }
            return output_len;
        }
    }
    return 0;
}

uint32_t LBLEAdvertisements::getAdvDataWithType(uint8_t type, uint8_t* dstBuf, uint32_t bufLen) const
{
    // search the ad payload in adv_data first.
    // Note that currently we ignore the scan response data.
    return getAdvDataWithTypeFromPayload(type, dstBuf, bufLen, adv_data);
}

bool LBLEAdvertisements::isValid() const
{
    return (adv_data.data_length > 0);
}

String LBLEAdvertisements::getAddress() const
{
    //TODO: implement device address parser
    return String();
}

String LBLEAdvertisements::getName() const
{
    uint8_t dataBuf[MAX_ADV_DATA_LEN + 1] = {0}; // extra 1 byte to ensure NULL-termination.

    if(getAdvDataWithType(BT_GAP_LE_AD_TYPE_NAME_SHORT, dataBuf, MAX_ADV_DATA_LEN))
    {
        return String((const char*)dataBuf);
    }
    else if(getAdvDataWithType(BT_GAP_LE_AD_TYPE_NAME_COMPLETE, dataBuf, MAX_ADV_DATA_LEN))
    {
        return String((const char*)dataBuf);
    }

    return String();
}

int8_t LBLEAdvertisements::getTxPower() const
{
    uint8_t dataBuf[MAX_ADV_DATA_LEN + 1] = {0}; // extra 1 byte to ensure NULL-termination.

    if(getAdvDataWithType(BT_GAP_LE_AD_TYPE_TX_POWER, dataBuf, MAX_ADV_DATA_LEN))
    {
        // TX Power is single byte.
        return *(uint8_t*)dataBuf;
    }

    return 0;
}

String LBLEAdvertisements::getManufacturer() const
{
    uint8_t dataBuf[MAX_ADV_DATA_LEN + 1] = {0}; // extra 1 byte to ensure NULL-termination.
    const uint32_t dataLen = getAdvDataWithType(BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC, dataBuf, MAX_ADV_DATA_LEN);

    if(dataLen)
    {
        const unsigned short vendorId = *(unsigned short*)(dataBuf);
        const char *name = getBluetoothCompanyName(vendorId);
        return String(name);
    }

    return String("Unknown");
}

uint8_t  LBLEAdvertisements::getAdvertisementFlag()const
{
    uint8_t dataBuf[MAX_ADV_DATA_LEN + 1] = {0}; // extra 1 byte to ensure NULL-termination.
    const uint32_t dataLen = getAdvDataWithType(BT_GAP_LE_AD_TYPE_FLAG, dataBuf, MAX_ADV_DATA_LEN);

    if(dataLen)
    {
        return dataBuf[0];
    }

    return 0;
}

LBLEUuid LBLEAdvertisements::getServiceUuid() const
{
    bt_uuid_t uuid_data = {0};
    uint8_t dataBuf[MAX_ADV_DATA_LEN + 1] = {0}; // extra 1 byte to ensure NULL-termination.
    uint32_t dataLen = 0;

    // Process "complete" uuid only - since we don't know how to handle partial uuid.
    // A "partial" uuid needs to be defined by the device manufacturer.
    dataLen = getAdvDataWithType(BT_GAP_LE_AD_TYPE_128_BIT_UUID_COMPLETE, dataBuf, MAX_ADV_DATA_LEN);
    if(dataLen)
    {
        memcpy(uuid_data.uuid, dataBuf, dataLen);
    }
    else if(getAdvDataWithType(BT_GAP_LE_AD_TYPE_32_BIT_UUID_COMPLETE, dataBuf, MAX_ADV_DATA_LEN))
    {
        bt_uuid_from_uuid32(&uuid_data, *(uint32_t*)dataBuf);
    }
    else if(getAdvDataWithType(BT_GAP_LE_AD_TYPE_16_BIT_UUID_COMPLETE, dataBuf, MAX_ADV_DATA_LEN))
    {
        bt_uuid_from_uuid16(&uuid_data, *(uint16_t*)dataBuf);
    }

    return LBLEUuid(uuid_data);
}

bool LBLEAdvertisements::getIBeaconInfo(LBLEUuid& uuid, uint16_t& major, uint16_t& minor, int8_t& txPower) const
{
    // for the iBeacon format, you may refer to
    // https://developer.mbed.org/blog/entry/BLE-Beacons-URIBeacon-AltBeacons-iBeacon/

    uint8_t dataBuf[MAX_ADV_DATA_LEN + 1] = {0}; // extra 1 byte to ensure NULL-termination.
    uint32_t dataLen = 0;
    do
    {
        dataLen = getAdvDataWithType(BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC, dataBuf, MAX_ADV_DATA_LEN);

        if(dataLen < 4)
            break;

        const unsigned short vendorId = *(unsigned short*)(dataBuf);

        if(0x4C != vendorId)
            break;

        unsigned char *iBeaconBuffer = (unsigned char*)(dataBuf + 2);

        const unsigned char beaconType = *(iBeaconBuffer++);
        const unsigned char beaconLength = *(iBeaconBuffer++);

        if(0x02 != beaconType)
            break;

        if(0x15 != beaconLength)
            break;

        // note that iBeacon UUID are reversed w.r.t. our BT system
        bt_uuid_t tmpUuid;
        for(int i = 0; i < 16; ++i)
        {
            tmpUuid.uuid[i] = iBeaconBuffer[15-i];
        }
        uuid = tmpUuid;
        iBeaconBuffer += 16;


        // note that the endian for major and minor are different.
        major = *((uint16_t*)iBeaconBuffer);
        major = (major >> 8) | ((major & 0xFF) << 8);
        iBeaconBuffer += 2;


        minor = *((uint16_t*)iBeaconBuffer);
        minor = (minor >> 8) | ((minor & 0xFF) << 8);
        iBeaconBuffer += 2;

        txPower = *(int8_t*)iBeaconBuffer;

        return true;

    } while (false);

    return false;
}


//////////////////////////////////////////////////////////////////////////////////
// LBLECentralClass
//////////////////////////////////////////////////////////////////////////////////
LBLECentralClass::LBLECentralClass():
    m_registered(false),
    m_scanning(false)
{
    // we delay the actual initialization to init(),
    // since this class is likely to
    // be instantiated as a static global object,
    // and we want to make sure the initialization
    // happens AFTER Wi-Fi initialization.
}

void LBLECentralClass::init()
{
    m_peripherals_found.reserve(MAX_DEVICE_LIST_SIZE);
    if(!m_registered)
    {
        // after scanning we'll receiving BT_GAP_LE_ADVERTISING_REPORT_IND multiple times.
        // so register and process it.
        LBLE.registerForEvent(BT_GAP_LE_ADVERTISING_REPORT_IND, this);

        // make sure we only register once
        m_registered = true;
    }
}

void LBLECentralClass::scan()
{
    if(m_scanning)
    {
        return;
    }

    bt_hci_cmd_le_set_scan_enable_t enbleSetting;
    enbleSetting.le_scan_enable = BT_HCI_ENABLE;
    enbleSetting.filter_duplicates = BT_HCI_ENABLE;		// Disable driver-level filter for duplicated adv.
    // We'll filter in our onEvent() handler.
    bt_hci_cmd_le_set_scan_parameters_t scan_para;
    scan_para.own_address_type = BT_HCI_SCAN_ADDR_RANDOM;
    scan_para.le_scan_type = BT_HCI_SCAN_TYPE_PASSIVE;		// Requesting scan-response from peripherals.
    // We use BT_HCI_SCAN_TYPE_PASSIVE because
    // we don't parse scan response data (yet).
    scan_para.scanning_filter_policy = BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS;
    scan_para.le_scan_interval = 0x0024;	// Interval between scans
    scan_para.le_scan_window = 0x0011;		// How long a scan keeps

    init();

    // reset scan list - the BLE device address usually changes overtime for privacy reasons.
    // and the "advertising interval" are usually in milliseconds.
    // so it does not mean much to keep old scann results.
    m_peripherals_found.clear();

    // set_scan is actually asynchronous - to ensure
    // proper calling sequence between scan() and stopScan(),
    // we wait for BT_GAP_LE_SET_SCAN_CNF.
    bool done = waitAndProcessEvent(
                    [&]()
    {
        bt_gap_le_set_scan(&enbleSetting, &scan_para);
    },

    BT_GAP_LE_SET_SCAN_CNF,

    [this](bt_msg_type_t, bt_status_t, void*)
    {
        m_scanning = true;
        return;
    }
                );

    if(!done)
    {
        pr_debug("bt_gap_le_set_scan failed!");
    }

    return;
}

void LBLECentralClass::stopScan()
{
    bt_hci_cmd_le_set_scan_enable_t enbleSetting;
    enbleSetting.le_scan_enable = BT_HCI_DISABLE;
    enbleSetting.filter_duplicates = BT_HCI_ENABLE;

    // set_scan is actually asynchronous - to ensure
    // m_peripherals_found does not get re-polulated,
    // we wait for BT_GAP_LE_SET_SCAN_CNF.
    waitAndProcessEvent(
        [&]()
    {
        pr_debug("calling bt_gap_le_set_scan with disable parameters")
        bt_gap_le_set_scan(&enbleSetting, NULL);
    },

    BT_GAP_LE_SET_SCAN_CNF,

    [this](bt_msg_type_t, bt_status_t, void*)
    {
        m_scanning = false;
        return;
    }
    );

    // we keep the BT_GAP_LE_ADVERTISING_REPORT_IND registered,
    // assuming that the underlying framework won't send any new messages
    return;
}

int LBLECentralClass::getPeripheralCount()
{
    return m_peripherals_found.size();
}

String LBLECentralClass::getAddress(int index)
{
    LBLEAddress addr(m_peripherals_found[index].address);
    return addr.toString();
}

LBLEAddress LBLECentralClass::getBLEAddress(int index)
{
    return LBLEAddress(m_peripherals_found[index].address);
}

String LBLECentralClass::getName(int index)
{
    LBLEAdvertisements parser(m_peripherals_found[index]);
    return parser.getName();
}

int32_t LBLECentralClass::getRSSI(int index)
{
    return m_peripherals_found[index].rssi;
}

int32_t LBLECentralClass::getTxPower(int index)
{
    LBLEAdvertisements parser(m_peripherals_found[index]);
    return parser.getTxPower();
}

LBLEUuid LBLECentralClass::getServiceUuid(int index) const
{
    LBLEAdvertisements parser(m_peripherals_found[index]);
    return parser.getServiceUuid();
}

bool LBLECentralClass::isIBeacon(int index) const
{
    LBLEAdvertisements parser(m_peripherals_found[index]);

    LBLEUuid uuid;
    uint16_t major, minor;
    int8_t txPower;
    return parser.getIBeaconInfo(uuid, major, minor, txPower);
}

bool LBLECentralClass::getIBeaconInfo(int index, LBLEUuid& uuid, uint16_t& major, uint16_t& minor, int8_t& txPower) const
{
    LBLEAdvertisements parser(m_peripherals_found[index]);
    return parser.getIBeaconInfo(uuid, major, minor, txPower);
}

String LBLECentralClass::getManufacturer(int index) const
{
    LBLEAdvertisements parser(m_peripherals_found[index]);
    return parser.getManufacturer();
}

uint8_t  LBLECentralClass::getAdvertisementFlag(int index) const
{
    LBLEAdvertisements parser(m_peripherals_found[index]);
    return parser.getAdvertisementFlag();
}

void LBLECentralClass::onEvent(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    if(BT_GAP_LE_ADVERTISING_REPORT_IND == msg)
    {
        const bt_gap_le_advertising_report_ind_t* pReport = (bt_gap_le_advertising_report_ind_t*)buff;

        switch(pReport->event_type)
        {
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_IND:
        {
            // advertising packet - check if we need to update existing entry
            // or appending new one.
            const size_t peripheralCount = m_peripherals_found.size();
            for(size_t i = 0; i < peripheralCount; ++i)
            {
                // check if already found
                if(LBLEAddress::equal_bt_address(m_peripherals_found[i].address, pReport->address))
                {
                    m_peripherals_found[i] = *pReport;
                    return;
                }
            }

            // not found, append
            if(m_peripherals_found.size() < MAX_DEVICE_LIST_SIZE)
            {
                m_peripherals_found.push_back(*pReport);

                // DEBUG:
#if 1
                LBLEAddress newAddr(pReport->address);
                pr_debug("new record [%s], total found: %d", newAddr.toString().c_str(), m_peripherals_found.size());
#endif
            }
            else
            {
                pr_debug("m_peripherals_found size exceeding %d, drop.", MAX_DEVICE_LIST_SIZE);
            }
            return;
        }
        break;
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_SCAN_RSP:
            // TODO: scan response - this usually carries extra information
            // we need to find matching address in g_peripherals_found list,
            // and insert the scan response info to that entry.
            break;
        default:
            break;
        }
    }
}

///////////////////////////////////////////////////////////////
//		LBLEClient
///////////////////////////////////////////////////////////////
LBLEClient::LBLEClient():
    m_connection(BT_HANDLE_INVALID)
{

}

LBLEClient::~LBLEClient()
{
    // ensure we are removed from the global table
    LBLE.unregisterForEvent(BT_GAP_LE_DISCONNECT_IND, this);
}

bool LBLEClient::connect(const LBLEAddress& address)
{
    bt_hci_cmd_le_create_connection_t conn_para;
    conn_para.le_scan_interval = 0x0010;
    conn_para.le_scan_window = 0x0010;
    conn_para.initiator_filter_policy = BT_HCI_CONN_FILTER_ASSIGNED_ADDRESS;
    conn_para.own_address_type = BT_ADDR_RANDOM;
    conn_para.conn_interval_min = 0x0006;
    conn_para.conn_interval_max = 0x0080;
    conn_para.conn_latency = 0x0000;
    conn_para.supervision_timeout = 0x07d0;
    conn_para.minimum_ce_length = 0x0000;
    conn_para.maximum_ce_length = 0x0050;

    conn_para.peer_address.type = address.m_addr.type;
    memcpy(conn_para.peer_address.addr, address.m_addr.addr, BT_BD_ADDR_LEN);

    // call bt_gap_le_connect, wait for BT_GAP_LE_CONNECT_IND
    // and store the connection_handle to this->m_connection
    bool done = waitAndProcessEvent(
                    // Call connect API
                    [&]()
    {
        bt_status_t result = bt_gap_le_connect(&conn_para);
        if(BT_STATUS_SUCCESS != result)
        {
            pr_debug("bt_gap_le_connect() failed with ret=0x%x", result);
        }
    },
    // wait for connect indication event...
    BT_GAP_LE_CONNECT_IND,
    // to update the `m_connection` member in BT task
    [this](bt_msg_type_t, bt_status_t, void* buf)
    {
        pr_debug("bt_gap_le_connect() recieves BT_GAP_LE_CONNECT_IND");
        const bt_gap_le_connection_ind_t *pConnectionInfo = (bt_gap_le_connection_ind_t*)buf;
        this->m_connection = pConnectionInfo->connection_handle;
        LBLE.registerForEvent(BT_GAP_LE_DISCONNECT_IND, this);
        return;
    }
                );

    if(done)
    {
        // we populate services in advance for ease of use
        discoverServices();
    }
    else
    {
        // not receiving BT_GAP_LE_CONNECT_IND:
        // we must "cancel" the connection otherwise the internal buffer for connection can leak.
        bool cancelled = waitAndProcessEvent(
                             // Call connect API
                             [&]()
        {
            bt_status_t result = bt_gap_le_cancel_connection();
            if(BT_STATUS_SUCCESS != result)
            {
                pr_debug("bt_gap_le_cancel_connection() failed with ret=0x%x", result);
            }
        },
        // wait for connect indication event...
        BT_GAP_LE_CONNECT_CANCEL_CNF,

        // and that's it - we don't have client side buffers to release.
        [this](bt_msg_type_t, bt_status_t, void* buf)
        {
            pr_debug("bt_gap_le_cancel_connection() recieves BT_GAP_LE_CONNECT_CANCEL_CNF");
            return;
        }
                         );
        if(!cancelled)
        {
            // oh no - we failed to cancel - this is unlikely to happen.
            pr_debug("WARNING: connect failed and bt_gap_le_cancel_connection() also failed!");
        }
    }

    return done;
}

bool LBLEClient::connected()
{
    return (m_connection != BT_HANDLE_INVALID);
}

/// Disconnect from the remote device
void LBLEClient::disconnect()
{
    if(!connected())
    {
        return;
    }

    pr_debug("disconect handle = %d", m_connection);

    // actively remove event handler - we are going to
    // disconnect by ourselves, so there is no need
    // to listen anymore.
    LBLE.unregisterForEvent(BT_GAP_LE_DISCONNECT_IND, this);

    bt_hci_cmd_disconnect_t disconn_para = {0};
    disconn_para.connection_handle = m_connection;
    disconn_para.reason = 0x13; //  REMOTE USER TERMINATED CONNECTION

    // call bt_gap_le_connect, wait for BT_GAP_LE_CONNECT_IND
    // and store the connection_handle to this->m_connection
    bool done = waitAndProcessEvent(
                    // Call connect API
                    [&disconn_para]()
    {
        bt_gap_le_disconnect(&disconn_para);
    },
    // wait for connect indication event...
    BT_GAP_LE_DISCONNECT_IND,
    // to update the `m_connection` member in BT task
    [this](bt_msg_type_t, bt_status_t, void* buf)
    {
        const bt_gap_le_disconnect_ind_t *pDisconnectInfo = (bt_gap_le_disconnect_ind_t*)buf;
        if(this->m_connection == pDisconnectInfo->connection_handle)
        {
            this->m_connection = BT_HANDLE_INVALID;
            // sucess or not, we proceed to resource clean up
            m_connection = BT_HANDLE_INVALID;
            m_services.clear();
            m_uuid2Handle.clear();
        }
        return;
    }
                );

    if(!done)
    {
        pr_debug("bt_gap_le_disconnect() called but fails to receive BT_GAP_LE_DISCONNECT_IND");
    }
}

void LBLEClient::onEvent(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    // note that we rely on the "isOnce()" behavior
    // to remove this event handler after invocation.
    if(BT_GAP_LE_DISCONNECT_IND == msg)
    {
        const bt_gap_le_disconnect_ind_t* pInfo = (bt_gap_le_disconnect_ind_t*)buff;
        if(pInfo->connection_handle == m_connection)
        {
            pr_debug("Disconnected with status = 0x%x, reason = 0x%x", (unsigned int)status, (unsigned int)pInfo->reason);
            // clear up scanned services and characteristics
            m_connection = BT_HANDLE_INVALID;
            m_services.clear();
            m_uuid2Handle.clear();
        }
    }
}

int LBLEClient::getServiceCount()
{
    return m_services.size();
}

bool LBLEClient::hasService(const LBLEUuid& uuid)
{
    auto found = std::find_if(m_services.begin(), m_services.end(), [&uuid](const LBLEServiceInfo& o) {
        return (bool)(o.uuid == uuid);
    });
    return (m_services.end() != found);
}

/// Get the number of Characteristics available on the connected remote device
///
/// \param index ranges from 0 to (getServiceCount() - 1).
/// \returns number of characteristics in the service.
int LBLEClient::getCharacteristicCount(int serviceIndex) {
    if (serviceIndex < 0 || serviceIndex >= getServiceCount()) {
        return 0;
    }

    return m_services[serviceIndex].m_characteristics.size();
}

LBLECharacteristicInfo LBLEClient::getCharacteristic(int serviceIndex, int characteristicIndex)
{
    if (serviceIndex < 0 || serviceIndex >= getServiceCount()) {
        return LBLECharacteristicInfo();
    }

    if (characteristicIndex < 0 || characteristicIndex >= getCharacteristicCount(serviceIndex)) {
        return LBLECharacteristicInfo();
    }

    return m_services[serviceIndex].m_characteristics[characteristicIndex];
}

LBLEUuid LBLEClient::getCharacteristicUuid(int serviceIndex, int characteristicIndex) {
    return getCharacteristic(serviceIndex, characteristicIndex).m_uuid;
}

LBLEUuid LBLEClient::getServiceUuid(int index)
{
    if(index < 0 || index >= getServiceCount())
    {
        return LBLEUuid();
    }

    return m_services[index].uuid;
}

String LBLEClient::getServiceName(int index)
{
    const LBLEUuid uuid = getServiceUuid(index);
    if(uuid.is16Bit())
    {
        const char* name = getBluetoothServiceName(uuid.getUuid16());
        if(name)
        {
            return String(name);
        }
    }
    return String();
}

int LBLEClient::discoverServices()
{
    // discover all services and store UUIDs
    // note that this search could require multiple request-response,
    // see https://community.nxp.com/thread/332233 for an example.

    if(!connected())
    {
        return 0;
    }

    // Prepare the search request - we will reuse this structure
    // across multiple requests, by updating the **starting_handle**
    bt_gattc_discover_primary_service_req_t searchRequest = {
        .opcode = BT_ATT_OPCODE_READ_BY_GROUP_TYPE_REQUEST,
        .starting_handle = 0x0001,
        .ending_handle = 0xFFFF,
        .type16 = BT_GATT_UUID16_PRIMARY_SERVICE
    };

    // serviceFound is the output variable,
    // shouldContinue is the stopping conditon. They will be updated in the event process lambda below.
    bool shouldContinue = false;
    bool done = false;

    // start searching from 1
    do
    {
        if(!connected())
        {
            pr_debug("disconnected (maybe by remote device) during enumerating services");
            break;
        }

        done = waitAndProcessEventMultiple(
                   // start service discovery
                   [this, &searchRequest]()
        {
            bt_gattc_discover_primary_service(m_connection, &searchRequest);
        },
        // wait for the event...
        BT_GATTC_DISCOVER_PRIMARY_SERVICE,
        // and process it in BT task context with this lambda
        [this, &searchRequest, &shouldContinue](bt_msg_type_t, bt_status_t status, void* buf) -> bool
        {
            // Parse the response to service UUIDs. It can be 16-bit or 128-bit UUID.
            // We can tell the difference from the response length `att_rsp->length`.
            const bt_gattc_read_by_group_type_rsp_t* rsp = (bt_gattc_read_by_group_type_rsp_t*)buf;
            uint16_t endIndex = 0, startIndex = 0;
            uint16_t uuid16 = 0;
            bt_uuid_t uuid128;
            const uint8_t *attrDataList = rsp->att_rsp->attribute_data_list;
            const uint32_t entryLength = rsp->att_rsp->length;
            // note that att_rsp will always have same length of each uuid-16 service,
            // because a response can only represent a single uuid-128 service.
            const uint8_t entryCount = (rsp->length - 2) / entryLength;
            for (int i = 0; i < entryCount; ++i) {
                memcpy(&startIndex, attrDataList + i * entryLength, 2);
                memcpy(&endIndex, attrDataList + i * entryLength + 2, 2);

                LBLEServiceInfo serviceInfo;

                if (entryLength == 6) {
                    // 16-bit UUID case
                    memcpy(&uuid16, attrDataList + i * entryLength + 4, entryLength - 4);
                    serviceInfo.uuid = LBLEUuid(uuid16);
                } else {
                    // 128-bit UUID case
                    memcpy(&uuid128.uuid, attrDataList + i * entryLength + 4, entryLength - 4);
                    serviceInfo.uuid = LBLEUuid(uuid128);
                }

                serviceInfo.startHandle = startIndex;
                serviceInfo.endHandle = endIndex;

                m_services.push_back(serviceInfo);
            }

            // check if we need to perform more search
            shouldContinue = (status == BT_ATT_ERRCODE_CONTINUE);

            // update starting handle for next round of search
            searchRequest.starting_handle = endIndex;

            // if we should continue, return FALSE so that
            // the handler will keep waiting for the next event.
            return !shouldContinue;
        }
               );
    } while(shouldContinue && done);

    if(m_services.size())
    {
        discoverCharacteristics();
    }

    return m_services.size();
}

int LBLEClient::discoverCharacteristics()
{
    if(!connected())
    {
        return 0;
    }

    int totalCharacteristicCount = 0;
    // for each service, find the value handle of each characteristic.
    for(size_t i = 0; i < m_services.size(); ++i)
    {
        pr_debug("Enumerate charc between handle (%d-%d)", m_services[i].startHandle, m_services[i].endHandle)
        totalCharacteristicCount += discoverCharacteristicsOfService(i);
    }

    return totalCharacteristicCount;
}

int LBLEClient::discoverCharacteristicsOfService(int serviceIndex)
{
    // discover all characteristics and store mapping between (UUID -> value handle)
    // note that this search could require multiple request-response,
    // see https://community.nxp.com/thread/332233 for an example.

    if(!connected())
    {
        return 0;
    }

    LBLEServiceInfo& service = m_services[serviceIndex];
    const LBLEServiceInfo& s = service;

    // Prepare the search request - we will reuse this structure
    // across multiple requests, by updating the **starting_handle**
    bt_gattc_discover_charc_req_t searchRequest = {
        .opcode = BT_ATT_OPCODE_READ_BY_TYPE_REQUEST,
        .starting_handle = s.startHandle,
        .ending_handle = s.endHandle,
        .type = {0}
    };
    uint16_t charcUuid = BT_GATT_UUID16_CHARC;
    bt_uuid_load(&searchRequest.type, (void*)&charcUuid, 2);

    // serviceFound is the output variable,
    // shouldContinue is the stopping conditon. They will be updated in the event process lambda below.
    bool shouldContinue = false;
    bool done = false;

    // start searching from 1
    do
    {
        if(!connected())
        {
            pr_debug("disconnected (maybe by remote device) during enumerating characteristics");
            break;
        }

        done = waitAndProcessEventMultiple(
                   // start characteristic discovery
                   [this, &searchRequest]()
        {
            bt_gattc_discover_charc(m_connection, &searchRequest);
        },
        // wait for the event...
        BT_GATTC_DISCOVER_CHARC,
        // and process it in BT task context with this lambda
        [this, &searchRequest, &shouldContinue, &service](bt_msg_type_t, bt_status_t status, void* buf) -> bool
        {
            pr_debug("discoverCharacteristicsOfService=0x%x", (unsigned int)status);

            // Parse the response to service UUIDs. It can be 16-bit or 128-bit
            const bt_gattc_read_by_type_rsp_t* rsp = (bt_gattc_read_by_type_rsp_t*)buf;
            uint16_t attributeHandle = 0, valueHandle = 0;
            uint8_t properties = 0;
            uint16_t uuid16 = 0;
            bt_uuid_t uuid128;
            const uint8_t *attrDataList = rsp->att_rsp->attribute_data_list;
            const uint32_t entryLength = rsp->att_rsp->length;
            // note that att_rsp will always have same length of each uuid-16 service,
            // because a response can only represent a single uuid-128 service.
            const uint8_t entryCount = (rsp->length - 2) / entryLength;
            for (int i = 0; i < entryCount; ++i) {
                memcpy(&attributeHandle, attrDataList + i * rsp->att_rsp->length, 2);
                memcpy(&properties, attrDataList + i * rsp->att_rsp->length + 2, 1);
                memcpy(&valueHandle, attrDataList + i * rsp->att_rsp->length + 3, 2);

                LBLEUuid uuidInsert;
                if (rsp->att_rsp->length < 20) {
                    // 16-bit UUID case
                    memcpy(&uuid16, attrDataList + i * rsp->att_rsp->length + 5, 2);
                    uuidInsert = LBLEUuid(uuid16);
                } else {
                    // 128-bit UUID case
                    memcpy(&uuid128.uuid, attrDataList + i * entryLength + 5, 16);
                    uuidInsert = LBLEUuid(uuid128);
                }

                // insert into LBLEClient's (UUID -> Handle) mapping table if it is not already in the map
                m_uuid2Handle.insert(std::make_pair(uuidInsert, valueHandle));

                // insert into LBLEServiceInfo's characteristic list
                service.m_characteristics.push_back(LBLECharacteristicInfo(valueHandle, uuidInsert));
            }

            // check if we need to perform more search
            shouldContinue = (status == BT_ATT_ERRCODE_CONTINUE) && (entryCount > 0);

            // update starting handle for next round of search
            searchRequest.starting_handle = valueHandle;

            // if we should continue, return FALSE so that
            // the handler will keep waiting for the next event.
            return !shouldContinue;
        }
               );
    } while(shouldContinue && done);

    return done;
}

////////////////////////////////////////////////////////////////////////////////
//          LBLECharacteristicInfo-based read and write
////////////////////////////////////////////////////////////////////////////////
LBLEValueBuffer LBLECharacteristicInfo::read(const bt_handle_t connection) const
{
    // connected, call bt_gattc_read_charc and wait for response event
    BT_GATTC_NEW_READ_CHARC_REQ(req, m_handle)

    // results are stored here by the event callback below.
    LBLEValueBuffer resultBuffer;

    waitAndProcessEvent(
        // start read request
        [&]()
    {
        bt_gattc_read_charc(connection, &req);
    },
    // wait for event...
    BT_GATTC_READ_CHARC,
    // and parse event result in bt task context
    [this, &resultBuffer, connection](bt_msg_type_t msg, bt_status_t, void* buf)
    {
        const bt_gattc_read_rsp_t *pReadResponse = (bt_gattc_read_rsp_t*)buf;
        if(BT_GATTC_READ_CHARC != msg || pReadResponse->connection_handle != connection)
        {
            pr_debug("not for our request, ignored");
            return;
        }

        do {
            // check error case
            if(BT_ATT_OPCODE_ERROR_RESPONSE == pReadResponse->att_rsp->opcode)
            {
                pr_debug("error reading attribute");
                pr_debug("error_opcode=%d", reinterpret_cast<bt_gattc_error_rsp_t*>(buf)->att_rsp->error_opcode);
                pr_debug("error_code=%d", reinterpret_cast<bt_gattc_error_rsp_t*>(buf)->att_rsp->error_code);
                break;
            }

            if(BT_ATT_OPCODE_READ_RESPONSE != pReadResponse->att_rsp->opcode)
            {
                pr_debug("Op code don't match! Opcode=%d", pReadResponse->att_rsp->opcode);
                break;
            }

            // Copy the data buffer content
            const uint8_t attrLength = pReadResponse->length - sizeof(pReadResponse->att_rsp->opcode);
            resultBuffer.resize(attrLength, 0);
            memcpy(&resultBuffer[0], ((uint8_t*)pReadResponse->att_rsp->attribute_value), attrLength);
        } while(false);
    }
    );

    return resultBuffer;
}

int LBLECharacteristicInfo::readInt(bt_handle_t connection) {
    return read(connection).asInt();
}

String LBLECharacteristicInfo::readString(bt_handle_t connection) {
    return read(connection).asString();
}

char LBLECharacteristicInfo::readChar(bt_handle_t connection) {
    return read(connection).asChar();
}

float LBLECharacteristicInfo::readFloat(bt_handle_t connection) {
    return read(connection).asFloat();
}

int LBLECharacteristicInfo::write(const bt_handle_t connection, const LBLEValueBuffer& value)
{
    // make sure this is smaller than MTU - header size
    const uint32_t MAXIMUM_WRITE_SIZE = 20;

    // value buffer too big
    if(value.size() > MAXIMUM_WRITE_SIZE)
    {
        pr_debug("write value too long");
        return 0;
    }

    LBLEValueBuffer reqBuf;
    reqBuf.resize(value.size() + sizeof(bt_att_write_req_t));

    bt_gattc_write_charc_req_t req;
    req.attribute_value_length = value.size();
    req.att_req = (bt_att_write_req_t*)&reqBuf[0];
    req.att_req->opcode = BT_ATT_OPCODE_WRITE_REQUEST;
    // the "attribute_handle" field requires a "value handle" actually.
    pr_debug("write_charc %s with value handle=%d", m_uuid.toString().c_str(), m_handle);
    req.att_req->attribute_handle = m_handle;
    memcpy(req.att_req->attribute_value, &value[0], value.size());

    bool done = waitAndProcessEvent(
                    // start read request
                    [&]()
    {
        const bt_status_t r = bt_gattc_write_charc(connection, &req);
        pr_debug("write result = 0x%x", r);
    },
    // wait for event...
    BT_GATTC_WRITE_CHARC,
    // and parse event result in bt task context
    [this, connection](bt_msg_type_t msg, bt_status_t, void* buf)
    {
        if(BT_GATTC_WRITE_CHARC != msg) {
            // not for our request
            pr_debug("got wrong messge");
            return;
        }

        const bt_gattc_write_rsp_t *pWriteResp = (bt_gattc_write_rsp_t*)buf;
        if(pWriteResp->connection_handle != connection) {
            pr_debug("got wrong handle=%d (%d)", pWriteResp->connection_handle, connection);
        }

        do {
            // check error case
            if(BT_ATT_OPCODE_ERROR_RESPONSE == pWriteResp->att_rsp->opcode)
            {
                pr_debug("error reading attribute");
                pr_debug("error_opcode=%d", reinterpret_cast<bt_gattc_error_rsp_t*>(buf)->att_rsp->error_opcode);
                pr_debug("error_code=%d", reinterpret_cast<bt_gattc_error_rsp_t*>(buf)->att_rsp->error_code);
                break;
            }

            if(BT_ATT_OPCODE_WRITE_RESPONSE != pWriteResp->att_rsp->opcode)
            {
                pr_debug("Op code don't match! Opcode=%d", pWriteResp->att_rsp->opcode);
                break;
            }
        } while(false);
    }
                );

    return done;
}

int LBLECharacteristicInfo::writeInt(bt_handle_t connection, int value) {
    const LBLEValueBuffer b(value);
    return write(connection, b);
}

int LBLECharacteristicInfo::writeString(bt_handle_t connection, const String& value) {
    const LBLEValueBuffer b(value);
    return write(connection, b);
}

int LBLECharacteristicInfo::writeChar(bt_handle_t connection, char value) {
    const LBLEValueBuffer b(value);
    return write(connection, b);
}

int LBLECharacteristicInfo::writeFloat(bt_handle_t connection, float value) {
    const LBLEValueBuffer b(value);
    return write(connection, b);
}

////////////////////////////////////////////////////////////////////////////////
//          Index-based read
////////////////////////////////////////////////////////////////////////////////

LBLEValueBuffer LBLEClient::readCharacteristic(int serviceIndex, int characteristicIndex)
{
    return getCharacteristic(serviceIndex, characteristicIndex).read(m_connection);
}

int LBLEClient::readCharacteristicInt(int serviceIndex, int characteristicIndex)
{
    return readCharacteristic(serviceIndex, characteristicIndex).asInt();
}

String LBLEClient::readCharacteristicString(int serviceIndex, int characteristicIndex)
{
    return LBLEClient::readCharacteristic(serviceIndex, characteristicIndex).asString();
}

char LBLEClient::readCharacteristicChar(int serviceIndex, int characteristicIndex)
{
    return LBLEClient::readCharacteristic(serviceIndex, characteristicIndex).asChar();
}

float LBLEClient::readCharacteristicFloat(int serviceIndex, int characteristicIndex)
{
    return readCharacteristic(serviceIndex, characteristicIndex).asFloat();
}

////////////////////////////////////////////////////////////////////////////////
//          Index-based write
////////////////////////////////////////////////////////////////////////////////

int LBLEClient::writeCharacteristic(int serviceIndex, int characteristicIndex, const LBLEValueBuffer& value)
{
    return getCharacteristic(serviceIndex, characteristicIndex).write(m_connection, value);
}

int LBLEClient::writeCharacteristicInt(int serviceIndex, int characteristicIndex, int value)
{
    return writeCharacteristic(serviceIndex, characteristicIndex, LBLEValueBuffer(value));
}

int LBLEClient::writeCharacteristicString(int serviceIndex, int characteristicIndex, const String& value)
{
    return writeCharacteristic(serviceIndex, characteristicIndex, LBLEValueBuffer(value));
}

int LBLEClient::writeCharacteristicChar(int serviceIndex, int characteristicIndex, char value)
{
    return writeCharacteristic(serviceIndex, characteristicIndex, LBLEValueBuffer(value));
}

int LBLEClient::writeCharacteristicFloat(int serviceIndex, int characteristicIndex, float value)
{
    return writeCharacteristic(serviceIndex, characteristicIndex, LBLEValueBuffer(value));
}

////////////////////////////////////////////////////////////////////////////////
//          UUID-based read
////////////////////////////////////////////////////////////////////////////////

LBLEValueBuffer LBLEClient::readCharacteristic(const LBLEUuid& uuid)
{
    if(!connected())
    {
        return LBLEValueBuffer();
    }

    // connected, call bt_gattc_read_using_charc_uuid and wait for response event
    // we need to check if the UUID have a known value handle.
    // Note that discoverCharacteristic() builds the mapping table.
    auto found = m_uuid2Handle.find(uuid);
    if(found == m_uuid2Handle.end())
    {
        pr_debug("cannot find characteristics");
        return 0;
    }

    LBLECharacteristicInfo c(found->second, found->first);
    return c.read(m_connection);
}

int LBLEClient::readCharacteristicInt(const LBLEUuid& uuid)
{
    return readCharacterstic(uuid).asInt();
}

String LBLEClient::readCharacteristicString(const LBLEUuid& uuid)
{
    return readCharacterstic(uuid).asString();
}

char LBLEClient::readCharacteristicChar(const LBLEUuid& uuid)
{
    return readCharacterstic(uuid).asChar();
}

float LBLEClient::readCharacteristicFloat(const LBLEUuid& uuid)
{
    return readCharacterstic(uuid).asFloat();
}

LBLEValueBuffer LBLEClient::readCharacterstic(const LBLEUuid& serviceUuid)
{
    return readCharacteristic(serviceUuid);
}

////////////////////////////////////////////////////////////////////////////////
//          UUID-based write
////////////////////////////////////////////////////////////////////////////////

int LBLEClient::writeCharacteristic(const LBLEUuid& uuid, const LBLEValueBuffer& value)
{
    if(!connected())
    {
        return 0;
    }

    // we need to check if the UUID have a known value handle.
    // Note that discoverCharacteristic() builds the mapping table.
    auto found = m_uuid2Handle.find(uuid);
    if(found == m_uuid2Handle.end())
    {
        pr_debug("cannot find characteristics");
        return 0;
    }

    LBLECharacteristicInfo c(found->second, found->first);
    return c.write(m_connection, value);
}

int LBLEClient::writeCharacteristicInt(const LBLEUuid& uuid, int value) {
    const LBLEValueBuffer b(value);
    return writeCharacteristic(uuid, b);
}

int LBLEClient::writeCharacteristicString(const LBLEUuid& uuid, const String& value) {
    const LBLEValueBuffer b(value);
    return writeCharacteristic(uuid, b);
}

int LBLEClient::writeCharacteristicChar(const LBLEUuid& uuid, char value) {
    const LBLEValueBuffer b(value);
    return writeCharacteristic(uuid, b);
}

int LBLEClient::writeCharacteristicFloat(const LBLEUuid& uuid, float value) {
    const LBLEValueBuffer b(value);
    return writeCharacteristic(uuid, b);
}
