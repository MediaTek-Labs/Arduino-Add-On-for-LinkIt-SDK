#include "MCS.h"
#include "MCS_debug.h"

/* ----------------------------------------------------------------------------
Utility function
---------------------------------------------------------------------------- */

static int _findHeader(const String& response, const String& header)
{
    int c = response.indexOf(header);
    if(c >= 0)
    {
        int e = response.indexOf("\r\n", c);
        return response.substring(c+header.length(), e).toInt();
    }
    return -1;
}

/* ----------------------------------------------------------------------------
class MCSDevice
---------------------------------------------------------------------------- */

MCSDevice::MCSDevice(const String& device_id, const String& device_key):
mServer("api.mediatek.com"),
mPort(80),
mDefTimeout(30*1000),
mId(device_id),
mKey(device_key),
mLastHB(0)
{

}

MCSDevice::MCSDevice(const String& device_id, const String& device_key, const String& server, int port):
mServer(server),
mPort(port),
mDefTimeout(30*1000),
mId(device_id),
mKey(device_key),
mLastHB(0)
{

}

MCSDevice::~MCSDevice()
{

}

void MCSDevice::addChannel(MCSDataChannel& channel)
{
    channel._setParent(this);
    mChannels.push_back(&channel);
}

bool MCSDevice::connected(void)
{
    return mSocket.connected();
}

bool MCSDevice::connect(void)
{
    if(connected())
        return true;
    
    mLastHB = 0;
    return _prepareSocket(mSocket);
}

void MCSDevice::process(int timeout_ms)
{
    // clear all channels` flag
    for(MCSDataChannel* it : mChannels)
        it->_clearUpdated();

    if(!connected())
        return;

    // send heart beat if necessary
    _keepAlive();

    // read from socket
    _readSocket(mSocket, mRecevieBuf, timeout_ms > 0 ? timeout_ms : mDefTimeout);
    if(!mRecevieBuf.length())
        return;

    _DEBUG_PRINT(String("[log]process:")+mRecevieBuf);

    // parse incoming data
    String body;
    if(_parsePattern(body))
    {
        _DEBUG_PRINT(String("[log]found:")+body);
        int f1 = body.indexOf(',');
        int f2 = body.indexOf(',', f1+1);

        String channel = body.substring(f1+1, f2);
        String params = body.substring(f2+1);

        if(channel.length() > 0) {
            _DEBUG_PRINT(String("[log]found:channel:")+channel+String(",params:")+params);

            for(MCSDataChannel* it : mChannels) {
                if(it->_match(channel)) {
                    _DEBUG_PRINT("[log]dispatched!");
                    it->_dispatch(params);
                }
            }
        }
    }
}

void MCSDevice::setDefaultTimeout(int timeout_ms)
{
    mDefTimeout = timeout_ms;
}

bool MCSDevice::_prepareSocket(WiFiClient& socket)
{
    _PROF_START(mcs_connect);
    if(!socket.connect(mServer.c_str(), mPort))
        return false;
    _PROF_END(mcs_connect);

    socket.print(_prepareRequest("GET", "connections.csv", "text/csv"));

    String responseBody;
    _PROF_START(mcs_wait_response);
    if(!_waitForResponse(socket, responseBody)) {
        _DEBUG_PRINT(String("[log]Response timeout: [")+responseBody+String("]"));
        socket.stop();
        return false;
    }
    _PROF_END(mcs_wait_response);
    socket.stop();

    /* parse body */
    int comma = responseBody.indexOf(',');
    String ip = responseBody.substring(0, comma);
    String port = responseBody.substring(comma+1);
    _DEBUG_PRINT(String("[log]Connecting to: ")+ip+String(":")+port);

    mRecevieBuf = "";
    mRecevieBuf.reserve(1024);
    _PROF_START(mcs_tcp_connect);
    if(!socket.connect(ip.c_str(), port.toInt()))
    {
        _DEBUG_PRINT("[log]Fail to connect!");
        socket.stop();
        return false;
    }
    _PROF_END(mcs_tcp_connect);

    return true;
}

