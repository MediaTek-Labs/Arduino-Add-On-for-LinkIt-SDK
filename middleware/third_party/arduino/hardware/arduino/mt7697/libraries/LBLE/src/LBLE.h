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

class LBLEUuid : public Printable
{
public:
	LBLEUuid();
	LBLEUuid(const char* uuidString);
	LBLEUuid(uint16_t uuid16);
	LBLEUuid(uint32_t uuid32);
	LBLEUuid(const bt_uuid_t& uuid_data);
	LBLEUuid(const LBLEUuid& rhs);

	bool isEmpty() const;

	bool is16Bit() const;
	uint16_t getUuid16() const;

	LBLEUuid & operator = (const bt_uuid_t &rhs);
	LBLEUuid & operator = (const LBLEUuid &rhs);
	LBLEUuid & operator = (const char* rhs);

	void toRawBuffer(uint8_t* uuidBuf, uint32_t bufLength) const;

	String toString() const;

	virtual size_t printTo(Print& p) const;

private:
	bt_uuid_t uuid_data;
};

class LBLEAddress
{

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


		friend class LBLECentral;
		friend class LBLEPeripheral;
};

extern LBLEClass LBLE;

#endif
