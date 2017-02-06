#include <stdint.h>

#include <system_mt7687.h>
#include <top.h>
#include <hal_gpio.h>
#include <hal_platform.h>

#include "adapter_layer.h"
#include "variant.h"

#include "log_dump.h"

/*
 * For The Serial
 *
 */
#ifdef ENABLE_STDIO_TTY
static bool hal_uart_enable_pinmux(hal_uart_port_t port)
{
	if (port == HAL_UART_0) {
		/* Set Pinmux to UART0  */
		hal_pinmux_set_function(HAL_GPIO_0, 7);		/* RTS */
		hal_pinmux_set_function(HAL_GPIO_1, 7);		/* CTS */
		hal_pinmux_set_function(HAL_GPIO_2, 7);		/* RX  */
		hal_pinmux_set_function(HAL_GPIO_3, 7);		/* TX  */

		return true;
	}

	if (port == HAL_UART_1) {
		/* Set Pinmux to UART0  */
		hal_pinmux_set_function(HAL_GPIO_36, 7);	/* RX  */
		hal_pinmux_set_function(HAL_GPIO_37, 7);	/* TX  */
		hal_pinmux_set_function(HAL_GPIO_38, 7);	/* RTS */
		hal_pinmux_set_function(HAL_GPIO_39, 7);	/* CTS */

		return true;
	}

	return false;
}

static bool init_stdio()
{
	hal_uart_status_t ret;
	hal_uart_config_t config = {
		.baudrate	= HAL_UART_BAUDRATE_115200,
		.word_length	= HAL_UART_WORD_LENGTH_8,
		.stop_bit	= HAL_UART_STOP_BIT_1,
		.parity		= HAL_UART_PARITY_NONE,
	};

	if (!hal_uart_enable_pinmux(ENABLE_STDIO_TTY))
		return false;

	ret = hal_uart_init(ENABLE_STDIO_TTY, &config);

	if (ret == HAL_UART_STATUS_OK)
		return true;

	/* FIXME Maybe here should blink the LED to hint error. */
	return false;
}

int __io_putchar(int ch)
{
	hal_uart_put_char(ENABLE_STDIO_TTY, ch);

	return ch;
}

int __io_getchar(void)
{
	// FIXME: TBD.
	return 0;
}
#else
static bool init_stdio()
{
	return false;
}

int __io_putchar(int ch)
{
	return ch;
}

int __io_getchar(void)
{
	// FIXME: TBD.
	return 0;
}
#endif

/*
 * For The System
 *
 */

static void init_sys_clk(void)
{
	top_xtal_init();

#if F_CPU == MCU_FREQUENCY_192MHZ
	cmnCpuClkConfigureTo192M();
#elif F_CPU == MCU_FREQUENCY_160MHZ
	cmnCpuClkConfigureTo160M();
#elif F_CPU == MCU_FREQUENCY_64MHZ
	cmnCpuClkConfigureTo64M();
#else
#error F_CPU should be one of MCU_FREQUENCY_192MHZ, MCU_FREQUENCY_160MHZ and MCU_FREQUENCY_64MHZ
#endif
	/* Enable flash clock to 64MHz */
	cmnSerialFlashClkConfTo64M();
}

void init_system(void)
{
	init_sys_clk();

	/* Init the UART for stdio */
	init_stdio();

    hal_flash_init();
}
