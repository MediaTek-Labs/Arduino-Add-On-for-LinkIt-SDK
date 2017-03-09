/* FreeRTOS headers */
#include <FreeRTOS.h>
#include <task.h>

/* system service headers */
#ifndef MTK_NVDM_ENABLE
#error "LinkIt 7697 Bluetooth relies on NVDM (to read local BT device address")
#endif
// #include "nvdm.h"

/* Bluetooth headers */
#include <task_def.h>
#include "utility/ard_ble.h"

//////////////////////////////////////////////////
// Workaround for CONSYS patch issue

/* wifi related header */
#include "wifi_api.h"
#include "lwip/ip4_addr.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"

static int32_t _wifi_event_handler(wifi_event_t event,
        uint8_t *payload,
        uint32_t length)
{
    struct netif *sta_if;

    //LOG_I(app, "wifi event: %d", event);

    switch(event)
    {
    case WIFI_EVENT_IOT_INIT_COMPLETE:
        //LOG_I(app, "wifi inited complete");
        break;
    }

    return 1;
}

static void _connsys_workaround()
{
    /* Wi-Fi must be initialized for BLE start-up */
    wifi_connection_register_event_handler(WIFI_EVENT_IOT_INIT_COMPLETE , _wifi_event_handler);

    wifi_config_t config = {0};
    config.opmode = WIFI_MODE_STA_ONLY;
    wifi_init(&config, NULL);

    lwip_tcpip_config_t tcpip_config = {{0}, {0}, {0}, {0}, {0}, {0}};
    lwip_tcpip_init(&tcpip_config, WIFI_MODE_STA_ONLY);
}


/*
 *	These initialization routines are based on LinkIt SDK v4, iot_sdk_demo::bt_init.c
 */

// max supported connection number
#define BT_CONNECTION_MAX   16

// max timer count
#define BT_TIMER_NUM 10

// Working buffers
#define BT_TX_BUF_SIZE 256
#define BT_RX_BUF_SIZE 1024
#define BT_TIMER_BUF_SIZE (BT_TIMER_NUM * BT_CONTROL_BLOCK_SIZE_OF_TIMER)
#define BT_CONNECTION_BUF_SIZE (BT_CONNECTION_MAX * BT_CONTROL_BLOCK_SIZE_OF_LE_CONNECTION)

BT_ALIGNMENT4(
static char timer_cb_buf[BT_TIMER_BUF_SIZE] //one timer control block is 20 bytes
);
BT_ALIGNMENT4(
static char connection_cb_buf[BT_CONNECTION_BUF_SIZE]
);
BT_ALIGNMENT4(
static char tx_buf[BT_TX_BUF_SIZE]
);
BT_ALIGNMENT4(
static char rx_buf[BT_RX_BUF_SIZE]
);

// This is should be a unique & requested MAC address for the end product.
// In the case of LinkIt 7697 HDK, the address is provided by the module manufacturer.
// We'll fill this value with the content from nvdm(flash) and efuse.
static bt_bd_addr_t g_local_public_addr = {0};
static void ard_ble_init_public_addr(void)
{
#if 1
	g_local_public_addr[0] = 0x9C;
	g_local_public_addr[1] = 0x65;
	g_local_public_addr[2] = 0xF9;
	g_local_public_addr[3] = 0x1E;
	g_local_public_addr[4] = 0xB2;
	g_local_public_addr[5] = 0xF1;
#else
    uint32_t size = 12;
    uint8_t buffer[18] = {0};
    uint8_t tmp_buf[3] = {0};

    // Public device address (MAC address) should be stored in NVDM
    // It is stored as HEX strings - so we read and convert them.
    if (NVDM_STATUS_OK == nvdm_read_data_item("BT", "address", buffer, &size))
    {
        for (i = 0; i < 6; ++i) {
            tmp_buf[0] = buffer[2 * i];
            tmp_buf[1] = buffer[2 * i + 1];
            g_local_public_addr[i] = (uint8_t)strtoul((char *)tmp_buf, NULL, 16);
        }
    }
#endif
}

static void ard_ble_bt_mm_init()
{
    bt_memory_init_packet(BT_MEMORY_TX_BUFFER, tx_buf, BT_TX_BUF_SIZE);
    bt_memory_init_packet(BT_MEMORY_RX_BUFFER, rx_buf, BT_RX_BUF_SIZE);
    bt_memory_init_control_block(BT_MEMORY_CONTROL_BLOCK_TIMER, timer_cb_buf, BT_TIMER_BUF_SIZE);
    bt_memory_init_control_block(BT_MEMORY_CONTROL_BLOCK_LE_CONNECTION, connection_cb_buf,
                                  BT_CONNECTION_BUF_SIZE);
}

// `bt_task` is a helper task that provided by the liblinkit.a
// that we can simply start it.
extern void bt_task(void * arg);

int ard_ble_begin(void)
{
    // We need to patch CONNSYS driver first
    _connsys_workaround();

	ard_ble_init_public_addr();
    ard_ble_bt_mm_init();
    
    #if 0
    LOG_I(common, "[BT]g_local_public_addr [%02X:%02X:%02X:%02X:%02X:%02X]\n", g_local_public_addr[5],
          g_local_public_addr[4], g_local_public_addr[3], g_local_public_addr[2], g_local_public_addr[1], g_local_public_addr[0]);
    log_config_print_switch(BT, DEBUG_LOG_ON);
    log_config_print_switch(BTMM, DEBUG_LOG_OFF);
    log_config_print_switch(BTHCI, DEBUG_LOG_ON);
    log_config_print_switch(BTL2CAP, DEBUG_LOG_ON);
    #endif
    
    if (pdPASS != xTaskCreate(bt_task, 
    						  BLUETOOTH_TASK_NAME, 
    						  BLUETOOTH_TASK_STACKSIZE/sizeof(StackType_t), 
    						  (void *)g_local_public_addr, 
    						  BLUETOOTH_TASK_PRIO, 
    						  NULL))
    {
        return 0;
    }

    return 1;
}

static int g_ard_ble_initialized = 0;
int ard_ble_is_ready(void)
{
	return g_ard_ble_initialized;
}

static void ard_ble_set_ready(void)
{
	g_ard_ble_initialized = 1;
}

// This is a mandatory callback for the BLE framework.
// All BLE events are routed to this callback for processing by BLE framework. 
// This callback is invoked in bt_task context.
bt_status_t bt_app_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    // Routing to event handlers
    switch(msg)
	{
    case BT_POWER_ON_CNF:
        bt_gap_le_set_random_address((bt_bd_addr_ptr_t)g_local_public_addr);
        break;
    case BT_GAP_LE_SET_RANDOM_ADDRESS_CNF:
        // initialization complete after setting random address
        ard_ble_set_ready();
        break;
    case BT_GAP_LE_ADVERTISING_REPORT_IND:
        ard_ble_central_onCentralEvents(msg, status, buff);
    	break;
	default:
		break;
	}

    // A generic fallback callback executed
    // after all other event processors
    ard_ble_postAllEvents(msg, status, buff);

    /*Listen all BT event*/
    return BT_STATUS_SUCCESS;
}


