#include <Arduino.h>
#include "LBLECentral.h"
#include <stdlib.h>
#include <vector>

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

String LBLECentral::getName(int index)
{
	bt_gap_le_advertising_report_ind_t dummy = {0};
	LBLEAdvertisements parser(g_peripherals_found[index], dummy);
	return parser.getName();
}

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
	bt_gap_le_advertising_report_ind_t dummy = {0};
	LBLEAdvertisements parser(g_peripherals_found[index], dummy);
	return parser.getIBeaconInfo(uuid, major, minor, txPower);
}

String LBLECentral::getManufacturer(int index) const
{
	bt_gap_le_advertising_report_ind_t dummy = {0};
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
	LBLECentral::processAdvertisement((bt_gap_le_advertising_report_ind_t*)buff);
}

}
