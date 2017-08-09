#include <LWatchDog.h>

void setup()
{
    Serial.begin(9600);
    Serial.println("System (re)Booted.");

    // to check if USR button is pressed
    pinMode(6, INPUT);

    // wait for 10 seconds before rebooting
    LWatchDog.begin(10);
    Serial.println("press the USR button, or the system reboot in 10 seconds");
}

void loop()
{
    static uint32_t feedTime = millis();
    static uint32_t lastReportTime = 0;
    // if USR button is pressed, feed the watchdog to prevent reset.
    if (digitalRead(6))
    {
        // feed the dog
        LWatchDog.feed();
        feedTime = millis();
        lastReportTime = 0;
        Serial.print("Watchdog fed.");
    }

    // Show how many seconds we've been staving
    const uint32_t starveTime = millis() - feedTime;
    if(starveTime - lastReportTime > 1000)
    {
        Serial.print("Watchdog has been starving for ");
        Serial.print(starveTime / 1000);
        Serial.println(" seconds...");
        lastReportTime = starveTime;
    }

    delay(100);
}