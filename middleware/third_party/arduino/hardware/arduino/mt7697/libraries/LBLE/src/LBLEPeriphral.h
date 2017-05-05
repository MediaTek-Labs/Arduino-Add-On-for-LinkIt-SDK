/*
// A BLE peripheral have 2 major operations:
//	* Advertising itself for BLE central device to scan.
//	* Optinally, it may also providing services consisting of attributes for BLE central to connect to.
*/

#ifndef LBLEPERIPHERAL_H
#define LBLEPERIPHERAL_H

#include <inttypes.h>
#include <vector>
#include <WString.h>
#include <LBLE.h>
#include <vector>
#include <memory>

extern "C"
{
#include "utility/ard_ble.h"
}

struct LBLEAdvDataItem
{
	// according to BLE spec
	static const uint32_t MAX_ADV_DATA_LEN = 0x1F;

	uint8_t adType;
	uint8_t adData[MAX_ADV_DATA_LEN];
	uint32_t adDataLen;

	void clear()
	{
		adType = 0;
		adDataLen = 0;
		memset(adData, 0, MAX_ADV_DATA_LEN);
	}
};

// refer to https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
enum EDDYSTONE_URL_PREFIX
{
	EDDY_HTTP_WWW = 0,		// "http://www."
	EDDY_HTTPS_WWW = 1,		// "https://www."
	EDDY_HTTP = 2,			// "http://"
	EDDY_HTTPS = 3			// "https://"
};

// refer to https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding
enum EDDYSTONE_URL_ENCODING
{
	EDDY_URL_NONE = 0xFFFFFFFF,	// No suffix to append
	EDDY_DOT_COM_SLASH = 0,		// ".com/"
	EDDY_DOT_ORG_SLASH = 1,		// ".org/"
	EDDY_DOT_EDU_SLASH = 2,		// ".edu/"
	EDDY_DOT_NET_SLASH = 3,		// ".net/"
	EDDY_DOT_INFO_SLASH = 4,	// ".info/"
	EDDY_DOT_BIZ_SLASH = 5,		// ".biz/"
	EDDY_DOT_GOV_SLASH = 6,		// ".gov/"
	EDDY_DOT_COM = 7,			// ".com"
	EDDY_DOT_ORG = 8,			// ".org"
	EDDY_DOT_EDU = 9,			// ".edu"
	EDDY_DOT_NET = 10,			// ".net"
	EDDY_DOT_INFO = 11,			// ".info"
	EDDY_DOT_BIZ = 12,			// ".biz"
	EDDY_DOT_GOV = 13			// ".gov"
};

class LBLEAdvertisementData
{
public:
	static const uint8_t DEFAULT_AD_FLAG = (BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE | BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED);

	LBLEAdvertisementData();
	LBLEAdvertisementData(const LBLEAdvertisementData& rhs);
	~LBLEAdvertisementData();

	// Create an iBeacon advertisement.
	//
	// This methods RESETS all the advertisement data fields
	// and replace them with iBeacon format (flag + manufacturer data)
	//
	// if you don't know which UUID to use, 
	// use LBLEUuid("74278BDA-B644-4520-8F0C-720EAF059935"),
	// since this is the UUID used by iOS AirLocate example.
	// (https://developer.apple.com/library/content/samplecode/AirLocate/Introduction/Intro.html)
	//
	// major, minor, and txPower are all user defined values.
	void configAsIBeacon(const LBLEUuid& uuid, 		
							  uint16_t major, 
							  uint16_t minor, 
							  int8_t txPower);

	// Configure an Eddystone URL
	// Note that total length must not exceed 17 bytes.
	// 
	// You can use prefix, suffix, and tail parameters to compress common URL parts to a single byte.
	// e.g. "https://www.mediatek.com" 
	//			=>  configAsEddystoneURL(EDDY_HTTPS_WWW, "mediatek", EDDY_DOT_COM)
	// e.g. "https://www.asp.net/learn"
	//			=>  configAsEddystoneURL(EDDY_HTTPS_WWW, "asp", EDDY_DOT_NET_SLASH, "learn")
	//
	// Please refer to https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
	// to know how the prefix/suffix/tails are expanded.
    void configAsEddystoneURL(EDDYSTONE_URL_PREFIX prefix,
							  const String& url,
							  EDDYSTONE_URL_ENCODING suffix = EDDY_URL_NONE,
							  const String& tail = String());

