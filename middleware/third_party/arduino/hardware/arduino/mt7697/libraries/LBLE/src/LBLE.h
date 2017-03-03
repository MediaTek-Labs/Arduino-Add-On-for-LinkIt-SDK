/*
  WiFi.h - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef LBLE_H
#define LBLE_H

#include <inttypes.h>

extern "C" {

}

class LBLEClass
{
	private:

	public:

		LBLEClass();

		/* Initializes the Bluetooth subsystem.
		 * This should be the called first prior to using other BLE APIs.
		 * After calling begin() you need to call ready(),
		 * and check if the subsystem is ready to use.
		 */
		int begin();

		/* Returns 0 when BLE subsystem is not ready to use.
		 * Returns 1 when it is ready to use.
		 */
		int ready();


		friend class LBLECentral;
		friend class LBLEPeripheral;
};

extern LBLEClass LBLE;

#endif
