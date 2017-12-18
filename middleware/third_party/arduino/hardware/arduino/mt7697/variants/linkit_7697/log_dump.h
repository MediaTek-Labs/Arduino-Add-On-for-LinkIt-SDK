#ifndef __LOG_DUMP_H__
#define __LOG_DUMP_H__

#include "hal_feature_config.h"

#ifndef MTK_HAL_NO_LOG_ENABLE

#include <stdio.h>
#include <hal_platform.h>
#include <hal_uart.h>

#define ENABLE_STDIO_TTY				HAL_UART_0

#ifdef __cplusplus
extern "C" {
#endif
#include <syslog.h>
#ifdef __cplusplus
}
#endif

#define pr_debug(format, arg...)				\
	do{LOG_I(common, "ARD:"format, ##arg);}while(0);

#else
#define pr_debug(format, arg...)
#endif

#endif
