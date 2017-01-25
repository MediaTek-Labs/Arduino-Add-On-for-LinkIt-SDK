#include "Arduino.h"

int led = 7;
int val = 0;

void setup() {
    Serial.begin(115200);
	pinMode(led, OUTPUT);
}

void loop() {
	digitalWrite(led, val);

	val = !val;

    Serial.println("Blink");

    delay(500);
}
