/*
 Servo.cpp - Interrupt driven Servo library for Arduino using 16 bit timers- Version 2
 Copyright (c) 2009 Michael Margolis.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */



#include <Arduino.h>

#include "Servo.h"
#include "adapter_layer.h"
#include "arduino_pins.h"
#include <hal_platform.h>
#include "hal_gpio.h"
#include "hal_pwm.h"

#define SERVO_MIN() (MIN_PULSE_WIDTH - this->min * 4)  // minimum value in uS for this servo
#define SERVO_MAX() (MAX_PULSE_WIDTH - this->max * 4)  // maximum value in uS for this servo

#define US_TO_TICK(us)    ((us)*PWM_SOURCE_CLOCK_VALUE/(1000*1000))
#define TICK_TO_US(t)     ((t)*(1000*1000)/PWM_SOURCE_CLOCK_VALUE)

/****************** end of static functions ******************************/

Servo::Servo()
{
  pinDesc = NULL;
}

uint8_t Servo::attach(int pin)
{
  return attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

uint8_t Servo::attach(int pin, int min, int max)
{
  uint32_t total_count = 0;
  pin_desc_t  *pin_desc = get_arduino_pin_desc((arduino_pin_t)pin);
  if (pin_desc == NULL)
    return INVALID_SERVO;

  pinDesc = pin_desc;

  min  = (MIN_PULSE_WIDTH - min)/4; //resolution of min/max is 4 uS
  max  = (MAX_PULSE_WIDTH - max)/4;

  hal_gpio_init(pin_desc->pin_no);
  hal_pinmux_set_function(pin_desc->pin_no, pin_desc->pin_mux_aon_sel_pwm);
  if(HAL_PWM_STATUS_OK != hal_pwm_set_frequency((hal_pwm_channel_t)pin_desc->pin_info_pwm_channel, (1000*1000)/REFRESH_INTERVAL, &total_count))
    return INVALID_SERVO-1;
  hal_pwm_set_duty_cycle((hal_pwm_channel_t)pin_desc->pin_info_pwm_channel, US_TO_TICK(DEFAULT_PULSE_WIDTH));
  hal_pwm_start((hal_pwm_channel_t)pin_desc->pin_info_pwm_channel);

  return pin_desc->pin_no;
}

void Servo::detach()
{
  pin_desc_t *pin_desc = (pin_desc_t*)pinDesc;
  hal_pwm_stop((hal_pwm_channel_t)pin_desc->pin_info_pwm_channel);
  pinDesc = NULL;
}

void Servo::write(int value)
{
  if(value < MIN_PULSE_WIDTH)
  {  // treat values less than 544 as angles in degrees (valid values in microseconds are handled as microseconds)
    if(value < 0) value = 0;
    if(value > 180) value = 180;
    value = map(value, 0, 180, SERVO_MIN(),  SERVO_MAX());
  }
  this->writeMicroseconds(value);
}

void Servo::writeMicroseconds(int value)
{
  pin_desc_t *pin_desc = (pin_desc_t*)pinDesc;

  if( value < SERVO_MIN() )          // ensure pulse width is valid
    value = SERVO_MIN();
  else if( value > SERVO_MAX() )
    value = SERVO_MAX();

  hal_pwm_set_duty_cycle((hal_pwm_channel_t)pin_desc->pin_info_pwm_channel, US_TO_TICK(value));
  hal_pwm_start((hal_pwm_channel_t)pin_desc->pin_info_pwm_channel);
}

int Servo::read() // return the value as degrees
{
  return map( this->readMicroseconds()+1, SERVO_MIN(), SERVO_MAX(), 0, 180);
}

int Servo::readMicroseconds()
{
  uint32_t duty_cycle;
  unsigned int pulsewidth;
  pin_desc_t *pin_desc = (pin_desc_t*)pinDesc;

  if(pinDesc)
  {
    hal_pwm_get_duty_cycle((hal_pwm_channel_t)pin_desc->pin_info_pwm_channel, &duty_cycle);
    pulsewidth = TICK_TO_US(duty_cycle);
  }
  else
    pulsewidth  = 0;

  return pulsewidth;
}

bool Servo::attached()
{
  return (pinDesc != NULL);
}



