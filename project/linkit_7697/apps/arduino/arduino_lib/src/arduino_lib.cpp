#include "Arduino.h"
#include <LWiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

void setup() {
	Serial.begin(115200);
	Serial.println("Hello");
	Serial.println("Setup done");
}

void loop() {
	delay(1);
}
