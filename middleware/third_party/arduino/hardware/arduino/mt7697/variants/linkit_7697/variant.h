#ifndef __VARIANT_H__
#define __VARIANT_H__

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include <stdbool.h>	/* For true or false */

#ifdef __cplusplus
extern "C"{
#endif // start of __cplusplus

// Note: To avoid to introduce in unnecessary macros and interfaces, redefine
// these following macros then include system_mt7687.h.
// #include <system_mt7687.h>
#define MCU_FREQUENCY_192MHZ			192000000
#define MCU_FREQUENCY_160MHZ			160000000
#define MCU_FREQUENCY_64MHZ			64000000
#define MCU_FREQUENCY_40MHZ			40000000

// Add for the compatibility of some drivers
#define clockCyclesPerMicrosecond() ( 192L ) //192 is Clock Cycle of LinkIt 7697 in MHz
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )

// In the Arduino IDE, the following macro have been defined by Arduino IDE.
// But for LinkIt SDK, we need define them at here.
#ifndef ARDUINO

#define ARDUINO					10801	// FIXME: {runtime.ide.version}
#define ARDUINO_linkit_7697				// FIXME: {build.board} Board Name
#define ARDUINO_ARCH_LINKIT_RTOS		// FIXME: {build.arch}
#define __MT7687__

// F_CPU Options:
// [VALID]   MCU_FREQUENCY_192MHZ
// [VALID]   MCU_FREQUENCY_160MHZ
// [VALID]   MCU_FREQUENCY_64MHZ
// [INVALID] MCU_FREQUENCY_40MHZ
#define F_CPU					MCU_FREQUENCY_192MHZ	// The main clock of MCU work
#endif

#define LED_BUILTIN				7

extern void init(void);
extern void post_init(void);

// Workaround the Wi-Fi double initialization issue:
// As of LinkIt SDK v4.x, we have to
// initializes Wi-Fi to have underlying CONNSYS module's
// firmware code patched before using BLE. 
//
// Thus, LBLE and LWIFI needs to
// communicate with each other to know if Wi-Fi 
// is already initialized.
extern void init_global_connsys();
extern void init_global_connsys_for_ble();
extern bool wifi_ready();

#ifdef __cplusplus
}
#endif // end of __cplusplus

#endif /* __VARIANT_H__ */
