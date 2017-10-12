#include <Wire.h>
#include <LRTC.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F);

// USR button pin is P6
const int usr_btn = 6;
bool reset_time = false;

// when the USR button is pressed, prepare to reset the time
void pin_change(void)
{
  reset_time = true;
}

void setup()
{
  // activate the USR button handling
  attachInterrupt(usr_btn, pin_change, RISING);
  // start the RTC module
  LRTC.begin();

  // init the 1602 (2 rows / 16 columns) LCD
  lcd.begin(16, 2);
}

void loop()
{
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

  // clear the LCD and set the string direction to left-to-right
  lcd.clear();

  // display the date
  lcd.setCursor(7, 0);
  sprintf(buffer, "%ld/%ld/%ld", LRTC.year(), LRTC.month(), LRTC.day());
  lcd.print(buffer);

  // display the time
  lcd.setCursor(8, 1);
  sprintf(buffer, "%.2ld:%.2ld:%.2ld", LRTC.hour(), LRTC.minute(), LRTC.second());
  lcd.print(buffer);

  delay(1000);
}