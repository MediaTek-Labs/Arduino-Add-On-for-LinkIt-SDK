#include "Arduino.h"

int led = 7;
int val = 0;

int key;
unsigned int analog_val = 0;

uint32_t freq;
uint32_t duty_cycle;

uint32_t total_cnt;

void setup() {
	Serial.begin(115200);
	pinMode(led, OUTPUT);

	Serial.println("finished setup!");
}

void loop() {
	if (Serial.available()) {
		key = Serial.read();

		if (key == 'a') {
			if (analog_val < 255) analog_val++;
		} else if (key == 'd') {
			if (analog_val > 0) analog_val--;
		} else if (key == 'A') {
			if (analog_val <= 245) analog_val+=10;
		} else if (key == 'D') {
			if (analog_val >= 10) analog_val-=10;
		}
	}

	analogWrite(8, analog_val);

	Serial.println(analog_val, 10);

	digitalWrite(led, val&0x1);
	val++;

	delay(1000);
}
