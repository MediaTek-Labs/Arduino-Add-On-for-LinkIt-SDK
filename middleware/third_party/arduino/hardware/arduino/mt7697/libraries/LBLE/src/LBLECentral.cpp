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

bool LBLEAdvertisements::getIBeaconInfo(LBLEUuid& uuid, uint16_t& major, uint16_t& minor, uint8_t& txPower) const
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

        bt_uuid_t tmpUuid;
        memcpy(tmpUuid.uuid, iBeaconBuffer, sizeof(tmpUuid.uuid));
        uuid = tmpUuid;
        iBeaconBuffer += 16;

        major = *(unsigned short*)(iBeaconBuffer);
        iBeaconBuffer += 2;

        minor = *(unsigned short*)(iBeaconBuffer);
        iBeaconBuffer += 2;

        txPower = *(int8_t*)iBeaconBuffer;

        return true;

    } while (false);

    return false;
}


//////////////////////////////////////////////////////////////////////////////////
// LBLECentralClass
//////////////////////////////////////////////////////////////////////////////////
LBLECentralClass::LBLECentralClass()
{

}

void LBLECentralClass::scan()
{
    bt_hci_cmd_le_set_scan_enable_t enbleSetting;
    enbleSetting.le_scan_enable = BT_HCI_ENABLE;
    enbleSetting.filter_duplicates = BT_HCI_DISABLE;		// Disable driver-level filter for duplicated adv.
                                                            // We'll filter in our onEvent() handler.
    bt_hci_cmd_le_set_scan_parameters_t scan_para;
    scan_para.own_address_type = BT_HCI_SCAN_ADDR_RANDOM;
    scan_para.le_scan_type = BT_HCI_SCAN_TYPE_PASSIVE;		// Requesting scan-response from peripherals.
                                                            // We use BT_HCI_SCAN_TYPE_PASSIVE because
                                                            // we don't parse scan response data (yet).
    scan_para.scanning_filter_policy = BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS;
    scan_para.le_scan_interval = 0x0024;	// Interval between scans
    scan_para.le_scan_window = 0x0011;		// How long a scan keeps

    // after scanning we'll receiving BT_GAP_LE_ADVERTISING_REPORT_IND
    // so register and process it.
    LBLE.registerForEvent(BT_GAP_LE_ADVERTISING_REPORT_IND, this);

    // set_scan is actually asynchronous - but since
    // we don't have anything to check, we keep going
    // without waiting for BT_GAP_LE_SET_SCAN_CNF.
    bt_gap_le_set_scan(&enbleSetting, &scan_para);

    return;
}

