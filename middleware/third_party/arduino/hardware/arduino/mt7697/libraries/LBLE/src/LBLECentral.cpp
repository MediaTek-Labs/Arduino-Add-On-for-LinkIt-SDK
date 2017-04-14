#include <Arduino.h>
#include "LBLECentral.h"
#include <stdlib.h>
#include <vector>
#include <functional>

extern "C" {
#include "utility/ard_ble.h"
#include "utility/ard_bt_company_id.h"
}

//////////////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////
// LBLEAdvertisements
//////////////////////////////////////////////////////////////////////////////////
LBLEAdvertisements::LBLEAdvertisements(bt_gap_le_advertising_report_ind_t& adv,
                                          bt_gap_le_advertising_report_ind_t& resp):
    adv_data(adv),resp_data(resp)
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
        if (ad_data_len > 0x1F || ad_data_len < 0)
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
    uint32_t ret = 0;

    // search the ad payload in adv_data first...
    ret = getAdvDataWithTypeFromPayload(type, dstBuf, bufLen, adv_data);
    if(ret)
    {
        return ret;
    }
    else
    {
        // then search for the resp_data.
        return getAdvDataWithTypeFromPayload(type, dstBuf, bufLen, resp_data);
    }
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
    if(dataLen = getAdvDataWithType(BT_GAP_LE_AD_TYPE_128_BIT_UUID_COMPLETE, dataBuf, MAX_ADV_DATA_LEN))
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
// LBLECentral
//////////////////////////////////////////////////////////////////////////////////

// global list of scanned peripherals(advertisements).
std::vector<bt_gap_le_advertising_report_ind_t> LBLECentral::g_peripherals_found;

LBLECentral::LBLECentral()
{

}

void LBLECentral::scan()
{
    bt_status_t result = BT_STATUS_SUCCESS;

    bt_hci_cmd_le_set_scan_enable_t enbleSetting;
    enbleSetting.le_scan_enable = BT_HCI_ENABLE;
    enbleSetting.filter_duplicates = BT_HCI_ENABLE;		// Enable driver-level filter for duplicated adv.

    bt_hci_cmd_le_set_scan_parameters_t scan_para;
    scan_para.own_address_type = BT_HCI_SCAN_ADDR_RANDOM;
    scan_para.le_scan_type = BT_HCI_SCAN_TYPE_ACTIVE;		// Requesting scan-response from peripherals.
                                                            // If you don't want scan-response,
                                                            // use BT_HCI_SCAN_TYPE_PASSIVE instead.
    scan_para.scanning_filter_policy = BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS;
    scan_para.le_scan_interval = 0x0024;	// Interval between scans
    scan_para.le_scan_window = 0x0011;		// How long a scan keeps

    // set_scan is actually asynchronous - but since
    // we don't have anything to check, we keep going
    // without waiting for BT_GAP_LE_SET_SCAN_CNF.
    result = bt_gap_le_set_scan(&enbleSetting, &scan_para);

    return;
}

void LBLECentral::stopScan()
{
    bt_status_t result = BT_STATUS_SUCCESS;

    bt_hci_cmd_le_set_scan_enable_t enbleSetting;
    enbleSetting.le_scan_enable = BT_HCI_DISABLE;
    enbleSetting.filter_duplicates = BT_HCI_DISABLE;

    // set_scan is actually asynchronous - but since
    // we don't have anything to check, we keep going
    // without waiting for BT_GAP_LE_SET_SCAN_CNF.
    result = bt_gap_le_set_scan(&enbleSetting, NULL);

    g_peripherals_found.clear();

    return;
}

int LBLECentral::getPeripheralCount()
{
    return g_peripherals_found.size();
}

String LBLECentral::getAddress(int index)
{
    return LBLEAddress::convertAddressToString(g_peripherals_found[index].address.addr);
}

LBLEAddress LBLECentral::getBLEAddress(int index)
{
    return LBLEAddress(g_peripherals_found[index].address.addr);
}

String LBLECentral::getName(int index)
{
    bt_gap_le_advertising_report_ind_t dummy = {0};
    LBLEAdvertisements parser(g_peripherals_found[index], dummy);
    return parser.getName();
}

class EventBlocker : public LBLEEventObserver
{
public:
    const bt_msg_type_t m_event;
    bool m_eventArrived;
    std::function<void(bt_msg_type_t , bt_status_t , void *)> m_handler;

    EventBlocker(bt_msg_type_t e, std::function<void(bt_msg_type_t , bt_status_t , void *)>& handler):
        m_event(e),
        m_eventArrived(false),
        m_handler(handler)
    {

    }

