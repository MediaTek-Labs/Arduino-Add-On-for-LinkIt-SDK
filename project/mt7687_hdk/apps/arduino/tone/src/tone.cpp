#include "Arduino.h"

int led = 7;
int val = 0;

int key;
unsigned int analog_val = 350;

uint32_t freq;
uint32_t duty_cycle;

uint32_t total_cnt;

void setup() {
	Serial.begin(115200);
	pinMode(led, OUTPUT);

	tone(8, analog_val);

	tone(9, analog_val, 30000);

	Serial.println("finished setup!");
}

void loop() {
	if (Serial.available()) {
		key = Serial.read();

		if (key == 'a') {
			analog_val += 1;
		} else if (key == 'd') {
			analog_val -= 1;
		} else if (key == 'A') {
			analog_val += 10;
		} else if (key == 'D') {
			analog_val -= 10;
		} else if (key == 'w') {
			analog_val += 100;
		} else if (key == 's') {
			analog_val -= 100;
		}

		tone(8, analog_val);

		tone(9, analog_val, 30000);
	}

	Serial.println(analog_val, 10);

	digitalWrite(led, val&0x1);
	val++;

	delay(1000);
}