void LBLECentralClass::stopScan()
{
    bt_hci_cmd_le_set_scan_enable_t enbleSetting;
    enbleSetting.le_scan_enable = BT_HCI_DISABLE;
    enbleSetting.filter_duplicates = BT_HCI_DISABLE;

    // set_scan is actually asynchronous - to ensure
    // m_peripherals_found does not get re-polulated,
    // we wait for BT_GAP_LE_SET_SCAN_CNF.
    waitAndProcessEvent(
                        [&]()
                        { 
                            bt_gap_le_set_scan(&enbleSetting, NULL);
                        },

                        BT_GAP_LE_SET_SCAN_CNF,

                        [this](bt_msg_type_t, bt_status_t, void*)
                        {
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
    uint8_t txPower;
    return parser.getIBeaconInfo(uuid, major, minor, txPower);
}

bool LBLECentralClass::getIBeaconInfo(int index, LBLEUuid& uuid, uint16_t& major, uint16_t& minor, uint8_t& txPower) const
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

static const char* get_event_type(uint8_t type)
{
    switch (type)
    {
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_IND:
            return "ADV_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_DIRECT_IND:
            return "ADV_DIRECT_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_SCAN_IND:
            return "ADV_SCAN_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_NONCONN_IND:
            return "ADV_NONCONN_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_SCAN_RSP:
            return "SCAN_RSP";
        default:
            return "NULL";
    }
}

void LBLECentralClass::onEvent(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    if(BT_GAP_LE_ADVERTISING_REPORT_IND == msg)
    {
        const bt_gap_le_advertising_report_ind_t* pReport = (bt_gap_le_advertising_report_ind_t*)buff;

        pr_debug("BT_GAP_LE_ADVERTISING_REPORT_IND with 0x%x", (unsigned int)status);
        pr_debug("advertisement event = %s", get_event_type(pReport->event_type));

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
                m_peripherals_found.push_back(*pReport);
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
                        bt_gap_le_connect(&conn_para);
                    },
                    // wait for connect indication event...
                    BT_GAP_LE_CONNECT_IND,
                    // to update the `m_connection` member in BT task
                    [this](bt_msg_type_t, bt_status_t, void* buf)
                    {
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
    
    return done;
}

bool LBLEClient::connected()
{
    return (m_connection != BT_HANDLE_INVALID);
}

void LBLEClient::onEvent(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    if(BT_GAP_LE_DISCONNECT_IND == msg)
    {
        const bt_gap_le_disconnect_ind_t* pInfo = (bt_gap_le_disconnect_ind_t*)buff;
        if(pInfo->connection_handle == m_connection)
        {
            pr_debug("Disconnected with status = 0x%x, reason = 0x%x", (unsigned int)status, (unsigned int)pInfo->reason);
            m_connection = BT_HANDLE_INVALID;
        }
    }
}

int LBLEClient::getServiceCount()
{
    return m_services.size();
}

bool LBLEClient::hasService(const LBLEUuid& uuid)
{
    auto found = std::find_if(m_services.begin(), m_services.end(), [&uuid](const LBLEServiceInfo& o){ return (bool)(o.uuid == uuid); });
    return (m_services.end() != found);
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
        done = waitAndProcessEvent(
                    // start service discovery
                    [this, &searchRequest]()
                    { 
                        bt_gattc_discover_primary_service(m_connection, &searchRequest);
                    },
                    // wait for the event...
                    BT_GATTC_DISCOVER_PRIMARY_SERVICE,
                    // and process it in BT task context with this lambda
                    [this, &searchRequest, &shouldContinue](bt_msg_type_t, bt_status_t status, void* buf)
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
                        for (int i = 0; i < entryCount; ++i){
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
                    }
        );
    }while(shouldContinue && done);

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

    // for each service, find the value handle of each characteristic.
    for(size_t i = 0; i < m_services.size(); ++i)
    {
      pr_debug("Enumerate charc between handle (%d-%d)", m_services[i].startHandle, m_services[i].endHandle)
      discoverCharacteristicsOfService(m_services[i]);
    }

    return m_characteristics.size();
}

int LBLEClient::discoverCharacteristicsOfService(const LBLEServiceInfo& s)
{
    // discover all characteristics and store mapping between (UUID -> value handle)
    // note that this search could require multiple request-response, 
    // see https://community.nxp.com/thread/332233 for an example.

    if(!connected())
    {
        return 0;
    }

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
        done = waitAndProcessEvent(
                    // start characteristic discovery
                    [this, &searchRequest]()
                    { 
                        bt_gattc_discover_charc(m_connection, &searchRequest);
                    },
                    // wait for the event...
                    BT_GATTC_DISCOVER_CHARC,
                    // and process it in BT task context with this lambda
                    [this, &searchRequest, &shouldContinue](bt_msg_type_t, bt_status_t status, void* buf)
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
                        for (int i = 0; i < entryCount; ++i){
                            memcpy(&attributeHandle, attrDataList + i * rsp->att_rsp->length, 2);
                            memcpy(&properties, attrDataList + i * rsp->att_rsp->length + 2, 1);
                            memcpy(&valueHandle, attrDataList + i * rsp->att_rsp->length + 3, 2);
                            

                            if (rsp->att_rsp->length < 20) {
                                // 16-bit UUID case
                                memcpy(&uuid16, attrDataList + i * rsp->att_rsp->length + 5, 2);
                                m_characteristics.insert(std::make_pair(LBLEUuid(uuid16), valueHandle));
                            } else {
                                // 128-bit UUID case
                                memcpy(&uuid128.uuid, attrDataList + i * entryLength + 5, 16);
                                m_characteristics.insert(std::make_pair(LBLEUuid(uuid128), valueHandle));
                            }
                        }
                            
                        // check if we need to perform more search
                        shouldContinue = (status == BT_ATT_ERRCODE_CONTINUE) && (entryCount > 0);

                        // update starting handle for next round of search
                        searchRequest.starting_handle = valueHandle;
                    }
        );
    }while(shouldContinue && done);

    return done;
}

