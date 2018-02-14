/*

*/

#ifndef LBLECentral_H
#define LBLECentral_H

#include <inttypes.h>
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

    ~LBLECentralClass() {};

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
    /// \param txPower Output parameter, the signed TX Power value in the iBeacon info.
    /// \returns true if the device has iBeacon info. false if the device is not an iBeacon device.
    bool getIBeaconInfo(int index, LBLEUuid& uuid, uint16_t& major, uint16_t& minor, int8_t& txPower)const;

public:
    // Event handlers and required data structure
    virtual bool isOnce() {
        return false;
    };
    virtual void onEvent(bt_msg_type_t msg, bt_status_t status, void *buff);

    static const size_t MAX_DEVICE_LIST_SIZE = 256;
    std::vector<bt_gap_le_advertising_report_ind_t> m_peripherals_found;
    static void processAdvertisement(const bt_gap_le_advertising_report_ind_t *report);

protected:
    bool m_registered;
    bool m_scanning;
    void init();

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
    bool getIBeaconInfo(LBLEUuid& uuid, uint16_t& major, uint16_t& minor, int8_t& txPower) const;

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

/// This (internal) helper class encapsulates the read/write
/// operations to a remote peripheral's characteristic, based on the ATT handle.
/// This class does not store the connection handle, 
/// instead it relies on the user to pass connection when performing
/// read / write operations. 
/// It is the user's responsibility to pass the correct connection handle.
class LBLECharacteristicInfo {
public:

    /// Constructing an invalid object.
    /// All the subsequent read/write operation fails silently.
    LBLECharacteristicInfo():
        m_handle(BT_HANDLE_INVALID),
        m_uuid()
    {

    }

    /// Constructing from ATT handle and UUID of the characteristic
    ///
    /// \param attHandle The ATT handle of the characteristic in the remote peripheral's GATT database
    /// \param uuid The UUID of the characteristic
    LBLECharacteristicInfo(uint16_t attHandle, const LBLEUuid& uuid):
        m_handle(attHandle),
        m_uuid(uuid)
    {
    }

    /// Read raw value buffer fomr a characteristic on remote device.
    ///
    /// \param connection The connection handle to the remote device
    /// \returns LBLEValueBuffer object that represents the raw value buffer.
    LBLEValueBuffer read(bt_handle_t connection) const;

    /// Read integer value from a characteristic on remote device.
    ///
    /// \param connection The connection handle to the remote device
    /// \returns integer value of the characteristic. 0 is returned if fails to read the characteristic.
    int readInt(bt_handle_t connection);

    /// Read string value from a characteristic on remote device.
    ///
    /// \param connection The connection handle to the remote device
    /// \returns string value of the characteristic. Empty string is returned if fails to read the characteristic.
    String readString(bt_handle_t connection);

    /// Read a single char byte value from a characteristic on remote device.
    ///
    /// \param connection The connection handle to the remote device
    /// \returns char value of the characteristic. 0 is returned if fails to read the characteristic.
    char readChar(bt_handle_t connection);

    /// Read float value from a characteristic on remote device.
    ///
    /// \param connection The connection handle to the remote device
    /// \returns float value of the characteristic. 0 is returned if fails to read the characteristic.
    float readFloat(bt_handle_t connection);

    /// Write the value of a characteristic on the remote device.
    ///
    /// \param connection the connection handle to the remote peripheral
    /// \param value The raw buffer value to write.
    int write(bt_handle_t connection, const LBLEValueBuffer& value);

    /// Hepler API that write a characteristic on the remote device as integer.
    ///
    /// \param connection the connection handle to the remote peripheral
    /// \param value The int value to write.
    int writeInt(bt_handle_t connection, int value);

    /// Hepler API that write a characteristic on the remote device as a string.
    ///
    /// \param connection the connection handle to the remote peripheral
    /// \param value The int value to write.
    int writeString(bt_handle_t connection, const String& value);