	// Create an advertisement that allows smartphones to connect
	// to this device.
	//
	// This methods RESETS all the advertisement data fields
	//
	// deviceName must be shorter than 27 bytes.
	//
	// You need to define corresponding GATT services
	// before start advertising your device.
	void configAsConnectableDevice(const char* deviceName);
	void configAsConnectableDevice(const char* deviceName, const LBLEUuid& uuid);

	// Append a AD type flag data
	void addFlag(uint8_t flag = DEFAULT_AD_FLAG);

	// Append a Device Name (Complete) flag data
	void addName(const char* deviceName);

	// Add a generic AD data
	//
	// Note that item.adDataLen is the length of the "adData" item,
	// not the length of the entire AD data length in the final payload.
	void addAdvertisementData(const LBLEAdvDataItem &item);

	// Convert to a raw advertisement data payload.
	uint32_t getPayload(uint8_t* buf, uint32_t bufLength) const;

private:
	std::vector<LBLEAdvDataItem> m_advDataList;
};

enum LBLEPermission
{
	LBLE_READ = BT_GATTS_REC_PERM_READABLE,
	LBLE_WRITE = BT_GATTS_REC_PERM_WRITABLE,
};

class LBLEAttributeInterface
{
public:
	static const uint32_t MAX_ATTRIBUTE_DATA_LEN = 512;
	virtual uint32_t onSize() const = 0;
	virtual uint32_t onRead(void *data, uint16_t size, uint16_t offset) = 0;
	virtual uint32_t onWrite(void *data, uint16_t size, uint16_t offset) = 0;

	virtual uint32_t getRecordCount() = 0;

	// @param recordIndex ranges from 0 ~ (getRecordCount - 1)
	// 
	// returns a generic bt_gatts_service_rec_t pointer
	// that points to variable-length GATT attribute records.
	// The user is reponsible to free() the returning buffer.
	virtual bt_gatts_service_rec_t* allocRecord(uint32_t recordIndex, uint16_t currentHandle) = 0;
};

class LBLECharacteristicBase : public LBLEAttributeInterface
{
public:	// method for Arduino users

	LBLECharacteristicBase(LBLEUuid uuid, uint32_t permission);

	// Check if a character is written
	bool isWritten();

public:
	// Each characteristic maps to 2 GATT attribute records
	virtual uint32_t getRecordCount() {return 2;};

	// common implementation for characteristics attributes
	virtual bt_gatts_service_rec_t* allocRecord(uint32_t recordIndex, uint16_t currentHandle);

protected:
	LBLEUuid m_uuid;
	uint32_t m_perm;
	bool m_updated;
};

// This class is used by LBLECharacteristicBuffer
struct LBLECharacteristicWrittenInfo
{
	uint16_t size;
	uint16_t offset;
};

// This characteristic is a 512-byte raw buffer initialized to zero
// When isWritten() is true, you can use getLastWrittenInfo()
// to check what part of the buffer is updated during the last write.
class LBLECharacteristicBuffer : public LBLECharacteristicBase
{
public:	// method for Arduino users

	LBLECharacteristicBuffer(LBLEUuid uuid, uint32_t permission);

	// Set value - size must not exceed MAX_ATTRIBUTE_DATA_LEN.
	void setValueBuffer(const uint8_t* buffer, size_t size);

	// Get value buffer content. (size + offset) must not exceed MAX_ATTRIBUTE_DATA_LEN.
	// Note that isWritten() flag turns off after calling getValue().
	void getValue(uint8_t* buffer, uint16_t size, uint16_t offset);

	// Check what part of the buffer is updated during last write operation
	const LBLECharacteristicWrittenInfo& getLastWrittenInfo() const;

public:	// for BLE framework
	virtual uint32_t onSize() const;
	virtual uint32_t onRead(void *data, uint16_t size, uint16_t offset);
	virtual uint32_t onWrite(void *data, uint16_t size, uint16_t offset);

private:
	uint8_t m_data[MAX_ATTRIBUTE_DATA_LEN];
	LBLECharacteristicWrittenInfo m_writtenInfo;
};

// This characterstic is a peristent 4-byte integer initialized to zero.
// The size is always 4 bytes.
class LBLECharacteristicInt : public LBLECharacteristicBase
{
public:	// method for Arduino users

	LBLECharacteristicInt(LBLEUuid uuid, uint32_t permission);

	// Set value
	void setValue(int value);

