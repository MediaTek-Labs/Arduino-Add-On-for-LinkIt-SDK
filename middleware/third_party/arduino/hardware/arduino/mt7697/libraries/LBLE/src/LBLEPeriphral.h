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

/// Enum for LBLEAdvertisementData::configAsEddystoneURL.
///
// refer to https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
enum EDDYSTONE_URL_PREFIX
{
    EDDY_HTTP_WWW = 0,		// "http://www."
    EDDY_HTTPS_WWW = 1,		// "https://www."
    EDDY_HTTP = 2,			// "http://"
    EDDY_HTTPS = 3			// "https://"
};

/// Enum for LBLEAdvertisementData::configAsEddystoneURL.
///
/// refer to https://github.com/google/eddystone/tree/master/eddystone-url#eddystone-url-http-url-encoding
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

/// This helper class helps to configure and parse BLE GAP advertisement packets.
///
/// BLE peripherals advertises information about their capabilities and other information
/// in advertisement packets. The packet is of finite length, but the format is very flexible.
///
/// This class helps you to create several common advertisement formats, e.g. ibeacon:
/// ~~~{.cpp}
/// LBLEAdvertisementData beaconData;
/// // This is a common AirLocate example UUID.
/// LBLEUuid uuid("E2C56DB5-DFFB-48D2-B060-D0F5A71096E0");
/// beaconData.configAsIBeacon(uuid, 01, 02, -40);
/// ~~~
///
/// You can then simply call `LBLEPeripheral.advertise` to start advertise:
/// ~~~{.cpp}
/// // start advertising
/// LBLEPeripheral.advertise(beaconData);
/// ~~~
class LBLEAdvertisementData
{
public:
    static const uint8_t DEFAULT_AD_FLAG = (BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE | BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED);

    LBLEAdvertisementData();
    LBLEAdvertisementData(const LBLEAdvertisementData& rhs);
    ~LBLEAdvertisementData();

    /// Create an iBeacon advertisement.
    ///
    /// This methods RESETS all the advertisement data fields
    /// and replace them with iBeacon format (flag + manufacturer data)
    ///
    /// if you don't know which UUID to use,
    /// use LBLEUuid("74278BDA-B644-4520-8F0C-720EAF059935"),
    /// since this is the UUID used by iOS AirLocate example.
    /// (https://developer.apple.com/library/content/samplecode/AirLocate/Introduction/Intro.html)
    ///
    /// major, minor, and txPower are all user defined values.
    void configAsIBeacon(const LBLEUuid& uuid,
                         uint16_t major,
                         uint16_t minor,
                         int8_t txPower);

    /// Configure an Eddystone URL
    /// Note that total length must not exceed 17 bytes.
    ///
    /// You can use prefix, suffix, and tail parameters to compress common URL parts to a single byte.
    /// e.g. "https://www.mediatek.com"
    ///			=>  configAsEddystoneURL(EDDY_HTTPS_WWW, "mediatek", EDDY_DOT_COM)
    /// e.g. "https://www.asp.net/learn"
    ///			=>  configAsEddystoneURL(EDDY_HTTPS_WWW, "asp", EDDY_DOT_NET_SLASH, "learn")
    ///
    /// Please refer to https://github.com/google/eddystone/tree/master/eddystone-url#url-scheme-prefix
    /// to know how the prefix/suffix/tails are expanded.
    void configAsEddystoneURL(EDDYSTONE_URL_PREFIX prefix,
                              const String& url,
                              EDDYSTONE_URL_ENCODING suffix = EDDY_URL_NONE,
                              const String& tail = String());

    /// Create an advertisement that allows BLE centrals, e.g. smartphones, to connect to this device.
    ///
    /// This methods RESETS all the advertisement data fields
    ///
    /// Note that you need to define corresponding GATT services with LBLEPeripheral
    /// before start advertising your device.
    ///
    /// \param deviceName must be shorter than 27 bytes.
    void configAsConnectableDevice(const char* deviceName);

