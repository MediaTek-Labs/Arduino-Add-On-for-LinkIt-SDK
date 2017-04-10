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
            _DEBUG_PRINT(String("[log]found:")+body);
            f1 = body.indexOf(',');
            f2 = body.indexOf(',', f1+1);

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

void MCSDevice::_keepAlive(void)
{
    unsigned long now = millis();
    if(mLastHB == 0 || now - mLastHB > 60*1000)   // heart beat every 60 secs
    {
        // send heart beat
        String hb = mId + String(",") + mKey + String(",");
        mSocket.print(hb);
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
    socket.print(params);
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
    req += " /mcs/v2/devices/";
    req += mId;
    req += "/";
    req += url;
    req += " HTTP/1.1\r\n";
    req += "Content-Type: '";
    req += content_type;
    req += "'\r\n";
    req += "Host: '";
    req += mServer;
    req += ":";
    req += String(mPort);
    req += "'\r\n";
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

            if(contentLen >= 0 && s.length() >= endOfRsp + contentLen)
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
    if(!socket.connect(mServer.c_str(), mPort))
        return false;
    _PROF_END(mcs_lite_connect);

    // print websocket headers
    String req = "";
    socket.print(req);

    return true;
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
class MCSControllerOnOff
---------------------------------------------------------------------------- */

MCSControllerOnOff::MCSControllerOnOff(const String& channel_id):
MCSDataChannel(channel_id),
mValue(false)
{
}

MCSControllerOnOff::~MCSControllerOnOff()
{
}

bool MCSControllerOnOff::value(void)
{
    if(valid())
        return mValue;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mValue;
    }

    return false;
}

void MCSControllerOnOff::_dispatch(const String& params)
{
    if(_update(params))
        _setUpdated();
}

bool MCSControllerOnOff::_update(const String& params)
{
    int b = params.toInt();
    bool v = (b == 1) ? true : false;

    if(!valid() || v != mValue)
    {
        mValue = v;
        _setValid();
        return true;
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
class MCSControllerCategory
---------------------------------------------------------------------------- */

MCSControllerCategory::MCSControllerCategory(const String& channel_id):
MCSDataChannel(channel_id),
mValue()
{
}

MCSControllerCategory::~MCSControllerCategory()
{
}

String MCSControllerCategory::value(void)
{
    if(valid())
        return mValue;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mValue;
    }
}

void MCSControllerCategory::_dispatch(const String& params)
{
    if(_update(params))
        _setUpdated();
}

bool MCSControllerCategory::_update(const String& params)
{
    String b = params;
    String v = b;

    if(!valid() || v != mValue)
    {
        mValue = v;
        _setValid();
        return true;
    }
    return false;
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
class MCSControllerInteger
---------------------------------------------------------------------------- */

MCSControllerInteger::MCSControllerInteger(const String& channel_id):
MCSDataChannel(channel_id),
mValue(0)
{
}

MCSControllerInteger::~MCSControllerInteger()
{
}

int MCSControllerInteger::value(void)
{
    if(valid())
        return mValue;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mValue;
    }
}

void MCSControllerInteger::_dispatch(const String& params)
{
    if(_update(params))
        _setUpdated();
}

bool MCSControllerInteger::_update(const String& params)
{
    int b = params.toInt();
    int v = b;

    if(!valid() || v != mValue)
    {
        mValue = v;
        _setValid();
        return true;
    }
    return false;
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
class MCSControllerFloat
---------------------------------------------------------------------------- */

MCSControllerFloat::MCSControllerFloat(const String& channel_id):
MCSDataChannel(channel_id),
mValue(0)
{
}

MCSControllerFloat::~MCSControllerFloat()
{
}

float MCSControllerFloat::value(void)
{
    if(valid())
        return mValue;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mValue;
    }
}

void MCSControllerFloat::_dispatch(const String& params)
{
    if(_update(params))
        _setUpdated();
}

bool MCSControllerFloat::_update(const String& params)
{
    float b = params.toFloat();
    float v = b;

    if(!valid() || v != mValue)
    {
        mValue = v;
        _setValid();
        return true;
    }
    return false;
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
class MCSControllerHex
---------------------------------------------------------------------------- */

MCSControllerHex::MCSControllerHex(const String& channel_id):
MCSDataChannel(channel_id),
mValue()
{
}

MCSControllerHex::~MCSControllerHex()
{
}

String MCSControllerHex::value(void)
{
    if(valid())
        return mValue;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mValue;
    }
}

void MCSControllerHex::_dispatch(const String& params)
{
    if(_update(params))
        _setUpdated();
}

bool MCSControllerHex::_update(const String& params)
{
    String b = params;
    String v = b;

    if(!valid() || v != mValue)
    {
        mValue = v;
        _setValid();
        return true;
    }
    return false;
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

bool MCSDisplayHex::set(String value)
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

String MCSDisplayHex::value(void)
{
    return mValue;
}

void MCSDisplayHex::_dispatch(const String& params)
{
    // do nothing for display channel
}


/* ----------------------------------------------------------------------------
class MCSControllerString
---------------------------------------------------------------------------- */

MCSControllerString::MCSControllerString(const String& channel_id):
MCSDataChannel(channel_id),
mValue()
{
}

MCSControllerString::~MCSControllerString()
{
}

String MCSControllerString::value(void)
{
    if(valid())
        return mValue;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mValue;
    }
}

void MCSControllerString::_dispatch(const String& params)
{
    if(_update(params))
        _setUpdated();
}

bool MCSControllerString::_update(const String& params)
{
    String b = params;
    String v = b;

    if(!valid() || v != mValue)
    {
        mValue = v;
        _setValid();
        return true;
    }
    return false;
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

MCSControllerGPS::MCSControllerGPS(const String& channel_id):
MCSDataChannel(channel_id),
mLatitude(),
mLongitude(),
mAltitude()
{
}

MCSControllerGPS::~MCSControllerGPS()
{
}

float MCSControllerGPS::latitude(void)
{
    if(valid())
        return mLatitude;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mLatitude;
    }
}

float MCSControllerGPS::longitude(void)
{
    if(valid())
        return mLongitude;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mLongitude;
    }
}

float MCSControllerGPS::altitude(void)
{
    if(valid())
        return mAltitude;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mAltitude;
    }
}

