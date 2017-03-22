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

LBLEAddress LBLEClass::getDeviceAddress()
{
	// the underlying framework passes an pointer
	// to global device address.
	return LBLEAddress(bt_gap_le_get_random_address());
}      

LBLEClass LBLE;

/////////////////////////////////////////////////////////////////////////////
// LBLEUuid helper class
/////////////////////////////////////////////////////////////////////////////
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

		// note that the string representation is
		// from HIGH to LOW - so we need to reverse here.
		data.uuid[15 - i] = val;
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

bool LBLEUuid::is16Bit() const
{
	return bt_uuid_is_uuid16(&uuid_data);
}

uint16_t LBLEUuid::getUuid16() const
{
	if(!is16Bit())
		return 0;

	return uuid_data.uuid16;
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
				"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
			    uuid_data.uuid[15],
			    uuid_data.uuid[14], 
			    uuid_data.uuid[13], 
			    uuid_data.uuid[12], 
			    uuid_data.uuid[11], 
			    uuid_data.uuid[10], 
			    uuid_data.uuid[9], 
			    uuid_data.uuid[8],
			    uuid_data.uuid[7], 
			    uuid_data.uuid[6], 
			    uuid_data.uuid[5], 
			    uuid_data.uuid[4], 
			    uuid_data.uuid[3], 
			    uuid_data.uuid[2], 
			    uuid_data.uuid[1], 
			    uuid_data.uuid[0]
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


/////////////////////////////////////////////////////////////////////////////
// LBLEAddress helper class
/////////////////////////////////////////////////////////////////////////////
LBLEAddress::LBLEAddress():
	m_addr(NULL)
{

}

LBLEAddress::LBLEAddress(bt_bd_addr_ptr_t addr)
{
	m_addr = addr;
}

LBLEAddress::~LBLEAddress()
{
	m_addr = NULL;
}

size_t LBLEAddress::printTo(Print& p) const
{
	const String str = toString();
	p.print(str);

	return str.length();
}

String LBLEAddress::toString() const
{
	return convertAddressToString(m_addr);
}

String LBLEAddress::convertAddressToString(const bt_bd_addr_ptr_t addr)
{
	if(NULL == addr)
	{
		return String();
	}

	// 6-byte MAC address in HEX with ":" as seperator, plus NULL terminator
    char addr_buf[sizeof(bt_bd_addr_t) * 2 + sizeof(bt_bd_addr_t) - 1 + 1] = {0};
    sprintf(addr_buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                addr[5],
                addr[4],
                addr[3],
                addr[2],
                addr[1],
                addr[0]);

    return String(addr_buf);
}

// returns true if lhs equals rhs address.
bool compare_bt_address(const bt_addr_t& lhs, const bt_addr_t&rhs)
{
    return (lhs.type == rhs.type) && (0 == memcmp(lhs.addr, rhs.addr, sizeof(lhs.addr)));
}


void ard_ble_postAllEvents(bt_msg_type_t msg, bt_status_t status, void *buff)
{
	// this is a filter hook, executed after all BT message handlers
#if 1
	pr_debug("ard_ble_postAllEvents: %04x : %04x : %08x", msg, status, buff);
#endif
    return;
}