    /// Create an advertisement with service UUID that allows BLE centrals, e.g. smartphones, to connect to this device.
    ///
    /// This methods RESETS all the advertisement data fields
    /// Note that you need to define corresponding GATT services with LBLEPeripheral
    /// before start advertising your device.
    ///
    /// \param deviceName must be shorter than 9 bytes when UUID is 128-bit.
    /// \param uuid service UUID to be included in advertisement
    void configAsConnectableDevice(const char* deviceName, const LBLEUuid& uuid);

    /// Append a AD type flag data
    ///
    /// AD flags are advertisement flags such as BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE.
    /// You can find valid values in Bluetooth specification.
    void addFlag(uint8_t flag = DEFAULT_AD_FLAG);

    /// Helper method that append a Device Name (Complete) data
    void addName(const char* deviceName);

    /// Add a generic AD data
    ///
    /// Note that item.adDataLen is the length of the "adData" item,
    /// not the length of the entire AD data length in the final payload.
    void addAdvertisementData(const LBLEAdvDataItem &item);

    /// Convert advertisement data into to a continuous raw advertisement data payload.
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

    // a callback from BLE service to assign attribute handle
    // back to the attribute instance. the instance
    // may choose to ignore this information
    virtual void assignHandle(uint16_t attrHandle) = 0;

    // send notification to the given connection
    virtual int notify(bt_handle_t connection) = 0;

    // send indication and wait for ACK to the given connection
    virtual int indicate(bt_handle_t connection) = 0;

    // @param recordIndex ranges from 0 ~ (getRecordCount - 1)
    //
    // returns a generic bt_gatts_service_rec_t pointer
    // that points to variable-length GATT attribute records.
    // The user is reponsible to free() the returning buffer.
    virtual bt_gatts_service_rec_t* allocRecord(uint32_t recordIndex, uint16_t currentHandle) = 0;

    // Since we're defaulting all characteristics with notification,
    // we need to add CCCD(Client Characteristic Configuration Descriptor)
    // for every user-created characteristics.
    virtual void onWriteIndicationNotificationFlag(uint16_t flag) = 0;
    virtual uint16_t onReadIndicationNotificationFlag() = 0;
};

class LBLECharacteristicBase : public LBLEAttributeInterface
{
public:	// method for Arduino users

    LBLECharacteristicBase(LBLEUuid uuid, uint32_t permission);

    LBLECharacteristicBase(LBLEUuid uuid);

    // Check if a character is written
    bool isWritten();

public: // Following methods are not meant to be called by Arduino users

    // Each characteristic maps to at least 2 GATT attribute records:
    //  1. UUID of the characteristic
    //  2. Actual Value
    //  3 or more. (Optional) descriptors of the characteristic
    //
    // Since we're defaulting all characteristics with notification,
    // we need to add CCCD(Client Characteristic Configuration Descriptor)
    // for every user-created characteristics.
    virtual uint32_t getRecordCount() {
        return 3;
    };

    // common implementation for characteristics attributes
    virtual bt_gatts_service_rec_t* allocRecord(uint32_t recordIndex, uint16_t currentHandle);

    // store the assigned attribute handle for notification/indication
    virtual void assignHandle(uint16_t attrHandle);

    virtual void onWriteIndicationNotificationFlag(uint16_t flag);

    virtual uint16_t onReadIndicationNotificationFlag();

    // Utility function for child class to call.
    // send notification with `buf` as data to the given connection
    int _notify(bt_handle_t connection, const LBLEValueBuffer& data);

    // Utility function for child class to call.
    // send indication with `buf` as data and wait for ACK to the given connection
    int _indicate(bt_handle_t connection, const LBLEValueBuffer& data);

private:
    int _notifyIndicate(uint8_t opcode, bt_handle_t connection, const LBLEValueBuffer& data);

protected:
    LBLEUuid m_uuid;
    uint32_t m_perm;
    bool m_updated;
    uint16_t m_attrHandle;
    uint16_t m_cccdFlag;
};

// This class is used by LBLECharacteristicBuffer
struct LBLECharacteristicWrittenInfo
{
    uint16_t size;
    uint16_t offset;
};

