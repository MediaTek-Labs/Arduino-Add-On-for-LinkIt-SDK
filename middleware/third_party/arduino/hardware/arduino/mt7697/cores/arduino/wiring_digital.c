#include "constants.h"		// Just for some macros, such as INPUT
#include "pins_arduino.h"
#include "wiring_digital.h"

#include <hal_gpio.h>

void pinMode(uint32_t pin, uint32_t mode)
{
	pin_desc_t  *pin_desc = NULL;

	pin_desc = get_arduino_pin_desc(pin);

	if (pin_desc == NULL)
		return;

	if (!pin_enable_digital(pin_desc))
		return;

	if (hal_gpio_init(pin_desc->pin_no) != HAL_GPIO_STATUS_OK)
		return;

	switch (mode) {
	case INPUT:
		hal_gpio_set_direction(pin_desc->pin_no, HAL_GPIO_DIRECTION_INPUT);
		break;

	case OUTPUT:
		hal_gpio_set_direction(pin_desc->pin_no, HAL_GPIO_DIRECTION_OUTPUT);
		break;

	case INPUT_PULLUP:
		hal_gpio_set_direction(pin_desc->pin_no, HAL_GPIO_DIRECTION_INPUT);
		hal_gpio_pull_up(pin_desc->pin_no);
		break;
	}

	return ;
}

void digitalWrite(uint32_t pin, uint32_t value)
{
	pin_desc_t  *pin_desc = NULL;

	pin_desc = get_arduino_pin_desc(pin);

	if (pin_desc == NULL)
		return;

	hal_gpio_set_output(pin_desc->pin_no, value);

	return;
}

int  digitalRead(uint32_t pin)
{
	pin_desc_t		*pin_desc = NULL;

	hal_gpio_data_t		value = 0;
	hal_gpio_direction_t	dir;

	pin_desc = get_arduino_pin_desc(pin);

	if (pin_desc == NULL)
		return value;

	if (hal_gpio_get_direction(pin_desc->pin_no,&dir) != HAL_GPIO_STATUS_OK)
		return value;

	if (dir ==  HAL_GPIO_DIRECTION_OUTPUT)
		hal_gpio_get_output(pin_desc->pin_no, &value);
	else
		hal_gpio_get_input(pin_desc->pin_no, &value);

	return value;
}
