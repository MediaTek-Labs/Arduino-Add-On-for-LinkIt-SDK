/*

*/

#ifndef LBLECentral_H
#define LBLECentral_H

#include <inttypes.h>
#include <vector>
#include <WString.h>
#include <LBLE.h>
#include <map>

extern "C"
{
#include "utility/ard_ble.h"
}

extern "C" bt_status_t bt_app_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff);
extern "C" void ard_ble_central_onCentralEvents(bt_msg_type_t msg, bt_status_t status, void *buff);

// This class allows LinkIt 7697 to act as a BLE central device.
// It can scan nearby BLE peripherals, checking if they are beacons or 
// devices that provides GATT services.
// This class does not provide functions to connect to remote 
// peripheral devices. Use LBLEClient class to connect to a remote
// device and read/write its characteristics.
class LBLECentralClass : public LBLEEventObserver
{
public:
	LBLECentralClass();

	/* Scan all nearby BLE peripherals.
	 * Note that it may took sometime for peripherals to get scanned.
	 *
	 * This puts the system into a scanning statie.
	 * The system keeps updating the peripheral list in the background.
	 *
	 * To get the scanned peripheral, call getPeripheralCount() and use
	 * methods such as `getPeripheralName` or `isBeacon`
	 * 
	 * The return value is the number of the peripherals currently found.
	 */
	void scan();

	/* Stop scanning nearby BLE peripherals. 
	 * The list of discovered peripherals are still kept but will no longer update.
	 */
	void stopScan();

	// Retrive the number of peripherals currently discovered
	int getPeripheralCount();

	// Get scanned peripheral information by index,
	// ranging from 0 to [getRemotePeripheralCount() - 1]
	String getAddress(int index);
	LBLEAddress getBLEAddress(int index);
	String getName(int index);
	int32_t getRSSI(int index);
	int32_t getTxPower(int index);
	LBLEUuid getServiceUuid(int index) const;
	String getManufacturer(int index) const;	

	// Advertisement Flags may be:
	//	BT_GAP_LE_AD_FLAG_LIMITED_DISCOVERABLE     (0x01)
	//  BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE     (0x02)
	//	BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED	   (0x04)
	//	
	//	There are also other flags, including:
	//	0x08: Bluetooth BR/EDR Controller
	//	0x10: Bluetooth BR/EDR Host
	//	which are not used by BLE-only devices.
	uint8_t  getAdvertisementFlag(int index)const;

	bool isIBeacon(int index) const;
	bool getIBeaconInfo(int index, LBLEUuid& uuid, uint16_t& major, uint16_t& minor, uint8_t& txPower)const;


public:
	// Event handlers and required data structure
	virtual bool isOnce() { return false; };
	virtual void onEvent(bt_msg_type_t msg, bt_status_t status, void *buff);

	std::vector<bt_gap_le_advertising_report_ind_t> m_peripherals_found;
	static void processAdvertisement(const bt_gap_le_advertising_report_ind_t *report);
};

class LBLEAdvertisements
{
public:
	LBLEAdvertisements(const bt_gap_le_advertising_report_ind_t& adv);

	virtual bool isValid() const;

	// Get the Bluetooth device address (MAC address) of the advertising device.
	String getAddress() const;

	// Get local name boardcasted. This may be empty (NULL).
	String getName() const;

	// Get transmitted Tx power of the advertisement
	int8_t getTxPower() const;

	// Get RSSI during scan
	int8_t getRSSI() const;

	LBLEUuid getServiceUuid() const;

	String getManufacturer() const;

	uint8_t getAdvertisementFlag()const;

	// returns false if this advertisment is not of iBeacon format.
	bool getIBeaconInfo(LBLEUuid& uuid, uint16_t& major, uint16_t& minor, uint8_t& txPower) const;

	// Copy the "original" advertisement packet data
	// bufLen should be the size of dstBuf.
	// The buffer will be truncated if bufLen is not long enough.
	// Note that the spec limits maxumimum advertisement data to be 31 bytes.
	//
	// This method always returns the required buffer size for the advertisement data.
	uint32_t getRawManufacturerData(uint8_t* dstBuf, uint32_t bufLen);

