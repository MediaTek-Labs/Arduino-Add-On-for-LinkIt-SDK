#ifndef __LREMOTE_H__

#include <LBLE.h>
#include <LBLEPeriphral.h>
#include <vector>

// Defines the type of the UI control
enum RCControlType {
    RC_LABEL = 1,
    RC_PUSHBUTTON = 2,
    RC_CIRCLEBUTTON = 3,
    RC_SWITCHBUTTON = 4,
    RC_SLIDER = 5,
};

// Background / primary color of the UI control
enum RCColorType {
    RC_GOLD = 1,
    RC_YELLOW,
    RC_BLUE,
    RC_GREEN,
    RC_PINK,
    RC_GREY,
};

// Event
enum RCControlEvent {
    RC_BTNDOWN = 1,
    RC_BTNUP,
    RC_VALUECHANGE,
};

struct RCEventInfo {
    uint8_t event;
    uint8_t data;
    uint8_t reserved;
    uint8_t seq;
};

class LRemoteUIControl {
public:
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

enum RemoteButtonEvent {
    BTN_NO_EVENT = 0,
    BTN_DOWN = 1,
    BTN_UP = 2
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

    uint8_t getValue() {
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

    uint8_t getValue() {
        consumeEvent();
        return m_lastEvent.data;
    }
};

class LRemoteSlider : public LRemoteUIControl {
public:
    LRemoteSlider() : LRemoteUIControl() {
        setType(RC_SLIDER);
    }

    bool isValueChanged() {
        return hasEvent();
    }

    uint8_t getValue() {
        consumeEvent();
        return m_lastEvent.data;
    }
};

class LRemoteClass {
public:

    void setName(const String& name) {
        m_deviceName = name;
    }

    void setGrid(int column, int row) {
        m_canvasRow = row;
        m_canvasColumn = column;
    }

    void addControl(LRemoteUIControl& control)
    {
        m_controls.push_back(&control);
    }

    void begin();

    void process();

protected:
    String m_deviceName;
    std::vector<LRemoteUIControl*> m_controls;
    int m_canvasRow;
    int m_canvasColumn;
};

extern LRemoteClass LRemote;
#endif