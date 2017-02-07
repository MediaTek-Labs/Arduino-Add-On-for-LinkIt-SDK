/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include "ut_app.h"

/* Start of changable configuration. */
#include "ut_app_config.h"
#include "bt_debug.h"
#include "bt_lwip.h"
#include <string.h>
#include "os.h"   
#ifdef MTK_BLE_SMTCN_ENABLE
#include "ble_smtcn.h"
#endif   
#if defined(MTK_BLE_GPIO_SERVICE) && defined(MTK_MCS_ENABLE) 
#include "mcs.h"   
#endif   
#ifdef MTK_BLE_GPIO_SERVICE
#include "bt_gattc_connect.h"  
#include "hrc.h"
#include "bt_gattc_handle_op.h"
#endif   
   
#ifdef BLE_THROUGHPUT
static uint8_t enable_dle = 0;
#endif
extern uint32_t bt_gatt_service_execute_write(uint16_t handle, uint8_t flag);
extern uint16_t conn_interval; /* this is for calculating ble throughput*/
/* Lower Tester Information (PTS) */
const uint8_t lt_addr_type = BT_ADDR_PUBLIC;
uint8_t lt_addr[6] = APP_LT_ADDR;
/* Fill ABCD0000000000000000000004030201 in pts for SM OOB. */
const uint8_t oob_data[] = "\x01\x02\x03\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xcd\xab";
/* End of changable configuration. */

extern void bt_gap_dump(void);
#ifdef BT_DEBUG
extern void bt_gap_debug_cmd_sending(uint8_t* buffer);
#endif

extern bt_bd_addr_t local_public_addr;
static bt_gap_le_local_config_req_ind_t local_config;

/* Start of flash. */
bt_gap_le_smp_pairing_config_t pairing_config_req, pairing_config_req_default = {
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING,
    .maximum_encryption_key_size = 16,
};

bt_gap_le_local_key_t local_key_req, local_key_req_default = {
    .encryption_info = {{0}},
    .master_id = {0},
    .identity_info = {{0x12,0x34,0x56,0x78,0x9a,0xbc,0xde,0xf0,0x19,0x28,0x55,0x33,0x68,0x33,0x56,0xde}},
    .signing_info = {{0}}
};
/* End of flash. */

/* Start of default configuration, don't edit here. */
bool sc_only, sc_only_default = false;

bt_hci_cmd_le_set_advertising_enable_t adv_enable, adv_enable_default = {
    .advertising_enable = BT_HCI_ENABLE,
};

bt_hci_cmd_le_set_advertising_parameters_t adv_para, adv_para_default = {
    .advertising_interval_min = 0x0800,
    .advertising_interval_max = 0x0800,
    .advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED,
    .advertising_channel_map = 7,
    .advertising_filter_policy = 0
};

#if 0
bt_hci_cmd_le_set_multi_advertising_enable_t multi_adv_enable, multi_adv_enable_default = {
    .advertising_enable = BT_HCI_ENABLE,
};

bt_hci_cmd_le_set_multi_advertising_parameters_t multi_adv_para, multi_adv_para_default = {
    .advertising_interval_min = 0x0800,
    .advertising_interval_max = 0x0800,
    .advertising_type = BT_HCI_ADV_TYPE_NON_CONNECTABLE_UNDIRECTED,
    .advertising_channel_map = 7,
    .advertising_filter_policy = 0
};
#endif

const bt_hci_cmd_le_set_scan_enable_t scan_enable = {
    .le_scan_enable = BT_HCI_ENABLE,
    .filter_duplicates = BT_HCI_ENABLE,
};

const bt_hci_cmd_le_set_scan_enable_t scan_disable = {
    .le_scan_enable = BT_HCI_DISABLE,
    .filter_duplicates = BT_HCI_DISABLE,
};

bt_hci_cmd_le_set_scan_parameters_t scan_para, scan_para_default = {
    .le_scan_type = BT_HCI_SCAN_TYPE_ACTIVE,
    .le_scan_interval = 0x0024,
    .le_scan_window = 0x0011,
    .own_address_type = BT_HCI_SCAN_ADDR_PUBLIC,
    .scanning_filter_policy = BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS,
};

bt_hci_cmd_le_create_connection_t connect_para, connect_para_default = {
    .le_scan_interval = 0x0010,
    .le_scan_window = 0x0010,
    .initiator_filter_policy = BT_HCI_CONN_FILTER_ASSIGNED_ADDRESS,
    .peer_address = {
        .type = BT_ADDR_PUBLIC,
    },
    .own_address_type = BT_ADDR_PUBLIC,
    .conn_interval_min = 0x0006,
    .conn_interval_max = 0x0320,
    .conn_latency = 0x0000,
    .supervision_timeout = 0x07d0,
    .minimum_ce_length = 0x0000,
    .maximum_ce_length = 0x0190,
};

bt_hci_cmd_disconnect_t disconnect_para, disconnect_para_default = {
    .connection_handle = 0x0200,
    .reason = BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION,
};

bt_hci_cmd_le_connection_update_t conn_update_para, conn_update_para_default = {
    .connection_handle = 0x0200,
    .conn_interval_min = 0x0320,
    .conn_interval_max = 0x0320,
    .conn_latency = 0x0006,
    .supervision_timeout = 0x0962,
    .minimum_ce_length = 0x0000,
    .maximum_ce_length = 0x0190,
};

bt_hci_cmd_read_rssi_t read_rssi = {
    .handle = 0x0200,
};

bt_hci_cmd_le_set_advertising_data_t adv_data, adv_data_default = {0};
bt_hci_cmd_le_set_scan_response_data_t scan_data, scan_data_default = {0};
//bt_hci_cmd_le_set_multi_advertising_data_t multi_adv_data, multi_adv_data_default = {0};
//bt_hci_cmd_le_set_multi_scan_response_data_t multi_scan_data, multi_scan_data_default = {0};

static uint8_t ut_app_reset_global_config_flag = true;
static uint8_t ut_app_reset_flash_flag = true;
bt_status_t (*ut_app_callback)(bt_msg_type_t, bt_status_t, void *) = NULL;
/* End of default configuration. */

bool bt_app_advertising = false;
bool bt_app_scanning = false;
bool bt_app_connecting = false;
bool bt_app_wait_peer_central_address_resolution_rsp = false;
#define BT_APP_RESOLVING_LIST_UPDATING 0x01
#define BT_APP_WHITE_LIST_UPDATING 0x02
uint8_t list_updating = 0;//combination of BT_APP_RESOLVING_LIST_UPDATING & BT_APP_WHITE_LIST_UPDATING

bt_status_t bt_app_gap_io_callback(void *input, void *output);
bt_status_t bt_app_sm_io_callback(void *input, void *output);
bt_status_t bt_app_l2cap_io_callback(void *input, void *output);
bt_status_t bt_app_gatts_io_callback(void *input, void *output);
bt_status_t bt_app_gattc_io_callback(void *input, void *output);
//bt_status_t bt_app_demo_io_callback(void *input, void *output);
bt_status_t bt_cmd_gattc_io_callback(void *input, void *output);

static const struct bt_app_callback_table_t {
    const char *name;
    bt_status_t (*io_callback)(void *, void *);
} bt_app_callback_table[] = {
    {"gap",     bt_app_gap_io_callback},
    {"sm",      bt_app_sm_io_callback},
    {"l2cap",   bt_app_l2cap_io_callback},
    {"gatts",   bt_app_gatts_io_callback},
    {"GATTC",   bt_app_gattc_io_callback},
    //{"demo",    bt_app_demo_io_callback},
#ifndef MTK_BLE_GPIO_SERVICE
    {"gatt",    bt_cmd_gattc_io_callback},
#endif
};

/*Weak symbol declaration for l2cap */
bt_status_t bt_app_l2cap_io_callback(void *input, void *output);
bt_status_t default_bt_app_l2cap_io_callback(void *input, void *output)
{
    return BT_STATUS_SUCCESS;
}

#if _MSC_VER >= 1500
    #pragma comment(linker, "/alternatename:_bt_app_l2cap_io_callback=_default_bt_app_l2cap_io_callback")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
    #pragma weak bt_app_l2cap_io_callback = default_bt_app_l2cap_io_callback
#else
    #error "Unsupported Platform"
#endif

/*Weak symbol declaration for sm */
bt_status_t bt_app_sm_io_callback(void *input, void *output);
bt_status_t default_bt_app_sm_io_callback(void *input, void *output)
{
    return BT_STATUS_SUCCESS;
}

#if _MSC_VER >= 1500
    #pragma comment(linker, "/alternatename:_bt_app_sm_io_callback=_default_bt_app_sm_io_callback")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
    #pragma weak bt_app_sm_io_callback = default_bt_app_sm_io_callback
#else
    #error "Unsupported Platform"
#endif


void ut_app_reset_flash()
{
    clear_bonded_info();
    pairing_config_req = pairing_config_req_default;
    local_key_req = local_key_req_default;
}

void ut_app_reset_global_config()
{
    ut_app_callback = NULL;
    adv_enable = adv_enable_default;
    adv_para = adv_para_default;
    scan_para = scan_para_default;
    connect_para = connect_para_default;
    disconnect_para = disconnect_para_default;
    conn_update_para = conn_update_para_default;
    adv_data = adv_data_default;
    scan_data = scan_data_default;
    sc_only = sc_only_default;
    //multi_adv_data = multi_adv_data_default;
    //multi_scan_data = multi_scan_data_default;
    //multi_adv_para = multi_adv_para_default;
    //multi_adv_enable = multi_adv_enable_default;
}

