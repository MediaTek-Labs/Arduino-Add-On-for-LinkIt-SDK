#ifndef MCS_H
#define MCS_H


#include <Arduino.h>
#include <vector>
#include "LWiFi.h"

class MCSDataChannel;
class MCSDevice
{
public:
    MCSDevice(const String& device_id, const String& device_key);
    ~MCSDevice();

    bool connected(void);

    bool connect(void);

    void process(int timeout_ms=0);

    void addChannel(MCSDataChannel* channel);

    void setDefaultTimeout(int timeout_ms);

protected:
    void _keepAlive(void);

    void _processSocket(int timeout_ms);

    // called by MCSDataChannel
    bool _uploadData(const String& params);

    // utility function, should not assume socket
    String _prepareRequest(const String& method, const String& url, const String& content_type, int content_len = 0);
    bool _waitForResponse(Client &client);
    int _findHeader(const String& response, const String& header);

private:
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

class MCSDataChannel
{
public:
    MCSDataChannel(const String& channel_id);
    ~MCSDataChannel();

    bool updated(void);

protected:
    void _setFlag(void);
    void _resetFlag(void);
    void _setParent(MCSDevice* parentObj);
    bool _uploadData(const String& params);
    bool _match(const String& channel_id);
    String _id() { return mId; }

    virtual void _dispatch(const String& params) = 0;

private:
    String mId;
    MCSDevice* mParent;
    int mFlag;

    friend class MCSDevice;
};

class MCSDataChannelSwitch : public MCSDataChannel
{
public:
    MCSDataChannelSwitch(const String& channel_id);
    ~MCSDataChannelSwitch();

    bool set(bool value);
    bool value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool mValue;
};

#endif