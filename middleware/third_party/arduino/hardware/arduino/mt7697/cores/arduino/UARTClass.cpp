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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hal_platform.h>
#include <hal_uart.h>
#include "pins_arduino.h"
#include "UARTClass.h"
#include "log_dump.h"

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

void uart_dma_irq_cb(hal_uart_callback_event_t event, void *user_data)
{
	UARTClass *p = (UARTClass *)(user_data);

	p->IrqHandler(event);
}

struct uart_info {
	hal_uart_port_t	uart_port;
	arduino_pin_t	uart_pin_rx;
	arduino_pin_t	uart_pin_tx;
};

#define UARTB					0
#define UARTA					1

struct uart_info uart_info_tab[HAL_UART_MAX] = {
	{
		.uart_port	= HAL_UART_0,
		.uart_pin_rx	= RXB,
		.uart_pin_tx	= TXB,
	}, {
		.uart_port	= HAL_UART_1,
		.uart_pin_rx	= RXA,
		.uart_pin_tx	= TXA,
	},
};

static uart_info *get_uart_info(uint32_t id)
{
	if (id >= ((sizeof(uart_info_tab))/(sizeof(uart_info))))
		return NULL;

	return &uart_info_tab[id];
}

static hal_uart_baudrate_t uart_translate_baudrate(const uint32_t baudrate)
{
	switch(baudrate)
	{
		case 300:
			return HAL_UART_BAUDRATE_300;
		case 1200:
			return HAL_UART_BAUDRATE_1200;
		case 2400:
			return HAL_UART_BAUDRATE_2400;
		case 4800:
			return HAL_UART_BAUDRATE_4800;
		case 9600:
			return HAL_UART_BAUDRATE_9600;
		case 19200:
			return HAL_UART_BAUDRATE_19200;
		case 38400:
			return HAL_UART_BAUDRATE_38400;
		case 57600:
			return HAL_UART_BAUDRATE_57600;
		case 115200:
			return HAL_UART_BAUDRATE_115200;
		case 230400:
			return HAL_UART_BAUDRATE_230400;
		case 460800:
			return HAL_UART_BAUDRATE_460800;
		case 921600:
			return HAL_UART_BAUDRATE_921600;
		default:
			return HAL_UART_BAUDRATE_MAX;
	}

	return HAL_UART_BAUDRATE_MAX;
}

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

void serialEvent()		__attribute__((weak));
void serialEvent1()		__attribute__((weak));

void serialEventRun(void)
{
	if (serialEvent && Serial.available())
		serialEvent();

	if (serialEvent1 && Serial1.available())
		serialEvent1();
}

#define ALERT_LENGTH	0
#define THRESHOLD	0

UARTClass::UARTClass( uint32_t dwId)
{
	uart_info *info	= NULL;

	info = get_uart_info(dwId);

	if (NULL != info) {
		_uart_pin_rx	= info->uart_pin_rx;
		_uart_pin_tx	= info->uart_pin_tx;
		_uart_port	= info->uart_port;
	} else {
		_uart_pin_rx	= INVALID_PIN_NO;
		_uart_pin_tx	= INVALID_PIN_NO;
		_uart_port	= hal_uart_port_t(-1);
	}
}

void UARTClass::begin(const uint32_t dwBaudRate)
{
	begin(dwBaudRate, Mode_8N1);
}

void UARTClass::begin(const uint32_t dwBaudRate, const UARTModes config)
{
	init(dwBaudRate, config);
}

void UARTClass::init(const uint32_t dwBaudRate, const uint32_t modeReg)
{
	pin_desc_t  *pin_desc	= NULL;

	pin_desc = get_arduino_pin_desc(_uart_pin_rx);
	if (!pin_enable_uart(pin_desc))
		return ;

	pin_desc = get_arduino_pin_desc(_uart_pin_tx);
	if (!pin_enable_uart(pin_desc))
		return ;

	hal_uart_config_t uart_config;

	uart_config.baudrate	= uart_translate_baudrate(dwBaudRate);
	uart_config.parity	= (hal_uart_parity_t)(modeReg & HARDSER_PARITY_MASK);
	uart_config.stop_bit	= (hal_uart_stop_bit_t)((modeReg & HARDSER_STOP_BIT_MASK) >> 2);
	uart_config.word_length	= (hal_uart_word_length_t)((modeReg & HARDSER_DATA_MASK) >> 3);

	if (HAL_UART_STATUS_OK != hal_uart_init(_uart_port, &uart_config))
		return ;

	hal_uart_dma_config_t dma_config;

	dma_config.receive_vfifo_alert_size	= RX_VFIFO_SIZE/2;
	dma_config.receive_vfifo_buffer		= _rx_vfifo;
	dma_config.receive_vfifo_buffer_size	= RX_VFIFO_SIZE;
	dma_config.receive_vfifo_threshold_size	= RX_VFIFO_SIZE-4;

	dma_config.send_vfifo_buffer		= _tx_vfifo;
	dma_config.send_vfifo_buffer_size	= TX_VFIFO_SIZE;
	dma_config.send_vfifo_threshold_size	= 0;

	if (HAL_UART_STATUS_OK != hal_uart_set_dma(_uart_port, &dma_config))
		return ;

	if (HAL_UART_STATUS_OK != hal_uart_register_callback(_uart_port, uart_dma_irq_cb, this))
		return ;

	// Make sure both ring buffers are initialized back to empty.
	_rx_buffer.clear();
}

void UARTClass::end(void)
{
	// Wait for any outstanding data to be sent
	flush();

	hal_uart_deinit(_uart_port);
}

int UARTClass::available(void)
{
	/*
	 * FIXME
	 * How to deal with the data in the VFIFO
	 * hal_uart_get_available_receive_bytes(_uart_port);
	 */
	int ret = _rx_buffer.available();
	if(ret)
	{
		return ret;
	}
	else
	{
		uint32_t length = 0;
		uint8_t  buffer[RX_VFIFO_SIZE] = {0};

		length = hal_uart_receive_dma(_uart_port, buffer, length);

		if (length > 0) {
			pr_debug("Receive Get: %lu bytes.\r\n", length);

			for (uint32_t i=0; i<length; i++) {
				_rx_buffer.store_char(buffer[i]);
			}
		}
	return _rx_buffer.available();
	}
}

int UARTClass::peek(void)
{
	return _rx_buffer.peek();
}

int UARTClass::read(void)
{
	return _rx_buffer.read_char();
}

void UARTClass::flush(void)
{
	/*
	 * Currently, send data by polling.
	 */
	return ;
}

size_t UARTClass::write(const uint8_t uc_data)
{
	hal_uart_put_char(_uart_port, uc_data);
	return 1;
}

void UARTClass::IrqHandler(hal_uart_callback_event_t event)
{
	if (event == HAL_UART_EVENT_READY_TO_READ)
	{
		uint32_t length;
		uint8_t  buffer[RX_VFIFO_SIZE] = {0};

		length = hal_uart_get_available_receive_bytes(_uart_port);

		if (length > 0) {
			pr_debug("VFIFO Get: %lu bytes.\r\n", length);

			length = hal_uart_receive_dma(_uart_port, buffer, length);
		}

		if (length > 0) {
			pr_debug("Receive Get: %lu bytes.\r\n", length);

			for (uint32_t i=0; i<length; i++) {
				_rx_buffer.store_char(buffer[i]);
			}
		}
	}
}

UARTClass Serial(UARTB);
UARTClass Serial1(UARTA);
