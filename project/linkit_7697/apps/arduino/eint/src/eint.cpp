#include "Arduino.h"

int led = 7;
int val = 0;

void pin_change(void)
{
	digitalWrite(led, val);
	val = !val;
    Serial.println("high");
}


void setup() {
	pinMode(led, OUTPUT);
	attachInterrupt(6, pin_change, RISING);
    Serial.begin(115200);
}

void loop() {
	delay(1000);
}
