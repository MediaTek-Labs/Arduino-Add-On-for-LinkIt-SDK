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

extern int __io_putchar(int ch);
extern int __io_getchar(void);

extern void init_system(void);

#ifdef __cplusplus
}
#endif // end of __cplusplus

#endif /* __ADAPTER_LAYER_H__ */