static char* get_event_type(uint8_t type)
{
    switch (type)
    {
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_IND:
            return "ADV_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_DIRECT_IND:
            return "ADV_DIRECT_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_SCAN_IND:
            return "ADV_SCAN_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_NONCONN_IND:
            return "ADV_NONCONN_IND";
        case BT_GAP_LE_ADV_REPORT_EVT_TYPE_ADV_SCAN_RSP:
            return "SCAN_RSP";
        default:
            return "NULL";
    }
}

extern char g_ble_scan_data[512];
void print_advertising_report(void *p)
{
    const bt_gap_le_advertising_report_ind_t *report = (bt_gap_le_advertising_report_ind_t *)p;

    BT_COLOR_SET(BT_COLOR_BLUE);
    BT_LOGI("APP", "========================================");
    BT_LOGI("APP", "CL:10Address:\t%s", bt_debug_addr2str(&report->address));
    BT_LOGI("APP", "CL:10Event Type:\t%s", get_event_type(report->event_type));
#ifdef MTK_BLE_GPIO_SERVICE
    BT_LOGI("hrc", "[HRAPP]heart_rate_scan_cb: -- start: type = %d", report->event_type);
    BT_LOGI("hrc", "[HRAPP]heart_rate_scan_cb: -- start: addr[0] = %x, addr[1] = %x, addr[2] = %x, addr[3] = %x, addr[4] = %x, addr[5] = %x\r\n",
            report->address.addr[0], report->address.addr[1],
            report->address.addr[2], report->address.addr[3], report->address.addr[4], report->address.addr[5]);
#endif
    uint8_t count, ad_data_len, ad_data_type, ad_data_idx;
    count = 0;
    uint8_t buff[100] = {0};
    while (count < report->data_length) {
        ad_data_len = report->data[count];
        /* Error handling for data length over 30 bytes. */
        if (ad_data_len >= 0x1F) {
            BT_LOGI("APP", "AD Data Length Error");
            break;
        }
        ad_data_type = report->data[count+1];
        count+=2;

        if (ad_data_type == BT_GAP_LE_AD_TYPE_FLAG) {
            if (report->data[count] & BT_GAP_LE_AD_FLAG_LIMITED_DISCOVERABLE) {
                BT_LOGI("APP", "CL:10AD Flags:\tLE Limited Discoverable Mode");
            } else if (report->data[count] & BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE) {
                BT_LOGI("APP", "CL:10AD Flags:\tLE General Discoverable Mode");
            } else {
                BT_LOGI("APP", "CL:10AD Flags:\tUnknown: 0x%02x", report->data[count]);
            }
            count+=(ad_data_len-1);
        } else if (ad_data_type == BT_GAP_LE_AD_TYPE_NAME_COMPLETE) {
            for (ad_data_idx=0; ad_data_idx<(ad_data_len-1); ad_data_idx++, count++) {
                buff[ad_data_idx] = report->data[count];
            }
            BT_LOGI("APP", "CL:10Complete Name:\t%s", buff);
#if defined(MTK_BLE_GPIO_SERVICE) && defined(MTK_MCS_ENABLE) 
            int total_len = os_strlen(g_ble_scan_data);
            int total_size = sizeof(g_ble_scan_data);
            char databuf[32] = {0};
            os_snprintf(databuf, sizeof(databuf),"%s(%x:%x:%x:%x:%x:%x)    ", buff, report->address.addr[0],
                      report->address.addr[1], report->address.addr[2], report->address.addr[3], report->address.addr[4],
                       report->address.addr[5]);
            int databuf_len = os_strlen(databuf) + 1;
            if ( (total_len + databuf_len) <= total_size )
                os_strlcpy(g_ble_scan_data + total_len, databuf, databuf_len);
            else
                BT_LOGE("hrc", "BT Scan table is overflow !");
#endif
        } else {
            count+=(ad_data_len-1);
        }
    }
    /* print raw data */
    printf("[I][APP] RAW DATA=0x");
    for (count = 0; count<report->data_length; count++){
        printf("%02x",report->data[count]);
    }
    printf("\n");
#if defined(MTK_BT_LWIP_ENABLE)
    bt_lwip_send(report->data, report->data_length);
    bt_lwip_send("\r\n", 5);
#endif

#if defined(MTK_BLE_GPIO_SERVICE) && defined(MTK_MCS_ENABLE)
    BT_LOGI("hrc", "BT Scan List : %s", g_ble_scan_data);
    mcs_update(3, 0, g_ble_scan_data);
#endif

    BT_LOGI("APP", "========================================");
    BT_COLOR_SET(BT_COLOR_WHITE);
}

void print_help()
{
    printf("Command line usage example: (ble h), (ble gap power_off)\n"
           "ble                                 -Common command\n"
           "  h                                 -Help\n"
           "  reset config off                  -Not reset config in each command\n"
           "  reset flash off                   -Not reset flash in each command\n"
           "  reset config on                   -Reset config in each command\n"
           "  reset flash on                    -Reset flash in each command\n"
           "  reset config                      -Reset config\n"
           "  reset flash                       -Reset flash\n"
           "  add resolving_list [peer_addr_type] [peer_addr] [peer_irk]\n"
           "                                    -Add the device to resolving list\n"
           "  rl clear                          -Clear the resolving list\n"
           "  set irk [16byte_irk]              -Set the IRK\n"
           "  ar [on/off]                       -Enable/disable address resolution\n"
           "  set address_timeout [time]        -Set resolvable private address timeout\n"
           "  hci [on/off]                      -Enable/disable HCI log, default disable\n"
           "  bond [connection handle]          -Bond the remote device\n"
           "  sm passkey [key]                  -Input the passkey\n"
           "  sm numeric compare [compare result]\n"
           "                                    -Input the numeric compare result\n"
           "  mitm on                           -Enable the MITM protection\n"
           "  lesc on                           -Enable LE secure connection\n"
           "======================================================\n"
           "ble gap                             -GAP command line\n"
           "  power_off                         -Power off BT\n"
           "  power_on                          -Power on BT\n"
           "  set_adv_data [data_buffer]        -Set advertising data\n"
           "  set_scan_rsp_data [data_buffer]   -Set scan response data\n"
           "  set_adv_params [min_interval] [max_interval] [adv_type] [own_addr_type] [peer_addr_type] [peer_addr] [channel_map] [filter_policy]\n"
           "                                    -Set advertising parameters\n"
           "  start_scan [scan_type] [interval] [window] [own addr type] [scan filter policy]\n"
           "                                    -Start scan\n"
           "  stop_scan                         -Stop scan\n"
           "  connect [address_type] [address]  -Connect the remote device\n"
           "  advanced_conn [scan interval] [scan window] [initiator filter policy] [peer address_type] [peer address] [own addr type] [conn interval min] [conn interval max] [conn latency] [supervision timeout] [minimum ce length] [max ce length]\n"
           "                                    -Connect the remote device by more parameters\n"
           "  update_conn [connection handle] [conn interval min] [conn interval max] [conn latency] [supervision timeout] [min ce length] [max ce length]\n"
           "                                    -Update connection\n"
           "  cancel connect                    -Cacel connection\n"
           "  disconnect [connection handle]    -Disconnect the connection\n"
           "  bond [connection handle] [io capability] [oob data flag] [auth req] [initiator key distribution] [responder key distribution]\n"
           "                                    -Bond the remote device\n"
           "  random_addr [address]             -Set the random address\n"
           "  read_rssi [connection handle]     -Read the RSSI\n"
           "  wl add [0:public/1:random] [address]  -Add the device to white list\n"
           "  wl remove [0:public/1:random] [address]  -Remove the device from white list\n"
           "  wl clear                          -Clear the white list\n"
           "  update data length [connection handle] [tx octets] [tx time] -Update the data length\n"
           "======================================================\n"
           "ble gatt                             -GATT command line\n"
           "  discover_all [connection handle]   -Discover all primary services, included services, characteristics, descriptors\n"
           "  read [connection handle] [attribute handle]      -Read characteristic value or descriptor\n"
           "  write [connection handle] [attribute handle] [attribute value]     -Write characteristic value or descriptor\n"
           );
}

void copy_str_to_addr(uint8_t *addr, const char *str)
{
    unsigned int i, value;
    int using_long_format = 0;
    int using_hex_sign = 0;

    if (str[2] == ':' || str[2] == '-') {
        using_long_format = 1;
    }

    if (str[1] == 'x') {
        using_hex_sign = 2;
    }

    for (i=0; i<6; i++) {
        sscanf(str+using_hex_sign+i*(2+using_long_format), "%02x", &value);
        addr[5-i] = (uint8_t) value;
    }
}

void copy_str_to_byte(uint8_t *des, const char *str, uint32_t len)
{
    unsigned int i, value = 0;
    int using_long_format = 0;
    int using_hex_sign = 0;

    if (str[2] == ':' || str[2] == '-') {
        using_long_format = 1;
    }

    if (str[1] == 'x') {
        using_hex_sign = 2;
    }

    for (i=0; i<len; i++) {
        sscanf(str+using_hex_sign+i*(2+using_long_format), "%02x", &value);
        des[i] = (uint8_t) value;
        value = 0;
    }
}

