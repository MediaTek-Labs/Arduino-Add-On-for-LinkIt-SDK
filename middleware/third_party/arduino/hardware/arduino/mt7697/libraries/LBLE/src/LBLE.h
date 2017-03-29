/*

*/

#ifndef LBLE_H
#define LBLE_H

#include <inttypes.h>
#include <WString.h>
#include <Printable.h>

extern "C" {
#include "utility/ard_ble.h"
}

// returns true if lhs equals rhs address.
bool compare_bt_address(const bt_addr_t& lhs, const bt_addr_t&rhs);

class LBLEUuid : public Printable
{
public: // Constructors
	LBLEUuid();
	LBLEUuid(const char* uuidString);
	LBLEUuid(uint16_t uuid16);
	LBLEUuid(uint32_t uuid32);
	LBLEUuid(const bt_uuid_t& uuid_data);
	LBLEUuid(const LBLEUuid& rhs);

	LBLEUuid & operator = (const bt_uuid_t &rhs);
	LBLEUuid & operator = (const LBLEUuid &rhs);
	LBLEUuid & operator = (const char* rhs);

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

		friend class LBLECentral;
		friend class LBLEPeripheral;
};

extern LBLEClass LBLE;

#endif
