#include "LBLE.h"
#include <stdio.h>
#include <Print.h>

extern "C" {
#include "utility/ard_ble.h"
}

LBLEClass::LBLEClass()
{
}

int LBLEClass::begin()
{
	return ard_ble_begin();
}

int LBLEClass::ready()
{
	return ard_ble_is_ready();
}

LBLEClass LBLE;

LBLEUuid::LBLEUuid()
{
	memset(&uuid_data, 0, sizeof(uuid_data));
}

LBLEUuid::LBLEUuid(const char* uuidString)
{
	bt_uuid_load(&uuid_data, uuidString, strlen(uuidString));
}

LBLEUuid::LBLEUuid(uint16_t uuid16)
{
	bt_uuid_from_uuid16(&uuid_data, uuid16);
}

LBLEUuid::LBLEUuid(uint32_t uuid32)
{
	bt_uuid_from_uuid16(&uuid_data, uuid32);
}

LBLEUuid::LBLEUuid(const bt_uuid_t& rhs):
	uuid_data(rhs)
{

}

LBLEUuid::LBLEUuid(const LBLEUuid& rhs):
	uuid_data(rhs.uuid_data)
{

}

LBLEUuid & LBLEUuid::operator = (const bt_uuid_t &rhs)
{
	uuid_data = rhs;

	return *this;
}

LBLEUuid & LBLEUuid::operator = (const LBLEUuid &rhs)
{
	if(this == &rhs) return *this;

	uuid_data = rhs.uuid_data;

	return *this;
}

LBLEUuid & LBLEUuid::operator = (const char* rhs)
{
	bt_uuid_load(&uuid_data, rhs, strlen(rhs));
	return *this;
}

bool LBLEUuid::isEmpty() const
{
	const static bt_uuid_t zero_data = {0};
	return (0 == memcmp(&uuid_data, &zero_data, sizeof(bt_uuid_t)));
}

String LBLEUuid::toString() const
{
	char str[37] = {0};

	if(bt_uuid_is_uuid16(&uuid_data))
	{
		sprintf(str, 
			"0x%04x", 
		    uuid_data.uuid16
		);
	}
	else if(bt_uuid_is_uuid32(&uuid_data))
	{
		sprintf(str, 
			"0x%08x", 
		    uuid_data.uuid32
		);
	}
	else
	{
		sprintf(str, 
				"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
			    uuid_data.uuid[0],
			    uuid_data.uuid[1], 
			    uuid_data.uuid[2], 
			    uuid_data.uuid[3], 
			    uuid_data.uuid[4], 
			    uuid_data.uuid[5], 
			    uuid_data.uuid[6], 
			    uuid_data.uuid[7],
			    uuid_data.uuid[8], 
			    uuid_data.uuid[9], 
			    uuid_data.uuid[10], 
			    uuid_data.uuid[11], 
			    uuid_data.uuid[12], 
			    uuid_data.uuid[13], 
			    uuid_data.uuid[14], 
			    uuid_data.uuid[15]
		);
	}

	return String(str);
}

size_t LBLEUuid::printTo(Print& p) const
{
	String str = toString();
	p.print(str);

	return str.length();
}