    /// Hepler API that write a characteristic on the remote device as char.
    ///
    /// \param connection the connection handle to the remote peripheral
    /// \param value The char value to write.
    int writeChar(bt_handle_t connection, char value);

    /// Hepler API that write a characteristic on the remote device as float.
    ///
    /// \param connection the connection handle to the remote peripheral
    /// \param value The float value to write.
    int writeFloat(bt_handle_t connection, float value);

    uint16_t m_handle;      // The ATT handle in the remote peripheral's GATT database.
    LBLEUuid m_uuid;        // Supplemental information about the class UUID of this characteristic.
};

/// This (internal) helper class encapsulates service information.
/// about a service in the remote peripheral device.
/// It also keeps track of the characteristics of the device.
///
/// Note the order and index of the m_characteristics are used to 
/// identify a characteristic.
struct LBLEServiceInfo
{
    LBLEUuid uuid; // Service UUID
    uint16_t startHandle; // Start of the ATT handle range in the remote GATT database
    uint16_t endHandle; // End of the ATT handle range in the remote GATT database
    std::vector<LBLECharacteristicInfo> m_characteristics; // Characteristics that belong to this service
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

    virtual ~LBLEClient();

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

    /// Check if a given service UUID is available on the connected remote device.
    ///
    /// \param serviceUuid The UUID of the service to check
    /// \returns true if the remote device supports the service. false otherwise.
    bool hasService(const LBLEUuid& serviceUuid);

    /// Get the number of services available on the connected remote device
    int getServiceCount();

    /// Get service uuid by index. serviceIndex should range from 0 to (getServiceCount() - 1).
    ///
    /// \param serviceIndex Ranges from 0 to (getServiceCount() - 1).
    /// \returns UUID of the service
    LBLEUuid getServiceUuid(int serviceIndex);

    /// Helper function that returns name of the service if it is know.
    ///
    /// \param serviceIndex Ranges from 0 to (getServiceCount() - 1).
    /// \returns Service name.
    String getServiceName(int serviceIndex);

    /// Get the number of Characteristics available on the connected remote device
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \returns number of characteristics in the given service.
    int getCharacteristicCount(int serviceIndex);

    /// Get Characteristic uuid by service and characteristic indices.
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \param characteristicIndex ranges from 0 to (getCharacteristicCount(serviceIndex) - 1).
    /// \returns UUID of the Characteristic
    LBLEUuid getCharacteristicUuid(int serviceIndex, int characteristicIndex);

    /////////////////////////////////////////////////////////////////////
    //                  UUID-based read and write
    /////////////////////////////////////////////////////////////////////

    /// DEPRECATED. There are typo in method name. This method is kept to prevent breaking old examples.
    ///
    /// \param uuid The UUID of the characteristic to read from.
    /// \returns LBLEValueBuffer object that represents the raw value buffer.
    LBLEValueBuffer readCharacterstic(const LBLEUuid& uuid);

    /// Read raw value buffer fomr a characteristic on remote device, given its UUID.
    /// Note that it is possible to have multiple characteristics with the same UUID.
    /// In this case, only the 1st characteristic with respect to the 
    /// service and characteristic indices is used.
    ///
    /// \param uuid The UUID of the characteristic to read from.
    /// \returns LBLEValueBuffer object that represents the raw value buffer.
    LBLEValueBuffer readCharacteristic(const LBLEUuid& uuid);

    /// Read integer value from a characteristic on remote device, given its UUID.
    /// Note that it is possible to have multiple characteristics with the same UUID.
    /// In this case, only the 1st characteristic with respect to the 
    /// service and characteristic indices is used.
    ///
    /// \param uuid The UUID of the characteristic to read from.
    /// \returns integer value of the characteristic. 0 is returned if fails to read the characteristic.
    int readCharacteristicInt(const LBLEUuid& uuid);

