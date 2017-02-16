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

extern "C" {
#include <string.h>
}

#include <Arduino.h>
#include <constants.h>
#include <pin_mux.h>
#include <arduino_pins.h>
#include <hal_platform.h>
#include "hal_i2c_master.h"

#include "Wire.h"

extern "C" {
#include "log_dump.h"
#include "delay.h"
}

#define I2C_PORT_IDLE				0x5555
#define I2C_PORT_BUSY				0xAAAA

static volatile uint32_t i2c_port0_state = I2C_PORT_IDLE;
static void i2c_port0_irq_hanlder(uint8_t slave_address, hal_i2c_callback_event_t event, void *user_data)
{
	i2c_port0_state = I2C_PORT_IDLE;
}

static void i2c_port0_wait_idel(void)
{
	while (i2c_port0_state != I2C_PORT_IDLE) {
		;
	}
}

static void i2c_port0_set_busy(void)
{
	i2c_port0_state = I2C_PORT_BUSY;
}

static bool i2c_port0_is_busy(void)
{
	return i2c_port0_state == I2C_PORT_BUSY;
}

TwoWire::TwoWire(uint8_t i2c_port)
{
	this->i2c_port = i2c_port;

	transmissionBegun = false;
}

void TwoWire::begin(void) {
	// Master Mode
	pin_desc_t	*pin_desc;
	hal_i2c_config_t i2c_init;

	if ((hal_i2c_port_t)i2c_port == HAL_I2C_MASTER_0) {
		pin_desc = get_arduino_pin_desc(SCL);
		if (NULL == pin_desc)
			return ;

		if (!pin_enable_i2c(pin_desc))
			return ;

		pin_desc = get_arduino_pin_desc(SDA);
		if (NULL == pin_desc)
			return ;

		if (!pin_enable_i2c(pin_desc))
			return ;

		i2c_init.frequency = HAL_I2C_FREQUENCY_400K;

		if (hal_i2c_master_init((hal_i2c_port_t)i2c_port, &i2c_init)
				!= HAL_I2C_STATUS_OK)
			return;

		/* Register callback function */
		if (hal_i2c_master_register_callback((hal_i2c_port_t)i2c_port,
					i2c_port0_irq_hanlder, NULL)
				!= HAL_I2C_STATUS_OK)
			return;
	}
}

void TwoWire::setClock(uint32_t baudrate) {
	i2c_port0_wait_idel();

	switch (baudrate) {
		case 400000:
			hal_i2c_master_set_frequency((hal_i2c_port_t)i2c_port,
					HAL_I2C_FREQUENCY_400K);
			break;

		case 200000:
			hal_i2c_master_set_frequency((hal_i2c_port_t)i2c_port,
					HAL_I2C_FREQUENCY_200K);
			break;

		case 100000:
			hal_i2c_master_set_frequency((hal_i2c_port_t)i2c_port,
					HAL_I2C_FREQUENCY_100K);
			break;

		case  50000:
			hal_i2c_master_set_frequency((hal_i2c_port_t)i2c_port,
					HAL_I2C_FREQUENCY_50K);
			break;
	}

	return;
}

void TwoWire::end() {
	hal_i2c_master_deinit((hal_i2c_port_t)i2c_port);
}

uint8_t TwoWire::requestFrom(uint8_t address, size_t quantity, bool stopBit)
{
	if (quantity == 0)
		return 0;

	if (!stopBit)
		return 0;

	i2c_port0_wait_idel();

	rxBuffer.clear();
	hal_i2c_master_receive_dma((hal_i2c_port_t)i2c_port, address,
			(uint8_t *)(rxBuffer._aucBuffer),
			(uint32_t)quantity);


	i2c_port0_set_busy();
	i2c_port0_wait_idel();
	rxBuffer._iHead = quantity;

	return rxBuffer.available();
}

uint8_t TwoWire::requestFrom(uint8_t address, size_t quantity)
{
	return requestFrom(address, quantity, true);
}

void TwoWire::beginTransmission(uint8_t address) {
	// save address of target and clear buffer
	txAddress = address;
	txBuffer.clear();

	transmissionBegun = true;
}

// Errors:
//  0 : Success
//  1 : Data too long
//  2 : NACK on transmit of address
//  3 : NACK on transmit of data
//  4 : Other error
uint8_t TwoWire::endTransmission(bool stopBit)
{
	if (!stopBit)
		return 4;

	transmissionBegun = false ;

	/*
	 * If there is nothing in the txBuffer, that is to say, it needn't to
	 * start transmisstion action.
	 */
	if (txBuffer.available() == 0)
		return 0;

	i2c_port0_wait_idel();
	i2c_port0_set_busy();

	if (HAL_I2C_STATUS_OK  != hal_i2c_master_send_dma((hal_i2c_port_t)i2c_port, txAddress,
			(uint8_t *)(txBuffer._aucBuffer),
			(uint32_t)txBuffer.available()))
	{
		// TODO: assert here? it's unlikely to fail here.
	}

	// in Arduino, the endTransmission() should be return AFTER the transmission completes
	i2c_port0_wait_idel();

	return 0;
}

uint8_t TwoWire::endTransmission()
{
	return endTransmission(true);
}

size_t TwoWire::write(uint8_t ucData)
{
	// Store the data that will be send into the txBuffer

	// Return 0 if transmission hasn't been begun or the txBuffer is full.
	if ( !transmissionBegun || txBuffer.isFull()) {
		return 0 ;
	}

	txBuffer.store_char(ucData);
	return 1 ;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
	// Store these data in a array that will be send into the txBuffer

	for (size_t i = 0; i < quantity; ++i) {
		if(!write(data[i]))
			return i;
	}

	// All data stored
	return quantity;
}

int TwoWire::available(void)
{
	return rxBuffer.available();
}

int TwoWire::read(void)
{
	return rxBuffer.read_char();
}

int TwoWire::peek(void)
{
	return rxBuffer.peek();
}

void TwoWire::flush(void)
{
	// Do nothing, use endTransmission(..) to force
	// data transfer.
}

TwoWire Wire(HAL_I2C_MASTER_0);