void MCSDevice::_sendHB(WiFiClient& socket)
{
    socket.print(mId + String(",") + mKey + String(","));
}

bool MCSDevice::_parsePattern(String& result)
{
    // parse incoming data
    String tag = mId + String(",") + mKey + String(",");
    int f1 = mRecevieBuf.indexOf(tag);
    if(f1 >= 0)
    {
        String body;
        int f2 = mRecevieBuf.indexOf(tag, f1+tag.length());
        if(f2 >= 0)
        {
            body = mRecevieBuf.substring(f1 + tag.length(), f2);
            mRecevieBuf.remove(0, f2);
        }
        else
        {
            body = mRecevieBuf.substring(f1 + tag.length());
            mRecevieBuf.remove(0, mRecevieBuf.length());
        }

        if(body.length() > 0) {
            result = body;
            return true;
        }
    }
    return false;
}

void MCSDevice::_keepAlive(void)
{
    unsigned long now = millis();
    if(mLastHB == 0 || now - mLastHB > _getHBperiod())
    {
        // send heart beat
        _sendHB(mSocket);
        mLastHB = now;

        _DEBUG_PRINT(String("[log]heart beat [")+String(mLastHB)+String("]"));
    }
}

void MCSDevice::_readSocket(WiFiClient& socket, String& readBuffer, int timeout_ms)
{
    unsigned long deadline = millis() + timeout_ms;
    int d = (timeout_ms/10 < 10) ? 10 : (timeout_ms/10);
    int c;
    while(millis() < deadline) {
        while(socket.available()) {
            c = socket.read();
            if(c < 0)
                continue;

            readBuffer += (char)c;
        }
        delay(d);
    }
}

bool MCSDevice::_uploadDataPoint(const String& params)
{
    WiFiClient socket;
    String responseBody;

    _PROF_START(upload_connect);
    if(!socket.connect(mServer.c_str(), mPort))
        return false;
    _PROF_END(upload_connect);

    socket.print(_prepareRequest("POST", "datapoints.csv", "text/csv", params.length()));
    socket.println(params);
    socket.println();

    _PROF_START(upload_wait_response);
    bool result = _waitForResponse(socket, responseBody);
    _PROF_END(upload_wait_response);

    if(!result) {
        _DEBUG_PRINT(String("[log]Response timeout: [")+responseBody+String("]"));
    }

    socket.stop();
    return result;
}

bool MCSDevice::_getDataPoint(const String& channel_id, String& responseBody)
{
    WiFiClient socket;

    _PROF_START(get_connect);
    if(!socket.connect(mServer.c_str(), mPort))
        return false;
    _PROF_END(get_connect);

    socket.print(_prepareRequest("GET", String("datachannels/") + channel_id + String("/datapoints.csv"), "text/csv"));

    _PROF_START(get_wait_response);
    bool result = _waitForResponse(socket, responseBody);
    _PROF_END(get_wait_response);

    if(!result) {
        _DEBUG_PRINT(String("[log]Response timeout: [")+responseBody+String("]"));
    }

    socket.stop();
    return result;
}


String MCSDevice::_prepareRequest(const String& method, const String& url, const String& content_type, int content_len)
{
    String req = method;
    req += " ";
    req += _getAPIPath();
    req += "/devices/";
    req += mId;
    req += "/";
    req += url;
    req += " HTTP/1.1\r\n";
    req += "Content-Type: ";
    req += content_type;
    req += "\r\n";
    req += "Host: ";
    req += mServer;
    req += ":";
    req += String(mPort);
    req += "\r\n";
    if(method.equals("GET")) {
        req += "Connection: close\r\n";
    }
    req += "deviceKey: ";
    req += mKey;
    if(content_len > 0) {
        req += "\r\nContent-Length: ";
        req += String(content_len);
    }
    req += "\r\n\r\n";

    _DEBUG_PRINT(String("[log]_prepareRequest: [")+req+String("]"));
    return req;
}

