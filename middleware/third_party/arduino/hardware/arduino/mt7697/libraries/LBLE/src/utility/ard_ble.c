/* FreeRTOS headers */
#include <FreeRTOS.h>
#include <task.h>

/* system service headers */
#ifndef MTK_NVDM_ENABLE
#error "LinkIt 7697 Bluetooth relies on NVDM to read local BT device address"
#endif
#include <nvdm.h>
#include <hal_trng.h>
#include <hal_efuse.h>

/* Bluetooth headers */
#include <task_def.h>
#include "utility/ard_ble.h"

//////////////////////////////////////////////////
// Workaround for CONSYS patch issue
#include <variant.h>
static void _connsys_workaround()
{
    // Wi-Fi must be initialized for BLE start-up
    // declared in Arduino core's "variant.h"
    init_global_connsys_for_ble();
}
//////////////////////////////////////////////////

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
// We'll read the address from nvdm(flash) and efuse.
static bt_bd_addr_t g_local_public_addr = {0};

static int read_bt_address_from_efuse(bt_bd_addr_t addr)
{
    static const uint32_t bt_addr_offset = 0x1A;

    return (HAL_EFUSE_OK == hal_efuse_read(bt_addr_offset, (uint8_t*)addr, 6));
}

void generate_random_device_address(bt_bd_addr_t addr)
{
    uint32_t ret = 0;
    uint32_t random_seed = 0;

    ret = hal_trng_init();
    if (HAL_TRNG_STATUS_OK != ret) {
        // LOG_I(common, "[BT]generate_random_address--error 1");
    }
    for(int i = 0; i < 30; ++i) {
        ret = hal_trng_get_generated_random_number(&random_seed);
        if (HAL_TRNG_STATUS_OK != ret) {
            // LOG_I(common, "[BT]generate_random_address--error 2");
        }
        // LOG_I(common, "[BT]generate_random_address--trn: 0x%x", random_seed);
    }
    /* randomly generate address */
    ret = hal_trng_get_generated_random_number(&random_seed);
    if (HAL_TRNG_STATUS_OK != ret) {
        // LOG_I(common, "[BT]generate_random_address--error 3");
    }

    // LOG_I(common, "[BT]generate_random_address--trn: 0x%x", random_seed);
    addr[0] = random_seed & 0xFF;
    addr[1] = (random_seed >> 8) & 0xFF;
    addr[2] = (random_seed >> 16) & 0xFF;
    addr[3] = (random_seed >> 24) & 0xFF;
    ret = hal_trng_get_generated_random_number(&random_seed);
    if (HAL_TRNG_STATUS_OK != ret) {
        // LOG_I(common, "[BT]generate_random_address--error 3");
    }
    // LOG_I(common, "[BT]generate_random_address--trn: 0x%x", random_seed);
    addr[4] = random_seed & 0xFF;
    addr[5] = (random_seed >> 8) & 0xCF;
    hal_trng_deinit();
}

static void ard_ble_init_public_addr(void)
{
    uint32_t size = 12;
    uint8_t buffer[18] = {0};
    uint8_t tmp_buf[3] = {0};

    // Search for public device address (MAC address) 
    //  * efuse
    //  * NVDM (Flash)
    //  * if all the above fails, generate one.
    if(read_bt_address_from_efuse(g_local_public_addr))
    {
        // read bytes directly
        LOG_I(common, "[BT]read address from efuse");
    }
    else if (NVDM_STATUS_OK == nvdm_read_data_item("BT", "address", buffer, &size))
    {
        // It is stored as HEX strings - so we read and convert them.
        LOG_I(common, "[BT]read BT address --ok");
        for (int i = 0; i < 6; ++i) {
            tmp_buf[0] = buffer[2 * i];
            tmp_buf[1] = buffer[2 * i + 1];
            g_local_public_addr[i] = (uint8_t)strtoul((char *)tmp_buf, NULL, 16);
        }
    }
    else
    {
        generate_random_device_address(g_local_public_addr);
    }

    LOG_I(common, "[BT]g_local_public_addr [%02X:%02X:%02X:%02X:%02X:%02X]\n", 
                g_local_public_addr[5],
                g_local_public_addr[4], 
                g_local_public_addr[3], 
                g_local_public_addr[2], 
                g_local_public_addr[1], 
                g_local_public_addr[0]);
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
    // special event handlers that cannot use the Dispatcher-Observer mechanism
    switch(msg)
	{
    case BT_POWER_ON_CNF:
        bt_gap_le_set_random_address((bt_bd_addr_ptr_t)g_local_public_addr);
        break;
    case BT_GAP_LE_SET_RANDOM_ADDRESS_CNF:
        // initialization complete after setting random address
        ard_ble_set_ready();
        break;
	default:
		break;
	}

    // Dispatch to all observers
    ard_ble_postAllEvents(msg, status, buff);

    // Listen to all BT event
    return BT_STATUS_SUCCESS;
}

static const bt_gatts_primary_service_16_t bt_if_gap_primary_service = {
    .rec_hdr = {
        .uuid_ptr = &BT_GATT_UUID_PRIMARY_SERVICE,
        .perm = BT_GATTS_REC_PERM_READABLE,
        .value_len = 2,
    },
    .uuid16 = BT_GATT_UUID16_GAP_SERVICE
};


