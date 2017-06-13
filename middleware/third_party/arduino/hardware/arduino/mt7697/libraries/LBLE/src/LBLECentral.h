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

/// Singleton class that creates a BLE central device.
///
/// This class allows LinkIt 7697 to act as a BLE central device.
/// It can scan nearby BLE peripherals, checking if they are beacons or 
/// devices that provides GATT services.
///
/// This class does not provide functions to connect to remote 
/// peripheral devices. Use LBLEClient class instead to connect to a remote
/// device and read/write its characteristics.
class LBLECentralClass : public LBLEEventObserver
{
public:
	/// Do not instantiate this class by yourself. Use `LBLE` singleton instance instead.
	LBLECentralClass();

	/// \brief Start scanning nearby BLE peripherals
	///
	/// This puts the system into a scanning statie.
	/// The system keeps updating the peripheral list in the background.
	/// 
	/// The result of `getPeripheralCount()` will be updated once a new peripheral is found.
	/// 
	/// Note that it may took sometime for peripherals to get scanned.
	///
	/// To get the scanned peripheral, call getPeripheralCount() and use
	/// methods such as `getPeripheralName` or `isBeacon`, for example:
	///
	/// ~~~{.cpp}
	///   // start scanning
	///   LBLECentral.scan();
	///
	///	  // wait for 5 seconds to scan
	///   for(int i = 0; i < 5; ++i)
	///   {
	///     delay(1000);
	///     Serial.print(".");
	///   }
	///
	///   // list name of the peripherals found.
	///   const int found = LBLECentral.getPeripheralCount();
	///   for (int i = 0; i < found; ++i) {
    ///	      Serial.println(LBLECentral.getName(i));
	///	  }
	/// ~~~
	///
	/// The return value is the number of the peripherals currently found.
	void scan();

	/// Stop scanning nearby BLE peripherals. 
	///
	/// Stops the system from scanning nearby peripherals.
	/// The list of discovered peripherals are still kept but will no longer be updated.
	void stopScan();

	/// Retrive the number of peripherals currently discovered
	int getPeripheralCount();

	///////////////////////////////////////////////////
	/// Get scanned peripheral information by index
	///////////////////////////////////////////////////

	/// Get address of the peripheral in the list of scanned devices. 
	///
	/// \param index ranges from `0` to `(getRemotePeripheralCount() - 1)`.
	/// \returns device address (in string format) of the scanned peripheral.
	String getAddress(int index);

	/// Get address of the peripheral in the list of scanned devices. 
	///
	/// \param index ranges from `0` to `(getRemotePeripheralCount() - 1)`.
	/// \returns device address of the scanned peripheral. 
	LBLEAddress getBLEAddress(int index);

	/// Get device name of the peripheral in the list of scanned devices. 
	///
	/// \param index ranges from `0` to `(getRemotePeripheralCount() - 1)`.
	/// \returns device name
	String getName(int index);

	/// Get scanned RSSI of the peripheral in the list of scanned devices. 
	///
	/// \param index ranges from `0` to `(getRemotePeripheralCount() - 1)`.
	/// \returns RSSI value in dbm.
	int32_t getRSSI(int index);

	/// Get txPower value in the advertisement packet of the peripheral in the list of scanned devices. 
	///
	/// \param index ranges from `0` to `(getRemotePeripheralCount() - 1)`.
	/// \returns TX power field, if any, in the advertisement packet.
	int32_t getTxPower(int index);

	/// Get service UUID in the advertisement packet of the peripheral in the list of scanned devices. 
	///
	/// \param index ranges from `0` to `(getRemotePeripheralCount() - 1)`.
	/// \returns service UUID, if any, in the advertisement packet.
	LBLEUuid getServiceUuid(int index) const;

	/// Get manufacturer info in the advertisement packet of the peripheral in the list of scanned devices. 
	///
	/// \param index ranges from `0` to `(getRemotePeripheralCount() - 1)`.
	/// \returns manufacturer id, if any, in the advertisement packet.
	String getManufacturer(int index) const;	