bool MCSDevice::_waitForResponse(Client& client, String& responseBody)
{
    unsigned long deadline = millis() + mDefTimeout;
    String s;
    int c;
    int endOfRsp;
    int contentLen = -1;

    s.reserve(511);
    while(millis() < deadline) {
        while(client.available()) {
            c = client.read();
            if(c < 0)
                continue;

            s += (char)c;

            // response end = CRLFx2
            if(s.length() > 4 && strcmp("\r\n\r\n", s.end()-4)==0)
            {
                endOfRsp = s.length();
                // check for content-length
                contentLen = _findHeader(s, "Content-Length: ");
            }

            if(contentLen >= 0 && ((int)s.length()) >= endOfRsp + contentLen)
            {
                responseBody = s.substring(endOfRsp, endOfRsp+contentLen);
                return true;
            }
        }
        delay(100);
    }
    responseBody = s;
    return false;
}

/* ----------------------------------------------------------------------------
class MCSLiteDevice
---------------------------------------------------------------------------- */

MCSLiteDevice::MCSLiteDevice(const String& device_id, const String& device_key, const String& server, int port):
MCSDevice(device_id, device_key, server, port)
{

}

MCSLiteDevice::~MCSLiteDevice()
{

}

bool MCSLiteDevice::_prepareSocket(WiFiClient& socket)
{
    _PROF_START(mcs_lite_connect);
    if(!socket.connect(mServer.c_str(), 8000))  // 8000 is hardcoded
    {
        socket.stop();
        _DEBUG_PRINT(String("[log]socket.connect fail: [")+mServer+String(":8000]"));
        return false;
    }
    _PROF_END(mcs_lite_connect);

    // print websocket headers
    String req = "GET";
    req += " /deviceId/";
    req += mId;
    req += "/deviceKey/";
    req += mKey;
    req += "/csv HTTP/1.1\r\n";
    req += "Upgrade: websocket\r\n";
    req += "Connection: Upgrade\r\n";
    req += "Sec-WebSocket-Version: 13\r\n";
    req += "Sec-WebSocket-Key: L159VM0TWUzyDxwJEIEzjw==\r\n";
    req += "Host: ";
    req += mServer;
    req += "\r\n";
    req += "Origin: null\r\n\r\n";
    socket.print(req);

    if(!_waitForWSResponse(socket))
    {
        socket.stop();
        _DEBUG_PRINT(String("[log]fail to get response!"));
        return false;
    }

    mRecevieBuf = "";
    mRecevieBuf.reserve(1024);
    return true;
}

void MCSLiteDevice::_sendHB(WiFiClient& socket)
{
    unsigned char frame = (0x01 << 4);
    socket.print(frame);
}

bool MCSLiteDevice::_parsePattern(String& result)
{
    // parse incoming data
    String tag = mId + String(",") + mKey + String(",");
    int f1 = mRecevieBuf.indexOf(tag);
    if(f1 >= 0)
    {
        _DEBUG_PRINT(String("[log]_parsePattern: [")+mRecevieBuf+String("]"));
        String body;
        int f2 = mRecevieBuf.indexOf(tag, f1+tag.length());
        if(f2 >= 0)
        {
            body = mRecevieBuf.substring(f1 + tag.length(), f2);
            mRecevieBuf.remove(0, f2);
        }
        else
        {
            body = mRecevieBuf.substring(f1 + tag.length());
            mRecevieBuf.remove(0, mRecevieBuf.length());
        }

        if(body.length() > 0) {
            result = body;
            return true;
        }
    }
    return false;
}

bool MCSLiteDevice::_waitForWSResponse(Client& client)
{
    unsigned long deadline = millis() + mDefTimeout;
    String s;
    int c;

    s.reserve(511);
    while(millis() < deadline) {
        while(client.available()) {
            c = client.read();
            if(c < 0)
                continue;

            s += (char)c;

            // response end = CRLFx2
            if(s.length() > 4 && strcmp("\r\n\r\n", s.end()-4)==0)
            {
                return true;
            }
        }
        delay(100);
    }
    _DEBUG_PRINT(String("[log]_waitForWSResponse: [")+s+String("]"));
    return false;
}

