/*
  Arduino.h - Main include file for the Arduino SDK
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

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

#ifndef Arduino_h
#define Arduino_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "binary.h"

// For CPP Header
// ====== For Some Peripheral ==================================================
#include "UARTClass.h"
#include "Tone.h"
#include "wiring_pulse.h"
// ====== For some Common Operations ===========================================
#include "WMath.h"		// Some math operations for Arduino App

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

// ====== For Compiler Specific Symbols ========================================
#if defined	( __CC_ARM   )		/* Keil uVision 4 */
	#define __ASM			__asm
	#define __INLINE		__inline
	#define WEAK			(__attribute__ ((weak)))
#elif defined	( __ICCARM__ )		/* IAR Ewarm 5.41+ */
	#define __ASM			__asm
	#define __INLINE		inline
	#define WEAK			__weak
#elif defined	(  __GNUC__  )		/* GCC */
	#define __ASM			__asm
	#define __INLINE		inline
	#define WEAK			__attribute__ ((weak))
#endif

// ====== For Common Macro and definition ======================================
#include "constants.h"
#include "WCharacter.h"

// ====== For Variant ==========================================================
#include "variant.h"
#include "pins_arduino.h"

// ====== For Sketch ===========================================================
extern void setup( void ) ;
extern void loop( void ) ;

// ====== For Some Peripheral ==================================================
#include "delay.h"
#include "wiring_digital.h"
#include "wiring_interrupt.h"
#include "wiring_analog.h"
#include "wiring_shift.h"

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // Arduino_h
