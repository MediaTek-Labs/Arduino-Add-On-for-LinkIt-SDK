#ifndef MCS_H
#define MCS_H


#include <Arduino.h>
#include <Printable.h>
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

    /// \brief  To add a data channel instance to a MCSDevice instance.
    ///
    /// \param channel an instance of a subclass of MCSDataChannel.
    void addChannel(MCSDataChannel& channel);

    bool connected(void);

    bool connect(void);

    void process(int timeout_ms=0);

    void setDefaultTimeout(int timeout_ms);

protected:
    virtual bool _prepareSocket(WiFiClient& socket);
    virtual void _sendHB(WiFiClient& socket);
    virtual unsigned long _getHBperiod() { return 60*1000; }
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
    virtual unsigned long _getHBperiod() { return 10*1000; }
    virtual String _getAPIPath() { return "/api";}
    virtual bool _parsePattern(String& result);

private:
    bool _waitForWSResponse(Client& client);
};

/* ------------------------------------------------------------------------ */

/**
    \brief Base class for all data channel classes. Do not instantiate this class.

    All data channel classes extend MCSDataChannel, which is used to create 
    a data channel instance mapping to the one created on the MCS server and 
    provide operations on a data channel, including getting and uploading 
    data points.
 
    For example, an ON/OFF controller channel can be represented by:
    ~~~{.cpp}
    MCSControllerOnOff channelOnOff("channel_id_1");
    device.addChannel(channelOnOff);    // device is an instance of MCSDevice or MCSLiteDevice.
    
    device.connect();
    bool controllerValue = channel.value();
    ~~~

    Data channel classes provide set() and value() methods for each data channel as 
    they have different value types. 
    
    Use the extended classes in your sketch to create a specific type of data channels.
    There are 2 major sub-class categories, **controller channel** and **display channel**. 
    
    Controller channels, such as MCSControllerOnOff and MCSControllerInteger, 
    represents controller channels on MCS. 
    
    Display channels such as MCSDisplayFloat and MCSDisplayString, 
    maps to display channels instead. 
 */
class MCSDataChannel
{
public:

    ///
    /// Creates a data channel that can be added to a device that is an instance of MCSDevice or MCSLiteDevice.
    ///
    MCSDataChannel(const String& channel_id);

    /// \cond PRIVATE
    ~MCSDataChannel();
    /// \endcond

    /// To check if there is updated data point received for a specified data channel.
    ///
    /// @returns true if there is an updated data point received for this data channel, false if there isn't.
    bool updated(void)  { return mUpdated; }

    /// To check if there is valid value received or set from a specified data channel.
    ///
    /// @returns true if there is a valid value received or set for this data channel, false if there isn't.
    bool valid(void)    { return mInited; }

    /// \cond PRIVATE
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
    /// \endcond
};

/// \cond PRIVATE
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
/// \endcond
/* ------------------------------------------------------------------------ */

/**
    Template base class for all controller data channel
 */