/// \brief represents a typeless raw buffer GATT attribute.
///
/// This characteristic is a raw buffer initialized to zero.
/// The length of the buffer is limited to MAX_ATTRIBUTE_DATA_LEN
/// When isWritten() is true, you can use getLastWrittenInfo()
/// to check what part of the buffer is updated during the last write.
class LBLECharacteristicBuffer : public LBLECharacteristicBase
{
public:	// method for Arduino users

    LBLECharacteristicBuffer(LBLEUuid uuid, uint32_t permission);

    LBLECharacteristicBuffer(LBLEUuid uuid);

    /// Set value with raw buffer
    ///
    /// \param buffer pointer to raw buffer of value.
    ///               The input buffer is copied to the internal
    ///				  buffer of the LBLECharacteristicBuffer object.
    /// \param size must not exceed MAX_ATTRIBUTE_DATA_LEN.
    void setValueBuffer(const uint8_t* buffer, size_t size);

    /// Set value buffer size and initialize all content to 0
    void setValueBufferWithSize(size_t size);

    /// Get value buffer content.
    ///
    /// Retrieve buffer content of this LBLECharacteristicBuffer object.
    /// Note that (size + offset) must not exceed MAX_ATTRIBUTE_DATA_LEN;
    /// Note that isWritten() returns `false` after calling getValue().
    ///
    /// \param buffer pointer to the output buffer to be written
    /// \param size available size of the buffer to write to
    /// \param offset initial offset to the internal buffer of LBLECharacteristicBuffer.
    ///		   for example, if the internal buffer is {0, 1, 2, 3},
    ///		   ~~~{.cpp}
    ///		   getValue(buf, 4, 2); // offset 2 bytes
    ///		   ~~~
    ///		   will write `buf` with content {2, 3}.
    void getValue(uint8_t* buffer, uint16_t size, uint16_t offset);

    /// Check what part of the buffer is updated during last write operation
    const LBLECharacteristicWrittenInfo& getLastWrittenInfo() const;

public:	// for BLE framework
    virtual uint32_t onSize() const;
    virtual uint32_t onRead(void *data, uint16_t size, uint16_t offset);
    virtual uint32_t onWrite(void *data, uint16_t size, uint16_t offset);
    virtual int notify(bt_handle_t connection);
    virtual int indicate(bt_handle_t connection);

private:
    LBLEValueBuffer m_data;
    LBLECharacteristicWrittenInfo m_writtenInfo;
};

/// This characterstic is a peristent 4-byte integer initialized to zero.
/// The size is always 4 bytes.
class LBLECharacteristicInt : public LBLECharacteristicBase
{
public:	// method for Arduino users

    /// Create characteristic
    ///
    /// In most cases the characteristic should be created in global scope,
    /// to ensure it is alive after LBLEPeripheral.begin().
    ///
    /// \param uuid UUID for this characteristic
    /// \param permission read/write permission. (currently ignored).
    LBLECharacteristicInt(LBLEUuid uuid, uint32_t permission);

    LBLECharacteristicInt(LBLEUuid uuid);

    /// Set value of the characteristic.
    void setValue(int value);

    /// Retrieve value, note that isWritten() flag turns off after calling getValue()
    int getValue();

public:	// for BLE framework
    virtual uint32_t onSize() const;
    virtual uint32_t onRead(void *data, uint16_t size, uint16_t offset);
    virtual uint32_t onWrite(void *data, uint16_t size, uint16_t offset);
    virtual int notify(bt_handle_t connection);
    virtual int indicate(bt_handle_t connection);

private:
    int m_data;
};

/// \brief represents a GATT characterstic that is a NULL-terminated strings.
///
/// This is a "string" attribute. A NULL terminater
/// is always automatically inserted after each write operation.
///
/// That is, the string value is always "reset" after each write operation,
/// instead of appending/replacing part of the existing string value.
///
/// Example:
///  * the central device sends "YES" (3 bytes)
///  * the central device then sends "NO"(2 bytes)
///  * the resulting value is "NO\0" instead of "NOS\0".
///
/// The reason for this design is to make it more intuitive
/// to use with AppInventor's BluetoothLE.WriteStringValue block.
///
/// If you need behavior that supports replacing part of the buffer,
/// use LBLECharacteristicBuffer instead.
class LBLECharacteristicString : public LBLECharacteristicBase
{
public:	// method for Arduino users

