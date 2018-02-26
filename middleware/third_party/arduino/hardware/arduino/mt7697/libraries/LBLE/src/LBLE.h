/*

*/

#ifndef LBLE_H
#define LBLE_H

#include <inttypes.h>
#include <WString.h>
#include <Printable.h>
#include <delay.h>
#include <vector>
#include <map>

extern "C" {
    /* FreeRTOS headers */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "utility/ard_ble.h"
}


///	\brief Represents a 128-bit or 16-bit BT UUID.
///
/// This class represents BT UUIDs that are used
/// to identifie characteristics and other BLE GATT
/// attributes.
///
/// Many GATT Service and characteristic APIs relies on this
/// class to identify different attributes.
///
/// This class supports the Printable interface and therefore
/// can be passed to `Serial.print`, e.g.
///
/// ~~~{.cpp}
/// // print out address found during LBLECentral.scan().
/// Serial.print(LBLECentral.getAddress(i));
/// ~~~
///
/// \see LBLECentral, LBLEPeripheral
/// \see LBLECharacteristicInt, LBLECharacteristicString
class LBLEUuid : public Printable
{
public: // Constructors

    /// \brief Creates an empty UUID.
    LBLEUuid();

    /// \brief Creates 128-bit UUID
    ///
    /// Creates UUID from 128-bit string representation, e.g.
    /// ~~~{.cpp}
    /// LBLEUuid uuid("E2C56DB5-DFFB-48D2-B060-D0F5A71096E0");
    /// ~~~
    LBLEUuid(const char* uuidString);

    /// \brief Creates 16-bit Bluetooth assigned UUID.
    ///
    /// Creates an assigned 16-bit BT UUID from an unsigned short.
    /// In the example below, we create an UUID for "Device Information" service.
    /// See https://www.bluetooth.com/specifications/gatt/services
    /// for a list of assigned numbers for services.
    /// ~~~{.cpp}
    /// LBLEUuid uuid(0x180A);	// create UUID for Device Information service.
    /// ~~~
    LBLEUuid(uint16_t uuid16);

    /// Initialize from MTK BLE framework's bt_uuid_t struct.
    LBLEUuid(const bt_uuid_t& uuid_data);

    /// Copy constructor
    LBLEUuid(const LBLEUuid& rhs);

public:
    LBLEUuid & operator = (const bt_uuid_t &rhs);
    LBLEUuid & operator = (const LBLEUuid &rhs);
    LBLEUuid & operator = (const char* rhs);

    unsigned char equals(const LBLEUuid &rhs) const;
    unsigned char operator == (const LBLEUuid &rhs) const {
        return equals(rhs);
    }
    bool operator<(const LBLEUuid &rhs) const;


public:

    /// Returns String representation format for UI.
    String toString() const;

    /// Implements Pritable interface.
    virtual size_t printTo(Print& p) const;

public:	// Helper function

    /// Convert LBLEUuid object to raw 128-bit buffer
    void toRawBuffer(uint8_t* uuidBuf, uint32_t bufLength) const;

    /// \brief Check if the UUID is empty (just created).
    ///
    /// \returns true if the UUID is empty
    /// \returns false if the UUID is not empty
    bool isEmpty() const;

    /// \brief Check if this UUID is a 16-bit UUID.
    ///
    /// Check if the UUID is an 16-bit UUID, which is assigned by BT SIG, or
    /// a generic, developer-generated 128-bit UUID.
    ///
    /// \returns true if the UUID is 16-bit assigned number.
    ///			 Note that this method does not check
    ///			 if the 16-bit is actually a BT SIG assigned
    ///			 number; it simply check if it is 16-bit long.
    /// \returns false if the UUID is 128-bit UUID.
    bool is16Bit() const;

    /// \brief Get the unsigned short representation of a 16-bit assigned UUID.
    ///
    /// \returns unsigned short value of the 16-bit assigned UUID.
    /// \returns 0 if the LBLEUuid is not a 16-bit UUID.
    uint16_t getUuid16() const;

public:
    bt_uuid_t uuid_data;
};

///	\brief Represents the device address of a BLE device.
///
/// This class represents Bluetooth device addresses.
/// The device address are used to identify and to
/// connect to different BLE devices. Note that
/// not all BLE devices can be connected.
///
/// This class supports the Printable interface and therefore
/// can be passed to `Serial.print`, e.g.
///
/// ~~~{.cpp}
/// // print out address found during LBLECentral.scan().
/// Serial.print(LBLECentral.getAddress(i));
/// ~~~
///
/// \see LBLEClient::connect, LBLECentralClass::getBLEAddress
class LBLEAddress : public Printable
{
public: // Constructors

