#include "LBLE.h"
#include <Arduino.h>
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

uint8_t ascii_to_uint8(char c)
{
	if('0' <= c && c <= '9')
		return c - '0';

	if('A' <= c && c <= 'F')
		return 10 + (c - 'A');

	if('a' <= c && c <= 'f')
		return 10 + (c - 'a');

	return 0;
}

void str_to_uuid(bt_uuid_t &data, const char* uuidStr)
{
	// https://www.ietf.org/rfc/rfc4122.txt
	const uint len = strlen(uuidStr);
	if(len != 36)
	{
		return;
	}

	for(int i = 0; i < 16; ++i)
	{
		uint8_t val = 0;

		if('-' == *uuidStr)
			++uuidStr;

		val = ascii_to_uint8(*uuidStr);
		val = (val << 4);
		++uuidStr;

		val += ascii_to_uint8(*uuidStr);
		++uuidStr;

		data.uuid[i] = val;
	}
}

LBLEUuid::LBLEUuid(const char* uuidString)
{
	str_to_uuid(uuid_data, uuidString);
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
	str_to_uuid(uuid_data, rhs);
	return *this;
}

bool LBLEUuid::isEmpty() const
{
	const static bt_uuid_t zero_data = {0};
	return (0 == memcmp(&uuid_data, &zero_data, sizeof(bt_uuid_t)));
}

void LBLEUuid::toRawBuffer(uint8_t* uuidBuf, uint32_t bufLength) const
{
	// input check
	if(NULL == uuidBuf || 16 > bufLength)
	{
		return;
	}

	// full 16 bytes of 128-bit uuid.
	memcpy(uuidBuf, uuid_data.uuid, 16);
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


void ard_ble_postAllEvents(bt_msg_type_t msg, bt_status_t status, void *buff)
{
#if 0
	Serial.print("ard_ble_postAllEvents:");
	Serial.print(msg, HEX);
	Serial.print(":");
	Serial.println(status, HEX);

    switch(msg)
    {
    case BT_POWER_ON_CNF:
        Serial.print("[BT_POWER_ON_CNF]=");
        Serial.println(status, HEX);
        break;

    case BT_GAP_LE_SET_RANDOM_ADDRESS_CNF: 
        Serial.print("[BT_GAP_LE_SET_RANDOM_ADDRESS_CNF]=");
        Serial.println(status, HEX);
        break;

    case BT_GAP_LE_SET_ADVERTISING_CNF:
        Serial.print("[BT_GAP_LE_SET_ADVERTISING_CNF]=");
        Serial.println(status, HEX);
        break;

    case BT_GAP_LE_DISCONNECT_IND:
        Serial.print("[BT_GAP_LE_DISCONNECT_IND]=");
        Serial.println(status, HEX);
        break;

    case BT_GAP_LE_CONNECT_IND:
        Serial.print("[BT_GAP_LE_CONNECT_IND]=");
        Serial.println(status, HEX);
        break;
    }
#endif
    return;
}