	// Copy the "original" advertisement packet data
	// bufLen should be the size of dstBuf.
	// The buffer will be truncated if bufLen is not long enough.
	// Note that the spec limits maxumimum advertisement data to be 31 bytes.
	//
	// This method always returns the required buffer size for the advertisement data.
	uint32_t getRawAdvertisementData(uint8_t* dstBuf, uint32_t bufLen);
	
	// Copy the "original" scan response packet data.
	// Note that scan response is optional and may be empty (all zero).
	//
	// bufLen should be the size of dstBuf.
	// The buffer will be truncated if bufLen is not long enough.
	// Note that the spec limits maxumimum advertisement data to be 31 bytes.
	//
	// This method always returns the required buffer size for the advertisement data.
	uint32_t getRawScanResponseData(uint8_t* dstBuf, uint32_t bufLen);

	static const uint32_t MAX_ADV_DATA_LEN = 0x1F;

private:
	
	const bt_gap_le_advertising_report_ind_t& adv_data;

	// searches if a specific AD Type field exists in advertisement or scan response data.
	// copy the content if found, and returns the length of the AD data found.
	// otherwise 0 is returned.
	//
	// param type: one of the advertisement type in Bluetooth specification.
	//			   refer to bt_gap_le.h for macro constants like BT_GAP_LE_AD_TYPE_FLAG.
	uint32_t getAdvDataWithType(uint8_t type, uint8_t* dstBuf, uint32_t bufLen) const;

	static uint32_t getAdvDataWithTypeFromPayload(uint8_t type, 
												  uint8_t* dstBuf, uint32_t bufLen, 
												  const bt_gap_le_advertising_report_ind_t& payload);
};

struct LBLEServiceInfo
{
	LBLEUuid uuid;
	uint16_t startHandle;
	uint16_t endHandle;
};

class LBLEValueBuffer : public std::vector<uint8_t>
{
public:
	LBLEValueBuffer();
	LBLEValueBuffer(int intValue);
	LBLEValueBuffer(float floatValue);
	LBLEValueBuffer(char charValue);
	LBLEValueBuffer(const String& strValue);

	template<typename T>void shallowInit(T value);
};

class LBLEClient : public LBLEEventObserver
{
public:
	LBLEClient();
	
	// Connect to a remote peripheral device.
	// You can use LBLECentralClass to scan nearby devices and 
	// get their addresses
	//
	// This function also implicitly enumerates all the services
	// and characteristics on the remote device, so it may take a while
	// for this function to return.
	bool connect(const LBLEAddress& address);

	// check if connected to remote device.
	bool connected();

	// disconnect from the remote device
	void disconnect();
	
	// get number of services available on the connected remote device
	int getServiceCount();

	// get service uuid from index. Index should range from 0 to (getServiceCount() - 1).
	LBLEUuid getServiceUuid(int index);

	// Helper function that returns name of the service if it is know.
	String getServiceName(int index);

	// check if a given service is available on the connected remote device.
	bool hasService(const LBLEUuid& uuid);
	
	// Read a characteristic from the remote device.
	LBLEValueBuffer readCharacterstic(const LBLEUuid& uuid);
	int readCharacteristicInt(const LBLEUuid& uuid);
	String readCharacteristicString(const LBLEUuid& uuid);
	char readCharacteristicChar(const LBLEUuid& uuid);
	float readCharacteristicFloat(const LBLEUuid& uuid);

	// Write a characteristic of the remote device.
	int writeCharacteristic(const LBLEUuid& uuid, const LBLEValueBuffer& value);
	int writeCharacteristicInt(const LBLEUuid& uuid, int value);
	int writeCharacteristicString(const LBLEUuid& uuid, const String& value);
	int writeCharacteristicChar(const LBLEUuid& uuid, char value);
	int writeCharacteristicFloat(const LBLEUuid& uuid, float value);

public:
	virtual void onEvent(bt_msg_type_t msg, bt_status_t status, void *buff);

protected:
	bt_handle_t m_connection;
	std::vector<LBLEServiceInfo> m_services;
	std::map<LBLEUuid, uint16_t> m_characteristics;
	
	// enumerate all service info from remote device
	int discoverServices();

	// Read all characteristic on remote device.
	// This could take a while.
	int discoverCharacteristics();

	// Enumerate all characteristics, given a service ID.
	int discoverCharacteristicsOfService(const LBLEServiceInfo& s);
};

extern LBLECentralClass LBLECentral;
#endif
