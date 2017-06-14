#include "Arduino.h"
#include "LTimer.h"

int led = 7;
int val = 0;

LTimer timer0(LTIMER_0);
LTimer timer1(LTIMER_1);

void _callback0(void *usr_data)
{
    val = !val;

    digitalWrite(led, val);
}

void _callback1(void *usr_data)
{
    Serial.println("Hi, I'm Timer1");
}

void setup() {
    pinMode(led, OUTPUT);
    Serial.begin(115200);

    timer0.begin();
    timer1.begin();

    timer0.start(500, LTIMER_REPEAT_MODE, _callback0, NULL);
    timer1.start(250, LTIMER_REPEAT_MODE, _callback1, NULL);
}

void loop() {

}