	/// Get the advertisement flag of the scanned device.
	///
	/// Advertisement Flags may be the result of bitwise-or of following values:
	///
	///	 * `BT_GAP_LE_AD_FLAG_LIMITED_DISCOVERABLE (0x01)`
	///  * `BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE (0x02)`
	///	 * `BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED (0x04)`
	///	
	///	There are also other flags, including:
	///
	///	 * 0x08: Bluetooth BR/EDR Controller
	///	 * 0x10: Bluetooth BR/EDR Host
	///
	///	which are not used by BLE-only devices.
	///
	/// \param index ranges from `0` to `(getRemotePeripheralCount() - 1)`.
	/// \returns 
	uint8_t  getAdvertisementFlag(int index)const;

	/// Check if the peripheral is an iBeacon device.
	///
	/// \param index ranges from `0` to `(getRemotePeripheralCount() - 1)`.
	/// \returns true if the device broadcasts iBeacon format advertisements, false otherwise.
	bool isIBeacon(int index) const;

	/// Get iBeacon infomrmation from the advertisement data of an iBeacon device.
	///
	/// You can refer to this article on the iBeacon info fields:
	/// https://developer.mbed.org/blog/entry/BLE-Beacons-URIBeacon-AltBeacons-iBeacon/#iBeacon
	///
	/// \param index ranges from `0` to `(getRemotePeripheralCount() - 1)`.
	/// \param uuid Output parameter of the iBeacon info - this UUID is defined by the iBeacon device.
	/// \param major Output parameter, the major number in the iBeacon info.
	/// \param minor Output parameter, the minor number in the iBeacon info.
	/// \param minor Output parameter, the TX Power value in the iBeacon info.
	/// \returns true if the device has iBeacon info. false if the device is not an iBeacon device.
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

/// This class encapsulates raw buffer operations used by LBLEClient
///
/// When writing or reading GATT attributes, we need to convert
/// to raw buffers and meaningful data types used by users.
/// 
/// This class helps users to convert to these raw buffer
/// values when reading or writing GATT attributes.
class LBLEValueBuffer : public std::vector<uint8_t>
{
public:
	/// Default constructor creates an empty buffer.
	LBLEValueBuffer();

	/// Create a raw buffer from an integer value.
	LBLEValueBuffer(int intValue);

	/// Create a raw buffer from a float value.
	LBLEValueBuffer(float floatValue);

	/// Create a raw buffer from a single-byte character value.
	LBLEValueBuffer(char charValue);

	/// Create a raw buffer from a NULL-terminated string.
	/// The resulting buffer contains the trailing NULL bytel.
	LBLEValueBuffer(const String& strValue);

	template<typename T>void shallowInit(T value);
};

/// This class allows users to create connections to remote peripheral devices.
///
/// To access device attributes, create LBLEClient object and
/// call LBLEClient::connect to connect to the device in address:
/// ~~~{.cpp}
/// // assume LBLECentral::scan has already performed.
/// // connect to the 1st device scanned.
/// LBLEAddress serverAddress = LBLECentral.getBLEAddress(0);
/// client.connect(serverAddress);
/// while(!client.connected()){
///		delay(100);	
///	}
/// // now we are connnected.
/// ~~~
/// 
/// Upon successful connection, connected() returns true.
///
/// The user can then query if certain services exists on the remote device
/// with getServiceCount(), getServiceUuid() and hasService().
/// 
/// Once the user confirmed that a service is available, the user may use
/// read/write APIs such as readCharacteristicString() or writeCharacteristicFloat()
/// to read and write values of the characteristic.
/// 
/// Note that there is no extra "type" checking, so it is up to the user to make sure
/// the value types are matched between read/write APIs and the remote device.
class LBLEClient : public LBLEEventObserver
{
public:
	LBLEClient();
	