    bool done() const
    {
        return m_eventArrived;
    }

    virtual void onEvent(bt_msg_type_t msg, bt_status_t status, void* buff)
    {
        if(m_event == msg)
        {
            m_eventArrived = true;
            m_handler(msg, status, buff);
        }
    }
};

int32_t LBLECentral::getRSSI(int index)
{
    return g_peripherals_found[index].rssi;
}

int32_t LBLECentral::getTxPower(int index)
{
    bt_gap_le_advertising_report_ind_t dummy = {0};
    LBLEAdvertisements parser(g_peripherals_found[index], dummy);
    return parser.getTxPower();
}

LBLEUuid LBLECentral::getServiceUuid(int index) const
{
    bt_gap_le_advertising_report_ind_t dummy = {0};
    LBLEAdvertisements parser(g_peripherals_found[index], dummy);
    return parser.getServiceUuid();
}

bool LBLECentral::isIBeacon(int index) const
{
    bt_gap_le_advertising_report_ind_t dummy = {0};
    LBLEAdvertisements parser(g_peripherals_found[index], dummy);

    LBLEUuid uuid;
    uint16_t major, minor;
    uint8_t txPower;
    return parser.getIBeaconInfo(uuid, major, minor, txPower);
}

bool LBLECentral::getIBeaconInfo(int index, LBLEUuid& uuid, uint16_t& major, uint16_t& minor, uint8_t& txPower) const
{
    bt_gap_le_advertising_report_ind_t dummy;
    LBLEAdvertisements parser(g_peripherals_found[index], dummy);
    return parser.getIBeaconInfo(uuid, major, minor, txPower);
}

String LBLECentral::getManufacturer(int index) const
{
    bt_gap_le_advertising_report_ind_t dummy;
    LBLEAdvertisements parser(g_peripherals_found[index], dummy);
    return parser.getManufacturer();
}

