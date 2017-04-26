/*

*/
#include <Arduino.h>
#include <LBLE.h>
#include <constants.h>
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
    // for size check - we simply truncate the device name
    item.adDataLen = std::min((size_t)sizeof(item.adData), (size_t)strlen(deviceName));

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


void LBLEAdvertisementData::configAsEddystoneURL(EDDYSTONE_URL_PREFIX prefix,
                                                    const String& url,
                                                    EDDYSTONE_URL_ENCODING suffix,
                                                    const String& tail)
{
    const uint32_t MAX_ENCODED_URL_SIZE = 17; 
    const int8_t txPower = -30;

    // remove all existing AD
    m_advDataList.clear();

    // populate AD according to Eddystone spec:
    // https://github.com/google/eddystone/blob/master/protocol-specification.md
    LBLEAdvDataItem item;

    // Advtertisement type flag
    addFlag(BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED | BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE);

    // Complete List of 16-bit Service UUIDs with 0xFEAA
    item.clear();
    item.adType = BT_GAP_LE_AD_TYPE_16_BIT_UUID_COMPLETE;
    *(uint16_t*)item.adData = 0xFEAA;
    item.adDataLen = 2;
    addAdvertisementData(item);

    // Service Data - contains "Eddystone frame"
    // https://github.com/google/eddystone/tree/master/eddystone-url
    item.clear();
    item.adType = BT_GAP_LE_AD_TYPE_SERVICE_DATA;

    // Frame content:
    uint8_t* pData = item.adData;
    *(uint16_t*)pData = 0xFEAA;
    pData += 2;
    // frametype 0x10 = URL
    *pData++ = 0x10;
    // TxPower
    *pData++ = (uint8_t)txPower;
    // URL scheme prefix
    const uint8_t scheme = prefix;
    *pData++ = scheme;

    // URL string
    size_t urlBudget = MAX_ENCODED_URL_SIZE;
    // truncate if too long
    const size_t addedURLLength = std::min(urlBudget, (size_t)url.length());
    if(addedURLLength)
    {
        memcpy(pData, url.c_str(), addedURLLength);
        pData += addedURLLength;
        urlBudget -= addedURLLength;
    }
    
    // optionally appending suffix & tail
    if(suffix != EDDY_URL_NONE)
    {
        if(urlBudget < 1)
        {
            // not enough budget
            return;
        }

        *pData++ = suffix;
        urlBudget -= 1;
    }

    if(tail.length())
    {
        if(tail.length() > urlBudget)
        {
            // tail too long
            return;
        }

        memcpy(pData, tail.c_str(), tail.length());
        pData += tail.length();
        urlBudget -= tail.length();
    }

    item.adDataLen = (pData - item.adData);
    
    addAdvertisementData(item);
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
    uint32_t sizeRequired = 0;
    for(size_t i = 0; i < m_advDataList.size(); ++i)
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
    for(size_t i = 0; i < m_advDataList.size(); ++i)
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
    for(size_t i = 0; i < m_attributes.size(); ++i)
    {
        for(uint32_t r = 0; r < m_attributes[i]->getRecordCount(); ++r)
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
    const size_t size = m_records.size();
    for(size_t i = 0; i < size; ++i)
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
    pr_debug("attribute_callback [%d, %d, 0x%p, %d, %d, 0x%p", rw, handle, data, size, offset, user_data);

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
    m_uuid(uuid),
    m_perm(permission),
    m_updated(false)
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

    return copySize;
}



/////////////////////////////////////////////////////////////////////////////////////////////
// LBLEPeripheral
/////////////////////////////////////////////////////////////////////////////////////////////

// The singleton instance
LBLEPeripheralClass LBLEPeripheral;

LBLEPeripheralClass::LBLEPeripheralClass():
    m_clientCount(0)
{
    memset(&m_advParam, sizeof(m_advParam), 0);
    // default parameters
    m_advParam.advertising_interval_min = 0xCD;
    m_advParam.advertising_interval_max = 0xCD;
    m_advParam.advertising_type = BT_HCI_ADV_TYPE_NON_CONNECTABLE_UNDIRECTED;
    m_advParam.own_address_type = BT_ADDR_RANDOM;
    m_advParam.advertising_channel_map = 7;
    m_advParam.advertising_filter_policy = 0;
}

LBLEPeripheralClass::~LBLEPeripheralClass()
{
    
}

void LBLEPeripheralClass::advertise(const LBLEAdvertisementData& advData)
{
    // make a copy of advertisement data for re-advertising after disconnect event.
    // previous advertisement data will be cleared since m_pAdvData is unique_ptr.
    m_pAdvData = std::unique_ptr<LBLEAdvertisementData>(new LBLEAdvertisementData(advData));

    m_advParam.advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED;
    m_advParam.advertising_interval_min = 0x00C0;
    m_advParam.advertising_interval_max = 0x00C0;
    // start advertisement
    advertiseAgain();
}

void LBLEPeripheralClass::advertiseAsBeacon(const LBLEAdvertisementData& advData,
						                    uint32_t intervalMS,
						                    uint8_t txPower)
{
    // make a copy of advertisement data for re-advertising after disconnect event.
    // previous advertisement data will be cleared since m_pAdvData is unique_ptr.
    m_pAdvData = std::unique_ptr<LBLEAdvertisementData>(new LBLEAdvertisementData(advData));

    // Non-connectable and non-scannable advertisement packets interval
    // Should range from 100 ms to 10.24s in steps of 0.625ms.
    intervalMS = std::min((uint32_t)10240, intervalMS);
    intervalMS = std::max((uint32_t)100, intervalMS);

    // advertisement parameters:
    // for beacon advertisement we'll use multi-advertisement API.
    // multi-advertisement **must not** be connectable. This is a limitation.
    const uint16_t advertiseInterval = (uint16_t)intervalMS / 0.625f;
    m_advParam.advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED;
    m_advParam.advertising_interval_min = advertiseInterval;
    m_advParam.advertising_interval_max = advertiseInterval;

#if 1
    pr_debug("txPower %d is ignored", txPower);
    // use normal advertisement API
    advertiseAgain();
#else
    // TODO: currently the multi-advertisment API does not work due to 
    // instance limitation - looks like a bug.

    // use multi-advertisement API,
    // this API allows setting txPower but can only advertise "non-connectable" random addresses.
    bt_bd_addr_t randomAddress;
    generate_random_device_address(randomAddress);
    LBLEAddress debugaddr;
    memcpy(debugaddr.m_addr.addr, randomAddress, BT_BD_ADDR_LEN);
    Serial.print("generated address = ");
    Serial.println(debugaddr);

    // populate the advertisement data
    bt_hci_cmd_le_set_advertising_data_t hci_adv_data = {0};
    hci_adv_data.advertising_data_length = m_pAdvData->getPayload(hci_adv_data.advertising_data, 
                                                              sizeof(hci_adv_data.advertising_data));

    bt_hci_cmd_le_set_scan_response_data_t hci_scan_rsp_data = {0};

    delay(500);
    Serial.print("max instance = ");
    Serial.println((int)bt_gap_le_get_max_multiple_advertising_instances());
    delay(500);

    bt_status_t status = bt_gap_le_start_multiple_advertising(
                                                                1,		// Only allow 1 advertisement instance
                                                                txPower,
                                                                randomAddress,
                                                                &adv_param,
                                                                &hci_adv_data,
                                                                &hci_scan_rsp_data
                                                                );
                            delay(500);
                            if(status != BT_STATUS_SUCCESS)
                            {
                                Serial.print("failed calling start multiple advertising = ");
                                Serial.println(status, HEX);
                            }

    bool done = waitAndProcessEvent(
                        [&](){
                            return;
                        },

                        BT_GAP_LE_START_MULTIPLE_ADVERTISING_CNF,

                        [this](bt_msg_type_t msg, bt_status_t status, void* buf)
                        {
                            Serial.print("status=");
                            Serial.println(status);
                            return;
                        }
    );

    if(!done)
    {
        Serial.println("failed to multi-advertise!");
    }
#endif
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

    // note that advertisement parameters (m_advParam) are pre-configured in advertisement() methods.

    // populate the advertisement data
    bt_hci_cmd_le_set_advertising_data_t hci_adv_data;
    hci_adv_data.advertising_data_length = m_pAdvData->getPayload(hci_adv_data.advertising_data, 
                                                              sizeof(hci_adv_data.advertising_data));

    // start broadcasting
    bt_gap_le_set_advertising(&enable, &m_advParam, &hci_adv_data, NULL);

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
    return LBLEPeripheral.getServiceTable();
}

} // extern "C"

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
    for(size_t i = 0; i < m_services.size(); ++i)
    {
        currentHandle = m_services[i].begin(currentHandle);
        m_servicePtrTable.push_back(m_services[i].getServiceDataPointer());
    }

    // finally, make sure the pointer table is NULL-terminated,
    // as requested by the framework.
    m_servicePtrTable.push_back(NULL);

    // now we start listening to connection events
    LBLE.registerForEvent(BT_GAP_LE_CONNECT_IND, this);
    LBLE.registerForEvent(BT_GAP_LE_DISCONNECT_IND, this);

    return;
}

bool LBLEPeripheralClass::connected()
{
    return (m_clientCount > 0);
}

bool LBLEPeripheralClass::isOnce()
{ 
    // keep listening for connection events
    return false;
}

void LBLEPeripheralClass::onEvent(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    pr_debug("device onEvent, msg=0x%x, status=0x%x, buff=0x%p", (unsigned int)msg, (unsigned int)status, buff);
    switch(msg)
    {
    case BT_GAP_LE_CONNECT_IND:
        m_clientCount++;
        break;
    case BT_GAP_LE_DISCONNECT_IND:
        m_clientCount--;
        advertiseAgain();
        break;
    }
}