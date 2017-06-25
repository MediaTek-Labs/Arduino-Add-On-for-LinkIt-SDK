#include <LWiFi.h>
#include <WiFiClient.h>
#include "MCS.h"

// Assign AP ssid / password here
#define _SSID "your_ssid"
#define _KEY  "your_password"

// Assign device id / key of your test device
MCSDevice mcs("your_device_id", "your_device_key");

// Assign channel id 
// The test device should have 1 channel
// the first channel should be "Controller" - "GamePad"
MCSControllerGamePad gamepad("Gamepad");

void setup() {
  // setup Serial output at 9600
  Serial.begin(38400);

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
    Serial.println("read Gamepad value from MCS...");
    gamepad.value();    
  }
  Serial.print("done, Gamepad value = ");
  Serial.println(gamepad.value()); 

}

void loop() {
  // call process() to allow background processing, add timeout to avoid high cpu usage
  Serial.print("process(");
  Serial.print(millis());
  Serial.println(")");
  mcs.process(100); //1000?

  // updated flag will be cleared in process(), user must check it after process() call.
  if(gamepad.updated())
  {
    Serial.print("Gamepad updated, new value = ");
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
