#include <LWiFi.h>
#include <WiFiClient.h>
#include "MCS.h"

// Assign AP ssid / password here
#define _SSID "your_wifi_ap_ssid"
#define _KEY  "your_wifi_password"

// Assign device id / key of your test device
MCSDevice mcs("your_device_id", "your_device_key");

// Assign channel id 
// The test device should have a channel id "control_gamepad".
// the first channel should be "Controller" - "GamePad"
MCSControllerGamePad gamepad("control_gamepad");

void setup() {
  // setup Serial output at 9600
  Serial.begin(9600);

  // setup Wifi connection
  while(WL_CONNECTED != WiFi.status())
  {
    Serial.print("WiFi.begin(");
    Serial.print(_SSID);
    Serial.print(",");
    Serial.print(_KEY);
    Serial.println(")...");
    WiFi.begin(_SSID, _KEY);
  }
  Serial.println("WiFi connected !!");

  // setup MCS connection
  mcs.addChannel(gamepad);
    
  while(!mcs.connected())
  {
    Serial.println("MCS.connect()...");
    mcs.connect();
  }
  Serial.println("MCS connected !!");

  while(!gamepad.valid())
  {
    Serial.println("initialize gamepad default value...");
    gamepad.value();    
    // Note: At this moment we can "read" the values
    // of the gamepad - but the value is meaningless.
    // 
    // The MCS server returns that "last button pressed" 
    // in this cause - even if the user is not pressing any button
    // at this moment.
    // 
    // We read the values here simply to make the following
    // process() -> if(gamepad.updated()) check working.
  }

}

void loop() {
  // Note that each process consumes 1 command from MCS server.
  // The 100 millisecond timeout assumes that the server
  // won't send command rapidly.
  mcs.process(100);

  // updated flag will be cleared in process(), user must check it after process() call.
  if(gamepad.updated())
  {
    Serial.print("Gamepad event arrived, new value = ");
    Serial.println(gamepad.value());
  }

  // check if need to re-connect
  while(!mcs.connected())
  {
    Serial.println("re-connect to MCS...");
    mcs.connect();
    if(mcs.connected())
      Serial.println("MCS connected !!");
  }
} 
