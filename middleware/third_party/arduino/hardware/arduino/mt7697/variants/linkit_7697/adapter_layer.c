#include <stdint.h>

#include <system_mt7687.h>
#include <top.h>
#include <hal_gpio.h>
#include <hal_platform.h>
#include <syslog.h>
#include <FreeRTOS.h>
#include <task.h>
#include <bt_system.h>

#include "task_def.h"

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

#if 0
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

void init_system(void)
{
	init_sys_clk();

	/* Init the UART for stdio */
	init_stdio();

    hal_flash_init();

#if 0
    log_uart_init(HAL_UART_0);
    log_init(NULL, NULL, syslog_control_blocks);
#endif
}

/*
 *	For Bluetooth LE, copied from iot_sdk_demo::bt_init.c
 */
/* max supported connection number */
#define BT_CONNECTION_MAX   16

/* max timer count */
#define BT_TIMER_NUM 10

#define BT_TX_BUF_SIZE 256
#define BT_RX_BUF_SIZE 1024

#define BT_TIMER_BUF_SIZE (BT_TIMER_NUM * BT_CONTROL_BLOCK_SIZE_OF_TIMER)
#define BT_CONNECTION_BUF_SIZE (BT_CONNECTION_MAX* BT_CONTROL_BLOCK_SIZE_OF_LE_CONNECTION)

BT_ALIGNMENT4(
static char timer_cb_buf[BT_TIMER_BUF_SIZE]//one timer control block is 20 bytes
);
BT_ALIGNMENT4(
static char connection_cb_buf[BT_CONNECTION_BUF_SIZE]
);

BT_ALIGNMENT4(
static char tx_buf[BT_TX_BUF_SIZE]
);

BT_ALIGNMENT4(
static char rx_buf[BT_RX_BUF_SIZE]
);
extern void bt_task(void * arg);

static bt_bd_addr_t local_public_addr;

static void bt_preread_local_address(bt_bd_addr_t addr)
{
	// TODO: where should we store BT Mac Address?
	addr[0] = 0x00;
	addr[1] = 0x7e;
	addr[2] = 0x56;
	addr[3] = 0x50;
	addr[4] = 0x6e;
	addr[5] = 0xa0;
}

void bt_mm_init()
{
    bt_memory_init_packet(BT_MEMORY_TX_BUFFER, tx_buf, BT_TX_BUF_SIZE);
    bt_memory_init_packet(BT_MEMORY_RX_BUFFER, rx_buf, BT_RX_BUF_SIZE);
    bt_memory_init_control_block(BT_MEMORY_CONTROL_BLOCK_TIMER, timer_cb_buf, BT_TIMER_BUF_SIZE);
    bt_memory_init_control_block(BT_MEMORY_CONTROL_BLOCK_LE_CONNECTION, connection_cb_buf,
                                  BT_CONNECTION_BUF_SIZE);
}

extern void init_bt_subsys(void)
{
    bt_mm_init();
    bt_preread_local_address(local_public_addr);

    #if 0
    LOG_I(common, "[BT]local_public_addr [%02X:%02X:%02X:%02X:%02X:%02X]\n", local_public_addr[5],
          local_public_addr[4], local_public_addr[3], local_public_addr[2], local_public_addr[1], local_public_addr[0]);
    log_config_print_switch(BT, DEBUG_LOG_ON);
    log_config_print_switch(BTMM, DEBUG_LOG_OFF);
    log_config_print_switch(BTHCI, DEBUG_LOG_ON);
    log_config_print_switch(BTL2CAP, DEBUG_LOG_ON);
    #endif
    
    if (pdPASS != xTaskCreate(bt_task, BLUETOOTH_TASK_NAME, BLUETOOTH_TASK_STACKSIZE/sizeof(StackType_t), (void *)local_public_addr, BLUETOOTH_TASK_PRIO, NULL)) {
        LOG_E(common, "cannot create bt_task.");
    }
}