int LBLEClient::readCharacteristicInt(const LBLEUuid& uuid)
{
    LBLEValueBuffer b = readCharacterstic(uuid);
    int ret = 0;
    if(b.size() < sizeof(ret))
    {
        return 0;
    }

    ret = *((int*)&b[0]);

    return ret;
}

String LBLEClient::readCharacteristicString(const LBLEUuid& uuid)
{
    LBLEValueBuffer b = readCharacterstic(uuid);
    
    if(b.size())
    {
        // safe guard against missing terminating NULL
        if(b[b.size() - 1] != 0)
        {
            b.resize(b.size() + 1);
            b[b.size() - 1] = 0;
        }

        return String((const char*)&b[0]);
    }
    
    return String();
}

char LBLEClient::readCharacteristicChar(const LBLEUuid& uuid)
{
    LBLEValueBuffer b = readCharacterstic(uuid);
    char ret = 0;
    if(b.size() < sizeof(ret))
    {
        return 0;
    }

    ret = *((char*)&b[0]);

    return ret;
}

float LBLEClient::readCharacteristicFloat(const LBLEUuid& uuid)
{
    LBLEValueBuffer b = readCharacterstic(uuid);
    float ret = 0;
    if(b.size() < sizeof(ret))
    {
        return 0;
    }

    ret = *((float*)&b[0]);

    return ret;
}

LBLEValueBuffer LBLEClient::readCharacterstic(const LBLEUuid& serviceUuid)
{
    if(!connected())
    {
        return LBLEValueBuffer();
    }

    // connected, call bt_gattc_read_using_charc_uuid and wait for response event
    bt_gattc_read_using_charc_uuid_req_t req;
    req.opcode = BT_ATT_OPCODE_READ_BY_TYPE_REQUEST;
    req.starting_handle = 0x0001;
    req.ending_handle = 0xffff;
    req.type = serviceUuid.uuid_data;

    // results are stored here by the event callback below.
    LBLEValueBuffer resultBuffer;

    waitAndProcessEvent(
                    // start read request
                    [&]()
                    { 
                        bt_gattc_read_using_charc_uuid(m_connection, &req);
                    },
                    // wait for event...
                    BT_GATTC_READ_USING_CHARC_UUID,
                    // and parse event result in bt task context
                    [this, &resultBuffer](bt_msg_type_t msg, bt_status_t, void* buf)
                    {
                        const bt_gattc_read_by_type_rsp_t *pReadResponse = (bt_gattc_read_by_type_rsp_t*)buf;
                        if(BT_GATTC_READ_USING_CHARC_UUID != msg || pReadResponse->connection_handle != m_connection)
                        {
                            // not for our request
                            return;
                        }
                        
                        do{
                            // check error case
                            if(BT_ATT_OPCODE_ERROR_RESPONSE == pReadResponse->att_rsp->opcode)
                            {
                                const bt_gattc_error_rsp_t* pErr = (bt_gattc_error_rsp_t*)buf;
                                pr_debug("error reading attribute");
                                pr_debug("error_opcode=%d", pErr->att_rsp->error_opcode);
                                pr_debug("error_code=%d", pErr->att_rsp->error_code);
                                break;
                            }

                            if(BT_ATT_OPCODE_READ_BY_TYPE_RESPONSE != pReadResponse->att_rsp->opcode)
                            {
                                pr_debug("Op code don't match! Opcode=%d", pReadResponse->att_rsp->opcode);
                                break;
                            }

                            // Copy the data buffer content
                            const uint8_t listLength = pReadResponse->att_rsp->length - 2;
                            resultBuffer.resize(listLength, 0);
                            memcpy(&resultBuffer[0], ((uint8_t*)pReadResponse->att_rsp->attribute_data_list) + 2, listLength);
                        }while(false);
                    }
                );

    return resultBuffer;
}

