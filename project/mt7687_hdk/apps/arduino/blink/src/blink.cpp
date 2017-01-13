#include "Arduino.h"

int led = 5;
int val = 0;

void setup() {
	pinMode(led, OUTPUT);
}

void loop() {
	digitalWrite(led, val);

	val = !val;

	delay(1000);
}
