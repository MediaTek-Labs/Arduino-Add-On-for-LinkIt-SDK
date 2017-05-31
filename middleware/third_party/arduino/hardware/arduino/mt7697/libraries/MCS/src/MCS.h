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
    virtual void _sendHB(WiFiClient& socket);
    virtual int _getHBperiod() { return 60*1000; }
    virtual String _getAPIPath() { return "/mcs/v2";}
    virtual bool _parsePattern(String& result);

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
    virtual void _sendHB(WiFiClient& socket);
    virtual int _getHBperiod() { return 10*1000; }
    virtual String _getAPIPath() { return "/api";}
    virtual bool _parsePattern(String& result);

private:
    bool _waitForWSResponse(Client& client);
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
/* These are default, overloaded String <-> value conversion functions used by MCSControllerBase */
/* ------------------------------------------------------------------------ */
inline String MCSValueToString(const bool& value)
{
    return String(value ? "1": "0");
}

inline void MCSStringToValue(const String& params, bool& value)
{
    int b = params.toInt();
    value = (b == 1) ? true : false;
}

inline String MCSValueToString(const String& value)
{
    return value;
}

inline void MCSStringToValue(const String& params, String& value)
{
    value = params;
}

inline String MCSValueToString(const int& value)
{
    return String(value);
}

inline void MCSStringToValue(const String& params, int& value)
{
    value = params.toInt();
}

inline String MCSValueToString(const float& value)
{
    return String(value);
}

inline void MCSStringToValue(const String& params, float& value)
{
    value = params.toFloat();
}

inline String MCSValueToString(const long& value)
{
    return String(value, HEX);
}

inline void MCSStringToValue(const String& params, long& value)
{
    value = strtol(params.c_str(), NULL, 16);
}

/* ------------------------------------------------------------------------ */
template <typename T> class MCSControllerBase : public MCSDataChannel
{
public:
    MCSControllerBase(const String& channel_id):MCSDataChannel(channel_id){

    }

    T value(void){
        if(valid())
            return mValue;

        // retrieve latest data point from server
        String params;
        if(_getDataPoint(params))
        {
            _update(params);
            return mValue;
        }

        return T();
    }

    // Call this method to change the server-side value
    // of the controller channel.
    // 
    // Note that the result of value() does not change
    // until the server-side value is successfully updated,
    // and MCSDevice::process() has been called to 
    // parse the incomping server update.
    virtual bool setServerValue(T serverValue){
        // convert mValue to MCS String format,
        // and then upload to MCS.
        const bool result = _uploadDataPoint(_valueToString(serverValue));
        if(result)
        {
            // if upload succeeded,
            // we need to update local values once the server is updated.
            mValue = serverValue;

            // make the value "updated",
            // so the next process() call will properly update.
            _setValid();
            _setUpdated();
        }
        return result;
    }

protected:
    // default implementation between String <-> Value
    // Subclasses such as "HEX format" may need 
    // to override this method.
    virtual void _valueFromString(const String& param, T& value){
        MCSStringToValue(param, value);
    }

    // default implementation between String <-> Value
    // Subclasses such as "HEX format" may need 
    // to override this method.
    virtual String _valueToString(const T& value){
        return MCSValueToString(value);
    }

    // most subclasses won't be required to change 
    // the upload policy
    virtual void _dispatch(const String& params){
        if(_update(params))
            _setUpdated();
    }

    bool _update(const String& params){
        T v;
        _valueFromString(params, v);

        if(!valid() || v != mValue)
        {
            mValue = v;
            _setValid();
            return true;
        }
        return false;
    }

protected:
    T mValue;
};

/* ------------------------------------------------------------------------ */
class MCSControllerOnOff : public MCSControllerBase<bool>
{
public:
    MCSControllerOnOff(const String& channel_id);
    ~MCSControllerOnOff();
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
class MCSControllerCategory : public MCSControllerBase<String>
{
public:
    MCSControllerCategory(const String& channel_id);
    ~MCSControllerCategory();
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
class MCSControllerInteger : public MCSControllerBase<int>
{
public:
    MCSControllerInteger(const String& channel_id);
    ~MCSControllerInteger();
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
class MCSControllerFloat : public MCSControllerBase<float>
{
public:
    MCSControllerFloat(const String& channel_id);
    ~MCSControllerFloat();
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
class MCSControllerHex : public MCSControllerBase<long>
{
public:
    MCSControllerHex(const String& channel_id);
    ~MCSControllerHex();
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
class MCSControllerString : public MCSControllerBase<String>
{
public:
    MCSControllerString(const String& channel_id);
    ~MCSControllerString();
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
class MCSControllerGPS : public MCSControllerBase<String>
{
public:
    MCSControllerGPS(const String& channel_id);
    ~MCSControllerGPS();

    float latitude(void);
    float longitude(void);
    float altitude(void);

    void getGPSValue(float& latitude, float& longitude, float& altitude);
    
    virtual bool setServerValue(float latitude, float longitude, float altitude);
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
class MCSControllerGPIO : public MCSControllerBase<int>
{
public:
    MCSControllerGPIO(const String& channel_id);
    ~MCSControllerGPIO();
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
class MCSControllerAnalog : public MCSControllerBase<int>
{
public:
    MCSControllerAnalog(const String& channel_id);
    ~MCSControllerAnalog();
};


#endif