    /// Read string value from a characteristic on remote device, given its UUID.
    /// Note that it is possible to have multiple characteristics with the same UUID.
    /// In this case, only the 1st characteristic with respect to the 
    /// service and characteristic indices is used.
    ///
    /// \param uuid The UUID of the characteristic to read from.
    /// \returns string value of the characteristic. Empty string is returned if fails to read the characteristic.
    String readCharacteristicString(const LBLEUuid& uuid);

    /// Read a single char byte value from a characteristic on remote device, given its UUID.
    /// Note that it is possible to have multiple characteristics with the same UUID.
    /// In this case, only the 1st characteristic with respect to the 
    /// service and characteristic indices is used.
    ///
    /// \param uuid The UUID of the characteristic to read from.
    /// \returns char value of the characteristic. 0 is returned if fails to read the characteristic.
    char readCharacteristicChar(const LBLEUuid& uuid);

    /// Read float value from a characteristic on remote device, given its UUID.
    /// Note that it is possible to have multiple characteristics with the same UUID.
    /// In this case, only the 1st characteristic with respect to the 
    /// service and characteristic indices is used.
    ///
    /// \param uuid The UUID of the characteristic to read from.
    /// \returns float value of the characteristic. 0 is returned if fails to read the characteristic.
    float readCharacteristicFloat(const LBLEUuid& uuid);

    /// Write the value of a characteristic on the remote device, given its UUID.
    /// Note that it is possible to have multiple characteristics with the same UUID.
    /// In this case, only the 1st characteristic with respect to the 
    /// service and characteristic indices is used.
    ///
    /// \param uuid The UUID of the characteristic to write to.
    /// \param value The raw buffer value to write.
    int writeCharacteristic(const LBLEUuid& uuid, const LBLEValueBuffer& value);

    /// Hepler API that write a characteristic on the remote device as integer, given its UUID.
    /// Note that it is possible to have multiple characteristics with the same UUID.
    /// In this case, only the 1st characteristic with respect to the 
    /// service and characteristic indices is used.
    ///
    /// \param uuid The UUID of the characteristic to write to.
    /// \param value The int value to write.
    int writeCharacteristicInt(const LBLEUuid& uuid, int value);

    /// Hepler API that write a characteristic on the remote device as a string, given its UUID.
    /// Note that it is possible to have multiple characteristics with the same UUID.
    /// In this case, only the 1st characteristic with respect to the 
    /// service and characteristic indices is used.
    ///
    /// \param uuid The UUID of the characteristic to write to.
    /// \param value The int value to write.
    int writeCharacteristicString(const LBLEUuid& uuid, const String& value);

    /// Hepler API that write a characteristic on the remote device as char, given its UUID.
    /// Note that it is possible to have multiple characteristics with the same UUID.
    /// In this case, only the 1st characteristic with respect to the 
    /// service and characteristic indices is used.
    ///
    /// \param uuid The UUID of the characteristic to write to.
    /// \param value The char value to write.
    int writeCharacteristicChar(const LBLEUuid& uuid, char value);

    /// Hepler API that write a characteristic on the remote device as float, given its UUID.
    /// Note that it is possible to have multiple characteristics with the same UUID.
    /// In this case, only the 1st characteristic with respect to the 
    /// service and characteristic indices is used.
    ///
    /// \param uuid The UUID of the characteristic to write to.
    /// \param value The float value to write.
    int writeCharacteristicFloat(const LBLEUuid& uuid, float value);

    /////////////////////////////////////////////////////////////////////
    //                  index-based read and write
    /////////////////////////////////////////////////////////////////////

    /// Read the raw buffer value of a characteristic, given its service index 
    /// and characteristic index.
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \param characteristicIndex ranges from 0 to (getCharacteristicCount(serviceIndex) - 1).
    /// \returns A LBLEValueBuffer object representing the raw value of the characteristic
    LBLEValueBuffer readCharacteristic(int serviceIndex, int characteristicIndex);
    
