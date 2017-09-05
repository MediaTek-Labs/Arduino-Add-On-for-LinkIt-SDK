#ifndef __LREMOTE_H__

#include <LBLE.h>
#include <LBLEPeriphral.h>
#include <vector>

// Background / primary color of the UI control
enum RCColorType {
    RC_ORANGE = 1,
    RC_YELLOW,
    RC_BLUE,
    RC_GREEN,
    RC_PINK,
    RC_GREY,
};

// Set desired view orientation for your UI
enum RCOrientation {
    RC_PORTRAIT = 0,
    RC_LANDSCAPE = 1
};

// Internal event from BLE devce
enum RCControlEvent {
    RC_BTNDOWN = 1,
    RC_BTNUP,
    RC_VALUECHANGE,
};

// Internal Defines the type of the UI control
enum RCControlType {
    RC_LABEL = 1,
    RC_PUSHBUTTON = 2,
    RC_CIRCLEBUTTON = 3,
    RC_SWITCHBUTTON = 4,
    RC_SLIDER = 5,
};

// Internal data structure for BLE events
struct RCEventInfo {
    uint8_t seq;
    uint8_t event;
    uint16_t data;
};

// Internal structure for storing config data.
// the semantic is up to each subclass to interpret.
struct RCConfigData {
    uint16_t data1;
    uint16_t data2;
    uint16_t data3;
    uint16_t data4;
};

class LRemoteUIControl {
protected:
    LRemoteUIControl() :
        m_x(0),
        m_y(0),
        m_row(1),
        m_col(1),
        m_color(),
        m_type(),
        m_text(),
        m_eventSeq(0)
    {
        memset(&m_lastEvent, sizeof(m_lastEvent), 0);
    }
public:
    void setType(RCControlType type) {
        m_type = type;
    }

    void setPos(uint8_t x, uint8_t y) {
        m_x = x;
        m_y = y;
    }

    void setSize(uint8_t r, uint8_t c) {
        m_row = r;
        m_col = c;
    }

    void setColor(RCColorType color) {
        m_color = color;
    }

    void setText(const String& text) {
        m_text = text;
    }

public:

    // Each subclass of controls should be responsible
    // for filling this config data according
    // to their own definitions. The corresponding
    // mobile app counterpart (LinkIt Remote)
    // should decode the config data in the same fashion
    virtual void fillConfigData(RCConfigData &info) {
        // by default, we simply zero clear the config data
        memset(&info, 0, sizeof(info));
        return;
    }

protected:
    const RCEventInfo& getLastEvent() {
        consumeEvent();
        return m_lastEvent;
    }

    bool hasEvent() {
        return m_lastEvent.seq != m_eventSeq;
    }

    // after calling this, hasEvent() returns false,
    // unless a new event has arrived from BLE.
    void consumeEvent() {
        m_eventSeq = m_lastEvent.seq;
    }

public:
    // UI Control Properties
    uint8_t m_x;
    uint8_t m_y;
    uint8_t m_row;
    uint8_t m_col;
    RCColorType m_color;
    RCControlType m_type;
    String m_text;

    // Event sent from the Remote app
    RCEventInfo m_lastEvent;

    // Our "processed" event sequence ID
    uint8_t m_eventSeq;
};

class LRemoteLabel : public LRemoteUIControl {
public:
    LRemoteLabel() : LRemoteUIControl() {
        setType(RC_LABEL);
    }
};

class LRemoteButton : public LRemoteUIControl {
public:
    LRemoteButton() : LRemoteUIControl() {
        setType(RC_PUSHBUTTON);
    }

    bool isValueChanged() {
        return hasEvent();
    }

    // returns 1 if button is currently pressed
    // returns 0 if button is not pressed
    uint16_t getValue() {
        consumeEvent();
        return m_lastEvent.data;
    }
};

class LRemoteCircleButton : public LRemoteButton {
public:
    LRemoteCircleButton() : LRemoteButton() {
        setType(RC_CIRCLEBUTTON);
    }
};

class LRemoteSwitch : public LRemoteUIControl {
public:
    LRemoteSwitch() : LRemoteUIControl() {
        setType(RC_SWITCHBUTTON);
    }

    bool isValueChanged() {
        return hasEvent();
    }

    // returns 1 if switched on
    // returns 0 if switched off
    uint16_t getValue() {
        consumeEvent();
        return m_lastEvent.data;
    }
};

class LRemoteSlider : public LRemoteUIControl {
public:
    LRemoteSlider() : 
        LRemoteUIControl(),
        m_minValue(0),
        m_maxValue(100),
        m_initValue(0)
    {
        setType(RC_SLIDER);
    }

    bool isValueChanged() {
        return hasEvent();
    }

    // returns value according to remote slider value
    int16_t getValue() {
        consumeEvent();
        return static_cast<int16_t>(m_lastEvent.data);
    }

    // configures the value range for the slider
    void setValueRange(int16_t min, int16_t max, int16_t initialValue) {
        m_minValue = min;
        m_maxValue = max;
        m_initValue = initialValue;
        return;
    }

    virtual void fillConfigData(RCConfigData &info) {
        // data1 : min value
        // data2 : max value
        // data3 : initial value
        // data4 : reserved
        info.data1 = (uint16_t)m_minValue;
        info.data2 = (uint16_t)m_maxValue;
        info.data3 = (uint16_t)m_initValue;
        info.data4 = 0;
        return;
    }

protected:
    int16_t m_minValue;
    int16_t m_maxValue;
    int16_t m_initValue;
};

class LRemoteClass {
public:
    LRemoteClass():
        m_canvasColumn(1),
        m_canvasRow(1),
        m_orientation(RC_PORTRAIT),
        m_deviceName("LinkIt 7697")
    {

    }

    // Configure name of this device.
    // This will appear in the device list
    // of the LinkIt Remote app.
    void setName(const String& name) {
        m_deviceName = name;
    }

    // Set the canvas grid layout of the 
    void setGrid(int column, int row) {
        m_canvasRow = row;
        m_canvasColumn = column;
    }

    void addControl(LRemoteUIControl& control)
    {
        m_controls.push_back(&control);
    }

    void setOrientation(RCOrientation orientation) {
        m_orientation = orientation;
    }

    // Initializes a BLE peripheral
    // and start advertisement
    void begin();

    // Check if the LinkIt Remote app
    // has connected
    bool connected();

    // Process event sent from the
    // LinkIt Remote app.
    void process();

protected:
    String m_deviceName;
    std::vector<LRemoteUIControl*> m_controls;
    int m_canvasRow;
    int m_canvasColumn;
    RCOrientation m_orientation;
};

extern LRemoteClass LRemote;
#endif