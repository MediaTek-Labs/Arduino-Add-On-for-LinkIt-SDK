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

// In the Arduino IDE, the following macro have been defined by Arduino IDE.
// But for LinkIt SDK, we need define them at here.
#ifndef ARDUINO

#define ARDUINO					"1.7.8"	// FIXME: {runtime.ide.version}
#define ARDUINO_LINKIT					// FIXME: {build.board} Board Name
#define ARDUINO_ARCH_CORTEXM4				// FIXME: {build.arch}
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

// Workaround the Wi-Fi double initialization issue
// As of LinkIt SDK v4.2, we have to
// initializes Wi-Fi to have CONNSYS patched before
// using BLE. Thus, LBLE and LWIFI needs to
// communicate with each other to know if Wi-Fi 
// is already initialized.
extern void set_wifi_ready();
extern bool wifi_ready();

#ifdef __cplusplus
}
#endif // end of __cplusplus

#endif /* __VARIANT_H__ */
