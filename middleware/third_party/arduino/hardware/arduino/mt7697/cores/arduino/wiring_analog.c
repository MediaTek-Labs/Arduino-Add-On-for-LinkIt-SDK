/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifdef __cplusplus
extern "C" {
#endif

#include "wiring_analog.h"
#include "pins_arduino.h"
#include "delay.h"
#include <hal_adc.h>
#include <hal_pwm.h>
#include <log_dump.h>

static int _readResolution = 12; // MT7687 ADC: 12 Bit

void analogReadResolution(int res) { _readResolution = res; }

eAnalogReference analog_reference = AR_DEFAULT;

void analogReference(eAnalogReference ulMode) { analog_reference = ulMode; }

static inline uint32_t mapResolution(uint32_t value, uint32_t from,
                                     uint32_t to) {
  if (from == to)
    return value;
  if (from > to)
    return value >> (from - to);
  else
    return value << (to - from);
}

uint32_t analogRead(uint32_t ulPin) {
  pin_desc_t *pin_desc = NULL;
  uint32_t ulValue = 0;
  int adc_channel = -1;

  /*
   * FIXME: Currently, hal_adc_init() couldn't take a pin number as a
   * parameter. So, has_init isn't array currently. In theory, it should
   * be has_init[MAX_ADC_CHANNELS];
   */
  static bool has_init = false;

  // Note that Arduino UNO's implementation accepts both
  // pin # and ADC channel number. so we make a detection here:
  if (ulPin < HAL_ADC_CHANNEL_MAX) {
    // user gave use channel number, e.g.
    // ```analogRead(0);```
    adc_channel = (int)ulPin;
  } else {
    // user gave use pin number, e.g.
    // ```analogRead(A0)```
    //
    // In fact, not need to pinmux adc
    // This is only judge it whether has adc function
    pin_desc = get_arduino_pin_desc(ulPin);
    if (pin_desc == NULL)
      return -1;

    if (!pin_enable_analog(pin_desc))
      return -1;

    adc_channel = pin_get_adc_channel(pin_desc);
  }

  if (-1 == adc_channel)
    return -1;

  if (!has_init) {
    hal_adc_init();
    has_init = true;
  }

  hal_adc_get_data_polling(adc_channel, &ulValue);

  ulValue = mapResolution(ulValue, 12, _readResolution);

  /*
   * NOTE: If a Pin has configured as ADC Channel, it shouldn't used as
   * other function. So deinit is not necessary.
   * hal_adc_deinit();
   */

  return ulValue;
}

static uint32_t _writeResolution = 8;

void analogWriteResolution(uint32_t res) { _writeResolution = res; }

#define PWM_SRC_CLOCK HAL_PWM_CLOCK_40MHZ
#define PWM_FREQUENCY 1024 // 400KHz

void analogWrite(uint32_t ulPin, uint32_t ulValue) {
  pin_desc_t *pin_desc = NULL;
  static uint32_t total_count = 0;
  hal_pwm_running_status_t status = -1;
  uint8_t pwm_channel;

  pin_desc = get_arduino_pin_desc(ulPin);
  if (pin_desc == NULL)
    return;

  pwm_channel = pin_desc->pin_info_pwm_channel;

  if (ulValue > (uint32_t)((1 << _writeResolution) - 1))
    ulValue = (1 << _writeResolution) - 1;

  if (HAL_PWM_STATUS_OK != hal_pwm_get_running_status(pwm_channel, &status)) {
    // If fail to get PWM state,
    // we assume PWM is not initialized -> set to IDLE.
    // switch to PWM mode
    status = HAL_PWM_IDLE;
  }

  if (HAL_PWM_IDLE == status) {
    /* Set Pinmux to pwm mode */
    if (!pin_enable_pwm(pin_desc)) {
      pr_debug("pin_enable_pwm failed -> analogWrite failed\n");
      return;
    }

    if (HAL_PWM_STATUS_OK !=
        hal_pwm_set_frequency(pwm_channel, PWM_FREQUENCY, &total_count))
      return; /* hal_pwm_set_frequency fail */

    ulValue = ulValue * total_count / ((1 << _writeResolution) - 1);

    if (HAL_PWM_STATUS_OK != hal_pwm_set_duty_cycle(pwm_channel, ulValue))
      return; /* hal_pwm_set_duty_cycle fail */

    if (HAL_PWM_STATUS_OK != hal_pwm_start(pwm_channel))
      return; /* hal_pwm_start fail */

  } else if (HAL_PWM_BUSY == status) {
    ulValue = ulValue * total_count / ((1 << _writeResolution) - 1);

    if (HAL_PWM_STATUS_OK != hal_pwm_set_duty_cycle(pwm_channel, ulValue))
      return; /* hal_pwm_set_duty_cycle fail */
  }
}

#ifdef __cplusplus
}
#endif
