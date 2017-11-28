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

#ifndef _UART_CLASS_
#define _UART_CLASS_

#include "HardwareSerial.h"
#include "RingBuffer.h"

#include <hal_platform.h>
#include <hal_uart.h>
#include "pins_arduino.h"

#define HARDSER_PARITY_EVEN 	(0x02ul)
#define HARDSER_PARITY_ODD	(0x01ul)
#define HARDSER_PARITY_NONE 	(0x00ul)
#define HARDSER_PARITY_MASK	(0x03ul)

#define HARDSER_STOP_BIT_1	(0x00ul)
#define HARDSER_STOP_BIT_2 	(0x04ul)
#define HARDSER_STOP_BIT_MASK	(0x04ul)

#define HARDSER_DATA_5	 	(0x00ul)
#define HARDSER_DATA_6	 	(0x08ul)
#define HARDSER_DATA_7	 	(0x10ul)
#define HARDSER_DATA_8	 	(0x18ul)
#define HARDSER_DATA_MASK	(0x18ul)

#define SERIAL_5N1		UARTClass::Mode_5N1
#define SERIAL_6N1		UARTClass::Mode_6N1
#define SERIAL_7N1		UARTClass::Mode_7N1
#define SERIAL_8N1		UARTClass::Mode_8N1
#define SERIAL_5N2		UARTClass::Mode_5N2
#define SERIAL_6N2		UARTClass::Mode_6N2
#define SERIAL_7N2		UARTClass::Mode_7N2
#define SERIAL_8N2		UARTClass::Mode_8N2
#define SERIAL_5E1		UARTClass::Mode_5E1
#define SERIAL_6E1		UARTClass::Mode_6E1
#define SERIAL_7E1		UARTClass::Mode_7E1
#define SERIAL_8E1		UARTClass::Mode_8E1
#define SERIAL_5E2		UARTClass::Mode_5E2
#define SERIAL_6E2		UARTClass::Mode_6E2
#define SERIAL_7E2		UARTClass::Mode_7E2
#define SERIAL_8E2		UARTClass::Mode_8E2
#define SERIAL_5O1		UARTClass::Mode_5O1
#define SERIAL_6O1		UARTClass::Mode_6O1
#define SERIAL_7O1		UARTClass::Mode_7O1
#define SERIAL_8O1		UARTClass::Mode_8O1
#define SERIAL_5O2		UARTClass::Mode_5O2
#define SERIAL_6O2		UARTClass::Mode_6O2
#define SERIAL_7O2		UARTClass::Mode_7O2
#define SERIAL_8O2		UARTClass::Mode_8O2

class UARTClass : public HardwareSerial
{
	public:
		enum UARTModes {
			Mode_5N1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_NONE | HARDSER_DATA_5,
			Mode_6N1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_NONE | HARDSER_DATA_6,
			Mode_7N1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_NONE | HARDSER_DATA_7,
			Mode_8N1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_NONE | HARDSER_DATA_8,
			Mode_5N2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_NONE | HARDSER_DATA_5,
			Mode_6N2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_NONE | HARDSER_DATA_6,
			Mode_7N2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_NONE | HARDSER_DATA_7,
			Mode_8N2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_NONE | HARDSER_DATA_8,
			Mode_5E1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_EVEN | HARDSER_DATA_5,
			Mode_6E1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_EVEN | HARDSER_DATA_6,
			Mode_7E1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_EVEN | HARDSER_DATA_7,
			Mode_8E1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_EVEN | HARDSER_DATA_8,
			Mode_5E2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_EVEN | HARDSER_DATA_5,
			Mode_6E2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_EVEN | HARDSER_DATA_6,
			Mode_7E2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_EVEN | HARDSER_DATA_7,
			Mode_8E2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_EVEN | HARDSER_DATA_8,
			Mode_5O1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_ODD  | HARDSER_DATA_5,
			Mode_6O1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_ODD  | HARDSER_DATA_6,
			Mode_7O1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_ODD  | HARDSER_DATA_7,
			Mode_8O1 = HARDSER_STOP_BIT_1 | HARDSER_PARITY_ODD  | HARDSER_DATA_8,
			Mode_5O2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_ODD  | HARDSER_DATA_5,
			Mode_6O2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_ODD  | HARDSER_DATA_6,
			Mode_7O2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_ODD  | HARDSER_DATA_7,
			Mode_8O2 = HARDSER_STOP_BIT_2 | HARDSER_PARITY_ODD  | HARDSER_DATA_8,
		};

		UARTClass(uint32_t dwId);

		void begin(const uint32_t dwBaudRate);
		void begin(const uint32_t dwBaudRate, const UARTModes config);
		void end(void);
		int available(void);
		int peek(void);
		int read(void);
		void flush(void);
		size_t write(const uint8_t c);
		using Print::write; // pull in write(str) and write(buf, size) from Print

		void setInterruptPriority(uint32_t priority);
		uint32_t getInterruptPriority();

		void IrqHandler(hal_uart_callback_event_t event);

		operator bool() { return true; }; // UART always active

	protected:
		void init(const uint32_t dwBaudRate, const uint32_t config);

		hal_uart_port_t	_uart_port;
		arduino_pin_t	_uart_pin_rx;
		arduino_pin_t	_uart_pin_tx;

#define RX_VFIFO_SIZE		64
		uint8_t		_rx_vfifo[RX_VFIFO_SIZE];

#define TX_VFIFO_SIZE		4
		uint8_t		_tx_vfifo[TX_VFIFO_SIZE];

		RingBuffer	_rx_buffer;
};

extern UARTClass Serial;
extern UARTClass Serial1;
#endif // _UART_CLASS_