static uint32_t sm_passkey;
static uint8_t nc_value_correct[1];

bt_status_t app_clear_resolving_list()
{
    bt_status_t st = BT_STATUS_SUCCESS;
    st = bt_gap_le_set_resolving_list(BT_GAP_LE_CLEAR_RESOLVING_LIST, NULL);
    if (BT_STATUS_OUT_OF_MEMORY == st) {
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "Add device to Resolving List Failed [OOM]");
        BT_COLOR_SET(BT_COLOR_WHITE);
    }
    return st;
}
bt_status_t app_delete_dev_from_resolving_list(const bt_gap_le_bonding_info_t *bonded_info)
{
    bt_status_t st = BT_STATUS_SUCCESS;
    if (BT_ADDR_TYPE_UNKNOW != bonded_info->identity_addr.address.type){
        bt_hci_cmd_le_remove_device_from_resolving_list_t dev;
        dev.peer_identity_address = bonded_info->identity_addr.address;
        st = bt_gap_le_set_resolving_list(BT_GAP_LE_REMOVE_FROM_RESOLVING_LIST,(void*)&dev);
        if (BT_STATUS_OUT_OF_MEMORY == st) {
            BT_COLOR_SET(BT_COLOR_RED);
            BT_LOGI("APP", "Add device to Resolving List Failed [OOM]");
            BT_COLOR_SET(BT_COLOR_WHITE);
        }
    }
    return st;
}
bt_status_t app_add_dev_2_resolving_list(const bt_gap_le_bonding_info_t *bonded_info)
{
    bt_status_t st = BT_STATUS_SUCCESS;
    if (BT_ADDR_TYPE_UNKNOW != bonded_info->identity_addr.address.type){
        bt_hci_cmd_le_add_device_to_resolving_list_t dev;
        dev.peer_identity_address = bonded_info->identity_addr.address;
        os_memcpy(dev.peer_irk,&(bonded_info->identity_info), sizeof(dev.peer_irk));
        os_memcpy(dev.local_irk,&(local_key_req.identity_info), sizeof(dev.local_irk));
        st = bt_gap_le_set_resolving_list(BT_GAP_LE_ADD_TO_RESOLVING_LIST,(void*)&dev);
        if (BT_STATUS_OUT_OF_MEMORY == st) {
            BT_COLOR_SET(BT_COLOR_RED);
            BT_LOGI("APP", "Add device to Resolving List Failed [OOM]");
            BT_COLOR_SET(BT_COLOR_WHITE);
        }
    }
    return st;
}
/* If we have peer's identity address info(type/address), we will add identity info to white list, or
   Add peer_addr info if we don't have peer's identity address.
*/
bt_status_t app_add_dev_2_white_list(const bt_gap_le_bonding_info_t *bonded_info, const bt_addr_t *peer_addr)
{
    bt_status_t st = BT_STATUS_SUCCESS;
    if (BT_ADDR_TYPE_UNKNOW != bonded_info->identity_addr.address.type) {
        const bt_addr_t *bt_addr = (const bt_addr_t *)(&bonded_info->identity_addr);
        st = bt_gap_le_set_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, bt_addr);
    } else if(peer_addr->type != BT_ADDR_TYPE_UNKNOW){
        st = bt_gap_le_set_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, peer_addr);
    }
    if (BT_STATUS_OUT_OF_MEMORY == st){
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "CL:10Add device to White List Failed [OOM]");
        BT_COLOR_SET(BT_COLOR_WHITE);
    }
    return st;
}

static bool g_bt_hci_log_enable = false;

bool bt_hci_log_enabled(void)
{
    return g_bt_hci_log_enable;
}

#if defined(MTK_BLE_GPIO_SERVICE) && defined(MTK_MCS_ENABLE) 
    char g_ble_scan_data[512] = {0};
