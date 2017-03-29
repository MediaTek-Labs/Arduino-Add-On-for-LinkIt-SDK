/*

*/
#include <Arduino.h>
#include <LBLE.h>
#include "LBLEPeriphral.h"

extern "C"
{
#include "utility/ard_ble.h"
#include "utility/ard_bt_builtin_profile.h"
#include "utility/ard_bt_attr_callback.h"
}

LBLEAdvertisementData::LBLEAdvertisementData()
{
	m_advDataList.clear();
}

LBLEAdvertisementData::LBLEAdvertisementData(const LBLEAdvertisementData& rhs)
{
	m_advDataList = rhs.m_advDataList;
}

LBLEAdvertisementData::~LBLEAdvertisementData()
{
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
    // Note that in iBeacon the byte order is reversed.
    uint8_t uuidBuf[16] = {0};
    uint8_t* uuidAdvData = item.adData + 4;
    uuid.toRawBuffer(uuidBuf, 16);
    for(int i = 0; i < 16; ++i)
    {
    	uuidAdvData[i] = uuidBuf[15-i];
    }

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
// LBLEService
/////////////////////////////////////////////////////////////////////////////////////////////
LBLEService::LBLEService(const LBLEUuid& uuid):
	m_uuid(uuid)
{
	memset(&m_serviceData, 0, sizeof(m_serviceData));
}

LBLEService::LBLEService(const char* uuidString):
	m_uuid(uuidString)
{
	memset(&m_serviceData, 0, sizeof(m_serviceData));
}

bt_gatts_service_t* LBLEService::getServiceDataPointer()
{
	// check if begin() is called
	if(m_records.empty())
	{
		return NULL;
	}

	return &m_serviceData;
}

void LBLEService::addAttribute(LBLEAttributeInterface& attr)
{
	m_attributes.push_back(&attr);
}

uint16_t LBLEService::begin(uint16_t startingHandle)
{
	// we can only begin once
	if(!m_records.empty())
	{
		return 0;
	}
	
	// handle number must be globally unique and incresing
    uint16_t currentHandle = startingHandle;

	// allocate attribute for service
    // TODO: Assume 128-bit UUID
	bt_gatts_primary_service_128_t* pRecord = (bt_gatts_primary_service_128_t*) malloc(sizeof(bt_gatts_primary_service_128_t));
	pRecord->rec_hdr.uuid_ptr = &BT_GATT_UUID_PRIMARY_SERVICE;
    pRecord->rec_hdr.perm = BT_GATTS_REC_PERM_READABLE;
    pRecord->rec_hdr.value_len = 16;
    pRecord->uuid128 = m_uuid.uuid_data;
    
    currentHandle++;
    m_records.push_back((bt_gatts_service_rec_t*)pRecord);
    
    // Generate characterstics attribute records
    for(int i = 0; i < m_attributes.size(); ++i)
    {
    	for(int r = 0; r < m_attributes[i]->getRecordCount(); ++r)
    	{
    		bt_gatts_service_rec_t* pRec = m_attributes[i]->allocRecord(r, currentHandle);
    		if(pRec)
    		{
    			currentHandle++;
    			m_records.push_back(pRec);
    		}
    	}
    }

	// handle numbers
	m_serviceData.starting_handle = startingHandle;
	m_serviceData.ending_handle	= startingHandle + m_records.size() - 1;

	pr_debug("new service with handle from %04x to %04x", m_serviceData.starting_handle, m_serviceData.ending_handle);

	// we don't use encryption by default.
	m_serviceData.required_encryption_key_size = 0;
	m_serviceData.records = (const bt_gatts_service_rec_t**)&(m_records[0]);

	return m_serviceData.ending_handle + 1;
}

void LBLEService::end()
{
	// Note: all the records are malloc-ed in begin().
	const uint32_t size = m_records.size();
	for(int i = 0; i < size; ++i)
	{
		free(m_records[i]);
	}

	m_records.clear();
	memset(&m_serviceData, 0, sizeof(m_serviceData));
}

/////////////////////////////////////////////////////////////////////////////////////////////
// LBLECharacteristic
/////////////////////////////////////////////////////////////////////////////////////////////

// implement trampoline callback as requested in ard_bt_attr_callback.h
uint32_t ard_bt_callback_trampoline(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset, void* user_data)
{
	pr_debug("attribute_callback [%d, %d, %04x, %08x, %d, %d - %08x", rw, handle, data, size, offset, user_data);

	if(NULL == user_data)
	{
		pr_debug("NULL user data in ard_bt_callback_trampoline");
		return 0;
	}

	LBLEAttributeInterface* pThis = (LBLEAttributeInterface*)user_data;

	if(0 == size)
	{
		return pThis->onSize();
	}

	if (rw == BT_GATTS_CALLBACK_WRITE)
	{
        return pThis->onWrite(data, size, offset);
    }
    else if (rw == BT_GATTS_CALLBACK_READ)
    {
        return pThis->onRead(data, size, offset);
    }
    
    return 0;
}

LBLECharacteristicBase::LBLECharacteristicBase(LBLEUuid uuid, uint32_t permission):
	m_updated(false),
	m_perm(permission),
	m_uuid(uuid)
{

}

bool LBLECharacteristicBase::isWritten()
{
	return m_updated;
}

bt_gatts_service_rec_t* LBLECharacteristicBase::allocRecord(uint32_t recordIndex, uint16_t currentHandle)
{
	switch(recordIndex)
	{
	case 0:
		{
			// the first record is a "Characteristic UUID" attribute
			// it then points the the actual value entry by the "handle" field.
			bt_gatts_characteristic_128_t* pRec = (bt_gatts_characteristic_128_t*)malloc(sizeof(bt_gatts_characteristic_128_t));
			if(NULL == pRec)
			{
				return NULL;
			}	

		    pRec->rec_hdr.uuid_ptr = &BT_GATT_UUID_CHARC;
		    pRec->rec_hdr.perm = BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE;
		    pRec->rec_hdr.value_len = 19;
		    pRec->value.properties = BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_WRITE;
		    pRec->value.handle = currentHandle + 1;
		    pRec->value.uuid128 = m_uuid.uuid_data;
		    
		    return (bt_gatts_service_rec_t*)pRec;
    	}
	case 1:
		{
			// allocating callback function - not needed if SDK allows passing user data directly.
			bt_gatts_rec_callback_t callbackFunc = ard_bt_alloc_callback_slot((LBLEAttributeInterface*)this);
			if(NULL == callbackFunc)
			{
				return NULL;
			}	

			// the first record is a "Characteristic UUID" attribute
			// it then points the the actual value entry by the "handle" field.
			bt_gatts_characteristic_t* pRec = (bt_gatts_characteristic_t*)malloc(sizeof(bt_gatts_characteristic_t));
			if(NULL == pRec)
			{
				// TODO: if we can free callbackFunc, we should free it.
				return NULL;
			}	

	    	pRec->rec_hdr.uuid_ptr = &m_uuid.uuid_data;
	    	pRec->rec_hdr.perm = BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE;
	    	pRec->rec_hdr.value_len = 0;
	    	pRec->value.callback = callbackFunc;
	    	return (bt_gatts_service_rec_t*)pRec;
    	}
    default:
    	return NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// LBLECharacteristic (Raw Buffer)
/////////////////////////////////////////////////////////////////////////////////////////////
LBLECharacteristicBuffer::LBLECharacteristicBuffer(LBLEUuid uuid, uint32_t permission):
	LBLECharacteristicBase(uuid, permission)
{
	memset(m_data, 0, sizeof(m_data));
	m_writtenInfo.size = 0;
	m_writtenInfo.offset = 0;
}

// Set value - size must not exceed MAX_ATTRIBUTE_DATA_LEN.
void LBLECharacteristicBuffer::setValueBuffer(const uint8_t* buffer, size_t size)
{
	if(size > sizeof(m_data))
	{
		return;
	}

	memcpy(m_data, buffer, size);
}

// Get value buffer content. (size + offset) must not exceed MAX_ATTRIBUTE_DATA_LEN.
void LBLECharacteristicBuffer::getValue(uint8_t* buffer, uint16_t size, uint16_t offset)
{
	if((offset + size) > sizeof(m_data))
	{
		return;
	}

	memcpy(buffer, m_data + offset, size);
	m_updated = false;
}

// Retrieve value, note that isWritten() flag turns off after calling getValue()
const LBLECharacteristicWrittenInfo& LBLECharacteristicBuffer::getLastWrittenInfo() const
{
	return m_writtenInfo;
}

uint32_t LBLECharacteristicBuffer::onSize() const
{
	return sizeof(m_data);
}

uint32_t LBLECharacteristicBuffer::onRead(void *data, uint16_t size, uint16_t offset)
{
	// check if it's request to get attribute size
	if(0 == size)
	{
		return onSize();
	}

	const uint32_t dataSize = onSize();
    
    uint32_t copySize = (dataSize > offset) ? (dataSize - offset) : 0;
    copySize = (size > copySize) ? copySize : size;

    if(copySize)
    {
    	memcpy(data, m_data + offset, copySize);
    }

    return copySize;
}

uint32_t LBLECharacteristicBuffer::onWrite(void *data, uint16_t size, uint16_t offset)
{
	const uint32_t updateSize = size + offset;

	// abort writing if exceeding GATT limit (512 bytes)
	if(updateSize > MAX_ATTRIBUTE_DATA_LEN)
	{
		return 0;
	}

	// keep track of transaction info for getLastWrittenInfo()
	if(size)
	{
		m_writtenInfo.size = size;
		m_writtenInfo.offset = offset;
		memcpy(m_data + offset, data, size);	
		m_updated = true;
	}
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// LBLECharacteristic (String data)
/////////////////////////////////////////////////////////////////////////////////////////////
LBLECharacteristicString::LBLECharacteristicString(LBLEUuid uuid, uint32_t permission):
	LBLECharacteristicBase(uuid, permission)
{

}

void LBLECharacteristicString::setValue(const String& value)
{
	// m_update means "set by central device",
	// so clear it here.
	m_updated = false;
	m_data = value;
}

String LBLECharacteristicString::getValue()
{
	m_updated = false;
	return m_data;
}

uint32_t LBLECharacteristicString::onSize() const
{
	return m_data.length();
}

uint32_t LBLECharacteristicString::onRead(void *data, uint16_t size, uint16_t offset)
{
	const uint32_t dataSize = onSize();
    
    if (size == 0){
        return dataSize;
    }

    uint32_t copySize = (dataSize > offset) ? (dataSize - offset) : 0;
    copySize = (size > copySize) ? copySize : size;
    memcpy(data, ((uint8_t*)m_data.c_str()) + offset, copySize);

    return copySize;
}

uint32_t LBLECharacteristicString::onWrite(void *data, uint16_t size, uint16_t offset)
{
	const uint32_t finalStringSize = size + offset;

	Serial.print("onWrite String:");
	Serial.print(size);
	Serial.print(",");
	Serial.print(offset);

	// check if exceeding GATT limit
	if(finalStringSize > MAX_ATTRIBUTE_DATA_LEN)
	{
		return onSize();
	}

	// make sure our String is large enough to accomodate
	// final string and NULL terminator
	if(onSize() < finalStringSize)
	{
		if(!m_data.reserve(finalStringSize + 1))
		{
			// we failed to allocate larger buffer
			return onSize();
		}
	}

	// prevent working on the m_data's internal buffer directly
	// Note that we add NULL terminator - this is the behavior
	// of this "string" charactertistic, not GATT spec.
	uint8_t *pBuffer = (uint8_t*)malloc(finalStringSize + 1);
	if(pBuffer)
	{
		memset(pBuffer, 0, finalStringSize);
		m_data.getBytes(pBuffer, finalStringSize);
		memcpy(pBuffer + offset, data, size);
		// insert NULL terminator
		*(pBuffer + offset + size) = 0;
	}
	m_data = (const char*)pBuffer;
	free(pBuffer);
	pBuffer = NULL;

	m_updated = true;

	return onSize();
}

/////////////////////////////////////////////////////////////////////////////////////////////
// LBLECharacteristic (Integer data)
/////////////////////////////////////////////////////////////////////////////////////////////
LBLECharacteristicInt::LBLECharacteristicInt(LBLEUuid uuid, uint32_t permission):
	LBLECharacteristicBase(uuid, permission),
	m_data(0)
{

}

void LBLECharacteristicInt::setValue(int value)
{
	// m_update means "set by central device",
	// so clear it here.
	m_updated = false;
	m_data = value;
}

int LBLECharacteristicInt::getValue()
{
	m_updated = false;
	return m_data;
}

uint32_t LBLECharacteristicInt::onSize() const
{
	return sizeof(m_data);
}

uint32_t LBLECharacteristicInt::onRead(void *data, uint16_t size, uint16_t offset)
{
	const uint32_t dataSize = onSize();
    
    if (size == 0){
        return dataSize;
    }

    uint32_t copySize = (dataSize > offset) ? (dataSize - offset) : 0;
    copySize = (size > copySize) ? copySize : size;
    memcpy(data, ((uint8_t*)(&m_data)) + offset, copySize);

    return copySize;
}

uint32_t LBLECharacteristicInt::onWrite(void *data, uint16_t size, uint16_t offset)
{
	const uint32_t dataSize = onSize();
	uint32_t copySize = (dataSize > offset) ? (dataSize - offset) : 0;
    copySize = (size > copySize) ? copySize : size;
    memcpy(((uint8_t*)(&m_data)) + offset, data, copySize);

	m_updated = true;

	Serial.println("onWrite!");

	return copySize;
}



/////////////////////////////////////////////////////////////////////////////////////////////
// LBLEPeripheral
/////////////////////////////////////////////////////////////////////////////////////////////

// The singleton instance
LBLEPeripheralClass LBLEPeripheral;

LBLEPeripheralClass::LBLEPeripheralClass()
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
	ard_bt_set_gatts_device_name(name);
}

void LBLEPeripheralClass::addService(const LBLEService& service)
{
	m_services.push_back(service);	
}

const bt_gatts_service_t** LBLEPeripheralClass::getServiceTable()
{
	// user must call begin() to populate m_servicePtrTable first.
	if(m_servicePtrTable.empty())
	{
		return NULL;
	}

	return (const bt_gatts_service_t**)&m_servicePtrTable[0];
}

extern "C"
{
// This callback is called by BLE GATT framework
// when a remote device is connecting to this peripheral.
extern const bt_gatts_service_t bt_if_gap_service;			// 0x0001-
extern const bt_gatts_service_t bt_if_gatt_service_ro;		// 0x0011-
}

void LBLEPeripheralClass::begin()
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

	uint16_t currentHandle = USER_ATTRIBUTE_HANDLE_START;
	for(uint32_t i = 0; i < m_services.size(); ++i)
	{
		currentHandle = m_services[i].begin(currentHandle);
		m_servicePtrTable.push_back(m_services[i].getServiceDataPointer());
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
extern const bt_gatts_service_t bt_if_gap_service;			// 0x0001-
extern const bt_gatts_service_t bt_if_gatt_service_ro;		// 0x0011-
extern const bt_gatts_service_t bt_if_ble_smtcn_service;  	// 0x0014-

// Collects all service
const bt_gatts_service_t * g_gatt_server[] = {
    &bt_if_gap_service,         //0x0001
    &bt_if_gatt_service_ro,     //0x0011
    &bt_if_ble_smtcn_service,   //0x0014-0x0017
    NULL
};

// This callback is called by BLE GATT framework
// when a remote device is connecting to this peripheral.
const bt_gatts_service_t** bt_get_gatt_server()
{
#if 0
	return g_gatt_server;
#else
	return LBLEPeripheral.getServiceTable();
#endif
}

void ard_ble_peri_onName(const char* str, uint16_t handle)
{
	pr_debug("@%s:%04x", str, handle);
}

void ard_ble_peri_onConnect(bt_msg_type_t msg, bt_status_t status, void *buff)
{
	if(BT_GAP_LE_CONNECT_IND != msg || BT_STATUS_SUCCESS != status)
	{
		return;
	}
	pr_debug("device connected");
	const bt_gap_le_connection_ind_t* connect_ind = (bt_gap_le_connection_ind_t*)buff;
}

void ard_ble_peri_onDisconnect(bt_msg_type_t msg, bt_status_t status, void *buff)
{
	pr_debug("device disconnected");

	// for most cases, we'd like to
	// automatically re-start advertising, so that
	// this periphera can be found by other central devices.
	LBLEPeripheral.advertiseAgain();
}

} // extern "C"