int LBLEClient::writeCharacteristic(const LBLEUuid& uuid, const LBLEValueBuffer& value)
{
    // make sure this is smaller than MTU - header size
    const uint32_t MAXIMUM_WRITE_SIZE = 20;

    // we need to check if the UUID have a known value handle.
    // Note that discoverCharacteristic() builds the mapping table.
    auto found = m_characteristics.find(uuid);
    if(found == m_characteristics.end())
    {
        return 0;
    }

    // value buffer too big
    if(value.size() > MAXIMUM_WRITE_SIZE)
    {
        return 0;
    }

    LBLEValueBuffer reqBuf;
    reqBuf.resize(value.size() + sizeof(bt_att_write_req_t));

    bt_gattc_write_charc_req_t req;
    req.attribute_value_length = value.size();
    req.att_req = (bt_att_write_req_t*)&reqBuf[0];
    req.att_req->opcode = BT_ATT_OPCODE_WRITE_REQUEST;
    req.att_req->attribute_handle = found->second;
    memcpy(req.att_req->attribute_value, &value[0], value.size());

    bool done = waitAndProcessEvent(
                    // start read request
                    [&]()
                    { 
                        bt_gattc_write_charc(m_connection, &req);
                    },
                    // wait for event...
                    BT_GATTC_WRITE_CHARC,
                    // and parse event result in bt task context
                    [this](bt_msg_type_t msg, bt_status_t, void* buf)
                    {
                        const bt_gattc_write_rsp_t *pWriteResp = (bt_gattc_write_rsp_t*)buf;
                        if(BT_GATTC_WRITE_CHARC != msg || pWriteResp->connection_handle != m_connection)
                        {
                            // not for our request
                            return;
                        }
                        
                        do{
                            // check error case
                            if(BT_ATT_OPCODE_ERROR_RESPONSE == pWriteResp->att_rsp->opcode)
                            {
                                const bt_gattc_error_rsp_t* pErr = (bt_gattc_error_rsp_t*)buf;
                                pr_debug("error reading attribute");
                                pr_debug("error_opcode=%d", pErr->att_rsp->error_opcode);
                                pr_debug("error_code=%d", pErr->att_rsp->error_code);
                                break;
                            }

                            if(BT_ATT_OPCODE_WRITE_RESPONSE != pWriteResp->att_rsp->opcode)
                            {
                                pr_debug("Op code don't match! Opcode=%d", pWriteResp->att_rsp->opcode);
                                break;
                            }
                        }while(false);
                    }
                );

    return done;
}

int LBLEClient::writeCharacteristicInt(const LBLEUuid& uuid, int value){
    LBLEValueBuffer b(value);
    return writeCharacteristic(uuid, b);
}
int LBLEClient::writeCharacteristicString(const LBLEUuid& uuid, const String& value){
    LBLEValueBuffer b(value);
    return writeCharacteristic(uuid, b);
}
int LBLEClient::writeCharacteristicChar(const LBLEUuid& uuid, char value){
    LBLEValueBuffer b(value);
    return writeCharacteristic(uuid, b);
}
int LBLEClient::writeCharacteristicFloat(const LBLEUuid& uuid, float value){
    LBLEValueBuffer b(value);
    return writeCharacteristic(uuid, b);
}

void _characteristic_event_handler(bt_msg_type_t msg, bt_status_t, void *buff)
{
    if(BT_GATTC_CHARC_VALUE_NOTIFICATION == msg)
    {
        pr_debug("==============NOTIFICATION COMING===============");
        bt_gatt_handle_value_notification_t* pNoti = (bt_gatt_handle_value_notification_t*)buff;
        pr_debug("gatt length=(%d), opcode=(%d), handle=(%d)", 
                        pNoti->length,
                        pNoti->att_rsp->opcode,
                        pNoti->att_rsp->handle);
    }
}

//////////////////////////////////////////////////////////////////////////
//  LBLEValueBuffer
//////////////////////////////////////////////////////////////////////////
template<typename T>void LBLEValueBuffer::shallowInit(T value)
{
    this->resize(sizeof(value));
    memcpy(&(*this)[0], &value, sizeof(value));
}

LBLEValueBuffer::LBLEValueBuffer()
{
}

LBLEValueBuffer::LBLEValueBuffer(int intValue)
{
    shallowInit(intValue);
}

LBLEValueBuffer::LBLEValueBuffer(float floatValue)
{
    shallowInit(floatValue);
}

LBLEValueBuffer::LBLEValueBuffer(char charValue)
{
    shallowInit(charValue);
}

LBLEValueBuffer::LBLEValueBuffer(const String& strValue)
{
    resize(strValue.length() + 1);
    strValue.getBytes(&(*this)[0], size());
}