uint8_t  LBLECentral::getAdvertisementFlag(int index) const
{
    bt_gap_le_advertising_report_ind_t dummy = {0};
    LBLEAdvertisements parser(g_peripherals_found[index], dummy);
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

// process incoming peripheral advertisement
void LBLECentral::processAdvertisement(const bt_gap_le_advertising_report_ind_t *report)
{
    if(NULL == report)
    {
        return;
    }

    switch(report->event_type)
    {
    case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_IND:
        // advertising packet - add directly since we enabled `filter_duplicates`.
        g_peripherals_found.push_back(*report);
        break;
    case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_SCAN_RSP:
        //TODO: scan response - looking for matching address in g_peripherals_found list.
        /*
        for(int i = 0; i < g_peripherals_found.size(); ++i)
        {
            // Search for duplication entry
            if (compare_bt_address(g_peripherals_found[i].address, report->address))
            {
                g_peripherals_found[i] = *report;
            }
        }
        */
        break;
    default:
        // ignore other events
        break;
    }

#if 0

    if(report->data_length > LBLEAdvertisements::MAX_ADV_DATA_LEN)
    {
        Serial.print("Illegal adv data size=");
        Serial.println(report->data_length);
        return;
    }

    // Parse advertisement data
    Serial.print("Type:");
    Serial.println(get_event_type(report->event_type));

    Serial.print("RSSI:");
    Serial.println(report->rssi);

    Serial.print("Addr:");
    String addrStr = LBLEAddress::convertAddressToString(report->address.addr)
    Serial.print("data_length:");
    Serial.println(report->data_length);

    int cursor = 0;
    int ad_data_len = 0;
    int ad_data_type = 0;
    unsigned short ad_type_flag = 0;
    uint8_t buff[LBLEAdvertisements::MAX_ADV_DATA_LEN] = {0};
    while (cursor < report->data_length) {
        ad_data_len = report->data[cursor];
        Serial.print("AD: len=");
        Serial.print(ad_data_len);

        ad_data_type = report->data[cursor+1];
        Serial.print(", Type=");
        Serial.print(ad_data_type);
        Serial.print(" ");

        switch(ad_data_type)
        {
        case BT_GAP_LE_AD_TYPE_FLAG:
            Serial.print("BT_GAP_LE_AD_TYPE_FLAG ");
            ad_type_flag = *(unsigned short*)(report->data + cursor + 2);
            if(ad_type_flag & (0x1 << 0)) Serial.print("| LE Limited Discovery ");
            if(ad_type_flag & (0x1 << 1)) Serial.print("| LE Normal Discovery ");
            if(ad_type_flag & (0x1 << 2)) Serial.print("| No BR/EDR ");
            if(ad_type_flag & (0x1 << 3)) Serial.print("| Controller BR/EDR ");
            if(ad_type_flag & (0x1 << 4)) Serial.print("| Host BR/EDR ");
            break;
        case BT_GAP_LE_AD_TYPE_16_BIT_UUID_PART:
            Serial.println("BT_GAP_LE_AD_TYPE_16_BIT_UUID_PART");
            break;
        case BT_GAP_LE_AD_TYPE_16_BIT_UUID_COMPLETE:
            Serial.println("BT_GAP_LE_AD_TYPE_16_BIT_UUID_COMPLETE");
            break;
        case BT_GAP_LE_AD_TYPE_32_BIT_UUID_PART:
            Serial.println("BT_GAP_LE_AD_TYPE_32_BIT_UUID_PART");
            break;
        case BT_GAP_LE_AD_TYPE_32_BIT_UUID_COMPLETE:
            Serial.println("BT_GAP_LE_AD_TYPE_32_BIT_UUID_COMPLETE");
            break;
        case BT_GAP_LE_AD_TYPE_128_BIT_UUID_PART:
            Serial.println(" BT_GAP_LE_AD_TYPE_128_BIT_UUID_PART  ");
            break;
        case BT_GAP_LE_AD_TYPE_128_BIT_UUID_COMPLETE:
            {
                Serial.print(" BT_GAP_LE_AD_TYPE_128_BIT_UUID_COMPLETE | ");
                char str[37] = {};
                unsigned char *uuid = (unsigned char*)(report->data + cursor + 2);
                sprintf(str,
                    "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                    uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
                    uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]
                );
                Serial.print(str);
            }
            break;
        case BT_GAP_LE_AD_TYPE_TX_POWER:
            Serial.println(" BT_GAP_LE_AD_TYPE_TX_POWER  ");
            break;
        case BT_GAP_LE_AD_TYPE_COD:
            Serial.println(" BT_GAP_LE_AD_TYPE_COD  ");
            break;
        case BT_GAP_LE_AD_TYPE_SM_TK:
            Serial.println(" BT_GAP_LE_AD_TYPE_SM_TK  ");
            break;
        case BT_GAP_LE_AD_TYPE_SM_OOB_FLAG:
            Serial.println(" BT_GAP_LE_AD_TYPE_SM_OOB_FLAG  ");
            break;
        case BT_GAP_LE_AD_TYPE_SLAVE_CONNECTION_INTERVAL_RANGE:
            Serial.println(" BT_GAP_LE_AD_TYPE_SLAVE_CONNECTION_INTERVAL_RANGE  ");
            break;
        case BT_GAP_LE_AD_TYPE_16_BIT_SOLICITATION_UUID:
            Serial.println(" ---BT_GAP_LE_AD_TYPE_16_BIT_SOLICITATION_UUID-  ");
            break;
        case BT_GAP_LE_AD_TYPE_128_BIT_SOLICITATION_UUID:
            Serial.println(" BT_GAP_LE_AD_TYPE_128_BIT_SOLICITATION_UUID  ");
            break;
        case BT_GAP_LE_AD_TYPE_16_BIT_UUID_DATA:
            Serial.println(" BT_GAP_LE_AD_TYPE_16_BIT_UUID_DATA  ");
            break;
        case BT_GAP_LE_AD_TYPE_PUBLIC_TARGET_ADDRESS:
            Serial.println(" BT_GAP_LE_AD_TYPE_PUBLIC_TARGET_ADDRESS  ");
            break;
        case BT_GAP_LE_AD_TYPE_RANDOM_TARGET_ADDRESS:
            Serial.println(" BT_GAP_LE_AD_TYPE_RANDOM_TARGET_ADDRESS  ");
            break;
        case BT_GAP_LE_AD_TYPE_LE_BT_DEVICE_ADDRESS:
            Serial.println(" BT_GAP_LE_AD_TYPE_LE_BT_DEVICE_ADDRESS  ");
            break;
        case BT_GAP_LE_AD_TYPE_APPEARANCE:
            Serial.println(" BT_GAP_LE_AD_TYPE_APPEARANCE  ");
            break;
        case BT_GAP_LE_AD_TYPE_ADV_INTERVAL:
            Serial.println(" BT_GAP_LE_AD_TYPE_ADV_INTERVAL  ");
            break;
        case BT_GAP_LE_AD_TYPE_LE_ROLE:
            Serial.println(" BT_GAP_LE_AD_TYPE_LE_ROLE  ");
            break;
        case BT_GAP_LE_AD_TYPE_32_BIT_SOLICITATION_UUID:
            Serial.println(" BT_GAP_LE_AD_TYPE_32_BIT_SOLICITATION_UUID  ");
            break;
        case BT_GAP_LE_AD_TYPE_32_BIT_UUID_DATA:
            Serial.println(" BT_GAP_LE_AD_TYPE_32_BIT_UUID_DATA  ");
            break;
        case BT_GAP_LE_AD_TYPE_128_BIT_UUID_DATA:
            Serial.println(" BT_GAP_LE_AD_TYPE_128_BIT_UUID_DATA  ");
            break;
        case BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC:
            {
                Serial.print(" BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC ");
                unsigned short vendorId = *(unsigned short*)(report->data + cursor + 2);
                Serial.print(" vendor = ");
                Serial.print(vendorId);
                Serial.print("-");
                Serial.print(getBluetoothCompanyName(vendorId));

                // detect iBeacon
                if(vendorId == 0x4C)
                {
                    Serial.print(" iBeacon => ");
                    unsigned char *iBeaconBuffer = (unsigned char*)(report->data + cursor + 4);
                    unsigned char beaconType = *(iBeaconBuffer++);
                    unsigned char beaconLength = *(iBeaconBuffer++);

                    Serial.print(" beaconType = ");
                    Serial.print(beaconType);

                    Serial.print(" beaconLength = ");
                    Serial.print(beaconLength);

                    Serial.print(" UUID=");
                    char str[37] = {};
                    unsigned char *uuid = (unsigned char*)(iBeaconBuffer);
                    sprintf(str,
                        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
                        uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]
                    );
                    Serial.print(str);
                    iBeaconBuffer += 16;

                    unsigned short majorNum = *(unsigned short*)iBeaconBuffer;
                    Serial.print(" Major = ");
                    Serial.print(majorNum);
                    iBeaconBuffer += 2;

                    unsigned short minorNum = *(unsigned short*)iBeaconBuffer;
                    Serial.print(" Minor = ");
                    Serial.print(minorNum);
                    iBeaconBuffer += 2;

                    int8_t txPower = *(int8_t*)iBeaconBuffer;
                    Serial.print(" txPower = ");
                    Serial.print(txPower);
                }
            }

            break;
        case BT_GAP_LE_AD_TYPE_NAME_SHORT:
        case BT_GAP_LE_AD_TYPE_NAME_COMPLETE:
            Serial.print("BT_GAP_LE_AD_TYPE_NAME = ");
            strncpy((char*)buff, (const char*)(report->data + cursor + 2), ad_data_len - 2);
            Serial.println((const char*)buff);
            break;
        default:
            break;
        }

        /* Error handling for data length over 30 bytes. */
        if (ad_data_len > 0x1F || ad_data_len < 0) {
            return;
        }

        cursor += (ad_data_len + 1);

        Serial.println();
    }