/* ----------------------------------------------------------------------------
class MCSDataChannel
---------------------------------------------------------------------------- */

MCSDataChannel::MCSDataChannel(const String& channel_id):
mId(channel_id),
mParent(NULL),
mUpdated(false),
mInited(false)
{

}

MCSDataChannel::~MCSDataChannel()
{

}

bool MCSDataChannel::_match(const String& channel_id)
{
    return mId.equals(channel_id);
}

void MCSDataChannel::_setParent(MCSDevice* parentObj)
{
    mParent = parentObj;
}

bool MCSDataChannel::_uploadDataPoint(const String& params)
{
    String req = mId + String(",,") + params;
    if(mParent)
        return mParent->_uploadDataPoint(req);
    return false;
}

bool MCSDataChannel::_getDataPoint(String& params)
{
    if(mParent)
    {
        String body;
        bool result = mParent->_getDataPoint(mId, body);
        _DEBUG_PRINT(String("[log]_getDataPoint:")+body);
        if(result)
        {
            int f1 = body.indexOf(',');
            int f2 = body.indexOf(',', f1+1);
            params = body.substring(f2+1);
        }
        return result;
    }
    return false;
}

/* ----------------------------------------------------------------------------
class MCSDisplayOnOff
---------------------------------------------------------------------------- */

MCSDisplayOnOff::MCSDisplayOnOff(const String& channel_id):
MCSDataChannel(channel_id),
mValue(false)
{
}

MCSDisplayOnOff::~MCSDisplayOnOff()
{
}

bool MCSDisplayOnOff::set(bool value)
{
    if(valid() && value == mValue)
        return true;

    bool result = _uploadDataPoint(String(value ? "1": "0"));
    if(result)
    {
        _setValid();
        mValue = value;
    }
    return result;
}

bool MCSDisplayOnOff::value(void)
{
    return mValue;
}

void MCSDisplayOnOff::_dispatch(const String& params)
{
    // do nothing for display channel
}

/* ----------------------------------------------------------------------------
class MCSDisplayCategory
---------------------------------------------------------------------------- */

MCSDisplayCategory::MCSDisplayCategory(const String& channel_id):
MCSDataChannel(channel_id),
mValue()
{
}

MCSDisplayCategory::~MCSDisplayCategory()
{
}

bool MCSDisplayCategory::set(String value)
{
    if(valid() && value == mValue)
        return true;

    bool result = _uploadDataPoint(String(value));
    if(result)
    {
        _setValid();
        mValue = value;
    }
    return result;
}

String MCSDisplayCategory::value(void)
{
    return mValue;
}

void MCSDisplayCategory::_dispatch(const String& params)
{
    // do nothing for display channel
}

/* ----------------------------------------------------------------------------
class MCSDisplayInteger
---------------------------------------------------------------------------- */

MCSDisplayInteger::MCSDisplayInteger(const String& channel_id):
MCSDataChannel(channel_id),
mValue(0)
{
}

MCSDisplayInteger::~MCSDisplayInteger()
{
}

bool MCSDisplayInteger::set(int value)
{
    if(valid() && value == mValue)
        return true;

    bool result = _uploadDataPoint(String(value));
    if(result)
    {
        _setValid();
        mValue = value;
    }
    return result;
}

int MCSDisplayInteger::value(void)
{
    return mValue;
}

void MCSDisplayInteger::_dispatch(const String& params)
{
    // do nothing for display channel
}

/* ----------------------------------------------------------------------------
class MCSDisplayFloat
---------------------------------------------------------------------------- */

MCSDisplayFloat::MCSDisplayFloat(const String& channel_id):
MCSDataChannel(channel_id),
mValue(0)
{
}

