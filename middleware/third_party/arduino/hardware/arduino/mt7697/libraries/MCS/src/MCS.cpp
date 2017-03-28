#include "MCS.h"
#include "MCS_debug.h"

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

MCSDevice::~MCSDevice()
{

}

bool MCSDevice::connected(void)
{
    return mSocket.connected();
}

bool MCSDevice::connect(void)
{
    _PROF_START(mcs_connect);
    if(!mSocket.connect(mServer.c_str(), mPort))
        return false;
    _PROF_END(mcs_connect);

    mSocket.print(_prepareRequest("GET", "connections.csv", "text/csv"));

    _PROF_START(mcs_wait_response);
    if(!_waitForResponse(mSocket)) {
        _DEBUG_PRINT(String("[log]Response timeout: [")+mRecevieBuf+String("]"));
        mSocket.stop();
        return false;
    }
    _PROF_END(mcs_wait_response);
    mSocket.stop();

    /* parse body */
    int comma = mRecevieBuf.indexOf(',');
    String ip = mRecevieBuf.substring(0, comma);
    String port = mRecevieBuf.substring(comma+1);
    _DEBUG_PRINT(String("[log]Connecting to: ")+ip+String(":")+port);

    mRecevieBuf = "";
    mRecevieBuf.reserve(1024);
    _PROF_START(mcs_tcp_connect);
    if(!mSocket.connect(ip.c_str(), port.toInt()))
    {
        _DEBUG_PRINT("[log]Fail to connect!");
        mSocket.stop();
        return false;
    }
    _PROF_END(mcs_tcp_connect);

    return true;
}

void MCSDevice::process(int timeout_ms)
{
    // send heart beat if necessary
    _keepAlive();

    // clear all channels` flag
    for(MCSDataChannel* it : mChannels)
        it->_resetFlag();

    // read from socket
    _processSocket(timeout_ms > 0 ? timeout_ms : mDefTimeout);
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

void MCSDevice::addChannel(MCSDataChannel* channel)
{
    mChannels.push_back(channel);
    channel->_setParent(this);
}

void MCSDevice::setDefaultTimeout(int timeout_ms)
{
    mDefTimeout = timeout_ms;
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

void MCSDevice::_processSocket(int timeout_ms)
{
    unsigned long deadline = millis() + timeout_ms;
    int c;
    while(millis() < deadline) {
        while(mSocket.available()) {
            c = mSocket.read();
            if(c < 0)
                continue;

            mRecevieBuf += (char)c;
        }
    }
}

bool MCSDevice::_uploadData(const String& params)
{
    WiFiClient socket;

    _PROF_START(upload_connect);
    if(!socket.connect(mServer.c_str(), mPort))
        return false;
    _PROF_END(upload_connect);

    socket.print(_prepareRequest("POST", "datapoints.csv", "text/csv", params.length()));
    socket.print(params);
    socket.println();

    _PROF_START(upload_wait_response);
    bool result = _waitForResponse(socket);
    _PROF_END(upload_wait_response);

    if(!result) {
        _DEBUG_PRINT(String("[log]Response timeout: [")+mRecevieBuf+String("]"));
    }

    mRecevieBuf.remove(0, mRecevieBuf.length());
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

bool MCSDevice::_waitForResponse(Client &client)
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
                mRecevieBuf = s.substring(endOfRsp, endOfRsp+contentLen);
                return true;
            }
        }
        delay(100);
    }
    mRecevieBuf = s;
    return false;
}

int MCSDevice::_findHeader(const String& response, const String& header)
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
class MCSDataChannel
---------------------------------------------------------------------------- */

MCSDataChannel::MCSDataChannel(const String& channel_id):
mId(channel_id),
mParent(NULL),
mFlag(0)
{

}

MCSDataChannel::~MCSDataChannel()
{

}

bool MCSDataChannel::updated(void)
{
    return mFlag == 1;
}

void MCSDataChannel::_setFlag(void)
{
    mFlag = 1;
}

void MCSDataChannel::_resetFlag(void)
{
    mFlag = 0;
}

void MCSDataChannel::_setParent(MCSDevice* parentObj)
{
    mParent = parentObj;
}

bool MCSDataChannel::_uploadData(const String& params)
{
    if(mParent)
        return mParent->_uploadData(params);
    return false;
}

bool MCSDataChannel::_match(const String& channel_id)
{
    return mId.equals(channel_id);
}

/* ----------------------------------------------------------------------------
class MCSDataChannelSwitch
---------------------------------------------------------------------------- */

MCSDataChannelSwitch::MCSDataChannelSwitch(const String& channel_id):
MCSDataChannel(channel_id),
mValue(false)
{
}

MCSDataChannelSwitch::~MCSDataChannelSwitch()
{
}

bool MCSDataChannelSwitch::set(bool value)
{
    if(value == mValue)
        return true;

    String req(_id());
    req += value ? ",,1": ",,0";

    bool result = _uploadData(req);
    if(result)
        mValue = value; // only update value if success
    return result;
}

bool MCSDataChannelSwitch::value(void)
{
    return mValue;
}

void MCSDataChannelSwitch::_dispatch(const String& params)
{
    int b = params.toInt();

    bool v = (b == 1) ? true : false;
    if(v != mValue)
    {
        mValue = v;
        _setFlag();
    }
}
