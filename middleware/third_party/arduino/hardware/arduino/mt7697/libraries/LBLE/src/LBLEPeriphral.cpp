/*

*/
#include <Arduino.h>
#include <LBLE.h>
#include "LBLEPeriphral.h"

extern "C"
{
#include "utility/ard_ble.h"
}

LBLEAdvertisementData::LBLEAdvertisementData()
{
	Serial.println("alloc LBLEAdvertisementData");
	m_advDataList.clear();
}

LBLEAdvertisementData::LBLEAdvertisementData(const LBLEAdvertisementData& rhs)
{
	Serial.println("copy LBLEAdvertisementData");
	m_advDataList = rhs.m_advDataList;
}

LBLEAdvertisementData::~LBLEAdvertisementData()
{
	Serial.println("delete LBLEAdvertisementData");
	m_advDataList.clear();
}

void LBLEAdvertisementData::addAdvertisementData(const LBLEAdvDataItem &item)
{
	m_advDataList.push_back(item);	
}

void LBLEAdvertisementData::addFlag(uint8_t flag)
{
	LBLEAdvDataItem item;
	item.adType = BT_GAP_LE_AD_TYPE_FLAG;
	item.adData[0] = flag;
	item.adDataLen = 1;

	addAdvertisementData(item);
}


void LBLEAdvertisementData::addName(const char* deviceName)
{
	// name default to complete name
	LBLEAdvDataItem item;
	item.adType = BT_GAP_LE_AD_TYPE_NAME_COMPLETE;
	
	// note that in AD data the string does name
	// contain NULL-termination character.
	item.adDataLen = strlen(deviceName);

	// size check
	if(item.adDataLen > sizeof(item.adData))
		return;

	memcpy(item.adData, deviceName, item.adDataLen);

	addAdvertisementData(item);
}

void LBLEAdvertisementData::configAsConnectableDevice(const char* deviceName)
{
	// Default flag is LE discovrable
	addFlag();

	// Usually we need a name for iOS devices to list our peripheral device.
	addName(deviceName);
}

void LBLEAdvertisementData::configAsConnectableDevice(const char* deviceName, const LBLEUuid& uuid)
{
	// Default flag is LE discovrable
	addFlag();

	// UUID
	LBLEAdvDataItem item;
	if(uuid.is16Bit())
	{
		// 16-bit UUID
		item.adType = BT_GAP_LE_AD_TYPE_16_BIT_UUID_COMPLETE;
		item.adDataLen = 2;	// 16 Bit UUID = 2 bytes
		const uint16_t uuid16 = uuid.getUuid16();
		item.adData[0] = uuid16 & 0x00FF;
    	item.adData[1] = (uuid16 & 0xFF00)>>8;
	}
	else
	{
		// 128-bit UUID
		item.adType = BT_GAP_LE_AD_TYPE_128_BIT_UUID_COMPLETE;
		item.adDataLen = 16;	// 16 Bit UUID = 2 bytes
		uuid.toRawBuffer(item.adData, item.adDataLen);
	}
	addAdvertisementData(item);
	

	// Usually we need a name for iOS devices to list our peripheral device.
	addName(deviceName);
}

void LBLEAdvertisementData::configAsIBeacon(const LBLEUuid& uuid, 		
				  uint16_t major, 
				  uint16_t minor, 
				  int8_t txPower)
{
	// remove all existing AD
	m_advDataList.clear();

	// populate AD according to iBeacon spec
	LBLEAdvDataItem item;

	// Advtertisement type flag
	item.clear();
	item.adType = BT_GAP_LE_AD_TYPE_FLAG;
	item.adData[0] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED | BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE;
	item.adDataLen = 1;

	addAdvertisementData(item);

	// Manufacturer data
	item.clear();
	item.adType = BT_GAP_LE_AD_TYPE_MANUFACTURER_SPECIFIC;
	item.adDataLen = 0x19;	// always 25 bytes of manufacturer payload
	
	item.adData[0] = 0x4c;	// Apple vendor id(0x004c)
	item.adData[1] = 0x00;	// Apple vendor id(0x004c)

	item.adData[2] = 0x02;  // iBeacon type
    item.adData[3] = 0x15;  // iBeacon type

    // 16 bytes of user-specified UUID
    uuid.toRawBuffer(item.adData + 4, 16);  

    // 2 byte major number & 2 byte minor, note that the endian is different.
    (*(uint16_t*)(item.adData + 20)) = (major >> 8) | (major << 8);
    (*(uint16_t*)(item.adData + 22)) = (minor >> 8) | (minor << 8);

    // 1 byte TxPower (signed)
    item.adData[24] = (uint8_t)txPower;		

    addAdvertisementData(item);
}

