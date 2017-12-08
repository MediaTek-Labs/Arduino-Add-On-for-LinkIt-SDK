#include "constants.h"		// Just for some macros, such as INPUT
#include "cmsis.h"
#include "pin_mux.h"
#include "pins_arduino.h"
#include "wiring_interrupt.h"

#include <hal_eint.h>

static void default_hal_eint_callback(void *arduino_callback)
{
	//digitalWrite(5, 1);
	voidFuncPtr callback = arduino_callback;

	if (callback != NULL)
		callback();
}

void attachInterrupt(uint32_t pin, voidFuncPtr callback, uint32_t mode) {
	// by default we set the debounce time to 50 ms
	return attachInterruptWithDebounce(pin, callback, mode, 50);
}

void attachInterruptWithDebounce(uint32_t pin, voidFuncPtr callback, uint32_t mode, uint32_t debounceTime)
{
	static hal_eint_trigger_mode_t mode_to_eint_trigger[] = {
		[LOW]     = HAL_EINT_LEVEL_LOW,
		[HIGH]    = HAL_EINT_LEVEL_HIGH,
		[CHANGE]  = HAL_EINT_EDGE_FALLING_AND_RISING,
		[FALLING] = HAL_EINT_EDGE_FALLING,
		[RISING]  = HAL_EINT_EDGE_RISING
	};

	hal_eint_config_t	eint_config;
	pin_desc_t		*pin_desc = NULL;
	hal_eint_status_t	ret;

	pin_desc = get_arduino_pin_desc(pin);

	if (pin_desc == NULL)
		goto FAIL;

	if (!pin_has_eint(pin_desc))
		goto FAIL;

	if (mode>= sizeof(mode_to_eint_trigger)/sizeof(hal_eint_trigger_mode_t))
		goto FAIL;

	eint_config.trigger_mode  = mode_to_eint_trigger[mode];
	eint_config.debounce_time = debounceTime; // debounce time unit: ms

	ret = hal_eint_init(pin_get_eint_num(pin_desc), &eint_config);

	if (ret != HAL_EINT_STATUS_OK)
		goto FAIL;

	ret = hal_eint_register_callback(pin_get_eint_num(pin_desc),
			default_hal_eint_callback, callback);

	if (ret == HAL_EINT_STATUS_OK)
		return;

FAIL:
	/*
	 * FIXME: Maybe some debug log should be at here.
	 * return false;
	 */
	return ;
}

void detachInterrupt(uint32_t pin)
{
	pin_desc_t		*pin_desc = NULL;
	hal_eint_status_t	ret;

	pin_desc = get_arduino_pin_desc(pin);

	if (pin_desc == NULL)
		goto FAIL;

	if (!pin_has_eint(pin_desc))
		goto FAIL;

	ret = hal_eint_register_callback(pin_get_eint_num(pin_desc), NULL,NULL);

	if (ret == HAL_EINT_STATUS_OK)
		return;
FAIL:
	/*
	 * FIXME: Maybe some debug log should be at here.
	 * return false;
	 */
	return ;
}

void interrupts(void)
{
	__enable_irq();
}

void noInterrupts(void)
{
	__disable_irq();
}
