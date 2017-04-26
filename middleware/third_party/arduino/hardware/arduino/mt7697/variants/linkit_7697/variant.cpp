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

static void arduino_task(void *args)
{
	setup();

	while (1) {
		loop();
		if (serialEventRun) serialEventRun();
	}
	return ;
}

void init(void)
{
	init_system();

#ifdef FREERTOS
	// Create a Task to run arduino application
	xTaskCreate(arduino_task, "arduino_task", 1024, NULL, 1, NULL);
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
