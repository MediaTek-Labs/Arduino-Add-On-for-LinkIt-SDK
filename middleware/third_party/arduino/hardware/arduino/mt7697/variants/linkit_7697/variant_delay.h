#ifndef __VARIANT_DELAY_H__
#define __VARIANT_DELAY_H__

#ifdef __cplusplus
extern "C"{
#endif // start of __cplusplus

#include <stdint.h>
#include <sys/time.h>

extern uint32_t variant_millis(void);
extern void variant_delay(uint32_t ms);

extern int variant_gettimeofday(struct timeval *tv, void *ptz);

#ifdef __cplusplus
}
#endif // end of __cplusplus

#endif /* __VARIANT_DELAY_H__ */
