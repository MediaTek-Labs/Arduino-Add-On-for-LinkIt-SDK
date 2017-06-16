#include "variant.h"
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "adapter_layer.h"

#ifdef FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#endif

static void debug_task_stack_usage(void)
{
#if 0
	if(xTaskGetCurrentTaskHandle())
	{
		static UBaseType_t maxStackUsage = 0;
		const UBaseType_t stackUsageWord = uxTaskGetStackHighWaterMark(xTaskGetCurrentTaskHandle());
		if(stackUsageWord > maxStackUsage)
		{
			maxStackUsage = stackUsageWord;
			Serial.print("remaining stack quota=");
			// convert word to byte
			Serial.println(maxStackUsage * 4);
		}
	}
#endif
}

static void arduino_task(void *args)
{
	setup();
	debug_task_stack_usage();

	while (1) {
		loop();
		if (serialEventRun) serialEventRun();

		debug_task_stack_usage();	
	}
	return ;
}

void init(void)
{
	init_system();

#ifdef FREERTOS
	// Create a Task to run arduino application
	xTaskCreate(arduino_task, "arduino_task", 2048, NULL, 1, NULL);
#endif
}

void post_init(void)
{
#ifdef FREERTOS
	vTaskStartScheduler();
#else
	arduino_task(NULL);
#endif
}

static int _wifi_ready = 0;
void set_wifi_ready()
{
	_wifi_ready = 1;
}

bool wifi_ready()
{
	return (_wifi_ready > 0);
}


#ifdef __cplusplus
}
#endif