MCSDisplayFloat::~MCSDisplayFloat()
{
}

bool MCSDisplayFloat::set(float value)
{
    if(valid() && value == mValue)
        return true;

    bool result = _uploadDataPoint(String(value));
    if(result)
    {
        _setValid();
        mValue = value;
    }
    return result;
}

float MCSDisplayFloat::value(void)
{
    return mValue;
}

void MCSDisplayFloat::_dispatch(const String& params)
{
    // do nothing for display channel
}

/* ----------------------------------------------------------------------------
class MCSDisplayHex
---------------------------------------------------------------------------- */

MCSDisplayHex::MCSDisplayHex(const String& channel_id):
MCSDataChannel(channel_id),
mValue()
{
}

MCSDisplayHex::~MCSDisplayHex()
{
}

bool MCSDisplayHex::set(long value)
{
    if(valid() && value == mValue)
        return true;

    bool result = _uploadDataPoint(String(value, HEX));
    if(result)
    {
        _setValid();
        mValue = value;
    }
    return result;
}

long MCSDisplayHex::value(void)
{
    return mValue;
}

void MCSDisplayHex::_dispatch(const String& params)
{
    // do nothing for display channel
}

/* ----------------------------------------------------------------------------
class MCSDisplayString
---------------------------------------------------------------------------- */

MCSDisplayString::MCSDisplayString(const String& channel_id):
MCSDataChannel(channel_id),
mValue()
{
}

MCSDisplayString::~MCSDisplayString()
{
}

bool MCSDisplayString::set(String value)
{
    if(valid() && value == mValue)
        return true;

    bool result = _uploadDataPoint(String(value));
    if(result)
    {
        _setValid();
        mValue = value;
    }
    return result;
}

String MCSDisplayString::value(void)
{
    return mValue;
}

void MCSDisplayString::_dispatch(const String& params)
{
    // do nothing for display channel
}


/* ----------------------------------------------------------------------------
class MCSControllerGPS
---------------------------------------------------------------------------- */

bool MCSGPSValue::operator==(MCSGPSValue const& rhs) const
{
    if(this == &rhs)
    {
        return true;
    }

    // TODO: we can use better float equal comparator (epsilon-based)
    return (mLatitude == rhs.mLatitude) &&
           (mLongitude == rhs.mLongitude) &&
           (mAltitude == rhs.mAltitude);
}

bool MCSGPSValue::operator!=(MCSGPSValue const& rhs) const
{
    return !(*this == rhs);
}

bool MCSGPSValue::isValid()const
{
    // The Lat/Long values are initialized in 360.f
    // which should be "invalid" GPS value range.
    if(( -90.f > mLatitude) ||
       (  90.f < mLatitude) ||
       ( 180.f > mLongitude) ||
       (-180.f < mLongitude))
    {
        return false;
    }

    return true;
}

MCSGPSValue::operator bool()const
{
    return isValid();
}

size_t MCSGPSValue::printTo(Print& p) const
{
    return p.print(MCSValueToString(*this));
}

void MCSControllerGPS::getGPSValue(float& latitude, float& longitude, float& altitude)
{
    const MCSGPSValue v = value();
    latitude = v.mLatitude;
    longitude = v.mLongitude;
    altitude = v.mAltitude;
    return;
}

float MCSControllerGPS::latitude(void)
{
    return mValue.mLatitude;
}

float MCSControllerGPS::longitude(void)
{
    return mValue.mLongitude;
}

float MCSControllerGPS::altitude(void)
{
    return mValue.mAltitude;
}

bool MCSControllerGPS::setServerValue(float latitude, float longitude, float altitude)
{
    MCSGPSValue v;
    v.mLatitude = latitude;
    v.mLongitude = longitude;
    v.mAltitude = altitude;

    return MCSControllerBase::setServerValue(v);
}

/* ----------------------------------------------------------------------------
class MCSDisplayGPS
---------------------------------------------------------------------------- */

