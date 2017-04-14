/*

*/

#ifndef LBLE_H
#define LBLE_H

#include <inttypes.h>
#include <WString.h>
#include <Printable.h>
#include <map>
#include <functional>

extern "C" {
#include "utility/ard_ble.h"
}

// returns true if lhs equals rhs address.
bool equal_bt_address(const bt_addr_t& lhs, const bt_addr_t&rhs);

class LBLEUuid : public Printable
{
public: // Constructors
	LBLEUuid();
	LBLEUuid(const char* uuidString);
	LBLEUuid(uint16_t uuid16);
	LBLEUuid(uint32_t uuid32);
	LBLEUuid(const bt_uuid_t& uuid_data);
	LBLEUuid(const LBLEUuid& rhs);

public:
	LBLEUuid & operator = (const bt_uuid_t &rhs);
	LBLEUuid & operator = (const LBLEUuid &rhs);
	LBLEUuid & operator = (const char* rhs);

	unsigned char equals(const LBLEUuid &rhs) const;
	unsigned char operator == (const LBLEUuid &rhs) const {return equals(rhs);}


public:	// implementing Pritable
	String toString() const;
	virtual size_t printTo(Print& p) const;

public:	// Helper function
	void toRawBuffer(uint8_t* uuidBuf, uint32_t bufLength) const;
	bool isEmpty() const;
	bool is16Bit() const;
	uint16_t getUuid16() const;

public:
	bt_uuid_t uuid_data;
};

class LBLEAddress : public Printable
{
public: // Constructors
	LBLEAddress();
	LBLEAddress(bt_bd_addr_ptr_t addr);
	~LBLEAddress();

public:	// implementing Pritable
	String toString() const;
	virtual size_t printTo(Print& p) const;

public:	// Helper function
	static String convertAddressToString(const bt_bd_addr_ptr_t addr);

public:
	bt_bd_addr_ptr_t m_addr;
};

class LBLEEventObserver
{
public:
	// true: LBLEEventDispatcher should unregister this event after process it
	// false: LBLEEventDispatcher should keep this observer
	virtual bool isOnce() { return true; };
	virtual void onEvent(bt_msg_type_t msg, bt_status_t status, void *buff) = 0;
};

class LBLEEventDispatcher
{
public:
	// after dispatch, the observer is removed from registration table.
	void dispatch(bt_msg_type_t msg, bt_status_t status, void *buff);

	// insert an observer to the registration table.
	void addObserver(bt_msg_type_t msg, LBLEEventObserver* pObserver);

public:
	typedef std::multimap<bt_msg_type_t, LBLEEventObserver*> EventTable;
	EventTable m_table;

};

class EventBlocker : public LBLEEventObserver
{
public:
    const bt_msg_type_t m_event;
    bool m_eventArrived;
    std::function<void(bt_msg_type_t , bt_status_t , void *)> m_handler;

    EventBlocker(bt_msg_type_t e, std::function<void(bt_msg_type_t , bt_status_t , void *)>& handler):
        m_event(e),
        m_eventArrived(false),
        m_handler(handler)
    {

    }

    bool done() const
    {
        return m_eventArrived;
    }

    virtual void onEvent(bt_msg_type_t msg, bt_status_t status, void* buff)
    {
        if(m_event == msg)
        {
            m_eventArrived = true;
            m_handler(msg, status, buff);
        }
    }
};

// This helper function do ACTION and wait for MSG, if it arrives HANDLER is called.
bool waitAndProcessEvent(std::function<void(void)> action, 
                        bt_msg_type_t msg, 
                        std::function<void(bt_msg_type_t, bt_status_t, void *)> handler);

class LBLEClass
{
private:

public:
	LBLEClass();

	/* Initializes the Bluetooth subsystem.
	 * This should be the called first prior to using other BLE APIs.
	 * After calling begin() you need to call ready(),
	 * and check if the subsystem is ready to use.
 	 */
	int begin();

	/* Returns 0 when BLE subsystem is not ready to use.
	 * Returns 1 when it is ready to use.
	 */
	int ready();

	LBLEAddress getDeviceAddress();

	void registerForEvent(bt_msg_type_t msg, LBLEEventObserver* pObserver);
	void handleEvent(bt_msg_type_t msg, bt_status_t status, void *buff);

	friend class LBLECentral;
	friend class LBLEPeripheral;

public:
	LBLEEventDispatcher m_dispatcher;
};

extern LBLEClass LBLE;

#endif
