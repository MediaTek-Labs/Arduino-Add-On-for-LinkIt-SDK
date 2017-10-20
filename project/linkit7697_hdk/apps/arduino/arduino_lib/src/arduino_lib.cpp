#include "Arduino.h"
#include <LWiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <fota.h>

void setup() {
	Serial.begin(115200);
	Serial.println("Hello");
	Serial.println("Setup done");
    fota_trigger_update();
}

void loop() {
	delay(1);
}
