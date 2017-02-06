#include "adapter_layer.h"	// Define the FREERTOS Macro

#ifdef FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#endif


// Get current timestamp, unit: ms
// tick ==> ms
uint32_t variant_millis(void)
{
#ifdef FREERTOS
	return xTaskGetTickCount() * portTICK_RATE_MS;
#else
	// FIXME: TBD
#endif
}

void variant_delay( uint32_t ms )
{
#ifdef FREERTOS
	// The unit of portTICK_RATE_MS is ms/tick.
	// ms / portTICK_RATE_MS ==> xx ticks.
	vTaskDelay( ms / portTICK_RATE_MS );
#else
	// FIXME: TBD
#endif
}

#include <sys/time.h>

int variant_gettimeofday(struct timeval *tv, void *ptz)
{
#ifdef FREERTOS
	int ticks = xTaskGetTickCount();
	if(tv!=NULL) {
		tv->tv_sec = (ticks/1000);
		tv->tv_usec = (ticks%1000)*1000;
		return 0;
	}

	return -1;
#else
	// FIXME: TBD.
	return -1;
#endif
}