    /// Create characteristic
    ///
    /// In most cases the characteristic should be created in global scope,
    /// to ensure it is alive after LBLEPeripheral.begin().
    ///
    /// \param uuid UUID for this characteristic
    /// \param permission read/write permission. (currently ignored).
    LBLECharacteristicString(LBLEUuid uuid, uint32_t permission);

    LBLECharacteristicString(LBLEUuid uuid);

    /// Set value of the characteristic.
    void setValue(const String& value);

    /// Retrieve value, note that isWritten() flag turns off after calling getValue()
    String getValue();

public:	// for BLE framework
    virtual uint32_t onSize() const;
    virtual uint32_t onRead(void *data, uint16_t size, uint16_t offset);
    virtual uint32_t onWrite(void *data, uint16_t size, uint16_t offset);
    virtual int notify(bt_handle_t connection);
    virtual int indicate(bt_handle_t connection);

private:
    String m_data;
};

/// Represents a BLE service.
///
/// This class represents a BLE service in a peripheral device.
///
/// This object must be alive across entire BLE framework life cycle.
/// Therefore, you need to allocate it in global scope, and
/// then add characteristics by calling addAttribute().
///
/// ~~~{.cpp}
/// LBLEService myService("UUID_XYZ");
/// LBLECharacteristicString myCharacteristic("UUID_YZWV");
/// void setup(){
/// 	myService.addAttribute(myCharacteristic);
/// }
/// ~~~
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

/// Singleton class representing the local BLE periphral device
class LBLEPeripheralClass : public LBLEEventObserver
{
public:
    /// Do not instantiate by yourself. Use singleton object `LBLEPeripheral` instead.
    LBLEPeripheralClass();

    virtual ~LBLEPeripheralClass();

    /// start advertisement as a connectable device
    ///
    /// \param advertisementData The advertisement data to be advertised.
    /// \returns  0 when succeeded, -1 when payload too long, -2 for other errors.
    int advertise(const LBLEAdvertisementData& advertisementData);

    /// start advertisement as an non-connectable device (such as an iBeacon)
    ///
    /// \param advertisementData The advertisement packet to broadcast.
    /// \param intervalMS The advertisement interval, in milliseconds.
    /// \param txPower This controls the actual advertisement power in dbm.
    ///        Note: current version does not support adjusting txPower.
    int advertiseAsBeacon(const LBLEAdvertisementData& advertisementData,
                          uint32_t intervalMS = 700,
                          int8_t txPower = -30);

    /// start advertisement based on previous input of advertise
    /// \returns 0 when succeeded, -1 when payload too long, -2 for other errors.
    int advertiseAgain();

    /// stop advertisement and clears advertisement data
    /// advertiseAgain() fails after stopAdvertise();
    void stopAdvertise();

    /// Generic Access Profile (GAP) configuration
    void setName(const char* name);

    /// After setup services and characteristics
    /// you have to call begin make enable the GATT server
    void begin();

    /// returns true if there is a central device connecting to this peripheral.
    bool connected();

    /// disconnect all connected centrals (if any)
    void disconnectAll();

    /// broadcasting notification of the given GATT characteristic value to
    /// all connected devices, e.g. LBLEPeripheral.notify(myCharacteristic)
    /// Note that `myChar` must be added to the LBLEPeriphral as
    /// part of a service before calling this method.
    int notifyAll(LBLEAttributeInterface& characteristic);

    /// broadcasting indication of the given GATT characteristic value to
    /// all connected devices, e.g. LBLEPeripheral.notify(myCharacteristic)
    /// Note that `myChar` must be added to the LBLEPeriphral as
    /// part of a service before calling this method.
    int indicateAll(LBLEAttributeInterface& characteristic);

    /// configuring GATT Services. You must configure services
    /// before advertising the device. The services cannot change
    /// after being connected.
    ///
    /// \param service The service to be added into this peripheral. The service
    ///				   object is referenced instead of copied, so tt must be valid and alive
    ///				   all the time.
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

    std::vector<bt_handle_t> m_connections;

};

extern LBLEPeripheralClass LBLEPeripheral;


#endif // #ifndef LBLEPERIPHERAL_H