void MCSControllerGPS::_dispatch(const String& params)
{
    if(_update(params))
        _setUpdated();
}

bool MCSControllerGPS::_update(const String& params)
{
    int c1 = params.indexOf(',');
    int c2 = params.indexOf(',', c1+1);
    float v1 = params.substring(0, c1).toFloat();
    float v2 = params.substring(c1+1, c2).toFloat();
    float v3 = params.substring(c2+1).toFloat();

    if(!valid() || v1 != mLatitude || v2 != mLongitude || v3 != mAltitude)
    {
        mLatitude = v1;
        mLongitude = v2;
        mAltitude = v3;
        _setValid();
        return true;
    }
    return false;
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
class MCSControllerGPIO
---------------------------------------------------------------------------- */

MCSControllerGPIO::MCSControllerGPIO(const String& channel_id):
MCSDataChannel(channel_id),
mValue(0)
{
}

MCSControllerGPIO::~MCSControllerGPIO()
{
}

int MCSControllerGPIO::value(void)
{
    if(valid())
        return mValue;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mValue;
    }
}

void MCSControllerGPIO::_dispatch(const String& params)
{
    if(_update(params))
        _setUpdated();
}

bool MCSControllerGPIO::_update(const String& params)
{
    int b = params.toInt();
    int v = b;

    if(!valid() || v != mValue)
    {
        mValue = v;
        _setValid();
        return true;
    }
    return false;
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

MCSControllerPWM::MCSControllerPWM(const String& channel_id):
MCSDataChannel(channel_id),
mValue(),
mPeriod()
{
}

MCSControllerPWM::~MCSControllerPWM()
{
}

int MCSControllerPWM::value(void)
{
    if(valid())
        return mValue;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mValue;
    }
}

int MCSControllerPWM::period(void)
{
    if(valid())
        return mPeriod;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mPeriod;
    }
}

void MCSControllerPWM::_dispatch(const String& params)
{
    if(_update(params))
        _setUpdated();
}

bool MCSControllerPWM::_update(const String& params)
{
    int c = params.indexOf(',');
    float v1 = params.substring(0, c).toInt();
    float v2 = params.substring(c+1).toInt();

    if(!valid() || v1 != mValue || v2 != mPeriod)
    {
        mValue = v1;
        mPeriod = v2;
        _setValid();
        return true;
    }
    return false;
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

/* ----------------------------------------------------------------------------
class MCSControllerAnalog
---------------------------------------------------------------------------- */

MCSControllerAnalog::MCSControllerAnalog(const String& channel_id):
MCSDataChannel(channel_id),
mValue(0)
{
}

MCSControllerAnalog::~MCSControllerAnalog()
{
}

int MCSControllerAnalog::value(void)
{
    if(valid())
        return mValue;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mValue;
    }
}

void MCSControllerAnalog::_dispatch(const String& params)
{
    if(_update(params))
        _setUpdated();
}

bool MCSControllerAnalog::_update(const String& params)
{
    int b = params.toInt();
    int v = b;

    if(!valid() || v != mValue)
    {
        mValue = v;
        _setValid();
        return true;
    }
    return false;
}

/* ----------------------------------------------------------------------------
class MCSControllerGamepad
---------------------------------------------------------------------------- */

MCSControllerGamepad::MCSControllerGamepad(const String& channel_id):
MCSDataChannel(channel_id),
mValue()
{
}

MCSControllerGamepad::~MCSControllerGamepad()
{
}

String MCSControllerGamepad::value(void)
{
    if(valid())
        return mValue;

    // retrieve latest data point from server
    String params;
    if(_getDataPoint(params))
    {
        _update(params);
        return mValue;
    }
}

void MCSControllerGamepad::_dispatch(const String& params)
{
    if(_update(params))
        _setUpdated();
}

bool MCSControllerGamepad::_update(const String& params)
{
    String b = params;
    String v = b;

    if(!valid() || v != mValue)
    {
        mValue = v;
        _setValid();
        return true;
    }
    return false;
}
