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

/* ------------------------------------------------------------------------ */
class MCSControllerCategory : public MCSDataChannel
{
public:
    MCSControllerCategory(const String& channel_id);
    ~MCSControllerCategory();

    String value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool _update(const String& params);

private:
    String mValue;
};

/* ------------------------------------------------------------------------ */
class MCSDisplayCategory : public MCSDataChannel
{
public:
    MCSDisplayCategory(const String& channel_id);
    ~MCSDisplayCategory();

    bool set(String value);
    String value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    String mValue;
};

/* ------------------------------------------------------------------------ */
class MCSControllerInteger : public MCSDataChannel
{
public:
    MCSControllerInteger(const String& channel_id);
    ~MCSControllerInteger();

    int value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool _update(const String& params);

private:
    int mValue;
};

/* ------------------------------------------------------------------------ */
class MCSDisplayInteger : public MCSDataChannel
{
public:
    MCSDisplayInteger(const String& channel_id);
    ~MCSDisplayInteger();

    bool set(int value);
    int value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    int mValue;
};

/* ------------------------------------------------------------------------ */
class MCSControllerFloat : public MCSDataChannel
{
public:
    MCSControllerFloat(const String& channel_id);
    ~MCSControllerFloat();

    float value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool _update(const String& params);

private:
    float mValue;
};

/* ------------------------------------------------------------------------ */
class MCSDisplayFloat : public MCSDataChannel
{
public:
    MCSDisplayFloat(const String& channel_id);
    ~MCSDisplayFloat();

    bool set(float value);
    float value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    float mValue;
};

/* ------------------------------------------------------------------------ */
class MCSControllerHex : public MCSDataChannel
{
public:
    MCSControllerHex(const String& channel_id);
    ~MCSControllerHex();

    long value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool _update(const String& params);

private:
    long mValue;
};

/* ------------------------------------------------------------------------ */
class MCSDisplayHex : public MCSDataChannel
{
public:
    MCSDisplayHex(const String& channel_id);
    ~MCSDisplayHex();

    bool set(long value);
    long value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    long mValue;
};

/* ------------------------------------------------------------------------ */
class MCSControllerString : public MCSDataChannel
{
public:
    MCSControllerString(const String& channel_id);
    ~MCSControllerString();

    String value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool _update(const String& params);

private:
    String mValue;
};

/* ------------------------------------------------------------------------ */
class MCSDisplayString : public MCSDataChannel
{
public:
    MCSDisplayString(const String& channel_id);
    ~MCSDisplayString();

    bool set(String value);
    String value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    String mValue;
};

/* ------------------------------------------------------------------------ */
class MCSControllerGPS : public MCSDataChannel
{
public:
    MCSControllerGPS(const String& channel_id);
    ~MCSControllerGPS();

    float latitude(void);
    float longitude(void);
    float altitude(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool _update(const String& params);

private:
    float mLatitude;
    float mLongitude;
    float mAltitude;
};

/* ------------------------------------------------------------------------ */
class MCSDisplayGPS : public MCSDataChannel
{
public:
    MCSDisplayGPS(const String& channel_id);
    ~MCSDisplayGPS();

    bool set(const float latitude, const float longitude, const float altitude);
    float latitude(void);
    float longitude(void);
    float altitude(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    float mLatitude;
    float mLongitude;
    float mAltitude;
};

/* ------------------------------------------------------------------------ */
class MCSControllerGPIO : public MCSDataChannel
{
public:
    MCSControllerGPIO(const String& channel_id);
    ~MCSControllerGPIO();

    int value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool _update(const String& params);

private:
    int mValue;
};

/* ------------------------------------------------------------------------ */
class MCSDisplayGPIO : public MCSDataChannel
{
public:
    MCSDisplayGPIO(const String& channel_id);
    ~MCSDisplayGPIO();

    bool set(int value);
    int value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    int mValue;
};

/* ------------------------------------------------------------------------ */
class MCSControllerPWM : public MCSDataChannel
{
public:
    MCSControllerPWM(const String& channel_id);
    ~MCSControllerPWM();

    int value(void);
    int period(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool _update(const String& params);

private:
    int mValue;
    int mPeriod;
};

/* ------------------------------------------------------------------------ */
class MCSDisplayPWM : public MCSDataChannel
{
public:
    MCSDisplayPWM(const String& channel_id);
    ~MCSDisplayPWM();

    bool set(const int value, const int period);
    int value(void);
    int period(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    int mValue;
    int mPeriod;
};

/* ------------------------------------------------------------------------ */
class MCSControllerAnalog : public MCSDataChannel
{
public:
    MCSControllerAnalog(const String& channel_id);
    ~MCSControllerAnalog();

    int value(void);

protected:
    // override
    virtual void _dispatch(const String& params);

private:
    bool _update(const String& params);

private:
    int mValue;
};


#endif