
// From LinkIt SDK
#include <bt_hci.h>
#include <bt_gap_le.h>
#include <bt_debug.h>
#include <FreeRTOS.h>
#include <task.h>
#include <bt_system.h>

// From LinkIt 7697 board support package
#include <task_def.h>		

#include "utility/ard_ble.h"

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


// This is should be a unique & approved MAC address for the end product.
// In the case of LinkIt 7697 HDK, it is provided by the module manufacturer.
// We'll fill this value with the content from efuse in 
static bt_bd_addr_t local_public_addr = {0};
static void ard_ble_init_local_public_addr(void)
{
	// TODO: This should be read from efuse
	local_public_addr[0] = 0x00;
	local_public_addr[1] = 0x7e;
	local_public_addr[2] = 0x56;
	local_public_addr[3] = 0x50;
	local_public_addr[4] = 0x6e;
	local_public_addr[5] = 0xa0;
}

static void ard_ble_bt_mm_init()
{
    bt_memory_init_packet(BT_MEMORY_TX_BUFFER, tx_buf, BT_TX_BUF_SIZE);
    bt_memory_init_packet(BT_MEMORY_RX_BUFFER, rx_buf, BT_RX_BUF_SIZE);
    bt_memory_init_control_block(BT_MEMORY_CONTROL_BLOCK_TIMER, timer_cb_buf, BT_TIMER_BUF_SIZE);
    bt_memory_init_control_block(BT_MEMORY_CONTROL_BLOCK_LE_CONNECTION, connection_cb_buf,
                                  BT_CONNECTION_BUF_SIZE);
}

// This is a helper task that provided by the liblinkit.a
// that we can simply start it.
extern void bt_task(void * arg);

int ard_ble_begin(void)
{
	ard_ble_init_local_public_addr();

    ard_ble_bt_mm_init();
    

    #if 0
    LOG_I(common, "[BT]local_public_addr [%02X:%02X:%02X:%02X:%02X:%02X]\n", local_public_addr[5],
          local_public_addr[4], local_public_addr[3], local_public_addr[2], local_public_addr[1], local_public_addr[0]);
    log_config_print_switch(BT, DEBUG_LOG_ON);
    log_config_print_switch(BTMM, DEBUG_LOG_OFF);
    log_config_print_switch(BTHCI, DEBUG_LOG_ON);
    log_config_print_switch(BTL2CAP, DEBUG_LOG_ON);
    #endif
    
    if (pdPASS != xTaskCreate(bt_task, 
    						  BLUETOOTH_TASK_NAME, 
    						  BLUETOOTH_TASK_STACKSIZE/sizeof(StackType_t), 
    						  (void *)local_public_addr, 
    						  BLUETOOTH_TASK_PRIO, 
    						  NULL))
    {
        LOG_E(common, "cannot create bt_task.");
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
// All BLE events are routed to this callback for processing.
bt_status_t bt_app_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
	const char* msg_text = NULL;
	
	switch(msg)
	{
    case BT_POWER_ON_CNF:
    	msg_text = "BT_POWER_ON_CNF";
    	ard_ble_set_ready();
        break;

#if 0
    case BT_GAP_LE_SET_SCAN_CNF:
    	msg_text = "BT_GAP_LE_SET_SCAN_CNF";
    	break;

    case BT_GAP_LE_ADVERTISING_REPORT_IND:
    	msg_text = "BT_GAP_LE_ADVERTISING_REPORT_IND";
    	print_adv_report((bt_gap_le_advertising_report_ind_t*)buff);
    	break;
#endif
	default:
		break;
	}


    /*Listen all BT event*/
    return BT_STATUS_SUCCESS;
}