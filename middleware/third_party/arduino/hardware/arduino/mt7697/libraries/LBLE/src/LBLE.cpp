
#include <stdio.h>
#include <Print.h>
#include <vector>
#include "LBLE.h"


extern "C" {
#include "utility/ard_ble.h"
}

///////////////////////////////////////////////
//  Helper "auto lock" class. To be used in function scope.
//////////////////////////////////////////////
class LBLEAutoLock
{
public:
    LBLEAutoLock(SemaphoreHandle_t sem):
        m_sem(sem)
    {
        // keep waiting for the lock
        while(pdFALSE == xSemaphoreTakeRecursive(m_sem, 10))
        {
            pr_debug("spin wait sem=0x%x", (int)m_sem);
        }
    }

    ~LBLEAutoLock()
    {
        xSemaphoreGiveRecursive(m_sem);
    }
private:
    SemaphoreHandle_t m_sem;

    // forbid copy
    LBLEAutoLock(const LBLEAutoLock& rhs) {};
};

///////////////////////////////////////////////
//  LBLE Class implmentations
//////////////////////////////////////////////

LBLEClass::LBLEClass():
    m_dispatcherSemaphore(NULL)
{
}

int LBLEClass::begin()
{
    m_dispatcherSemaphore = xSemaphoreCreateRecursiveMutex();
    if(NULL == m_dispatcherSemaphore)
    {
        pr_debug("failed to create semaphore for dispatcher!");
        return 0;
    }
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
    bt_bd_addr_ptr_t randBDAddr = bt_gap_le_get_random_address();
    bt_addr_t btAddr;
    btAddr.type = BT_ADDR_RANDOM;
    memcpy(&btAddr.addr, randBDAddr, BT_BD_ADDR_LEN);
    return LBLEAddress(btAddr);
}

void LBLEClass::registerForEvent(bt_msg_type_t msg, LBLEEventObserver* pObserver)
{
    LBLEAutoLock lock(m_dispatcherSemaphore);
    if( (((int)pObserver) & 0xF0000000) == 0)
    {
        pr_debug("abnormal observer added: msg:0x%x, pobserver:0x%x", (int)msg, (int)pObserver);
    }

#if 1
    pr_debug("observer added: msg:0x%x, pobserver:0x%x", (int)msg, (int)pObserver);
#endif

    m_dispatcher.addObserver(msg, pObserver);
}

void LBLEClass::unregisterForEvent(bt_msg_type_t msg, LBLEEventObserver* pObserver)
{
    LBLEAutoLock lock(m_dispatcherSemaphore);
    m_dispatcher.removeObserver(msg, pObserver);
#if 1
    pr_debug("observer removed: msg:0x%x, pobserver:0x%x", (int)msg, (int)pObserver);
#endif
}

