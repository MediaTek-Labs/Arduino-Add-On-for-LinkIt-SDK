#ifndef MCS_H
#define MCS_H


#include <Arduino.h>
#include <vector>
#include "LWiFi.h"

class MCSDataChannel;
/* ------------------------------------------------------------------------ */
class MCSDevice
{
public:
    MCSDevice(const String& device_id, const String& device_key);
    MCSDevice(const String& device_id, const String& device_key, const String& server, int port);
    ~MCSDevice();

    void addChannel(MCSDataChannel& channel);

    bool connected(void);

    bool connect(void);

    void process(int timeout_ms=0);

    void setDefaultTimeout(int timeout_ms);

protected:
    virtual bool _prepareSocket(WiFiClient& socket);

    void _keepAlive(void);

    void _readSocket(WiFiClient& socket, String& readBuffer, int timeout_ms);

    // called by MCSDataChannel
    bool _uploadDataPoint(const String& params);
    bool _getDataPoint(const String& channel_id, String& responseBody);

    // utility function, should not assume socket
    String _prepareRequest(const String& method, const String& url, const String& content_type, int content_len = 0);
    bool _waitForResponse(Client& client, String& responseBody);

protected:
    String mServer;
    int mPort;
    int mDefTimeout;
    String mId;
    String mKey;
    std::vector<MCSDataChannel*> mChannels;

    WiFiClient mSocket;
    String mRecevieBuf;

    unsigned long mLastHB;

    friend class MCSDataChannel;
};

/* ------------------------------------------------------------------------ */
class MCSLiteDevice : public MCSDevice
{
public:
    MCSLiteDevice(const String& device_id, const String& device_key, const String& server, int port);
    ~MCSLiteDevice();

protected:
    virtual bool _prepareSocket(WiFiClient& socket);
};

/* ------------------------------------------------------------------------ */
class MCSDataChannel
{
public:
    MCSDataChannel(const String& channel_id);
    ~MCSDataChannel();

    bool updated(void)  { return mUpdated; }
    bool valid(void)    { return mInited; }

protected:
    bool _match(const String& channel_id);
    virtual void _dispatch(const String& params) = 0;

    void _setParent(MCSDevice* parentObj);
    bool _uploadDataPoint(const String& params);
    bool _getDataPoint(String& params);

    void _setUpdated(void)      { mUpdated = true; }
    void _clearUpdated(void)    { mUpdated = false; }
    void _setValid(void)       { mInited = true; }

private:
    String mId;
    MCSDevice* mParent;
    bool mUpdated;
    bool mInited;

    friend class MCSDevice;
};

/* ------------------------------------------------------------------------ */
class MCSControllerOnOff : public MCSDataChannel
{
public:
    MCSControllerOnOff(const String& channel_id);
    ~MCSControllerOnOff();

    bool value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool _update(const String& params);

private:
    bool mValue;
};

/* ------------------------------------------------------------------------ */
class MCSDisplayOnOff : public MCSDataChannel
{
public:
    MCSDisplayOnOff(const String& channel_id);
    ~MCSDisplayOnOff();

    bool set(bool value);
    bool value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool mValue;
};


#endif