uint32_t LBLEAdvertisementData::getPayload(uint8_t* buf, uint32_t bufLength) const
{
	// input check
	int sizeRequired = 0;
	for(uint32_t i = 0; i < m_advDataList.size(); ++i)
	{
		// data_len + (type + length)
		sizeRequired += (m_advDataList[i].adDataLen + 2);
	}

	if(NULL == buf || bufLength < sizeRequired)
	{
		return sizeRequired;
	}

	// populate the buffer
	uint8_t *cursor = buf;
	for(uint32_t i = 0; i < m_advDataList.size(); ++i)
	{
		const LBLEAdvDataItem& item = m_advDataList[i];
		// AD length
		cursor[0] = item.adDataLen + 1;
		// AD type
		cursor[1] = item.adType;
		// Ad content;
		memcpy(cursor + 2, item.adData, item.adDataLen);

		cursor += item.adDataLen + 2;
	}

	return sizeRequired;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// LBLEPeripheral
/////////////////////////////////////////////////////////////////////////////////////////////

// The singleton instance
LBLEPeripheralClass LBLEPeripheral;

LBLEPeripheralClass::LBLEPeripheralClass():
	m_attrHandle(USER_ATTRIBUTE_HANDLE_START)
{

}

LBLEPeripheralClass::~LBLEPeripheralClass()
{
	
}

void LBLEPeripheralClass::advertise(const LBLEAdvertisementData& advData)
{
	// make a copy of advertisement data for re-advertising after disconnect event.
    // previous advertisement data will be cleared since m_pAdvData is unique_ptr.
    m_pAdvData = std::unique_ptr<LBLEAdvertisementData>(new LBLEAdvertisementData(advData));

    // start advertisement
    advertiseAgain();
}
	
void LBLEPeripheralClass::stopAdvertise()
{
	// disable advertisement
    bt_hci_cmd_le_set_advertising_enable_t enable;
    enable.advertising_enable = BT_HCI_DISABLE;

    // start broadcasting
    bt_gap_le_set_advertising(&enable, NULL, NULL, NULL);

    m_pAdvData.release();

    // TODO: we better wait for the BT_GAP_LE_SET_ADVERTISING_CNF event.
}

void LBLEPeripheralClass::advertiseAgain()
{
	if(!m_pAdvData)
	{
		return;
	}

	// enable advertisement
    bt_hci_cmd_le_set_advertising_enable_t enable = {0};
    enable.advertising_enable = BT_HCI_ENABLE;

    // advertisement parameters
    bt_hci_cmd_le_set_advertising_parameters_t adv_param = {0};
    adv_param.advertising_interval_min = 0x00C0;
    adv_param.advertising_interval_max = 0x00C0;
	// TODO: for iBeacon-only devices, 
    // you may want to set advertising type to BT_HCI_ADV_TYPE_NON_CONNECTABLE_UNDIRECTED
    adv_param.advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED;
    adv_param.own_address_type = BT_ADDR_RANDOM;
    adv_param.advertising_channel_map = 7;
    adv_param.advertising_filter_policy = 0;

    // populate the advertisement data
    bt_hci_cmd_le_set_advertising_data_t hci_adv_data = {0};
    hci_adv_data.advertising_data_length = m_pAdvData->getPayload(hci_adv_data.advertising_data, 
    													      sizeof(hci_adv_data.advertising_data));

    // start broadcasting
    bt_gap_le_set_advertising(&enable, &adv_param, &hci_adv_data, NULL);

    // TODO: we better wait for the BT_GAP_LE_SET_ADVERTISING_CNF event.
}

void LBLEPeripheralClass::setName(const char* name)
{

}

const bt_gatts_service_t** LBLEPeripheralClass::getServiceTable()
{
	if(m_servicePtrTable.empty())
	{
		populateServicePointerTable();

		// If there are no services defined, return NULL.
		if(m_servicePtrTable.empty())
		{
			return NULL;
		}
	}
	return (const bt_gatts_service_t**)&m_servicePtrTable[0];
}

uint16_t LBLEPeripheralClass::allocAttrHandle()
{
	return m_attrHandle++;
}

extern "C"
{
// This callback is called by BLE GATT framework
// when a remote device is connecting to this peripheral.
extern const bt_gatts_service_t bt_if_gap_service;			// 0x0001-
extern const bt_gatts_service_t bt_if_gatt_service_ro;		// 0x0011-
}

void LBLEPeripheralClass::populateServicePointerTable()
{
	// Is there any user-defined services?
	if(m_services.empty())
	{
		// No - we do nothing
		return;
	}

	// insert mandatory GAP & GATT service - these are generated statically
	// in ard_bt_builtin_profile.c
	m_servicePtrTable.push_back(&bt_if_gap_service);
	m_servicePtrTable.push_back(&bt_if_gatt_service_ro);

	// this "handle" must be globally unique, 
	// as requested by BLE framework.
	assert(USER_ATTRIBUTE_HANDLE_START > bt_if_gap_service.ending_handle);
	assert(USER_ATTRIBUTE_HANDLE_START > bt_if_gatt_service_ro.ending_handle);

	for(uint32_t i = 0; i < m_services.size(); ++i)
	{
		m_servicePtrTable.push_back(&m_services[i]);
	}

	// finally, make sure the pointer table is NULL-terminated,
	// as requested by the framework.
	m_servicePtrTable.push_back(NULL);

	return;
}

////////////////////////////////////////////////////////////////////////
//	GATT related features
////////////////////////////////////////////////////////////////////////
extern "C"
{
// This callback is called by BLE GATT framework
// when a remote device is connecting to this peripheral.
extern const bt_gatts_service_t bt_if_gap_service;			// 0x0001-
extern const bt_gatts_service_t bt_if_gatt_service_ro;		// 0x0011-
extern const bt_gatts_service_t bt_if_ble_smtcn_service;  	// 0x0014-

// Server collects all service
const bt_gatts_service_t * g_gatt_server[] = {
    &bt_if_gap_service,         //0x0001
    &bt_if_gatt_service_ro,     //0x0011
    &bt_if_ble_smtcn_service,   //0x0014-0x0017
    NULL
};

const bt_gatts_service_t** bt_get_gatt_server()
{
	return g_gatt_server; // LBLEPeripheral.getServiceTable();
}

void ard_ble_peri_onName(const char* str, uint16_t handle)
{
	Serial.print("func:");
	Serial.println(str);
	Serial.print("handle:");
	Serial.println(handle);
}

void ard_ble_peri_onConnect(bt_msg_type_t msg, bt_status_t status, void *buff)
{
	if(BT_GAP_LE_CONNECT_IND != msg || BT_STATUS_SUCCESS != status)
	{
		return;
	}

	const bt_gap_le_connection_ind_t* connect_ind = (bt_gap_le_connection_ind_t*)buff;
	Serial.println("device connected");
}

void ard_ble_peri_onDisconnect(bt_msg_type_t msg, bt_status_t status, void *buff)
{
	Serial.println("device disconnected");

	// for most cases, we'd like to
	// automatically re-start advertising, so that
	// this periphera can be found by other central devices.
	LBLEPeripheral.advertiseAgain();
}

} // extern "C"

