#include <LWiFi.h>
#include <WiFiClient.h>
#include "MCS.h"

// Assign AP ssid / password here
#define _SSID "your_ssid"
#define _KEY  "your_password"

// Assign device id / key of your test device, the device should have 2 on/off data channel
MCSDevice mcs("your_device_id", "your_device_key");
// Assign channel id 
MCSDataChannelSwitch led("your_channel1_id");
MCSDataChannelSwitch btn("your_channel1_id");

#define LED_PIN 7
#define BTN_PIN 6

void setup() {
  // setup Serial output at 9600
  Serial.begin(9600);

  // setup LED/Button pin
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT);

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
  mcs.addChannel(&led);
  mcs.addChannel(&btn);
  while(!mcs.connected())
  {
    Serial.println("MCS.connect()...");
    mcs.connect();
  }
  Serial.println("MCS connected !!");
}

void loop() {
  // call process() to allow background processing, add timeout to avoid high cpu usage
  Serial.print("process(");
  Serial.print(millis());
  Serial.println(")");
  mcs.process(1000);
  
  // updated flag will be cleared in process(), user must check it after process() call.
  if(led.updated())
  {
    Serial.print("LED updated, new value = ");
    Serial.println(led.value());
    digitalWrite(LED_PIN, led.value() ? HIGH : LOW);
    if(!btn.set(led.value()))
    {
      Serial.print("Failed to update button, current value = ");
      Serial.println(btn.value());
    }
  }
  
  // check if need to re-connect
  while(!mcs.connected())
    mcs.connect();
}
