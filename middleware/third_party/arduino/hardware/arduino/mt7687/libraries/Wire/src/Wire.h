/*
 * TWI/I2C library for Arduino Zero
 * Copyright (c) 2015 Arduino LLC. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __WIRE_H__
#define __WIRE_H__

#include "Stream.h"
#include "variant.h"

#include "RingBuffer.h"
#define BUFFER_LENGTH				SERIAL_BUFFER_SIZE

class TwoWire : public Stream
{
	public:
		TwoWire(uint8_t i2c_port);

		void begin();
#ifdef I2C_SLAVE
		void begin(uint8_t);
#endif

		void end();
		void setClock(uint32_t);

		void    beginTransmission(uint8_t);
		uint8_t endTransmission(bool stopBit);
		uint8_t endTransmission(void);

		uint8_t requestFrom(uint8_t address, size_t quantity, bool stopBit);
		uint8_t requestFrom(uint8_t address, size_t quantity);

		size_t write(uint8_t data);
		size_t write(const uint8_t * data, size_t quantity);

		virtual int available(void);
		virtual int read(void);
		virtual int peek(void);
		virtual void flush(void);
		using Print::write;

#ifdef I2C_SLAVE
		void onService(void);
		void onReceive(void(*)(int));
		void onRequest(void(*)(void));

#else
		void onService(void) {};
		void onReceive(void(*)(int)) {};
		void onRequest(void(*)(void)) {};

#endif

	private:
		uint8_t		i2c_port;

		bool		transmissionBegun;

		// RX Buffer
		RingBuffer	rxBuffer;

		// TX buffer
		RingBuffer	txBuffer;
		uint8_t		txAddress;

#ifdef I2C_SLAVE
		// I2C slave Callback, but MT7687 doesn't support I2C slave.
		void (*onRequestCallback)(void);
		void (*onReceiveCallback)(int);
#else
		void (*onRequestCallback)(void) {};
		void (*onReceiveCallback)(int) {};
#endif
};

extern TwoWire Wire;

#endif