#endif
}

// handles Central-related events from BLE framework
extern "C"
{

void ard_ble_central_onCentralEvents(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch(msg)
    {
    case BT_GAP_LE_ADVERTISING_REPORT_IND:
        LBLECentral::processAdvertisement((bt_gap_le_advertising_report_ind_t*)buff);
        break;
    case BT_GATTC_CHARC_VALUE_NOTIFICATION:
        Serial.println("==============NOTIFICATION COMING===============");
        bt_gatt_handle_value_notification_t* pNoti = (bt_gatt_handle_value_notification_t*)buff;
        Serial.print("gatt length=");
        Serial.println(pNoti->length);
        Serial.print("Att opcode=");
        Serial.println(pNoti->att_rsp->opcode);
        Serial.print("Att handle=");
        Serial.println(pNoti->att_rsp->handle);
        Serial.print("Att value=");
        for(int i = 0; i < 10; ++i)
        {
            Serial.println(pNoti->att_rsp->attribute_value[i]);
        }
        
        break;
    }
}

}

///////////////////////////////////////////////////////////////
//		LBLEClient
///////////////////////////////////////////////////////////////

// do ACTION and wait for MSG, if it arrives HANDLER is called.
bool waitAndProcessEvent(std::function<void(void)> action, 
                        bt_msg_type_t msg, 
                        std::function<void(bt_msg_type_t, bt_status_t, void *)> handler)
{
    EventBlocker h(msg, handler);
    LBLE.registerForEvent(msg, &h);

    action();

    uint32_t start = millis();
    while(!h.done() && (millis() - start) < 10 * 1000)
    {
        delay(50);
    }

    return h.done();
}

