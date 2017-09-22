#ifndef __PIN_MUX_H__
#define __PIN_MUX_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <hal_platform.h>

typedef struct pin_desc
{
	hal_gpio_pin_t	pin_no;

	// The pin_mux_aon_sel register value for function selection
	uint8_t		pin_mux_aon_sel_gpio;	// GPIO_TOPAON
	uint8_t		pin_mux_aon_sel_adc;	// ADC
	uint8_t		pin_mux_aon_sel_pwm;	// PWM
	uint8_t		pin_mux_aon_sel_uart;	// UART related
	uint8_t		pin_mux_aon_sel_i2c;	// I2C
	uint8_t		pin_mux_aon_sel_spi;	// SPI

	// For pin extra info
	uint8_t		pin_info_eint_num;
	uint8_t		pin_info_pwm_channel;

	uint8_t		pin_current_function;
} pin_desc_t;

#define	PIN_DESC(gpio_no, sel_gpio, sel_adc, sel_pwm, sel_uart, sel_i2c, sel_spi, eint_num, pwm_ch)\
	{						\
		.pin_no			= gpio_no,	\
		.pin_mux_aon_sel_gpio	= sel_gpio,	\
		.pin_mux_aon_sel_adc	= sel_adc,	\
		.pin_mux_aon_sel_pwm	= sel_pwm,	\
		.pin_mux_aon_sel_uart	= sel_uart,	\
		.pin_mux_aon_sel_i2c	= sel_i2c,	\
		.pin_mux_aon_sel_spi	= sel_spi,	\
		.pin_info_eint_num	= eint_num,	\
		.pin_info_pwm_channel	= pwm_ch,	\
		.pin_current_function = 0xff,	\
	}

// TODO: SPI is TBD.
#define	PIN_DESC_0	PIN_DESC(HAL_GPIO_0,  8, -1, 9, 7, -1, -1,  0, HAL_PWM_0)  // UART0_RTS
#define	PIN_DESC_1	PIN_DESC(HAL_GPIO_1,  8, -1, 9, 7, -1, -1,  1, HAL_PWM_1)  // UART0_CTS
#define	PIN_DESC_2	PIN_DESC(HAL_GPIO_2,  8, -1, 9, 7, -1, -1, -1, HAL_PWM_23) // UART0_RX
#define	PIN_DESC_3	PIN_DESC(HAL_GPIO_3,  8, -1, 9, 7, -1, -1,  2, HAL_PWM_24) // UART0_TX
#define	PIN_DESC_4	PIN_DESC(HAL_GPIO_4,  8, -1, 9,-1, -1, -1,  3, HAL_PWM_2)
#define	PIN_DESC_5	PIN_DESC(HAL_GPIO_5,  8, -1, 9,-1, -1, -1,  4, HAL_PWM_3)
#define	PIN_DESC_6	PIN_DESC(HAL_GPIO_6,  8, -1, 9,-1, -1, -1,  5, HAL_PWM_4)
#define	PIN_DESC_7	PIN_DESC(HAL_GPIO_7,  8, -1, 9,-1, -1, -1,  6, HAL_PWM_5)

#define	PIN_DESC_24	PIN_DESC(HAL_GPIO_24, 8, -1, 9,-1,  4, -1, -1, HAL_PWM_25) // I2C1 CLK
#define	PIN_DESC_25	PIN_DESC(HAL_GPIO_25, 8, -1, 9,-1,  4, -1, -1, HAL_PWM_26) // I2C1 DATA
#define	PIN_DESC_26	PIN_DESC(HAL_GPIO_26, 8, -1, 9,-1, -1, -1, -1, HAL_PWM_27)
#define	PIN_DESC_27	PIN_DESC(HAL_GPIO_27, 8, -1, 9,-1,  4, -1, -1, HAL_PWM_28) // I2C0 CLK
#define	PIN_DESC_28	PIN_DESC(HAL_GPIO_28, 8, -1, 9,-1,  4, -1, -1, HAL_PWM_29) // I2C0 DATA
#define	PIN_DESC_29	PIN_DESC(HAL_GPIO_29, 8, -1, 9,-1, -1,  7, -1, HAL_PWM_30)
#define	PIN_DESC_30	PIN_DESC(HAL_GPIO_30, 8, -1, 9,-1, -1,  7, -1, HAL_PWM_31)
#define	PIN_DESC_31	PIN_DESC(HAL_GPIO_31, 8, -1, 9,-1, -1,  7, -1, HAL_PWM_32)
#define	PIN_DESC_32	PIN_DESC(HAL_GPIO_32, 8, -1, 9,-1, -1,  8, -1, HAL_PWM_33)
#define	PIN_DESC_33	PIN_DESC(HAL_GPIO_33, 8, -1, 9,-1, -1, -1, -1, HAL_PWM_34)
#define	PIN_DESC_34	PIN_DESC(HAL_GPIO_34, 8, -1, 9,-1, -1, -1, -1, HAL_PWM_35)
#define	PIN_DESC_35	PIN_DESC(HAL_GPIO_35, 8, -1, 9, 7, -1, -1, 19, HAL_PWM_18) // UART DBG TX
#define	PIN_DESC_36	PIN_DESC(HAL_GPIO_36, 8, -1, 9, 7, -1, -1, -1, HAL_PWM_19) // UART1 RX
#define	PIN_DESC_37	PIN_DESC(HAL_GPIO_37, 8, -1, 9, 7, -1, -1, 20, HAL_PWM_20) // UART1 TX
#define	PIN_DESC_38	PIN_DESC(HAL_GPIO_38, 8, -1, 9, 7, -1, -1, 21, HAL_PWM_21) // UART1 RTS
#define	PIN_DESC_39	PIN_DESC(HAL_GPIO_39, 8, -1, 9, 7, -1, -1, 22, HAL_PWM_22) // UART1 CTS

