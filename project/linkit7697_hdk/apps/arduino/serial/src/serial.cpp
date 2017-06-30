#include "Arduino.h"

int led = 7;
int val = 0;

void setup() {
	Serial.begin(115200);
	pinMode(led, OUTPUT);

	Serial.println("finished setup!");
}

void loop() {
	Serial.println(val, 16);
	digitalWrite(led, val&0x1);


	if (Serial.available()) {
		val = Serial.read();
	} else {
		val++;
	}

	delay(1000);
}
