#ifndef __DELAY_H__
#define __DELAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

extern uint32_t millis(void);
extern uint32_t micros(void);

extern void delayMicroseconds(unsigned int us);

extern void yield(void);

extern void delay(uint32_t ms);

static inline bool time_after(uint32_t a, uint32_t b)
{
	return ((int32_t)((b) - (a)) < 0);
}

#ifdef __cplusplus
}
#endif

#endif