	/// Connect to a remote peripheral device.
	///
	/// You can use LBLECentralClass to scan nearby devices and 
	/// get their addresses
	///
	/// This function also implicitly enumerates all the services
	/// and characteristics on the remote device, so it may take a while
	/// for this function to return.
	///
	/// \param address Address of the device to connect to. 
	///                You can use LBLECentral::getBLEAddress() to get the address of a scanned device.
	/// \returns true if the connection attempt starts successfully. Note that this does not mean
	///               the connection has been established.
	/// \returns false if fail to start connection.
	bool connect(const LBLEAddress& address);

	/// Check if the connection to remote device has been established.
	bool connected();

	/// Disconnect from the remote device
	void disconnect();
	
	/// Get the number of services available on the connected remote device
	int getServiceCount();

	/// Get service uuid by index. Index should range from 0 to (getServiceCount() - 1).
	///
	/// \param index ranges from 0 to (getServiceCount() - 1).
	/// \returns UUID of the service
	LBLEUuid getServiceUuid(int index);

	/// Helper function that returns name of the service if it is know.
	///
	/// \param index ranges from 0 to (getServiceCount() - 1).
	/// \returns Service name.
	String getServiceName(int index);

	/// Check if a given service is available on the connected remote device.
	///
	/// \param uuid The UUID of the service to check
	/// \returns true if the remote device supports the service. false otherwise.
	bool hasService(const LBLEUuid& uuid);
	
	/// Read raw value buffer fomr a characteristic on remote device.
	/// 
	/// \param uuid The UUID of the characteristic to read from.
	/// \returns LBLEValueBuffer object that represents the raw value buffer.
	LBLEValueBuffer readCharacterstic(const LBLEUuid& uuid);

	/// Read integer value from a characteristic on remote device.
	/// 
	/// \param uuid The UUID of the characteristic to read from.
	/// \returns integer value of the characteristic. 0 is returned if fails to read the characteristic.
	int readCharacteristicInt(const LBLEUuid& uuid);

	/// Read string value from a characteristic on remote device.
	/// 
	/// \param uuid The UUID of the characteristic to read from.
	/// \returns string value of the characteristic. Empty string is returned if fails to read the characteristic.
	String readCharacteristicString(const LBLEUuid& uuid);

	/// Read a single char byte value from a characteristic on remote device.
	/// 
	/// \param uuid The UUID of the characteristic to read from.
	/// \returns char value of the characteristic. 0 is returned if fails to read the characteristic.
	char readCharacteristicChar(const LBLEUuid& uuid);

	/// Read float value from a characteristic on remote device.
	/// 
	/// \param uuid The UUID of the characteristic to read from.
	/// \returns float value of the characteristic. 0 is returned if fails to read the characteristic.
	float readCharacteristicFloat(const LBLEUuid& uuid);

	/// Write the value of a characteristic on the remote device.
	///
	/// \param uuid The UUID of the characteristic to write to.
	/// \param value The raw buffer value to write.
	int writeCharacteristic(const LBLEUuid& uuid, const LBLEValueBuffer& value);

	/// Hepler API that write a characteristic on the remote device as integer.
	/// 
	/// \param uuid The UUID of the characteristic to write to.
	/// \param value The int value to write.
	int writeCharacteristicInt(const LBLEUuid& uuid, int value);
	
	/// Hepler API that write a characteristic on the remote device as a string.
	/// 
	/// \param uuid The UUID of the characteristic to write to.
	/// \param value The int value to write.
	int writeCharacteristicString(const LBLEUuid& uuid, const String& value);
	
	/// Hepler API that write a characteristic on the remote device as char.
	/// 
	/// \param uuid The UUID of the characteristic to write to.
	/// \param value The char value to write.
	int writeCharacteristicChar(const LBLEUuid& uuid, char value);
	
	/// Hepler API that write a characteristic on the remote device as float.
	/// 
	/// \param uuid The UUID of the characteristic to write to.
	/// \param value The float value to write.
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
