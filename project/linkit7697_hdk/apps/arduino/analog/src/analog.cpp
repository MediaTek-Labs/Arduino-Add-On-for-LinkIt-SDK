#include "Arduino.h"

int led = 7;
int val = 0;
unsigned int analog_val = 0;

void setup() {
	Serial.begin(115200);
	pinMode(led, OUTPUT);

	Serial.println("finished setup!");
}

void loop() {
	analog_val = analogRead(A0);

	analog_val = (analog_val * 2500) / 4095;
	Serial.println(analog_val, 10);

	digitalWrite(led, val&0x1);
	val++;

	delay(1000);
}