#endif
bt_status_t bt_app_io_callback(void *input, void *output)
{
    const char *cmd = input;

    BT_LOGI("APP", "CL:10bt_app_io_callback %s", cmd);
    if(ut_app_reset_global_config_flag) {
        ut_app_reset_global_config();
    }

    if(ut_app_reset_flash_flag) {
        ut_app_reset_flash();

    }

    if (UT_APP_CMP("?")) {
        print_help();
        return BT_STATUS_SUCCESS;
    }
    else if (UT_APP_CMP("hci on")) {
        g_bt_hci_log_enable = true;
    }
    else if (UT_APP_CMP("hci off")) {
        g_bt_hci_log_enable = false;
    }
    else if (UT_APP_CMP("reset config off")) {
        ut_app_reset_global_config_flag = false;
    }

    else if (UT_APP_CMP("reset flash off")) {
        ut_app_reset_flash_flag = false;
    }
    /* Usage: set pts_addr [pts address]*/
    else if (UT_APP_CMP("set pts_addr")) {
        const char *addr_str = cmd + 13;
        copy_str_to_addr(lt_addr, addr_str);
        BT_LOGI("APP", "change to lt_addr: %x-%x-%x-%x-%x-%x", lt_addr[5], lt_addr[4], lt_addr[3], lt_addr[2], lt_addr[1], lt_addr[0]);
    }
    else if (UT_APP_CMP("reset config on")) {
        ut_app_reset_global_config_flag = true;
    }

    else if (UT_APP_CMP("reset flash on")) {
        ut_app_reset_flash_flag = true;
    }

    else if (UT_APP_CMP("reset config")) {
        ut_app_reset_global_config();
    }

    else if (UT_APP_CMP("reset flash")) {
        ut_app_reset_flash();
    
    }

    else if (UT_APP_CMP("po")) {
        bt_power_on((bt_bd_addr_ptr_t)&local_public_addr, NULL);
        bt_gatts_set_max_mtu(128); /* This value should consider with MM Tx/Rx buffer size. */
    }

    else if (UT_APP_CMP("pf")) {
        bt_power_off();
    }

    /* Usage: advanced po [public address] [random address].
       Note:  Set N if you doesn't need it. */
    else if (UT_APP_CMP("advanced po")) {
        if (strlen(cmd) >= 12) {
            uint8_t public_addr[6]={0};
            uint8_t random_addr[6]={0};
            const char *addr_str = cmd + 12;

            /* Find public address */
            if (strncmp("N", addr_str, 1) != 0) {
                copy_str_to_addr(public_addr, addr_str);
            } else {
                public_addr[0] = 'N';
            }

            /* Jump to the start of the random address */
            uint32_t i=0;
            while (i<18) {
                if (strncmp(" ", addr_str, 1) == 0)
                    break;
                addr_str++;
                i++;
            }
            addr_str++;

            /* Find random address */
            if (strncmp("N", addr_str, 1) != 0) {
                copy_str_to_addr(random_addr, addr_str);
            } else {
                random_addr[0] = 'N';
            }

            bt_power_on((public_addr[0] == 'N'? NULL:public_addr),
                (random_addr[0] == 'N'? NULL:random_addr));
        } else {
            BT_COLOR_SET(BT_COLOR_RED);
            BT_LOGW("APP", "please input the specific public address and random address");
            BT_LOGW("APP", "format: advanced po [public address/N] [random address/N]");
            BT_COLOR_SET(BT_COLOR_WHITE);
        }
    }
    else if (UT_APP_CMP("set address_timeout")) {
        if (strlen(cmd) >= 20) {
            uint8_t timeout = (uint8_t)strtoul(cmd + 20, NULL, 10);
            bt_gap_le_set_resolvable_private_address_timeout(timeout);
        }
    }
    else if (UT_APP_CMP("set irk")) {
        if (strlen((char *)cmd) >= 8) {
            const char *key = cmd + 8;
            copy_str_to_byte(local_key_req_default.identity_info.irk, key, 16);
            local_key_req.identity_info = local_key_req_default.identity_info;
        }
    }
    else if (UT_APP_CMP("add resolving_list")) {
        if (strlen(cmd) >= 34) {
            uint8_t addr_type = (uint8_t)strtoul(cmd + 19, NULL, 10); 
            const char *addr_str = cmd + 21;
            const char *key = cmd + 34;
            if (addr_type != 0 && addr_type!= 1) {
                BT_COLOR_SET(BT_COLOR_RED);
                BT_LOGW("APP", "add resolving_list [0:public_indentity / 1:random_identity] [bt address] [irk]");
                BT_COLOR_SET(BT_COLOR_WHITE);
            } else {
                uint8_t addr[6];
                bt_hci_cmd_le_add_device_to_resolving_list_t dev;
                copy_str_to_addr(addr, addr_str);
                dev.peer_identity_address.type = addr_type;
                os_memcpy(dev.peer_identity_address.addr, addr, sizeof(addr));                
                copy_str_to_byte((uint8_t *)(&(dev.peer_irk)), key, 16);
                os_memcpy(dev.local_irk,&(local_key_req.identity_info), sizeof(dev.local_irk));
                bt_gap_le_set_resolving_list(BT_GAP_LE_ADD_TO_RESOLVING_LIST,(void*)&dev);
                if (BT_STATUS_OUT_OF_MEMORY == bt_gap_le_set_resolving_list(BT_GAP_LE_ADD_TO_RESOLVING_LIST,(void*)&dev)) {
                    BT_COLOR_SET(BT_COLOR_RED);
                    BT_LOGI("APP", "Add device to Resolving List Failed [OOM]");
                    BT_COLOR_SET(BT_COLOR_WHITE);
                }                
            }            
        }
        else {
            BT_COLOR_SET(BT_COLOR_RED);
            BT_LOGW("APP", "add resolving_list [2:public_indentity / 3:random_identity] [bt address] [irk]");
            BT_COLOR_SET(BT_COLOR_WHITE);
        }
    }
    /* Set first 6 bytes local irk */
    else if (UT_APP_CMP("local_irk set")) {
        if (strlen((char *)cmd) >= 14) {
            const char *key = cmd+ 14;
            copy_str_to_addr(local_key_req_default.identity_info.irk, key);
            local_key_req.identity_info = local_key_req_default.identity_info;
        }
    }
    /* update peer Central Address Resolution supporting */
    else if (UT_APP_CMP("check peer CAR supporting")) {
        const char *handle = cmd + 27;
        bt_gattc_read_using_charc_uuid_req_t req;
        uint16_t uuid = BT_SIG_UUID16_CENTRAL_ADDRESS_RESOLUTION;
        req.opcode = BT_ATT_OPCODE_READ_BY_TYPE_REQUEST;
        req.starting_handle = 0x0001;
        req.ending_handle = 0xffff;
        bt_uuid_load(&req.type, (void *)&uuid, 2);
        bt_gattc_read_using_charc_uuid((uint16_t)strtoul(handle, NULL, 16), &req);
        bt_app_wait_peer_central_address_resolution_rsp = true;
    }

    /* Usage: random address [random address].
       Note:  [random address] should be existed. */
    else if (UT_APP_CMP("random address")) {
        if (strlen(cmd) >= 15) {
            const char *addr_str = cmd + 15;
            uint8_t addr[6];
            copy_str_to_addr(addr, addr_str);

            bt_gap_le_set_random_address(addr);
        } else {
            BT_COLOR_SET(BT_COLOR_RED);
            BT_LOGW("APP", "please input the specific random address");
            BT_LOGW("APP", "random address [random address]");
            BT_COLOR_SET(BT_COLOR_WHITE);
        }
    }

    else if (UT_APP_CMP("ar on")) {
        /* Set address resolition enable*/
            bt_gap_le_set_address_resolution_enable(1);
    }
    else if (UT_APP_CMP("ar off")) {
        /* Set address resolition disable*/
            bt_gap_le_set_address_resolution_enable(0);
    }
    else if (UT_APP_CMP("rl add")) {
        uint8_t idx = (uint8_t)strtoul(cmd+7, NULL, 10);
        app_bt_bonded_info_t *app_bonded_info = NULL;
        app_bonded_info = find_bonded_info_by_index(idx);
        if (app_bonded_info != NULL) {
            bt_gap_le_bonding_info_t *bonded_info = &(app_bonded_info->info);
            //remove device from resolving list
            if (BT_STATUS_SUCCESS != app_add_dev_2_resolving_list(bonded_info)) {
                BT_LOGE("APP", "Add Device to Resolving List FAILED!!!");
            }
        } else {
            BT_LOGE("APP", "Can not find the bonded info idx[%d]. Please use \"list bond\" to check bonded info.",idx);
        }
    }
    else if (UT_APP_CMP("rl remove")) {
        uint8_t idx = (uint8_t)strtoul(cmd+10, NULL, 10);
        app_bt_bonded_info_t *app_bonded_info = NULL;
        app_bonded_info = find_bonded_info_by_index(idx);
        if (app_bonded_info != NULL) {
            bt_gap_le_bonding_info_t *bonded_info = &(app_bonded_info->info);
            //remove device from resolving list
            if (BT_STATUS_SUCCESS != app_delete_dev_from_resolving_list(bonded_info)) {
                BT_LOGE("APP", "Remove Device from Resolving List FAILED!!!");
            }
        } else {
            BT_LOGE("APP", "Can not find the bonded info idx[%d]. Please use \"list bond\" to check bonded info.",idx);
        }
    }
    else if (UT_APP_CMP("rl clear")) {
        app_clear_resolving_list();
    }
    /* Usage: wl add [0:public / 1:random] [bt address].
       Note:  default use #lt_addr_type and #lt_addr */
    else if (UT_APP_CMP("wl add")) {
        bt_addr_t device;
        if (strlen(cmd) >= 7) {

            uint8_t addr_type = (uint8_t)strtoul(cmd + 7, NULL, 10);

            if (addr_type != 0 && addr_type!= 1) {
                BT_COLOR_SET(BT_COLOR_RED);
                BT_LOGW("APP", "please input the correct address type");
                BT_LOGW("APP", "wl add [0:public / 1:random] [bt address]");
                BT_COLOR_SET(BT_COLOR_WHITE);
            } else {
                const char *addr_str = cmd + 9;
                uint8_t addr[6];
                copy_str_to_addr(addr, addr_str);

                device.type = addr_type;
                os_memcpy(device.addr, addr, sizeof(addr));
                bt_gap_le_set_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &device);
            }
        } else {
            device.type = lt_addr_type;
            os_memcpy(device.addr, lt_addr, sizeof(lt_addr));
            bt_gap_le_set_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &device);
        }
    }

    /* Usage: wl remove [0:public / 1:random] [bt address].
       Note:  default use #lt_addr_type and #lt_addr */
    else if (UT_APP_CMP("wl remove")) {
        bt_addr_t device;
        if (strlen(cmd) >= 10) {

            uint8_t addr_type = (uint8_t)strtoul(cmd + 10, NULL, 10);
            if (addr_type != 0 && addr_type!= 1) {
                BT_COLOR_SET(BT_COLOR_RED);
                BT_LOGW("APP", "please input the correct address type");
                BT_LOGW("APP", "wl add [0:public / 1:random] [bt address]");
                BT_COLOR_SET(BT_COLOR_WHITE);
            } else {
                const char *addr_str = cmd + 12;
                uint8_t addr[6];
                copy_str_to_addr(addr, addr_str);

                device.type = addr_type;
                os_memcpy(device.addr, addr, sizeof(addr));
                bt_gap_le_set_white_list(BT_GAP_LE_REMOVE_FROM_WHITE_LIST, &device);
            }
        } else {
            device.type = lt_addr_type;
            os_memcpy(device.addr, lt_addr, sizeof(lt_addr));
            bt_gap_le_set_white_list(BT_GAP_LE_REMOVE_FROM_WHITE_LIST, &device);
        }
    }

    else if (UT_APP_CMP("wl clear")) {
        bt_gap_le_set_white_list(BT_GAP_LE_CLEAR_WHITE_LIST, NULL);
    }
    /*advanced scan [scan type] [Own Address Type] [Scanning Filter Policy]
    */
    else if (UT_APP_CMP("advanced scan")) {
        uint8_t scan_type = (uint8_t)strtoul(cmd+14, NULL, 10);
        uint8_t own_address_type = (uint8_t)strtoul(cmd+16, NULL, 10);
        uint8_t policy = (uint8_t)strtoul(cmd+18, NULL, 10);
        BT_COLOR_SET(BT_COLOR_BLUE);
        BT_LOGI("APP", "Advanced Scan test");
        BT_LOGI("APP", "Scan Type[%d] Own Address Type[%d] Scanning Filter Policy[%d]\n",scan_type,own_address_type,policy);
        BT_COLOR_SET(BT_COLOR_WHITE);
        scan_para.le_scan_type = scan_type,
        scan_para.own_address_type = own_address_type,
        scan_para.scanning_filter_policy = policy,
        bt_app_scanning = true;
        bt_gap_le_set_scan(&scan_enable, &scan_para);
    }
    else if (UT_APP_CMP("gap dump")) {
        bt_gap_dump();
    }
#ifdef BT_DEBUG
    else if (UT_APP_CMP("bt debug cmd")) {
        uint32_t length = strlen("bt debug cmd ");
        uint32_t i = 0;
        uint8_t* cmd_data_buff = (uint8_t*)cmd;
        uint8_t value[4] = {*(cmd + length), *(cmd + length + 1), 0};
        while (value[0] != 0) {
            sscanf(value, "%02x", cmd_data_buff + (i>>1));
            i += 2;
            value[0] = *(cmd + length + i);
            value[1] = *(cmd + length + i + 1);
        }
        bt_gap_debug_cmd_sending(cmd_data_buff);
    }