bool LBLEClient::connect(const LBLEAddress& address)
{
    bt_hci_cmd_le_create_connection_t conn_para;
    conn_para.le_scan_interval = 0x0010;
    conn_para.le_scan_window = 0x0010;
    conn_para.initiator_filter_policy = BT_HCI_CONN_FILTER_ASSIGNED_ADDRESS;

    // TODO: we need to combine addresst ype (public/random) into LBLEAddress
    conn_para.peer_address.type = BT_ADDR_RANDOM;

    memcpy(conn_para.peer_address.addr, address.m_addr, BT_BD_ADDR_LEN);

    conn_para.own_address_type = BT_ADDR_RANDOM;
    conn_para.conn_interval_min = 0x0006;
    conn_para.conn_interval_max = 0x0080;
    conn_para.conn_latency = 0x0000;
    conn_para.supervision_timeout = 0x07d0;
    conn_para.minimum_ce_length = 0x0000;
    conn_para.maximum_ce_length = 0x0050;

    // call bt_gap_le_connect, wait for BT_GAP_LE_CONNECT_IND
    // and store the connection_handle to this->m_connection
    bool done = waitAndProcessEvent(
                            [&]()
                            { 
                                bt_gap_le_connect(&conn_para);
                            },

                            BT_GAP_LE_CONNECT_IND,

                            [this](bt_msg_type_t msg, bt_status_t status, void* buf)
                            {
                                const bt_gap_le_connection_ind_t *pConnectionInfo = (bt_gap_le_connection_ind_t*)buf;
                                this->m_connection = pConnectionInfo->connection_handle;
                                return;
                            }
                );

    return done;
}

bool LBLEClient::connected()
{
    return (m_connection != 0);
}

int LBLEClient::getServiceCount()
{
    return m_services.size();
}

LBLEUuid LBLEClient::getServiceUuid(int index)
{
    if(index < 0 || index >= getServiceCount())
    {
        return LBLEUuid();
    }

    return m_services[index];
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
                    [this, &searchRequest, &shouldContinue](bt_msg_type_t msg, bt_status_t status, void* buf)
                    {
                        // Parse the response to service UUIDs. It can be 16-bit or 128-bit
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
                            if (entryLength == 6) {
                                // 16-bit UUID case
                                memcpy(&uuid16, attrDataList + i * entryLength + 4, entryLength - 4);
                                m_services.push_back(LBLEUuid(uuid16));
                            } else {
                                // 128-bit UUID case
                                memcpy(&uuid128.uuid, attrDataList + i * entryLength + 4, entryLength - 4);
                                m_services.push_back(LBLEUuid(uuid128));
                            }
                        }
                            
                        // check if we need to perform more search
                        shouldContinue = (status == BT_ATT_ERRCODE_CONTINUE);
                        
                        // update starting handle for next round of search
                        searchRequest.starting_handle = endIndex;
                    }
        );
    }while(shouldContinue && done);

    return m_services.size();
}

LBLEValueBuffer LBLEClient::readCharacterstic(LBLEUuid serviceUuid)
{
    if(!connected())
    {
        return LBLEValueBuffer();
    }

    // connected, call bt_gattc_read_using_charc_uuid and wait for response event
    bt_gattc_read_using_charc_uuid_req_t req = {0};
    req.opcode = BT_ATT_OPCODE_READ_BY_TYPE_REQUEST;
    req.starting_handle = 0x0001;
    req.ending_handle = 0xffff;
    req.type = serviceUuid.uuid_data;

    // results are stored here by the event callback below.
    LBLEValueBuffer resultBuffer;

    bool done = waitAndProcessEvent(
                    // start read request
                    [&]()
                    { 
                        bt_gattc_read_using_charc_uuid(m_connection, &req);
                    },
                    // wait for event...
                    BT_GATTC_READ_USING_CHARC_UUID,
                    // and parse event result in bt task context
                    [this, &resultBuffer](bt_msg_type_t msg, bt_status_t status, void* buf)
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
                                Serial.print("Op code don't match! Opcode=");
                                Serial.println(pReadResponse->att_rsp->opcode);
                                break;
                            }

                            // Copy the data buffer content
                            const uint8_t listLength = pReadResponse->att_rsp->length - 2;
                            resultBuffer.resize(listLength + 1, 0);
                            memcpy(&resultBuffer[0], ((uint8_t*)pReadResponse->att_rsp->attribute_data_list) + 2, listLength);
                        }while(false);
                    }
                );

    return resultBuffer;
}