// Select GPIO mode when enable ADC
#define	PIN_DESC_57	PIN_DESC(HAL_GPIO_57, 8,  8, 9,-1, -1, -1, -1, HAL_PWM_36) // ADC_IN0
#define	PIN_DESC_58	PIN_DESC(HAL_GPIO_58, 8,  8, 9,-1, -1, -1, -1, HAL_PWM_37) // ADC_IN1
#define	PIN_DESC_59	PIN_DESC(HAL_GPIO_59, 8,  8, 9,-1, -1, -1, -1, HAL_PWM_38) // ADC_IN2
#define	PIN_DESC_60	PIN_DESC(HAL_GPIO_60, 8,  8, 9,-1, -1, -1, -1, HAL_PWM_39) // ADC_IN3

extern pin_desc_t *get_pin_desc(hal_gpio_pin_t pin_no);

// =============================================================================
// For the pin_mux_aon_sel value verification

static inline bool pin_mux_aon_sel_is_valid(uint8_t pin_mux_aon_sel)
{
// By the datasheet, we define the following
#define PIN_MUX_AON_SEL_BITS			4
#define PIN_MUX_AON_SEL_MASK			((1<<PIN_MUX_AON_SEL_BITS) - 1)
#define PIN_MUX_AON_SEL_MAX			9
#define PIN_MUX_AON_SEL_INVALID			0xFF

	if (pin_mux_aon_sel == PIN_MUX_AON_SEL_INVALID)
		return false;

	if (pin_mux_aon_sel & (~ PIN_MUX_AON_SEL_MASK))
		return false;

	if (pin_mux_aon_sel > PIN_MUX_AON_SEL_MAX)
		return false;

	return true;

#undef PIN_MUX_AON_SEL_BITS
#undef PIN_MUX_AON_SEL_MASK
#undef PIN_MUX_AON_SEL_MAX
#undef PIN_MUX_AON_SEL_INVALID
}

// =============================================================================
// For test the pin if it has the function

static inline bool pin_has_gpio(pin_desc_t *pin)
{
	return pin_mux_aon_sel_is_valid(pin->pin_mux_aon_sel_gpio);
}

static inline bool pin_has_adc(pin_desc_t *pin)
{
	return pin_mux_aon_sel_is_valid(pin->pin_mux_aon_sel_adc);
}

static inline bool pin_has_pwm(pin_desc_t *pin)
{
	return pin_mux_aon_sel_is_valid(pin->pin_mux_aon_sel_pwm);
}

static inline bool pin_has_uart(pin_desc_t *pin)
{
	return pin_mux_aon_sel_is_valid(pin->pin_mux_aon_sel_uart);
}

static inline bool pin_has_i2c(pin_desc_t *pin)
{
	return pin_mux_aon_sel_is_valid(pin->pin_mux_aon_sel_i2c);
}

static inline bool pin_has_spi(pin_desc_t *pin)
{
	return pin_mux_aon_sel_is_valid(pin->pin_mux_aon_sel_spi);
}

extern bool pin_has_eint(pin_desc_t *pin);

// =============================================================================
// Enable one function of a multi-function pin

extern bool pin_enable_digital(pin_desc_t *pin);
extern bool pin_enable_analog(pin_desc_t *pin);
extern bool pin_enable_pwm(pin_desc_t *pin);
extern bool pin_enable_uart(pin_desc_t *pin);
extern bool pin_enable_i2c(pin_desc_t *pin);
extern bool pin_enable_spi(pin_desc_t *pin);

// =============================================================================
// Get a pin extra information:
// Applying the following interfaces to get a pin extra information is a very
// good, strong recommendation.

static inline int pin_get_eint_num(pin_desc_t *pin)
{
	return pin->pin_info_eint_num;
}

static inline int pin_get_adc_channel(pin_desc_t *pin)
{
#define ADC_CHANNEL_BASE_PIN_NUM	HAL_GPIO_57
	if (!pin_has_adc(pin))
		return -1;

	return pin->pin_no - ADC_CHANNEL_BASE_PIN_NUM;
#undef  ADC_CHANNEL_BASE_PIN_NUM
}

static inline int pin_get_pwm_channel(pin_desc_t *pin)
{
	return (int8_t)pin->pin_info_pwm_channel;
}

#ifdef __cplusplus
}
#endif

#endif
