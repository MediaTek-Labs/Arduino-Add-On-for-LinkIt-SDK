#include "variant.h"
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "log_dump.h"
#include "adapter_layer.h"
#ifdef FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#include "wifi_api.h"
#include "ethernetif.h"
#endif

static void debug_task_stack_usage(void) {
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

static void arduino_task(void *args) {
	setup();
	debug_task_stack_usage();

	while (1) {
		loop();
		if (serialEventRun) serialEventRun();

		debug_task_stack_usage();	
	}
	return ;
}

void init(void) {
	init_system();

#ifdef FREERTOS
	// Create a Task to run arduino application
	xTaskCreate(arduino_task, "arduino_task", 2048, NULL, 1, NULL);
#endif
}

void post_init(void) {
#ifdef FREERTOS
	vTaskStartScheduler();
#else
	arduino_task(NULL);
#endif
}

static volatile int _wifi_ready = 0;
static void _set_wifi_ready() {
	_wifi_ready = 1;
}

bool wifi_ready() {
	return (_wifi_ready > 0);
}

static int32_t _wifi_ready_handler(wifi_event_t event,
                                   uint8_t *payload,
                                   uint32_t length) {

    if (event == WIFI_EVENT_IOT_INIT_COMPLETE) {
        pr_debug("WIFI_EVENT_IOT_INIT_COMPLETE received");
        _set_wifi_ready();
    }

    return 0;
}

static void init_global_connsys_impl(int enableWiFi) {
    if(wifi_ready()) {
        if(enableWiFi) {
            pr_debug("[wifi_init] turn on wi-fi");
            wifi_config_set_radio(1);
        }
        return;
    }

    wifi_connection_register_event_handler(WIFI_EVENT_IOT_INIT_COMPLETE , _wifi_ready_handler);
    wifi_config_t config;
    memset(&config, 0, sizeof(config));
    config.opmode = WIFI_MODE_STA_ONLY;
    strcpy((char *)config.sta_config.ssid, (const char *)" ");
    config.sta_config.ssid_length = strlen((const char *)config.sta_config.ssid);

    wifi_config_ext_t ex_config;
    memset(&ex_config, 0, sizeof(ex_config));
    ex_config.sta_auto_connect_present = 1; // validate "sta_auto_connect" config
    ex_config.sta_auto_connect = 0;         // don't auto-connect - we just want to initialize
    ex_config.sta_power_save_mode_present = 1;
    ex_config.sta_power_save_mode = WIFI_POWER_SAVING_MODE_LEGACY;  // try to save power

    pr_debug("[wifi_init] with legacy power saving mode");
    wifi_init(&config, &ex_config);

    // we must initialize lwip_tcpip, otherwise we won't receive WIFI_EVENT_IOT_INIT_COMPLETE
    lwip_tcpip_config_t tcpip_config;
    memset(&tcpip_config, 0, sizeof(tcpip_config));
    lwip_tcpip_init(&tcpip_config, config.opmode);
    
    // block until wifi is ready
    while (!wifi_ready()) {
        delay(10);
    }

    #if 0   // if we turn off wifi, and turn on again, subsequent wifi scan fails (as of v4.6.0)
    if(!enableWiFi) {
        pr_debug("[wifi_init] turn off wi-fi");
    wifi_config_set_radio(0);
    }
    #endif

    return;
}

void init_global_connsys_for_ble() {
    // pass 1 to turn on Wi-Fi when it is already initialized
    init_global_connsys_impl(0);
}


void init_global_connsys() {
    // pass 1 to turn on Wi-Fi when it is already initialized
    init_global_connsys_impl(1);
}

#ifdef __cplusplus
}
#endif