#endif
    else if (UT_APP_CMP("scan on")) {
        bt_app_scanning = true;
        bt_gap_le_set_scan(&scan_enable, &scan_para);
    }

    else if (UT_APP_CMP("scan off")) {
        bt_app_scanning = false;
        bt_gap_le_set_scan(&scan_disable, NULL);
    }

    else if (UT_APP_CMP("adv on")) {
        bt_app_advertising = true;
        adv_enable.advertising_enable = BT_HCI_ENABLE;
        bt_gap_le_set_advertising(&adv_enable, &adv_para, NULL, NULL);
    }

    else if (UT_APP_CMP("adv off")) {
        bt_app_advertising = false;
        bt_hci_cmd_le_set_advertising_enable_t enable;
        enable.advertising_enable = BT_HCI_DISABLE;
        bt_gap_le_set_advertising(&enable, NULL, NULL, NULL);
    }

    else if (UT_APP_CMP("bond off")) {
        pairing_config_req.auth_req &= ~BT_GAP_LE_SMP_AUTH_REQ_BONDING;
    }

  /*advanced adv [own addr type] [adv type] [advertising_filter_policy] [peer addr type] [peer BT addr]
    [own addr type] :0:public / 1:random/ 2: Gen RPA from resolving list or public address host provide/ 3: Gen RPA from resolving list or static random address host provide
    [adv type] : 0:ADV_IND, 1:ADV_DIRECT_IND high duty cycle, 2: ADV_SCAN_IND, 3:ADV_NONCONN_IND or 4.ADV_DIRECT_IND low duty cycle.
    [peer addr type]:0:public / 1:random
    [advertising_filter_policy]: define in spec, 0~4
    [peer addr type] : Chck src/hbif/bt_gap_le_spec.h BT_GAP_LE_AD_xxxx 0~4
    [peer BT Addr] : peer BT address for BT_GAP_LE_AD_CONNECTABLE_DIRECTED_HIGH or BT_GAP_LE_AD_CONNECTABLE_DIRECTED_LOW
    Hint: for [peer addr type] and [peer BT addr], you can refer bond info for the device we had bonded before.
   */
    else if (UT_APP_CMP("advanced adv")) {
        bt_hci_cmd_le_set_advertising_data_t adv_data = {
            .advertising_data_length=31,
            .advertising_data="DDDDDHUMMINGBIRD_ADV_DATA",
        };
        bt_hci_cmd_le_set_scan_response_data_t scan_data = {
            .scan_response_data_length=31,
            .scan_response_data = "DDSCAN_DATA_HUMMINGBIRD",
        };
        bt_app_advertising = true;
        os_memset(gatts_device_name, 0x00, sizeof(gatts_device_name));
        os_memcpy(gatts_device_name, &adv_data.advertising_data[5], 11);
        gap_appearance = 0x4567;
        adv_data.advertising_data[0]=2; //adv_length
        adv_data.advertising_data[1]=BT_GAP_LE_AD_TYPE_FLAG;
        adv_data.advertising_data[2]=BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED|BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE;
        adv_data.advertising_data[3]=21;//adv_length
        adv_data.advertising_data[4]=0x09;
        scan_data.scan_response_data[0] = 22; /* ADV length. */
        scan_data.scan_response_data[1] = 0x08;
        uint8_t own_addr_type = (uint8_t)strtoul(cmd+13, NULL, 10);
        uint8_t adv_type = (uint8_t)strtoul(cmd+15, NULL, 10);
        uint8_t policy = (uint8_t)strtoul(cmd+17, NULL, 10);
        uint8_t peer_addr_type = (uint8_t)strtoul(cmd+19, NULL, 10);


        const char *addr_str = cmd + 21;
        uint8_t addr[6];
        copy_str_to_addr(addr, addr_str);
        BT_COLOR_SET(BT_COLOR_BLUE);
        BT_LOGI("APP", "Advanced advertising test");
        BT_LOGI("APP", "own_addr_type[%d] adv_type[%d] adv_policy[%d] peer_addr_type[%d]",
            own_addr_type,adv_type,policy,peer_addr_type);
        BT_LOGI("APP", "peer_addr(%02x:%02x:%02x:%02x:%02x:%02x)",
            addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
        BT_COLOR_SET(BT_COLOR_WHITE);

        adv_enable.advertising_enable = true;

        adv_para.advertising_interval_min =0x0800;
        adv_para.advertising_interval_max =0x1000;
        adv_para.advertising_type = adv_type;
        adv_para.own_address_type = own_addr_type;
        adv_para.peer_address.type = peer_addr_type;
        os_memcpy(adv_para.peer_address.addr, &addr, 6);
        adv_para.advertising_channel_map = 7;
        adv_para.advertising_filter_policy = policy;
        if ((adv_para.advertising_type == 1) || (adv_para.advertising_type == 4)){
            bt_gap_le_set_advertising(&adv_enable, &adv_para, NULL, NULL);
        } else {
            bt_gap_le_set_advertising(&adv_enable, &adv_para, &adv_data, &scan_data);
        }
    }

  /*advanced connect [Initiator_Filter_Policy] [Own_Address_Type] [Peer_Address_Type] [Peer_Address]
    [Initiator_Filter_Policy] :0;white list is not used. 1;white list is used.
    [Own_Address_Type] : 0~4;Public/Random/RPA or Public/RPA or Random
    [Peer_Address_Type] : 0~4; Public/Random/Public Identity/Random Identity
    [Peer_Address] :
    Test case command for Privacy 1.2:
    [ar on]
    advanced connect 0 2 2 [Peer Identity Address]
    advanced connect 1 2 0 0x000000000000
   */
    else if (UT_APP_CMP("advanced connect")){
        uint8_t policy = (uint8_t)strtoul(cmd+17, NULL, 10);
        uint8_t own_address_type = (uint8_t)strtoul(cmd+19, NULL, 10);
        uint8_t peer_address_type = (uint8_t)strtoul(cmd+21, NULL, 10);


        const char *addr_str = cmd + 23;
        uint8_t addr[6];
        copy_str_to_addr(addr, addr_str);
        BT_COLOR_SET(BT_COLOR_BLUE);
        BT_LOGI("APP", "Advanced connect ");
        BT_LOGI("APP", "Initiator_Filter_Policy[%d] Own_Address_Type[%d] Peer_Address_Type[%d]",
            policy, own_address_type, peer_address_type);
        BT_LOGI("APP", "peer_addr(%02x:%02x:%02x:%02x:%02x:%02x)",
            addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
        BT_COLOR_SET(BT_COLOR_WHITE);

        connect_para.initiator_filter_policy = policy;
        connect_para.own_address_type = own_address_type;
        connect_para.peer_address.type = peer_address_type;
        os_memcpy(connect_para.peer_address.addr, addr, sizeof(addr));

        bt_gap_le_connect(&connect_para);
    }

    /* Usage: connect [0:public / 1:random] [bt address].
       Note:  default use #lt_addr_type and #lt_addr */
    else if (UT_APP_CMP("connect")) {
        if (strlen(cmd) >= 8) {
            uint8_t peer_addr_type = (uint8_t)strtoul(cmd + 8, NULL, 10);

            const char *addr_str = cmd + 10;
            uint8_t addr[6];
            copy_str_to_addr(addr, addr_str);
#ifdef BLE_THROUGHPUT
            //const char *conn_interval = cmd + 23;
            enable_dle = (uint8_t)strtoul(cmd + 23, NULL, 10);
            uint16_t interval_conn = (uint16_t)strtoul(cmd + 25, NULL, 10);
            //uint16_t interval_conn = (uint16_t)strtoul(cmd + 23, NULL, 10);
            connect_para.conn_interval_min = interval_conn;
            connect_para.conn_interval_max = interval_conn;
            // 0x50 is for BLE4.2
            //connect_para.conn_interval_min = 0x50;
            //connect_para.conn_interval_max = 0x50;
#endif
            connect_para.peer_address.type = peer_addr_type;
            os_memcpy(connect_para.peer_address.addr, addr, sizeof(addr));
            bt_gap_le_connect(&connect_para);
        } else {
            connect_para.peer_address.type = lt_addr_type;
            os_memcpy(connect_para.peer_address.addr, lt_addr, sizeof(lt_addr));
            bt_gap_le_connect(&connect_para);
        }
    }

    else if (UT_APP_CMP("cancel connect")) {
        bt_gap_le_cancel_connection();
    }

    /* Usage:   disconnect <handle in hex>
       Example: disconnect 0200 */
    else if (UT_APP_CMP("disconnect")) {
        const char *handle = cmd + strlen("disconnect ");
        disconnect_para.connection_handle = (uint16_t)strtoul(handle, NULL, 16);
        BT_LOGI("APP", "connection_handle(0x%04x)",disconnect_para.connection_handle);
        bt_gap_le_disconnect(&disconnect_para);
    }

    else if (UT_APP_CMP("read rssi")) {
        bt_gap_le_read_rssi(&read_rssi);
    }

    else if (UT_APP_CMP("update conn param")) {
        bt_gap_le_update_connection_parameter(&conn_update_para);
    }

    /* Usage: update data length <handle in hex> <tx octets in hex> <tx time in hex>.
       Example: update data length 0200 0030 0500*/
    else if (UT_APP_CMP("update data length")) {
        bt_hci_cmd_le_set_data_length_t data_length;
        data_length.connection_handle = (uint16_t)strtoul(cmd + 19, NULL, 16);
        data_length.tx_octets = (uint16_t)strtoul(cmd + 24, NULL, 16);
        data_length.tx_time = (uint16_t)strtoul(cmd + 29, NULL, 16);
        if (data_length.connection_handle > 0x0f00 ||
           (data_length.tx_octets < 0x001B || data_length.tx_octets > 0x00FB) ||
           (data_length.tx_time < 0x0148 || data_length.tx_time > 0x0848)) {
            BT_LOGW("APP", "Usage: update data length <handle in hex> <tx octets in hex> <tx time in hex>.");
            BT_LOGW("APP", "The range of connection handle is 0x0000-0x0EFF");
            BT_LOGW("APP", "The range of tx octets is 0x001B-0x00FB");
            BT_LOGW("APP", "The range of tx time is 0x0148-0x0848");
        }
        else {
            BT_LOGI("APP", "update data length handle(%04x) tx_octets(%04x) tx_time(%04x)",
                data_length.connection_handle,data_length.tx_octets,data_length.tx_time);
            bt_gap_le_update_data_length(&data_length);
        }
    }

    /* Usage:   bond <handle in hex>
       Example: bond 0200 */
    else if (UT_APP_CMP("bond")) {
        const char *handle = cmd + strlen("bond ");

        bt_gap_le_bond(strtoul(handle, NULL, 16), &pairing_config_req);
    }
    else if (UT_APP_CMP("sm passkey")) {
        sm_passkey = (uint32_t)atoi(cmd + 11);
    }
    else if (UT_APP_CMP("sm numeric compare")) {
        nc_value_correct[0] = *((uint8_t *)(cmd + 19));
    }
    else if (UT_APP_CMP("remove bond")) {
        bt_addr_t addr = {
            .type = (uint8_t)strtoul(cmd + strlen("remove bond "), NULL, 10),
        };
        copy_str_to_addr(addr.addr, cmd + strlen("remove bond 0 "));
        cancel_bonded_info(&addr);
    }

    else if (UT_APP_CMP("list bond")) {
        BT_COLOR_SET(BT_COLOR_BLUE);
        dump_bonded_info_list();
        BT_COLOR_SET(BT_COLOR_WHITE);
    }

    else if (UT_APP_CMP("list connection")) {
        BT_COLOR_SET(BT_COLOR_BLUE);
        dump_connection_info_list();
        BT_COLOR_SET(BT_COLOR_WHITE);
    }

    else if (UT_APP_CMP("show status")) {
        BT_COLOR_SET(BT_COLOR_BLUE);
        BT_LOGD("APP", "Advertising:\t%s", bt_app_advertising?"ON":"OFF");
        BT_LOGD("APP", "Scanning:\t%s", bt_app_scanning?"ON":"OFF");
        BT_LOGD("APP", "Connecting:\t%s", bt_app_connecting?"ON":"OFF");
        BT_LOGD("APP", "MITM:\t\t%s", pairing_config_req.auth_req & BT_GAP_LE_SMP_AUTH_REQ_MITM?"ON":"OFF");
        BT_LOGD("APP", "Bonding:\t%s", pairing_config_req.auth_req & BT_GAP_LE_SMP_AUTH_REQ_BONDING?"ON":"OFF");
        BT_LOGD("APP", "LESC:\t\t%s", pairing_config_req.auth_req & BT_GAP_LE_SMP_AUTH_REQ_SECURE_CONNECTION?"ON":"OFF");
        BT_LOGD("APP", "OOB:\t\t%s", pairing_config_req.oob_data_flag?"ON":"OFF");
        switch (pairing_config_req.io_capability) {
        case BT_GAP_LE_SMP_DISPLAY_ONLY:
            BT_LOGD("APP", "IO Capability:\tBT_GAP_LE_SMP_DISPLAY_ONLY");
            break;
        case BT_GAP_LE_SMP_KEYBOARD_DISPLAY:
            BT_LOGD("APP", "IO Capability:\tBT_GAP_LE_SMP_KEYBOARD_DISPLAY");
            break;
        default:
            BT_LOGD("APP", "IO Capability:\t%d", pairing_config_req.io_capability);
        }
        BT_LOGD("APP", "Master LTK:\t%s", pairing_config_req.initiator_key_distribution & BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY?"ON":"OFF");
        BT_LOGD("APP", "Master CSRK:\t%s", pairing_config_req.initiator_key_distribution & BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN?"ON":"OFF");
        BT_LOGD("APP", "Master IRK:\t%s", pairing_config_req.initiator_key_distribution & BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY?"ON":"OFF");
        BT_LOGD("APP", "Slave LTK:\t%s", pairing_config_req.responder_key_distribution & BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY?"ON":"OFF");
        BT_LOGD("APP", "Slave CSRK:\t%s", pairing_config_req.responder_key_distribution & BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN?"ON":"OFF");
        BT_LOGD("APP", "Slave IRK:\t%s", pairing_config_req.responder_key_distribution & BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY?"ON":"OFF");
        dump_bonded_info_list();
        dump_connection_info_list();
        BT_COLOR_SET(BT_COLOR_WHITE);
    }

    /* GATTC signed write wo rsp 0201 xxxx value. */
    else if (UT_APP_CMP("dist csrk")) {
        pairing_config_req.initiator_key_distribution |= BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN;
        pairing_config_req.responder_key_distribution |= BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN;
    }

    else if (UT_APP_CMP("dist ltk")) {
        pairing_config_req.initiator_key_distribution |= BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY;
        pairing_config_req.responder_key_distribution |= BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY;
    }

    else if (UT_APP_CMP("dist irk")) {
        pairing_config_req.initiator_key_distribution |= BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY;
        pairing_config_req.responder_key_distribution |= BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY;
    }

    else if (UT_APP_CMP("mitm on")) {
        pairing_config_req.auth_req |= BT_GAP_LE_SMP_AUTH_REQ_MITM;
    }

    else if (UT_APP_CMP("lesc only")) {
        sc_only = true;
    }

    else if (UT_APP_CMP("lesc on")) {
        pairing_config_req.auth_req |= BT_GAP_LE_SMP_AUTH_REQ_SECURE_CONNECTION;
    }

    else if (UT_APP_CMP("keyboard only")) {
        pairing_config_req.io_capability = BT_GAP_LE_SMP_KEYBOARD_ONLY;
    }

    else if (UT_APP_CMP("display only")) {
        pairing_config_req.io_capability = BT_GAP_LE_SMP_DISPLAY_ONLY;
    }

    else if (UT_APP_CMP("display yn")) {
        pairing_config_req.io_capability = BT_GAP_LE_SMP_DISPLAY_YES_NO;
    }

    else if (UT_APP_CMP("keyboard display")) {
        pairing_config_req.io_capability = BT_GAP_LE_SMP_KEYBOARD_DISPLAY;
    }

    else if (UT_APP_CMP("no io")) {
        pairing_config_req.io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT;
    }

#ifdef MTK_BLE_GPIO_SERVICE
    else if (UT_APP_CMP("gpio client g")) {
        BT_LOGI("APP", "start gpio client\n");
        gattc_service_init();
        heart_rate_init();
    }
    //added other CMD
    else if (UT_APP_CMP("gpio client c")) {
        //ex : ble gpio client c 1 AAAAAAAAAAAA
        BT_LOGI("GATTC", "gattc_start_connect: -- start\r\n");
        //bt_status_t status = BT_STATUS_FAIL;
        uint8_t addr[6], peer_type;
        
        peer_type = (uint8_t)strtoul((const char*)cmd + 14, NULL, 10);
        BT_LOGI("GATTC", "connect: peer_type = %d", peer_type);
        const char *addr_str = (const char*)cmd + 14 + 2;
        copy_str_to_addr(addr, addr_str);        
        BT_LOGI("GATTC", "connect: addr-- start: addr[0] = %04x, addr[1] = %04x,addr[2] = %04x,addr[3] = %04x,addr[4] = %04x,addr[5] =  %04x\r\n",
                addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
        bt_gattc_connect(addr, peer_type);
        
    }
    else if (UT_APP_CMP("gpio client d")) {
        BT_LOGI("GATTC", "gattc_start_disconnect: -- start\r\n" );
        //bt_status_t status = BT_STATUS_FAIL;
        const char *handle = (const char*)cmd + 14;
        uint16_t connection_handle = (uint16_t)strtoul(handle, NULL, 16);
        BT_LOGI("GATTC", "gattc_start_disconnect: handle = %x\r\n", connection_handle);

        app_bt_connection_cb_t *cb = find_conneciton_info_by_handle(connection_handle);
        if (cb != NULL) {
            bt_gattc_disconnect(connection_handle);
            heart_rate_deinit();
        } else {
            BT_LOGI("GATTC", "gattc_start_disconnect: connection handle no exist");
        }
    }
    else if (UT_APP_CMP("gpio client e")) {
        //bt_status_t status = BT_STATUS_FAIL;
        bt_hci_cmd_le_set_scan_enable_t disenable;
        disenable.le_scan_enable = BT_HCI_DISABLE;
        disenable.filter_duplicates = BT_HCI_DISABLE;
        bt_gap_le_set_scan(&disenable, NULL);
   
    }
    else if (UT_APP_CMP("gpio client s")) {
        //bt_status_t status = BT_STATUS_FAIL;
        bt_hci_cmd_le_set_scan_enable_t enable;
        enable.le_scan_enable = BT_HCI_ENABLE;
        enable.filter_duplicates = BT_HCI_ENABLE;
        os_memset(g_ble_scan_data, 0, sizeof(g_ble_scan_data));
        ble_smtcn_stop_adv();
        bt_gattc_set_scan(&enable);        
    }
    else if (UT_APP_CMP("gpio client r")) {
        //ex : ble gpio client r 0201 0703

        const char *handle = cmd + 14;
        const char *attribute_handle = cmd + 19;

        bt_gattc_read_charc_req_t req;

        req.opcode = BT_ATT_OPCODE_READ_REQUEST;
        req.attribute_handle = (uint16_t)strtoul(attribute_handle, NULL, 16);

        bt_gattc_read_charc((uint16_t)strtoul(handle, NULL, 16), &req);
    }
    else if (UT_APP_CMP("gpio client w")) {
        //ex : ble gpio client w 0201 0703 OFF
        //ex : ble gpio client w 0201 0703 ON
        
        const char *handle = cmd + 14; //23
        const char *attribute_handle = cmd + 19; //28
        const char *attribute_value = cmd + 24; //33

        bt_gattc_write_charc_req_t req;
        req.attribute_value_length = strlen(attribute_value);
        uint8_t buffer[20] = {0};

        req.att_req = (bt_att_write_req_t *)buffer;
        req.att_req->opcode = BT_ATT_OPCODE_WRITE_REQUEST;
        req.att_req->attribute_handle = (uint16_t)strtoul(attribute_handle, NULL, 16);
        os_memcpy(req.att_req->attribute_value, attribute_value, strlen(attribute_value));

        bt_gattc_write_charc((uint16_t)strtoul(handle, NULL, 16), &req);

    }

#endif


#ifdef MTK_BLE_SMTCN_ENABLE
    else if (UT_APP_CMP("wifi smart")) {
        BT_LOGI("APP", "[DTP]start adv\n");
        ble_smtcn_init();
        ble_smtcn_set_adv();
    }
#endif   
    else {
        int i;
        for (i=0;i<sizeof(bt_app_callback_table)/sizeof(struct bt_app_callback_table_t);i++) {
            if (UT_APP_CMP(bt_app_callback_table[i].name)) {
                return bt_app_callback_table[i].io_callback(input, output);
            }
        }
        BT_LOGE("APP", "%s: command not found", cmd);
    }

    return BT_STATUS_SUCCESS;
}

bt_gap_le_bonding_info_t *bt_gap_le_get_bonding_info(const bt_addr_t remote_addr)
{
    app_bt_bonded_info_t* bonded_info = get_bonded_info(&remote_addr, 1);
    if (bonded_info) {
        return &(bonded_info->info);
    }
    return NULL;
}
 
bt_gap_le_local_config_req_ind_t *bt_gap_le_get_local_config(void)
{
    local_config.local_key_req = &local_key_req;
    local_config.sc_only_mode_req = sc_only;
 
    return &local_config;
}

bt_status_t bt_gap_le_get_pairing_config(bt_gap_le_bonding_start_ind_t *ind)
{
    ind->pairing_config_req = pairing_config_req;

    return BT_STATUS_SUCCESS;
}


bt_status_t bt_app_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    BT_COLOR_SET(BT_COLOR_GREEN);
    BT_LOGI("APP", "CL:10%s: status(0x%04x)", __FUNCTION__, status);
    BT_COLOR_SET(BT_COLOR_WHITE);

#ifdef MTK_BLE_SMTCN_ENABLE
    ble_smtcn_event_callback(msg, status, buff);
#endif

#ifdef MTK_BLE_GPIO_SERVICE
    status = bt_gattc_event_callback(msg, status, buff);
#endif

    switch(msg) {
    /* GAP */
    case BT_POWER_ON_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_POWER_ON_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        {
            uint8_t idx;
            for (idx = 0; idx <BT_CONNECTION_MAX; idx++) {
                app_bt_bonded_info_t *app_bonded_info = NULL;
                app_bonded_info = find_bonded_info_by_index(idx);
                if (app_bonded_info != NULL) {
                    bt_gap_le_bonding_info_t *bonded_info = &(app_bonded_info->info);
                    //update resolving list
                    if (BT_STATUS_SUCCESS != app_add_dev_2_resolving_list(bonded_info)) {
                        BT_LOGE("APP", "Add Device to Resolving List FAILED!!!");
                    }
                    //update white list(use identity address or address)
                    if (BT_STATUS_SUCCESS != app_add_dev_2_white_list(bonded_info, &(app_bonded_info->bt_addr))) {
                        BT_LOGE("APP", "Add Device to White List FAILED!!!");
                    }
                }
            }
            /* set RPA timeout */
            bt_gap_le_set_resolvable_private_address_timeout(0x0384);
        }
        BT_COLOR_SET(BT_COLOR_WHITE);
        bt_app_advertising = false;
        bt_app_scanning = false;
        bt_app_connecting = false;
        
#ifdef MTK_BLE_GPIO_SERVICE
       //start BT device, name HRG, Macaddress AAAAAAAAAAAA
       //printf("Start BT device, name HRG, Macaddress AAAAAAAAAAAA \n\r");
       clear_bonded_info(); 
       ble_gpio_set_adv();
#endif 
        break;
    case BT_GAP_LE_SET_RANDOM_ADDRESS_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_SET_RANDOM_ADDRESS_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
    case BT_GAP_LE_SET_WHITE_LIST_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_SET_WHITE_LIST_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        list_updating = list_updating | (~BT_APP_WHITE_LIST_UPDATING);
        if (list_updating == 0x00){
            if (bt_app_advertising){
                adv_enable.advertising_enable = BT_HCI_ENABLE;
                bt_gap_le_set_advertising(&adv_enable, NULL, NULL, NULL);
            }
            if (bt_app_scanning){
                bt_gap_le_set_scan(&scan_enable, &scan_para);
            }
        }
        break;
    case BT_GAP_LE_SET_RESOLVING_LIST_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_SET_RESOLVING_LIST_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        list_updating = list_updating | (~BT_APP_RESOLVING_LIST_UPDATING);
        if (list_updating == 0x00){
            if (bt_app_advertising){
                adv_enable.advertising_enable = BT_HCI_ENABLE;
                bt_gap_le_set_advertising(&adv_enable, NULL, NULL, NULL);
            }
            if (bt_app_scanning){
                bt_gap_le_set_scan(&scan_enable, &scan_para);
            }
        }
        break;
    case BT_GAP_LE_SET_ADDRESS_RESOLUTION_ENABLE_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_SET_ADDRESS_RESOLUTION_ENABLE_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
    case BT_GAP_LE_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
    case BT_GAP_LE_SET_ADVERTISING_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_SET_ADVERTISING_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
    case BT_GAP_LE_SET_SCAN_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_SET_SCAN_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
    case BT_GAP_LE_ADVERTISING_REPORT_IND:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_ADVERTISING_REPORT_IND %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        print_advertising_report(buff);
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
    case BT_GAP_LE_CONNECT_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_CONNECT_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        bt_app_connecting = status == BT_STATUS_SUCCESS;
        break;
    case BT_GAP_LE_CONNECT_IND:
    {
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_CONNECT_IND %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_BLUE);

        bt_gap_le_connection_ind_t *connection_ind = (bt_gap_le_connection_ind_t *)buff;
        BT_LOGI("APP", "connection handle=0x%04x", connection_ind->connection_handle);
        BT_LOGI("APP", "role=%s",(connection_ind->role == BT_ROLE_MASTER)? "Master" : "Slave");
        BT_LOGI("APP", "peer address:%s", bt_debug_addr2str(&connection_ind->peer_addr));
#ifdef BLE_THROUGHPUT
        printf("connection handle=0x%04x\n", connection_ind->connection_handle);
        printf("peer address:%s\n", bt_debug_addr2str(&connection_ind->peer_addr));
#endif
        BT_COLOR_SET(BT_COLOR_WHITE);
        if (status == BT_STATUS_SUCCESS) {
            add_connection_info(buff);
            bt_handle_t handle = connection_ind->connection_handle;
            disconnect_para.connection_handle = handle;
            conn_update_para.connection_handle = handle;
            read_rssi.handle = handle;
            conn_interval = (connection_ind->conn_interval * 5)/4;
        #ifdef BLE_THROUGHPUT
            if (enable_dle) {
                bt_hci_cmd_le_set_data_length_t data_length;
                data_length.connection_handle = handle;
                data_length.tx_octets = 0xFA;
                data_length.tx_time = 0x150;
                bt_gap_le_update_data_length(&data_length);
            }
        #endif
        }
        bt_app_advertising = false;
        break;
    }
    case BT_GAP_LE_CONNECT_CANCEL_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_CONNECT_CANCEL_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        bt_app_connecting =  false;
        break;
    case BT_GAP_LE_DISCONNECT_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_DISCONNECT_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
    case BT_GAP_LE_DISCONNECT_IND:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_DISCONNECT_IND %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        delete_connection_info(buff);
        break;
    case BT_GAP_LE_CONNECTION_UPDATE_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_CONNECTION_UPDATE_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
    case BT_GAP_LE_CONNECTION_UPDATE_IND:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_CONNECTION_UPDATE_IND %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
    case BT_GAP_LE_BONDING_REPLY_REQ_IND:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_BONDING_REPLY_REQ_IND %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        {
            if (buff == NULL) {
                BT_LOGI("APP", "status = %d", status);
                BT_COLOR_SET(BT_COLOR_WHITE);
                return status;
            }
            bt_gap_le_bonding_reply_t rsp = {{{0}}};
            bt_gap_le_bonding_reply_req_ind_t *ind = (bt_gap_le_bonding_reply_req_ind_t *)buff;
            if (ind->method & BT_GAP_LE_SMP_PASSKEY_DISPLAY_MASK) {
                BT_COLOR_SET(BT_COLOR_BLUE);
                printf("------------------------------>Passkey: %06u<---------------------------------\n", ind->passkey_display);
                BT_COLOR_SET(BT_COLOR_WHITE);

            } else if (ind->method & BT_GAP_LE_SMP_PASSKEY_INPUT_MASK) {
                uint32_t i;
                BT_COLOR_SET(BT_COLOR_BLUE);
                sm_passkey = 0;
                printf("\nInput passkey: \n");
                //wait for input
                BT_COLOR_SET(BT_COLOR_WHITE);
                for (i = 0; i < 40; i++) {
                    if (sm_passkey != 0) {
                        break;
                    }
                    bt_os_layer_sleep_task(1000);
                }
                rsp.passkey = sm_passkey;
                status = bt_gap_le_bonding_reply(ind->handle, &rsp);
            } else if (ind->method == BT_GAP_LE_SMP_OOB) {
                os_memcpy(rsp.oob_data, oob_data, 16);

                status = bt_gap_le_bonding_reply(ind->handle, &rsp);
            } else if (ind->method & BT_GAP_LE_SMP_NUMERIC_COMPARISON_MASK) {
                uint32_t i;
                BT_COLOR_SET(BT_COLOR_BLUE);
                printf("------------------------------>Passkey: %06u<---------------------------------\n", ind->passkey_display);
                sm_passkey = 0;
                printf("\nConfirm numeric number:Y/N\n");
                BT_COLOR_SET(BT_COLOR_WHITE);
                for (i = 0; i < 40; i++) {
                    if (nc_value_correct[0] != 0) {
                        break;
                    }
                    bt_os_layer_sleep_task(1000);
                }
                if (nc_value_correct[0]!='n' && nc_value_correct[0]!='N') {
                    rsp.nc_value_matched = true;
                } else {
                    rsp.nc_value_matched = false;
                }

                status = bt_gap_le_bonding_reply(ind->handle, &rsp);
            }
        }
        break;
    case BT_GAP_LE_BONDING_COMPLETE_IND:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_BONDING_COMPLETE_IND %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        if (status == BT_STATUS_SUCCESS) {
        const bt_gap_le_bonding_complete_ind_t *ind = (bt_gap_le_bonding_complete_ind_t*)buff;
        app_bt_bonded_info_t *bonded_info = find_bonded_info_by_handle(ind->handle);
        app_bt_connection_cb_t *con = find_conneciton_info_by_handle(ind->handle);
        /* If peer identity address is not null, update to resolving list and white list*/
            //If advertising or scanning is enable, Disable advertising or scanning.
            if (bt_app_advertising){
                adv_enable.advertising_enable = BT_HCI_DISABLE;
                bt_gap_le_set_advertising(&adv_enable, NULL, NULL, NULL);
            }
            if (bt_app_scanning){
                bt_gap_le_set_scan(&scan_disable, NULL);
            }
            // If we got IRK/Identity address from peer, we have to change
            // 1. connection info's bd address; app_bt_connection_cb_t
            // 2. bonding info's bd address; app_bt_bonded_info_t
            if (BT_ADDR_TYPE_UNKNOW != bonded_info->info.identity_addr.address.type){
                /*Because value of bonded_info->info.identity_addr.address_type is 0[Public Identity] or 1[Random Identity],
                 *but Identity address type were definied 2 or 3 in spec.
                 *We have to "+2" for synchronization.
                */
                con->peer_addr = bonded_info->info.identity_addr.address;
                con->peer_addr.type += 2;
                bonded_info->bt_addr = bonded_info->info.identity_addr.address;
                bonded_info->bt_addr.type += 2;
            }
            //update resolving list
                if (BT_STATUS_SUCCESS == app_add_dev_2_resolving_list(&(bonded_info->info))) {
                    list_updating = list_updating | BT_APP_RESOLVING_LIST_UPDATING;
                }
            //update white list(use identity address or address)
            if (BT_STATUS_SUCCESS == app_add_dev_2_white_list(&(bonded_info->info), &(con->peer_addr))) {
                    list_updating = list_updating | BT_APP_WHITE_LIST_UPDATING;
            }
        }
        break;
    case BT_GAP_LE_READ_RSSI_CNF:
    {
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_READ_RSSI_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_BLUE);

        const bt_hci_evt_cc_read_rssi_t *rssi = (bt_hci_evt_cc_read_rssi_t *)buff;
        BT_LOGI("APP", "connection handle=0x%04x", rssi->handle);
        if (rssi->rssi == 127) {
            BT_LOGI("APP", "rssi cannot be read");
        } else {
            if ((rssi->rssi>>7)>0){
                BT_LOGI("APP", "rssi=%ddBm", ((~rssi->rssi)&0xFF)+0x01);
            } else {
                BT_LOGI("APP", "rssi=%ddBm", rssi->rssi);
            }
        }
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
    }    
    case BT_GAP_LE_UPDATE_DATA_LENGTH_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_UPDATE_DATA_LENGTH_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
    case BT_GAP_LE_SET_TX_POWER_CNF:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_SET_TX_POWER_CNF %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        break;
#if 0
    case  BT_GAP_LE_MULTI_ADVERTISING_STATE_CHANGE_IND:
    {
        BT_LOGI("APP", "BT_GAP_LE_MULTI_ADVERTISING_STATE_CHANGE_IND");
        bt_gap_le_multi_advertising_state_change_ind_t *state_change_t = 
            (bt_gap_le_multi_advertising_state_change_ind_t *)buff;
        BT_LOGI("APP", "instance:%d, reason:0x%02x, connection handle:0x%04x",
            state_change_t->instance,
            state_change_t->reason,
            state_change_t->connection_handle);
        break;
    }
#endif
#ifdef BT_BQB
    case BT_GAP_LE_BQB_DISCONNECT_REQ_IND:
        BT_COLOR_SET(BT_COLOR_RED);
        BT_LOGI("APP", "BT_GAP_LE_BQB_DISCONNECT_REQ_IND %s",
            (status == BT_STATUS_SUCCESS)? "Success":"Failed");
        BT_COLOR_SET(BT_COLOR_WHITE);
        return bt_gap_le_disconnect(&disconnect_para);
#endif
    case BT_GATTC_READ_USING_CHARC_UUID:
        {
            if (bt_app_wait_peer_central_address_resolution_rsp == true){
                bt_app_wait_peer_central_address_resolution_rsp = false;
                BT_COLOR_SET(BT_COLOR_RED);
                BT_LOGI("APP", "Read Peer Central Address Resolution characteristic");
                BT_COLOR_SET(BT_COLOR_BLUE);
                bt_gattc_read_by_type_rsp_t rsp = *((bt_gattc_read_by_type_rsp_t *)buff);
                if (rsp.att_rsp->opcode == BT_ATT_OPCODE_READ_BY_TYPE_RESPONSE){

                    if (status == BT_STATUS_SUCCESS && rsp.att_rsp == NULL) {
                        BT_LOGI("APP", "Read Peer Central Address Resolution characteristic FINISHED!!");
                        BT_COLOR_SET(BT_COLOR_WHITE);
                        break;
                    }

                    if (rsp.att_rsp == NULL) {
                        BT_LOGI("APP", "status = %d", status);
                        BT_COLOR_SET(BT_COLOR_WHITE);
                        break;
                    }

                    uint8_t *attribute_data_list = rsp.att_rsp->attribute_data_list;
                    uint8_t Peer_CAR_supporting = 0;

                    if (rsp.att_rsp->length - 2 == 1){
                        Peer_CAR_supporting = *((uint8_t *)(attribute_data_list + 2));
                        BT_LOGI("APP", "Peer Central Address Resolution Supporting= %d",Peer_CAR_supporting);
                    }

                } else if (rsp.att_rsp->opcode == 0x1) {
                    bt_gattc_error_rsp_t error_rsp = *((bt_gattc_error_rsp_t *)buff);
                    BT_LOGI("APP", "Can not find Peer Central Address Resolution");
                    BT_LOGI("APP", "Error_opcode=0x%02x, error_code=0x%02x",error_rsp.att_rsp->error_opcode, error_rsp.att_rsp->error_code);
                } else {
                    BT_LOGI("APP", "Read Peer Central Address Resolution Error:Can not handle feedback");
                }
                BT_COLOR_SET(BT_COLOR_WHITE);
                return BT_STATUS_SUCCESS;
            }
        }
    }

    if (status == BT_STATUS_OUT_OF_MEMORY) {
        return BT_STATUS_OUT_OF_MEMORY;
    }

    if (ut_app_callback) {
        status = ut_app_callback(msg, status, buff);
    }

    return status;
}



