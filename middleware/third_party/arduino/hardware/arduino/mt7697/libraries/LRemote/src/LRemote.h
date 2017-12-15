#ifndef __LREMOTE_H__

#include <LBLE.h>
#include <LBLEPeriphral.h>
#include <vector>

#define PROTOCOL_VERSION (3) 

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
enum RCOrientation { RC_PORTRAIT = 0, RC_LANDSCAPE = 1 };

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
  uint8_t seq;            // from Mobile, increment sequence serial number
  uint8_t controlIndex;   // from Mobile, index into the control array of the event origin
  uint8_t event;          // from Mobile, event type
  uint8_t processedSeq;   // from Arduino, last processed serial number.
  uint16_t data;          // from Mobile, data of the event
};

// Internal structure for UI value/text update. This is a varialbe-length structure.
struct RCUIUpdateInfo {
  uint8_t controlIndex;   // from Device, index into the control array of the event origin
  uint8_t dataSize;       // from Device, data length in bytes. The type is implicitly defined by the type of the control.
  uint8_t data[1];        // from Device, data of the event. Currently only null-terminated string possible.
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
  LRemoteUIControl()
      : m_x(0), m_y(0), m_row(1), m_col(1), m_color(), m_type(), m_text(),
        m_eventSeq(0) {
    memset(&m_lastEvent, sizeof(m_lastEvent), 0);
  }

public:

  /// Set the UI type of the control
  void setType(RCControlType type) { m_type = type; }

  /// Set the position of the control on the canvas grid.
  /// The upper left corner is (0, 0).
  /// This value must be called before LRemote.begin().
  void setPos(uint8_t x, uint8_t y) {
    m_x = x;
    m_y = y;
  }

  /// Set the row height and column width on the canvas grid.
  /// This value must be called before LRemote.begin().
  void setSize(uint8_t r, uint8_t c) {
    m_row = r;
    m_col = c;
  }

  /// Set the 
  /// This value must be called before LRemote.begin().
  void setColor(RCColorType color) { m_color = color; }

  /// Set the label text of the UI control
  /// This value must be called before LRemote.begin().
  void setText(const String &text) { m_text = text; }

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
  const RCEventInfo &getLastEvent() {
    consumeEvent();
    return m_lastEvent;
  }

  bool hasEvent() { return m_lastEvent.seq != m_eventSeq; }

  // after calling this, hasEvent() returns false,
  // unless a new event has arrived from BLE.
  void consumeEvent() { m_eventSeq = m_lastEvent.seq; }

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

  // the self-referencing index in the parent m_control
  // this is used to send UI update event to mobile app
  uint8_t m_controlIndex; 

  // Our "processed" event sequence ID
  uint8_t m_eventSeq;
};

class LRemoteLabel : public LRemoteUIControl {
public:
  LRemoteLabel() : LRemoteUIControl() { setType(RC_LABEL); }

  /// Change the label text on the mobile app UI.
  /// The maximum length for the `text` String is 15 bytes (excluding termination null byte).
  /// Strings longer than 15 bytes will be truncated.
  void updateText(const String &text);
};

class LRemoteButton : public LRemoteUIControl {
public:
  LRemoteButton() : LRemoteUIControl() { setType(RC_PUSHBUTTON); }

  bool isValueChanged() { return hasEvent(); }

  // returns 1 if button is currently pressed
  // returns 0 if button is not pressed
  uint16_t getValue() {
    consumeEvent();
    return m_lastEvent.data;
  }
};

class LRemoteCircleButton : public LRemoteButton {
public:
  LRemoteCircleButton() : LRemoteButton() { setType(RC_CIRCLEBUTTON); }
};

class LRemoteSwitch : public LRemoteUIControl {
public:
  LRemoteSwitch() : LRemoteUIControl() { setType(RC_SWITCHBUTTON); }

  bool isValueChanged() { return hasEvent(); }

  // returns 1 if switched on
  // returns 0 if switched off
  uint16_t getValue() {
    consumeEvent();
    return m_lastEvent.data;
  }
};

class LRemoteSlider : public LRemoteUIControl {
public:
  LRemoteSlider()
      : LRemoteUIControl(), m_minValue(0), m_maxValue(100), m_initValue(0) {
    setType(RC_SLIDER);
  }

  bool isValueChanged() { return hasEvent(); }

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
  LRemoteClass()
      : m_canvasColumn(1), m_canvasRow(1), m_orientation(RC_PORTRAIT),
        m_deviceName("LinkIt 7697"), m_bleInitialized(false),
        m_peripheralBegan(false) {}

  /// Configure name of this device.
  /// This will appear in the device list of the LinkIt Remote app.
  /// Note that BLE advertisement have length limitation,
  /// so this name may be truncated when being advertised.
  ///
  /// \param name: device name
  void setName(const String &name) { m_deviceName = name; }

  /// Set the canvas grid as a referencing coordinate for control position and
  /// size.
  void setGrid(int column, int row) {
    m_canvasRow = row;
    m_canvasColumn = column;
  }

  /// Add a UI control element to the canvas
  ///
  /// \param control : an instance of the UI control classes, including:
  ///                     - LRemoteButton
  ///                     - LRemoteSlider
  ///                     - LRemoteLabel
  ///                     - LRemoteSwitch
  ///                     - LRemoteCircleButton
  ///         Note that the control instance must have the same life span
  ///         as LRemote - so these instances should be defined in **global
  ///         scope**
  /// \returns none
  void addControl(LRemoteUIControl &control) {
    // prevent duplicated entry
    for (const auto pControl : m_controls) {
      if (pControl == &control) {
        return;
      }
    }
    m_controls.push_back(&control);
  }

  /// Set desired UI orientation for the remote UI.
  ///
  /// \param orientation: can be RC_PORTRAIT or RC_LANDSCAPE.
  ///                     The default value for orientation is portrait.
  void setOrientation(RCOrientation orientation) {
    m_orientation = orientation;
  }

  /// Initialize the underlying BLE device and start advertisement.
  /// This API implicitly calls LBLE.begin()
  void begin();

  /// Update the config value of text label.
  /// This affects the defautl value when the mobile app
  /// re-connects.
  void updateTextLabel();

  /// Disconnect and stop advertising the device
  /// Note that the control info added by addControl() are kept as-is.
  void end();

  /// Remove all controls from the canvas.
  /// Do not call this method after begin() is called.
  void removeAllControl() {
    m_controls.clear();
  }

  /// Check if the LinkIt Remote app has connected
  bool connected();

  /// Process event sent from the LinkIt Remote app.
  /// Call this API periodically in the loop() function.
  /// Failing to call this API will cause incorrect state and value of the UI
  /// controls.
  void process();

protected:
  // The BLE subsystem and GATT server
  // can only initliaze once -
  // so we need flags to ensure they are init exactly once.
  void initBLEOnce();
  void initPeripheralOnce();

protected:
  String m_deviceName;
  std::vector<LRemoteUIControl *> m_controls;
  int m_canvasRow;
  int m_canvasColumn;
  RCOrientation m_orientation;
  bool m_bleInitialized;
  bool m_peripheralBegan;
};

extern LRemoteClass LRemote;
#endif