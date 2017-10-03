#include <LRTC.h>

// USR button pin is P6
const int usr_btn = 6;
bool reset_time = false;

// when the USR button is pressed, prepare to reset the time
void pin_change(void)
{
  reset_time = true;
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  // activate the USR button handling
  attachInterrupt(usr_btn, pin_change, RISING);

  Serial.println("RTC test");

  // start the RTC module
  LRTC.begin();
}

void loop() {
  char buffer[64];

  // check if the time needs to be reset
  if (reset_time)
  {
    // set the time to 2017/10/2 16:10:30
    LRTC.set(2017, 10, 2, 16, 10, 30);
    reset_time = false;
  }

  // get time from the RTC module
  LRTC.get();

  // display the time
  sprintf(buffer, "%ld/%ld/%ld %.2ld:%.2ld:%.2ld",
    LRTC.year(), LRTC.month(), LRTC.day(), LRTC.hour(), LRTC.minute(), LRTC.second());

  Serial.println(buffer);

  // blink LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}

