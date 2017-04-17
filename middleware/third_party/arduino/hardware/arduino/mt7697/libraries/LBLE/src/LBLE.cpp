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
	bt_addr_t btAddr = {0};
	bt_bd_addr_ptr_t randBDAddr = bt_gap_le_get_random_address();
	btAddr.type = BT_ADDR_RANDOM;
	memcpy(&btAddr.addr, randBDAddr, BT_BD_ADDR_LEN);
	return LBLEAddress(btAddr);
}

void LBLEClass::registerForEvent(bt_msg_type_t msg, LBLEEventObserver* pObserver)
{
	m_dispatcher.addObserver(msg, pObserver);
}

void LBLEClass::handleEvent(bt_msg_type_t msg, bt_status_t status, void *buff)
{
	m_dispatcher.dispatch(msg, status, buff);
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

unsigned char LBLEUuid::equals(const LBLEUuid &rhs) const
{
	return bt_uuid_equal(&uuid_data, &rhs.uuid_data);
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
	m_addr()
{
	memset(&m_addr, sizeof(m_addr), 0);
}

LBLEAddress::LBLEAddress(const bt_addr_t& btAddr)
{
	m_addr.type = btAddr.type;
	memcpy(m_addr.addr, btAddr.addr, BT_BD_ADDR_LEN);
}

LBLEAddress::~LBLEAddress()
{
	
}

size_t LBLEAddress::printTo(Print& p) const
{
	const String str = toString();
	p.print(str);

	return str.length();
}

String LBLEAddress::toString() const
{
	return convertBluetoothAddressToString(m_addr);
}

const char* LBLEAddress::getAddressTypeString(bt_addr_type_t addrType)
{
	switch(addrType)
	{
	case BT_ADDR_PUBLIC:
		return "PUB";
	case BT_ADDR_RANDOM:
		return "RAN";
	case BT_ADDR_PUBLIC_IDENTITY:
		return "PID";
	case BT_ADDR_RANDOM_IDENTITY:
		return "RID";
	default:
		return "---";
	}
}

String LBLEAddress::convertBluetoothAddressToString(const bt_addr_t& btAddr)
{
	// 6-byte MAC address in HEX with ":" as seperator, 
	// and 5-byte address type, e.g. "(PUB)",
	// plus NULL terminator
    char buf[BT_BD_ADDR_LEN * 2 + BT_BD_ADDR_LEN - 1 + 5 + 1] = {0};
    sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X(%s)",
                btAddr.addr[5],
                btAddr.addr[4],
                btAddr.addr[3],
                btAddr.addr[2],
                btAddr.addr[1],
                btAddr.addr[0],
				getAddressTypeString(btAddr.type)
				);

    return String(buf);
}

String LBLEAddress::convertDeviceAddressToString(const bt_bd_addr_ptr_t addr)
{
	if(NULL == addr)
	{
		return String();
	}

	// 6-byte MAC address in HEX with ":" as seperator, 
	// plus NULL terminator
    char addr_buf[BT_BD_ADDR_LEN * 2 + BT_BD_ADDR_LEN - 1 + 1] = {0};
    sprintf(addr_buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                addr[5],
                addr[4],
                addr[3],
                addr[2],
                addr[1],
                addr[0]);

    return String(addr_buf);
}

unsigned char LBLEAddress::equals(const LBLEAddress& rhs) const
{
	if(this == &rhs)
	{
		return 1;
	}

	return equal_bt_address(this->m_addr, rhs.m_addr);
}

// returns true if lhs equals rhs address.
bool LBLEAddress::equal_bt_address(const bt_addr_t& lhs, const bt_addr_t&rhs)
{
    return (lhs.type == rhs.type) && (0 == memcmp(lhs.addr, rhs.addr, BT_BD_ADDR_LEN));
}

LBLEAddress& LBLEAddress::operator = (const LBLEAddress &rhs)
{
	if(this == &rhs)
		return *this;

	m_addr.type = rhs.m_addr.type;
	memcpy(m_addr.addr, rhs.m_addr.addr, BT_BD_ADDR_LEN);
}


void ard_ble_postAllEvents(bt_msg_type_t msg, bt_status_t status, void *buff)
{
	// this is a filter hook, executed after all BT message handlers
	
	LBLE.handleEvent(msg, status, buff);
    return;
}

void LBLEEventDispatcher::addObserver(bt_msg_type_t msg, LBLEEventObserver* pObserver)
{
	m_table.insert(std::make_pair(msg, pObserver));
}

void LBLEEventDispatcher::dispatch(bt_msg_type_t msg, bt_status_t status, void *buff)
{
	EventTable::iterator i = m_table.find(msg);
	// execute observer's callback and pop the element found 
	while(i != m_table.end())
	{
		// process event
		i->second->onEvent(msg, status, buff);
		// check if we need to remove after processing the event
		// make a copy before we advance to next item
		auto j = i;
		++i;
		// 
		if(j->second->isOnce())
		{
			m_table.erase(j);
		}
	}
}

bool waitAndProcessEvent(std::function<void(void)> action, 
                        bt_msg_type_t msg, 
                        std::function<void(bt_msg_type_t, bt_status_t, void *)> handler)
{
    EventBlocker h(msg, handler);
    LBLE.registerForEvent(msg, &h);

    action();

    uint32_t start = millis();
    while(!h.done() && (millis() - start) < 10 * 1000)
    {
        delay(50);
    }

    return h.done();
}