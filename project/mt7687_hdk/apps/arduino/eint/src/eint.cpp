#include "Arduino.h"

int led = 7;
int val = 0;

void pin_change(void)
{
	digitalWrite(led, val);
	val = !val;
}


void setup() {
	pinMode(led, OUTPUT);
	attachInterrupt(4, pin_change, CHANGE);
}

void loop() {
	delay(1000);
}
