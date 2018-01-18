#include <stdint.h>

#include <system_mt7687.h>
#include <top.h>
#include <hal_gpio.h>
#include <hal_pwm.h>
#include <hal_flash.h>
#include <flash_map.h>
#include <hal_platform.h>
#include <hal_cache.h>
#include <hal_sleep_manager.h>
#include <syslog.h>
#include <FreeRTOS.h>
#include <task.h>
#include <bt_system.h>

#include "task_def.h"

#include "adapter_layer.h"
#include "variant.h"
#include "log_dump.h"

extern void tickless_init(void);

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

static void init_usr_led(void)
{
	// Make sure the USR LED (GPIO36 / pin 7) is OFF 
	// by system default.
	hal_pinmux_set_function(HAL_GPIO_36, 8);
	hal_gpio_set_direction(HAL_GPIO_36, HAL_GPIO_DIRECTION_OUTPUT);
	hal_gpio_set_output(HAL_GPIO_36, 0);
}

#ifndef MTK_HAL_NO_LOG_ENABLE
LOG_CONTROL_BLOCK_DECLARE(wifi);
LOG_CONTROL_BLOCK_DECLARE(common);
LOG_CONTROL_BLOCK_DECLARE(BT);
LOG_CONTROL_BLOCK_DECLARE(BTMM);
LOG_CONTROL_BLOCK_DECLARE(BTHCI);
LOG_CONTROL_BLOCK_DECLARE(BTL2CAP);

log_control_block_t *syslog_control_blocks[] = {
      &LOG_CONTROL_BLOCK_SYMBOL(wifi),
      &LOG_CONTROL_BLOCK_SYMBOL(common),
	  &LOG_CONTROL_BLOCK_SYMBOL(BT),
	  &LOG_CONTROL_BLOCK_SYMBOL(BTMM),
	  &LOG_CONTROL_BLOCK_SYMBOL(BTHCI),
	  &LOG_CONTROL_BLOCK_SYMBOL(BTL2CAP)
};
#endif

static int32_t cache_enable(hal_cache_size_t cache_size)
{
    hal_cache_region_t region, region_number;
    hal_cache_region_config_t region_cfg_tbl[] = {
        /* cache_region_address, cache_region_size(both MUST be 4k bytes aligned) */
        {
            /* Set FreeRTOS code on XIP flash to cacheable. Please refer to memory layout dev guide for more detail. */
            FLASH_BASE + CM4_CODE_BASE, CM4_CODE_LENGTH
        }
        /* add cache regions below if you have any */
    };
    region_number = (hal_cache_region_t)(sizeof(region_cfg_tbl) / sizeof(region_cfg_tbl[0]));
    if (region_number > HAL_CACHE_REGION_MAX) {
        return -1;
    }
    hal_cache_init();
    /* Set the cache size to 32KB. It will ocuppy the TCM memory size */
    hal_cache_set_size(cache_size);
    for (region = HAL_CACHE_REGION_0; region < region_number; region++) {
        hal_cache_region_config(region, &region_cfg_tbl[region]);
        hal_cache_region_enable(region);
    }
    for (; region < HAL_CACHE_REGION_MAX; region++) {
        hal_cache_region_disable(region);
    }
    hal_cache_enable();
    return 0;
}


void init_system(void)
{
	init_sys_clk();

	cache_enable(HAL_CACHE_SIZE_32KB);

	// ensure the USR LED is default OFF
	init_usr_led();

	/* Init the UART for stdio */
	init_stdio();

    hal_flash_init();

    // Pre-initialize PWM to highest clock.
    // This resets pin status so we put it in init_system().
    hal_pwm_init(PWM_SOURCE_CLOCK);

#ifndef MTK_HAL_NO_LOG_ENABLE
    log_uart_init(HAL_UART_0);
	log_config_print_switch(BTMM, DEBUG_LOG_OFF);
    log_init(NULL, NULL, syslog_control_blocks);
#endif

#if configUSE_TICKLESS_IDLE == 2
    if (hal_sleep_manager_init() == HAL_SLEEP_MANAGER_OK) {
        tickless_init();
        sleepdrv_get_sleep_mode(HAL_SLEEP_MODE_SLEEP);
    }

#endif

}