    /// Default constructor creates an invalid address.
    LBLEAddress();

    /// Initialize from LinkIt SDK's bt_addr_t struct
    LBLEAddress(const bt_addr_t& btAddr);

    ~LBLEAddress();

public:	// implementing Pritable
    String toString() const;

    /// Implements Printable interface
    virtual size_t printTo(Print& p) const;

    // returns true if lhs equals rhs address.
    static bool equal_bt_address(const bt_addr_t& lhs, const bt_addr_t&rhs);
    unsigned char equals(const LBLEAddress &rhs) const;
    unsigned char operator == (const LBLEAddress &rhs) const {
        return equals(rhs);
    }
    LBLEAddress& operator = (const LBLEAddress &rhs);


public:	// Helper function
    static String convertDeviceAddressToString(const bt_bd_addr_ptr_t addr);
    static String convertBluetoothAddressToString(const bt_addr_t& addr);
    static const char* getAddressTypeString(bt_addr_type_t addrType);

public:
    bt_addr_t m_addr;
};

/// This class encapsulates raw buffer operations used by LBLEClient/LBLEPeripheral
///
/// When writing or reading GATT attributes, we need to convert
/// to raw buffers and meaningful data types used by users.
///
/// This class helps users to convert to these raw buffer
/// values when reading or writing GATT attributes.
class LBLEValueBuffer : public std::vector<uint8_t>
{
// constructors and initialization
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

// type conversion helpers
public:
    /// interprets buffer content as int32_t
    /// 0 is returned when there are errors.
    int asInt() const;

    /// interprets buffer content as null-terminated character string.
    /// Empty string is returned when there are errors.
    String asString() const;

    /// interprets buffer content as char
    /// 0 is returned when there are errors.
    char asChar() const;

    /// interprets buffer content as float
    /// 0.f is returned when there are errors.
    float asFloat() const;

};

template<typename T>void LBLEValueBuffer::shallowInit(T value)
{
    this->resize(sizeof(value), 0);
    memcpy(&(*this)[0], &value, sizeof(value));
}

// Interface of an event observer
class LBLEEventObserver
{
public:
    virtual ~LBLEEventObserver() {};

    // \returns true: LBLEEventDispatcher should unregister this event after process it
    // \returns false: LBLEEventDispatcher should keep this observer
    virtual bool isOnce() {
        return true;
    };

    // callback function for events
    virtual void onEvent(bt_msg_type_t msg, bt_status_t status, void *buff) = 0;
};

// Registration table for BT events and corresponding observers.
class LBLEEventDispatcher
{
public:
    // after dispatch, the observer is removed from registration table.
    void dispatch(bt_msg_type_t msg, bt_status_t status, void *buff);

    // insert an observer to the registration table.
    void addObserver(bt_msg_type_t msg, LBLEEventObserver* pObserver);

    // remove an observer from the registration table.
    void removeObserver(bt_msg_type_t msg, LBLEEventObserver* pObserver);

public:
    typedef std::multimap<bt_msg_type_t, LBLEEventObserver*> EventTable;
    EventTable m_table;

};

/**
	\brief LBLEClass is the class for the singleton `LBLE`.

	Do not instantiate this class by yourself.
	Use LBLE instead of instantiating the LBLEClass by yourself.
	For example, call
	~~~{.cpp}
	#include <LBLE.h>

	void setup(){
		LBLE.begin();
		while(!LBLE.ready()){delay(10);}
	}
	~~~
 */
class LBLEClass
{
private:

public:
    /** Constructors for LBLEClass. Do not instantite this class by yourself.
     *
     * This class is intended to be used as a singleton. Use the global
     * `LBLE` object, instead of instantiate this class by yourself.
     */
    LBLEClass();

    /** Initializes the Bluetooth subsystem.
     *
     * This method should be the called first prior to using other BLE APIs.
     * After calling begin() you need to call ready(),
     * and check if the subsystem is ready to use.
     */
    int begin();

    /** Check if the BLE subsystem is now ready to use.
     *
     * \returns 0 when BLE subsystem is not ready to use.
     * \returns 1 when BLE subsystem is ready to use.
     */
    int ready();

    /** Get the device address of LinkIt 7697 device.
     *
     * \returns an LBLEAddress object representing the device address.
     */
    LBLEAddress getDeviceAddress();

