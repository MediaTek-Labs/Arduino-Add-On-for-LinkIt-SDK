#include "Arduino.h"
#include "LFlash.h"
#include <string.h>

#define BUFFER_LENGTH   (4)

#define SECTION_NAME    "MONITOR"
#define PROPERTY_NAME   "BTN_CLICK"

#define LED_PIN         (7)
#define BUTTON_PIN      (6)

static uint32_t _count = 0;

void show_message(uint32_t times)
{
    Serial.print("USR button has been pressed for ");
    Serial.print(times);
    Serial.print(" time");

    if (times < 2)
    {
        Serial.println(".");
    }
    else
    {
        Serial.println("s.");
    }
}

void button_press(void)
{
    _count++;

    LFlash.write(
        SECTION_NAME,
        PROPERTY_NAME,
        LFLASH_RAW_DATA,
        (const uint8_t *)&_count,
        sizeof(_count));

    show_message(_count);
}

void setup() {
    uint32_t size = BUFFER_LENGTH;

    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    if (LFlash.begin() != LFLASH_OK)
    {
        Serial.println("Flash init failed.");
        return;
    }

    // flash init done and okay
    digitalWrite(LED_PIN, LOW);

    attachInterrupt(BUTTON_PIN, button_press, RISING);

    LFlash.read(SECTION_NAME, PROPERTY_NAME, (uint8_t *)&_count, &size);
    show_message(_count);
}

void loop() {
    delay(1000);
}