MCSDisplayGPS::MCSDisplayGPS(const String& channel_id):
MCSDataChannel(channel_id),
mLatitude(),
mLongitude(),
mAltitude()
{
}

MCSDisplayGPS::~MCSDisplayGPS()
{
}

bool MCSDisplayGPS::set(const float latitude, const float longitude, const float altitude)
{
    if(valid() && latitude == mLatitude && longitude == mLongitude && altitude == mAltitude)
        return true;

    String payload = String(latitude)+String(",")+String(longitude)+String(",")+String(altitude);
    bool result = _uploadDataPoint(payload);
    if(result)
    {
        _setValid();
        mLatitude = latitude;
        mLongitude = longitude;
        mAltitude = altitude;
    }
    return result;
}

float MCSDisplayGPS::latitude(void)
{
    return mLatitude;
}

float MCSDisplayGPS::longitude(void)
{
    return mLongitude;
}

float MCSDisplayGPS::altitude(void)
{
    return mAltitude;
}

void MCSDisplayGPS::_dispatch(const String& params)
{
    // do nothing for display channel
}

/* ----------------------------------------------------------------------------
class MCSDisplayGPIO
---------------------------------------------------------------------------- */

MCSDisplayGPIO::MCSDisplayGPIO(const String& channel_id):
MCSDataChannel(channel_id),
mValue(0)
{
}

MCSDisplayGPIO::~MCSDisplayGPIO()
{
}

bool MCSDisplayGPIO::set(int value)
{
    if(valid() && value == mValue)
        return true;

    bool result = _uploadDataPoint(String(value));
    if(result)
    {
        _setValid();
        mValue = value;
    }
    return result;
}

int MCSDisplayGPIO::value(void)
{
    return mValue;
}

void MCSDisplayGPIO::_dispatch(const String& params)
{
    // do nothing for display channel
}


/* ----------------------------------------------------------------------------
class MCSControllerPWM
---------------------------------------------------------------------------- */
bool MCSPWMValue::operator==(const MCSPWMValue& rhs)const
{
    if(this == &rhs)
    {
        return true;
    }

    return (mDutyCycle == rhs.mDutyCycle) &&
           (mPeriod == rhs.mPeriod);
}

bool MCSPWMValue::operator!=(const MCSPWMValue& rhs)const
{
    return !(*this == rhs);
}

MCSPWMValue::operator bool()const
{
    return isValid();
}

bool MCSPWMValue::isValid()const
{
    // cycles and period cannot be zero or negative.
    if(0 >= mDutyCycle ||
       0 >= mPeriod)
    {
        return false;
    }

    return true;
}

size_t MCSPWMValue::printTo(Print& p) const
{
    size_t len = 0;
    len += p.print(mDutyCycle);
    len += p.print(",");
    len += p.print(mPeriod);
    return len;
}

int MCSControllerPWM::dutyCycle(void)
{
    MCSPWMValue v = value();
    return v.mDutyCycle;
}

int MCSControllerPWM::period(void)
{
    MCSPWMValue v = value();
    return v.mPeriod;
}

/* ----------------------------------------------------------------------------
class MCSDisplayPWM
---------------------------------------------------------------------------- */

MCSDisplayPWM::MCSDisplayPWM(const String& channel_id):
MCSDataChannel(channel_id),
mValue(),
mPeriod()
{
}

MCSDisplayPWM::~MCSDisplayPWM()
{
}

bool MCSDisplayPWM::set(const int value, const int period)
{
    if(valid() && value == mValue && period == mPeriod)
        return true;

    String payload = String(value)+String(",")+String(period);
    bool result = _uploadDataPoint(payload);
    if(result)
    {
        _setValid();
        mValue = value;
    }
    return result;
}

int MCSDisplayPWM::value(void)
{
    return mValue;
}

int MCSDisplayPWM::period(void)
{
    return mPeriod;
}

void MCSDisplayPWM::_dispatch(const String& params)
{
    // do nothing for display channel
}