    void registerForEvent(bt_msg_type_t msg, LBLEEventObserver* pObserver);
    void unregisterForEvent(bt_msg_type_t msg, LBLEEventObserver* pObserver);
    void handleEvent(bt_msg_type_t msg, bt_status_t status, void *buff);

    friend class LBLECentral;
    friend class LBLEPeripheral;

protected:
    LBLEEventDispatcher m_dispatcher;
    SemaphoreHandle_t m_dispatcherSemaphore;

};

/**
	\addgroup LBLE is the singleton instance for the BLE subsystem

	Use LBLE instead of instantiating the LBLEClass by yourself.
	For example, call
	~~~{.cpp}
	#include <LBLE.h>

	void setup(){
		LBLE.begin();
		while(!LBLE.ready()){delay(10);}
	}
	~~~
 */
extern LBLEClass LBLE;

// A helper class that taks a bt_msg_type_t event and a templated function
// object (callback function or lambda) and registers them.
// The function object is called when the event arrives.
template<typename F> class EventBlocker : public LBLEEventObserver
{
public:
    EventBlocker(bt_msg_type_t e, const F& handler):
        m_handler(handler),
        m_event(e),
        m_eventArrived(false)
    {

    }

    bool done() const
    {
        return m_eventArrived;
    }

    virtual bool isOnce()
    {
        return true;
    };

    virtual void onEvent(bt_msg_type_t msg, bt_status_t status, void* buff)
    {
        if(m_event == msg)
        {
            m_handler(msg, status, buff);
            m_eventArrived = true;
        }
    }

private:
    const F& m_handler;
    const bt_msg_type_t m_event;
    bool m_eventArrived;
};

// This helper function turns async BT APP event calls into a blocking call.
// When calling this function, it:
// 1. Call `action` function
// 2. Your context will enter a `delay()` loop with a 10-second timeout.
// 3. In BT task, it wait for `msg`, if it arrives, `handler` is called in BT task.
// 4. Upon event arrival or timeout, your context exits the loop and continues.
template<typename A, typename F> bool waitAndProcessEvent(const A& action, bt_msg_type_t msg, const F& handler)
{
    EventBlocker<F> h(msg, handler);
    LBLE.registerForEvent(msg, &h);

    action();

    uint32_t start = millis();
    while(!h.done() && (millis() - start) < 10 * 1000)
    {
        delay(50);
    }

    LBLE.unregisterForEvent(msg, &h);

    return h.done();
}


// Modified version of EventBlockerMultiple. 
// This EventBlockerMultiple relies on the return value of `handler`
// to determine `done()`.
template<typename F> class EventBlockerMultiple : public LBLEEventObserver
{
public:
    EventBlockerMultiple(bt_msg_type_t e, const F& handler):
        m_handler(handler),
        m_event(e),
        m_eventArrived(false),
        m_allEventProcessed(false)
    {

    }

    bool done() const
    {
        return m_eventArrived && m_allEventProcessed;
    }

    virtual bool isOnce()
    {
        // the client must remove EventBlocker
        // manually from the listeners.
        return false;
    };

    virtual void onEvent(bt_msg_type_t msg, bt_status_t status, void* buff)
    {
        if(m_event == msg)
        {
            m_eventArrived = true;

            // Note that some action, may result in multiple events.
            // For example, bt_gattc_discover_charc may invoke 
            // multiple BT_GATTC_DISCOVER_CHARC events.
            //
            // We need to rely on the message handlers
            // to tell us if all events have been processed or not.
            //
            // all event processed = `m_handler` returns `true`.
            // event not processed or waiting for more event = `m_handler` returns `false`.
            m_allEventProcessed = m_handler(msg, status, buff);
        }
    }

private:
    const F& m_handler;
    const bt_msg_type_t m_event;
    bool m_eventArrived;
    bool m_allEventProcessed;
};

// Modified version of waitAndProcessEvent.
// it uses EventBlockerMultiple instead of EventBlocker.
template<typename A, typename F> bool waitAndProcessEventMultiple(const A& action, bt_msg_type_t msg, const F& handler)
{
    EventBlockerMultiple<F> h(msg, handler);
    LBLE.registerForEvent(msg, &h);

    action();

    uint32_t start = millis();
    while(!h.done() && (millis() - start) < 10 * 1000)
    {
        delay(50);
    }

    LBLE.unregisterForEvent(msg, &h);

    return h.done();
}
#endif