	// Retrieve value, note that isWritten() flag turns off after calling getValue()
	int getValue();

public:	// for BLE framework
	virtual uint32_t onSize() const;
	virtual uint32_t onRead(void *data, uint16_t size, uint16_t offset);
	virtual uint32_t onWrite(void *data, uint16_t size, uint16_t offset);

private:
	int m_data;
};

// This is a "string" attribute. A NULL terminater
// is always automatically inserted after each write operation.
//
// That is, the string value is always "reset" after each write operation, 
// instead of appending/replacing part of the existing string value.
//
// Example:
//  * the central device sends "YES" (3 bytes)
//  * the central device then sends "NO"(2 bytes)
//  * the resulting value is "NO\0" instead of "NOS\0".
//
// The reason for this design is to make it more intuitive
// to use with AppInventor's BluetoothLE.WriteStringValue block.
//
// If you need behavior that supports replacing part of the buffer,
// use LBLECharacteristicBuffer instead.
class LBLECharacteristicString : public LBLECharacteristicBase
{
public:	// method for Arduino users

	LBLECharacteristicString(LBLEUuid uuid, uint32_t permission);

	// Set value
	void setValue(const String& value);

	// Retrieve value, note that isWritten() flag turns off after calling getValue()
	String getValue();

public:	// for BLE framework
	virtual uint32_t onSize() const;
	virtual uint32_t onRead(void *data, uint16_t size, uint16_t offset);
	virtual uint32_t onWrite(void *data, uint16_t size, uint16_t offset);

private:
	String m_data;
};

class LBLEService
{
public:
	LBLEService(const LBLEUuid& uuid);
	LBLEService(const char* uuidString);

	void addAttribute(LBLEAttributeInterface& attr);
	
	// Allocates underlying record tables for BLE framework.
	// Accepts the globally ordered handle
	// returns the last handle + 1.
	uint16_t begin(uint16_t startingHandle);

	void end();

	bt_gatts_service_t* getServiceDataPointer();
private:	
	LBLEUuid m_uuid;
	bt_gatts_service_t m_serviceData; 				// service record for BLE framework
	std::vector<bt_gatts_service_rec_t*> m_records; // attribute records (multiple types) for BLE framework
	std::vector<LBLEAttributeInterface*> m_attributes; // pointers to attribute objects
};

// Singleton class representing the local BLE periphral device
class LBLEPeripheralClass : public LBLEEventObserver
{
public:
	LBLEPeripheralClass();
	~LBLEPeripheralClass();

	// start advertisement as a connectable device
	void advertise(const LBLEAdvertisementData& advertisementData);

	// start advertisement as an non-connectable device (such as an iBeacon)
	//
	// @param advertisementData The advertisement packet to broadcast.
	// @param intervalMS The advertisement interval, in milliseconds.
	// @param txPower This controls the actual advertisement power in dbm. 
	//        Note: current version does not support adjusting txPower.
	void advertiseAsBeacon(const LBLEAdvertisementData& advertisementData,
						   uint32_t intervalMS = 700,
						   uint8_t txPower = -30);
	
	// start advertisement based on previous input of advertise
	void advertiseAgain();

	// stop advertisement and clears advertisement data
	// advertiseAgain() fails after stopAdvertise();
	void stopAdvertise();

	// Generic Access Profile (GAP) configuration
	void setName(const char* name);

	// After setup services and characteristics
	// you have to call begin make enable the GATT server
	void begin();

	// returns true if there is a central device connecting to this peripheral.
	bool connected();

	// configuring GATT Services. You must configure services
	// before advertising the device. The services cannot change
	// after being connected.
	void addService(const LBLEService& service);

	const bt_gatts_service_t** getServiceTable();

public:
	virtual void onEvent(bt_msg_type_t msg, bt_status_t status, void *buff);
	virtual bool isOnce();

private:
	const static uint16_t USER_ATTRIBUTE_HANDLE_START = 0x00A0;
	std::vector<const bt_gatts_service_t*> m_servicePtrTable;
	std::vector<LBLEService> m_services;			// container for services in the server
	
	std::unique_ptr<LBLEAdvertisementData> m_pAdvData;
	bt_hci_cmd_le_set_advertising_parameters_t m_advParam;

	uint16_t m_clientCount;
	
};

extern LBLEPeripheralClass LBLEPeripheral;


#endif // #ifndef LBLEPERIPHERAL_H
