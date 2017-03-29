#ifndef __ADAPTER_LAYER_H__
#define __ADAPTER_LAYER_H__

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/
#include <stdbool.h>	/* For true or false */

#ifdef __cplusplus
extern "C"{
#endif // start of __cplusplus

#define FREERTOS				// FIXME: It's temporary to stay here!!

#define PWM_SOURCE_CLOCK          HAL_PWM_CLOCK_2MHZ
#define PWM_SOURCE_CLOCK_VALUE    (2*1000*1000)

extern int __io_putchar(int ch);
extern int __io_getchar(void);

extern void init_system(void);

extern void init_bt_subsys(void);

#ifdef __cplusplus
}
#endif // end of __cplusplus

#endif /* __ADAPTER_LAYER_H__ */