    /// Read integer value of a characteristic, given its service index 
    /// and characteristic index.
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \param characteristicIndex ranges from 0 to (getCharacteristicCount(serviceIndex) - 1).
    /// \returns A LBLEValueBuffer object representing the raw value of the characteristic
    int readCharacteristicInt(int serviceIndex, int characteristicIndex);

    /// Read string value from a characteristic on remote device, given its service index 
    /// and characteristic index.
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \param characteristicIndex ranges from 0 to (getCharacteristicCount(serviceIndex) - 1).
    /// \returns string value of the characteristic. Empty string is returned if fails to read the characteristic.
    String readCharacteristicString(int serviceIndex, int characteristicIndex);

    /// Read a single char byte value from a characteristic on remote device, given its service index 
    /// and characteristic index.
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \param characteristicIndex ranges from 0 to (getCharacteristicCount(serviceIndex) - 1).
    /// \returns char value of the characteristic. 0 is returned if fails to read the characteristic.
    char readCharacteristicChar(int serviceIndex, int characteristicIndex);

    /// Read float value from a characteristic on remote device, given its service index 
    /// and characteristic index.
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \param characteristicIndex ranges from 0 to (getCharacteristicCount(serviceIndex) - 1).
    /// \returns float value of the characteristic. 0 is returned if fails to read the characteristic.
    float readCharacteristicFloat(int serviceIndex, int characteristicIndex);

    /// Write the value of a characteristic on the remote device, given its service index 
    /// and characteristic index.
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \param characteristicIndex ranges from 0 to (getCharacteristicCount(serviceIndex) - 1).
    /// \param value The raw buffer value to write.
    int writeCharacteristic(int serviceIndex, int characteristicIndex, const LBLEValueBuffer& value);

    /// Hepler API that write a characteristic on the remote device as integer, given its service index 
    /// and characteristic index.
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \param characteristicIndex ranges from 0 to (getCharacteristicCount(serviceIndex) - 1).
    /// \param value The int value to write.
    int writeCharacteristicInt(int serviceIndex, int characteristicIndex, int value);

    /// Hepler API that write a characteristic on the remote device as a string, given its service index 
    /// and characteristic index.
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \param characteristicIndex ranges from 0 to (getCharacteristicCount(serviceIndex) - 1).
    /// \param value The int value to write.
    int writeCharacteristicString(int serviceIndex, int characteristicIndex, const String& value);

    /// Hepler API that write a characteristic on the remote device as char, given its service index 
    /// and characteristic index.
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \param characteristicIndex ranges from 0 to (getCharacteristicCount(serviceIndex) - 1).
    /// \param value The char value to write.
    int writeCharacteristicChar(int serviceIndex, int characteristicIndex, char value);

    /// Hepler API that write a characteristic on the remote device as float, given its service index 
    /// and characteristic index.
    ///
    /// \param serviceIndex ranges from 0 to (getServiceCount() - 1).
    /// \param characteristicIndex ranges from 0 to (getCharacteristicCount(serviceIndex) - 1).
    /// \param value The float value to write.
    int writeCharacteristicFloat(int serviceIndex, int characteristicIndex, float value);

public:
    // internal event handler for BT module to callback to
    virtual void onEvent(bt_msg_type_t msg, bt_status_t status, void *buff);

protected:
    bt_handle_t m_connection; // connection handle
    std::vector<LBLEServiceInfo> m_services; // services discovered by discoverServices()
    std::map<LBLEUuid, uint16_t> m_uuid2Handle; // a look-up table for UUID-based read and write operations

    // Enumerate all service info from remote device
    int discoverServices();

    // Read all characteristic on remote device.
    // This could take a while.
    int discoverCharacteristics();

    // Enumerate all characteristics, given a service index.
    int discoverCharacteristicsOfService(int serviceIndex);

    // Retrieve a characteristic by its service and characteristic indices.
    LBLECharacteristicInfo getCharacteristic(int serviceIndex, int characteristicIndex);
};

extern LBLECentralClass LBLECentral;
#endif