template <typename T> class MCSControllerBase : public MCSDataChannel
{
public:
    /// Instantiate from a channel ID
    /// Note that the mValue object is initialized with default constructor.
    MCSControllerBase(const String& channel_id):
        MCSDataChannel(channel_id),
        mValue()
    {

    }

    /// \cond PRIVATE
    virtual ~MCSControllerBase(){

    }
    /// \endcond

    /// Get the locally cache value.
    /// If not present, get value from server.
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

    /// Call this method to change the server-side value
    /// of the controller channel.
    /// 
    /// Note that the result of value() does not change
    /// until the server-side value is successfully updated,
    /// and MCSDevice::process() has been called to 
    /// parse the incomping server update.
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
    /// default implementation between String <-> Value
    /// Subclasses such as "HEX format" may need 
    /// to override this method.
    virtual void _valueFromString(const String& param, T& value){
        MCSStringToValue(param, value);
    }

    /// default implementation between String <-> Value
    /// Subclasses such as "HEX format" may need 
    /// to override this method.
    virtual String _valueToString(const T& value){
        return MCSValueToString(value);
    }

    /// most subclasses won't be required to change 
    /// the upload policy
    virtual void _dispatch(const String& params){
        if(_update(params))
            _setUpdated();
    }

    /// parse the MCS string data and assign it
    /// to the mValue member.
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

/// ON/OFF controller channel. The value type is bool.
class MCSControllerOnOff : public MCSControllerBase<bool>
{
public:
    MCSControllerOnOff(const String& channel_id):MCSControllerBase(channel_id){
    }
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

/// Category controller channel. The value type is String.
class MCSControllerCategory : public MCSControllerBase<String>
{
public:
    MCSControllerCategory(const String& channel_id):MCSControllerBase(channel_id){
    }
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

/// Integer controller channel. The value type is int.
class MCSControllerInteger : public MCSControllerBase<int>
{
public:
    MCSControllerInteger(const String& channel_id):MCSControllerBase(channel_id){
    }
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

/// Float controller channel. The value type is float.
class MCSControllerFloat : public MCSControllerBase<float>
{
public:
    MCSControllerFloat(const String& channel_id):MCSControllerBase(channel_id){
    }
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

/// HEX controller channel. The value type is long(integer).
class MCSControllerHex : public MCSControllerBase<long>
{
public:
    MCSControllerHex(const String& channel_id):MCSControllerBase(channel_id){
    }
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

/// String controller channel. The value type is String.
class MCSControllerString : public MCSControllerBase<String>
{
public:
    MCSControllerString(const String& channel_id):MCSControllerBase(channel_id){
    }
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

/// Value type for MCSControllerGPS.
struct MCSGPSValue : public Printable{

    /// Default constructor of MCSGPSValue.
    MCSGPSValue():
        mLatitude(360.f),
        mLongitude(360.f),
        mAltitude(0.f){
    }

    bool operator==(const MCSGPSValue& rhs)const;
    bool operator!=(const MCSGPSValue& rhs)const;
    explicit operator bool()const;
    bool isValid()const;

    /// Implement Printable interface to support
    /// ~~~{.cpp}
    /// Serial.println(MCSGPSValue());
    /// ~~~
    virtual size_t printTo(Print& p) const;

    float mLatitude;    ///<  ranges from -90.f ~ 90.f, see https://mcs.mediatek.com/resources/zh-TW/latest/api_references/
    float mLongitude;   ///<  ranges from -180.f ~ 180.f, see https://mcs.mediatek.com/resources/zh-TW/latest/api_references/
    float mAltitude;    ///<  see https://mcs.mediatek.com/resources/zh-TW/latest/api_references/
};

inline String MCSValueToString(const MCSGPSValue& value)
{
    const String payload = String(value.mLatitude)+String(",")+String(value.mLongitude)+String(",")+String(value.mAltitude);
    return payload;
}

inline void MCSStringToValue(const String& params, MCSGPSValue& value)
{
    // parse the string value
    const int c1 = params.indexOf(',');
    const int c2 = params.indexOf(',', c1+1);
    value.mLatitude = params.substring(0, c1).toFloat();
    value.mLongitude = params.substring(c1+1, c2).toFloat();
    value.mAltitude = params.substring(c2+1).toFloat();
}

/// GPS Controller channel. Value type is MCSGPSValue.
class MCSControllerGPS : public MCSControllerBase<MCSGPSValue>
{
public:
    MCSControllerGPS(const String& channel_id):MCSControllerBase(channel_id){
    }

    /// Helper method that returns mLatitude field in mValue.
    float latitude(void);

    /// Helper method that returns mLongitude field in mValue.
    float longitude(void);

    /// Helper method that returns mAltitude field in mValue.
    float altitude(void);

    /// Helper method to get all fields from mValue, which is of type
    /// MCSGPSValue.
    void getGPSValue(float& latitude, float& longitude, float& altitude);
    
    /// Helper method to set server value directly without a MCSGPSValue.
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
    MCSControllerGPIO(const String& channel_id):MCSControllerBase(channel_id){
    }
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

/// Value type for MCSControllerPWM.
struct MCSPWMValue : public Printable{
    MCSPWMValue():
        mDutyCycle(0),
        mPeriod(0){
    }

    bool operator==(const MCSPWMValue& rhs)const;
    bool operator!=(const MCSPWMValue& rhs)const;
    explicit operator bool()const;
    bool isValid()const;
    virtual size_t printTo(Print& p) const;

    // https://mcs.mediatek.com/resources/zh-TW/latest/api_references/
    int mDutyCycle;  ///< the "value" in MCS API
    int mPeriod;     ///< the "period" in MCS API
};

inline String MCSValueToString(const MCSPWMValue& value)
{
    const String payload = String(value.mDutyCycle)+String(",")+String(value.mPeriod);
    return payload;
}

inline void MCSStringToValue(const String& params, MCSPWMValue& value)
{
    const int c = params.indexOf(',');
    value.mDutyCycle = params.substring(0, c).toInt();
    value.mPeriod = params.substring(c+1).toInt();
    return;
}

class MCSControllerPWM : public MCSControllerBase<MCSPWMValue>
{
public:
    MCSControllerPWM(const String& channel_id):MCSControllerBase(channel_id){
    }

    int dutyCycle(void);
    int period(void);
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
    MCSControllerAnalog(const String& channel_id):MCSControllerBase(channel_id){
    }
};

/* ------------------------------------------------------------------------ */
enum MCSGamePadButton{
    BTN_UP = 1,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_A,
    BTN_B,
    BTN_INVALID
};

enum MCSGamePadButtonEvent{
    BTN_PRESSED = 1,
    BTN_RELEASED = 0,
    BTN_NO_EVENT = -1
};

struct MCSGamePadValue : public Printable
{
    MCSGamePadButton button;           // The button of the update
    MCSGamePadButtonEvent event;    // The event associated with the button

    MCSGamePadValue():
        button(BTN_INVALID),
        event(BTN_NO_EVENT)
    {
    }

    bool operator==(const MCSGamePadValue& rhs)const;
    bool operator!=(const MCSGamePadValue& rhs)const;
    explicit operator bool()const;
    bool isValid()const;

    String toString() const;
    
    /// Implement Printable interface to support
    /// ~~~{.cpp}
    /// Serial.println(MCSGPSValue());
    /// ~~~
    virtual size_t printTo(Print& p) const;
};

String MCSValueToString(const MCSGamePadValue& value);

void MCSStringToValue(const String& params, MCSGamePadValue& value);

class MCSControllerGamePad : public MCSControllerBase<MCSGamePadValue>
{
public:
    MCSControllerGamePad(const String& channel_id):MCSControllerBase(channel_id){
    }

    inline MCSGamePadButton button()
    {
        return value().button;
    }

    inline MCSGamePadButtonEvent event()
    {
        return value().event;
    }
};

#endif