void LBLEClass::handleEvent(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    LBLEAutoLock lock(m_dispatcherSemaphore);
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

bool LBLEUuid::operator<(const LBLEUuid &rhs) const
{
    const bool rhs16 = rhs.is16Bit();
    const bool rhs32 = bt_uuid_is_uuid32(&rhs.uuid_data);

    if(is16Bit())
    {
        if(rhs16) {
            return (uuid_data.uuid16 < rhs.uuid_data.uuid16);
        }

        // 16-bit is always smaller than 32/128
        return true;
    }
    else if(bt_uuid_is_uuid32(&uuid_data))
    {
        if(rhs32) {
            return (uuid_data.uuid32 < rhs.uuid_data.uuid32);
        }

        // 32-bit is larger than 16-bit
        if(rhs16) {
            return false;
        }

        // 128-bit case
        return true;
    }
    else
    {
        if(rhs16) {
            return false;
        }

        if(rhs32) {
            return false;
        }

        // 128-bit: compare from MSB to LSB
        for(int i = 0; i < 16; ++i)
        {
            const uint8_t byteLhs = uuid_data.uuid[15 - i];
            const uint8_t byteRhs = rhs.uuid_data.uuid[15 - i];
            if(byteLhs != byteRhs)
            {
                return (byteLhs < byteRhs);
            }
        }
    }

    return false;
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
                "0x%08lx",
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
    memset(&m_addr, 0, sizeof(m_addr));
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

    return *this;
}


void ard_ble_postAllEvents(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    // this is a filter hook, executed after all BT message handlers

    LBLE.handleEvent(msg, status, buff);
    return;
}

void LBLEEventDispatcher::addObserver(bt_msg_type_t msg, LBLEEventObserver* pObserver)
{
    // pr_debug("add observer: i->second:%p", (int)msg, pObserver);
    m_table.insert(std::make_pair(msg, pObserver));
}

void LBLEEventDispatcher::removeObserver(bt_msg_type_t msg, LBLEEventObserver* pObserver)
{
    // we cannot remove the key (msg),
    // since there may be other observers
    // registered to the same key.
    // So we loop over the matching elements
    // and check the handler pointer.
    auto keyRange = m_table.equal_range(msg);
    auto i = keyRange.first;
    while(i != keyRange.second)
    {
        // advance iterator first before we remove some element.
        auto toRemove = i++;

        // if match, remove the element
        if(toRemove->second == pObserver)
        {
            m_table.erase(toRemove);
            return;
        }
    }
}

void LBLEEventDispatcher::dispatch(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    // pr_debug("dispatch: msg:0x%x", msg);
    auto keyRange = m_table.equal_range(msg);
    auto i = keyRange.first;

    std::vector<EventTable::iterator> removeList;
    // execute observer's callback and pop the element found
    while(i != keyRange.second)
    {
        if(i->first != msg)
        {
            break;
        }

        if(i->second)
        {
#if 0
            {
                pr_debug("dispatch found: msg:0x%x, i->second:%p", (int)msg, i->second);
            }
#endif
            // check if we need to remove after processing the event
            // make a copy before we advance to next item
            if(i->second->isOnce())
            {
                removeList.push_back(i);
            }

            // process event
            i->second->onEvent(msg, status, buff);
        }
        else
        {
            pr_debug("dangling observer on event 0x%x", msg);
        }

        // advance to next registered observer.
        ++i;
    }

    // now we clear any "once" observers, if any.
    for(auto&& r : removeList)
    {
        m_table.erase(r);
    }
}

//////////////////////////////////////////////////////////////////////////
//  LBLEValueBuffer
//////////////////////////////////////////////////////////////////////////
LBLEValueBuffer::LBLEValueBuffer()
{
}

LBLEValueBuffer::LBLEValueBuffer(int intValue)
{
    shallowInit(intValue);
}

LBLEValueBuffer::LBLEValueBuffer(float floatValue)
{
    shallowInit(floatValue);
}

LBLEValueBuffer::LBLEValueBuffer(char charValue)
{
    shallowInit(charValue);
}

LBLEValueBuffer::LBLEValueBuffer(const String& strValue)
{
    resize(strValue.length() + 1);
    strValue.getBytes(&(*this)[0], size());
}

// interprets buffer content as null-terminated character string.
// Empty string is returned when there are errors.
String LBLEValueBuffer::asString() const
{
    if(!this->size())
    {
        return String();
    }
    
    // Make sure we have terminating NULL before passing to String().
    if(this->back() == '\0')
    {
        return String((const char*)&this->front());
    }
    else
    {
        // since String() does not allow us to initialize
        // with buffer + length, we use a temporary buffer object instead.
        std::vector<char> tempBuffer;
        tempBuffer.resize(this->size() + 1, 0);
        if(tempBuffer.size() >= this->size())
        {
            memcpy(&tempBuffer.front(), &this->front(), this->size());
            return String((const char*)&tempBuffer.front());
        }
    } 

    return String();
}

int LBLEValueBuffer::asInt() const
{
    int ret = 0;
    if(this->size() < sizeof(ret))
    {
        return 0;
    }

    ret = *((const int*)&this->at(0));

    return ret;
}

char LBLEValueBuffer::asChar() const
{
    char ret = '\0';
    if(this->size() < sizeof(ret))
    {
        return 0;
    }

    ret = *((const char*)&this->front());

    return ret;
}

float LBLEValueBuffer::asFloat() const
{
    float ret = 0.f;
    if(this->size() < sizeof(ret))
    {
        return 0;
    }

    ret = *((const float*)&this->front());

    return ret;
}