bt_status_t bt_gatts_get_authorization_check_result(bt_gatts_authorization_check_req_t *req)
{
    bt_gap_le_bonding_info_t *bonded_info = &(find_bonded_info_by_handle(req->connection_handle)->info);
    BT_LOGI("APP", "Peer ask to access attribute with authorization requirement.");
    BT_LOGI("APP", "connection[0x%04x] attribute handle[0x%04x] [%s]",req->connection_handle, req->attribute_handle,
            req->read_write==BT_GATTS_CALLBACK_READ? "Read":"Write");
    BT_LOGI("APP", "Security mode[0x%02x]",bonded_info->key_security_mode);
    if ((bonded_info->key_security_mode & BT_GAP_LE_SECURITY_AUTHENTICATION_MASK) >0) {
        /* If you agree peer device can access all characteristic with
           authorization permission, you can set #BT_GAP_LE_SECURITY_AUTHORIZATION_MASK
           flag, and GATTS will not call for authorization check again. */
        bonded_info->key_security_mode = bonded_info->key_security_mode |BT_GAP_LE_SECURITY_AUTHORIZATION_MASK;
        /* If application accept peer access this attribute. */
        return BT_STATUS_SUCCESS;
    } else {
        /* If application reject peer access this attribute. */
        return BT_STATUS_UNSUPPORTED;
    }

}
