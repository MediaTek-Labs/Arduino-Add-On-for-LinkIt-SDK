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

class LBLEGATTAttributes : public bt_gatts_service_rec_t
{

};

class LBLEGATTService : public bt_gatts_service_t
{
public:
	LBLEGATTService(const LBLEUuid& uuid);
	LBLEGATTService& addCharacteristic(const LBLEUuid& uuid);

private:
	// std::vector<bt_gatt_service_rec_t> m_attributes;	// container for attributes in services
};

class LBLEPeripheralClass
{
public:
	LBLEPeripheralClass();
	~LBLEPeripheralClass();

	// start advertisement
	void advertise(const LBLEAdvertisementData& advertisementData);
	
	// start advertisement based on previous input of advertise.
	void advertiseAgain();

	// stop advertisement and clears advertisement data.
	// advertiseAgain() fails after stopAdvertise();
	void stopAdvertise();

	// Generic Access Profile (GAP) configuration
	void setName(const char* name);

	// configuring GATT Services. You must configure services
	// before advertising the device. The services cannot change
	// after being connected.
	void addService(const LBLEGATTService& service);

	const bt_gatts_service_t** getServiceTable();

private:
	void populateServicePointerTable();
	uint16_t allocAttrHandle();

private:
	const static uint16_t USER_ATTRIBUTE_HANDLE_START = 0x00A0;
	std::vector<const bt_gatts_service_t*> m_servicePtrTable;
	std::vector<bt_gatts_service_t> m_services;			// container for services in the server
	
	std::unique_ptr<LBLEAdvertisementData> m_pAdvData;
	uint16_t m_attrHandle;
};

extern LBLEPeripheralClass LBLEPeripheral;


#endif // #ifndef LBLEPERIPHERAL_H
