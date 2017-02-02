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

#include "bt_debug.h"
#include "bt_gap_le.h"
#include "bt_gattc_connect.h"
#include "bt_gattc_srv.h"
#include "connection_info.h"
#include "hrc.h"
#include <stdlib.h>
#include "gatt_service.h"

extern bt_hci_cmd_le_set_advertising_parameters_t adv_para; //MTK_BLE_GPIO_SERVICE

#if 0 //MTK_BLE_GPIO_SERVICE
static bt_gap_le_local_config_req_ind_t g_hr_app_local_config;

bt_gap_le_local_key_t hr_local_key_req= {
    .encryption_info = {{0}},
    .master_id = {0},
    .identity_info = {{0x12,0x34,0x56,0x78,0x9a,0xbc,0xde,0xf0,0x19,0x28,0x55,0x33,0x68,0x33,0x56,0xde}},
    .signing_info = {{0}}
};

bt_gap_le_smp_pairing_config_t hr_pairing_config_req = {//mitm, bond, oob
    .maximum_encryption_key_size = 16,
    .io_capability = BT_GAP_LE_SMP_NO_INPUT_NO_OUTPUT,
    .auth_req = BT_GAP_LE_SMP_AUTH_REQ_BONDING,
    .oob_data_flag = BT_GAP_LE_SMP_OOB_DATA_NOT_PRESENTED,
    .initiator_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
    .responder_key_distribution = BT_GAP_LE_SMP_KEY_DISTRIBUTE_ENCKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_IDKEY | BT_GAP_LE_SMP_KEY_DISTRIBUTE_SIGN,
};

bool hr_sc_only= false;

#endif

//void gattc_connect_cb(bt_status_t status, app_bt_connection_cb_t *param);

static gattc_conn_t *gattc_create_conn(app_bt_connection_cb_t *conn_info);

static bt_status_t gattc_set_adv(void);

static void copy_str_to_addr(uint8_t *addr, const char *str);

//void gattc_disconnect_cb(bt_status_t status, bt_hci_evt_disconnect_complete_t *param);


static void copy_str_to_addr(uint8_t *addr, const char *str)
{
    unsigned int value;
    int using_long_format = 0;
    int using_hex_sign = 0;

    if (str[2] == ':' || str[2] == '-') {
        using_long_format = 1;
    }

    if (str[1] == 'x') {
        using_hex_sign = 2;
    }

    for (int i = 0; i < 6; i++) {
        sscanf(str + using_hex_sign + i * (2 + using_long_format), "%02x", &value);
        addr[5 - i] = (uint8_t) value;
    }
}


//bt_status_t bt_hr_io_callback(void *input, void *output)
bt_status_t bt_hr_io_callback(uint8_t *cmd)
{
    uint8_t len;
    bt_status_t status = BT_STATUS_FAIL;
    //const char *cmd = input;
    len = strlen((const char *)cmd);
    BT_LOGI("GATTC", "bt_hr_io_callback: -- start: cmd[0] = %c, len = %d\r\n", cmd[0], len);
    //clear_bonded_info();
    #if 0
    if (UT_APP_CMP("bt hr c")) {
        BT_LOGI("GATTC", "gattc_start_connect: -- start\r\n");
        if (len <= 2) {
            BT_LOGI("GATTC", "please help to check BT address is ok\r\n");
            return status;
        }
        uint8_t addr[6], peer_type;
        peer_type = (uint8_t)strtoul(cmd + strlen("bt hr c "), NULL, 10);
        BT_LOGI("GATTC", "connect: peer_type = %d", peer_type);
        const char *addr_str = cmd + strlen("bt hr c ") + 2;
        copy_str_to_addr(addr, addr_str);        
        BT_LOGI("GATTC", "connect: addr-- start: addr[0] = %04x, addr[1] = %04x,addr[2] = %04x,addr[3] = %04x,addr[4] = %04x,addr[5] =  %x04x\r\n",
                addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
        status = bt_gattc_connect(addr, peer_type);
    } else if (UT_APP_CMP("bt hr d")) {
        BT_LOGI("GATTC", "gattc_start_disconnect: -- start\r\n" );
        const char *handle = cmd + strlen("d ");
        uint16_t connection_handle = (uint16_t)strtoul(handle, NULL, 16);
        BT_LOGI("GATTC", "gattc_start_disconnect: handle = %x\r\n", connection_handle);

        app_bt_connection_cb_t *cb = find_conneciton_info_by_handle(connection_handle);
        if (cb != NULL) {
            status = bt_gattc_disconnect(connection_handle);
        } else {
            BT_LOGI("GATTC", "gattc_start_disconnect: connection handle no exist");
        }
    } else if (UT_APP_CMP("po")) {
        status = bt_power_on(NULL, NULL);
        bt_gatts_set_max_mtu(128); /* This value should consider with MM Tx/Rx buffer size. */
    } else if (UT_APP_CMP("bt hr f")) {

        status = bt_power_off();

    } else if (UT_APP_CMP("bt hr e")) {
        bt_hci_cmd_le_set_scan_enable_t disenable;
        disenable.le_scan_enable = BT_HCI_DISABLE;
        disenable.filter_duplicates = BT_HCI_DISABLE;
        status = bt_gap_le_set_scan(&disenable, NULL);
    } else if (UT_APP_CMP("bt hr s")) {
    bt_hci_cmd_le_set_scan_enable_t enable;
        enable.le_scan_enable = BT_HCI_ENABLE;
        enable.filter_duplicates = BT_HCI_ENABLE;
        status = bt_gattc_set_scan(&enable);
    } else if (UT_APP_CMP("bt hr a")) {
        status = gattc_set_adv();
    }
    #endif
        if (cmd[0] == 'c') {
        BT_LOGI("GATTC", "gattc_start_connect: -- start\r\n");
        if (len <= 2) {
            BT_LOGI("GATTC", "please help to check BT address is ok\r\n");
            return status;
        }
        uint8_t addr[6], peer_type;
        peer_type = (uint8_t)strtoul((const char*)cmd + strlen("c "), NULL, 10);
        BT_LOGI("GATTC", "connect: peer_type = %d", peer_type);
        const char *addr_str = (const char*)cmd + strlen("c ") + 2;
        copy_str_to_addr(addr, addr_str);        
        BT_LOGI("GATTC", "connect: addr-- start: addr[0] = %04x, addr[1] = %04x,addr[2] = %04x,addr[3] = %04x,addr[4] = %04x,addr[5] =  %x04x\r\n",
                addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
        status = bt_gattc_connect(addr, peer_type);
    } else if (cmd[0] == 'd') {
        BT_LOGI("GATTC", "gattc_start_disconnect: -- start\r\n" );
        const char *handle = (const char*)cmd + strlen((const char*)("d "));
        uint16_t connection_handle = (uint16_t)strtoul(handle, NULL, 16);
        BT_LOGI("GATTC", "gattc_start_disconnect: handle = %x\r\n", connection_handle);

        app_bt_connection_cb_t *cb = find_conneciton_info_by_handle(connection_handle);
        if (cb != NULL) {
            status = bt_gattc_disconnect(connection_handle);
        } else {
            BT_LOGI("GATTC", "gattc_start_disconnect: connection handle no exist");
        }
    } else if (cmd[0] == 'f') {

        status = bt_power_off();

    } else if (cmd[0] == 'e') {
        bt_hci_cmd_le_set_scan_enable_t disenable;
        disenable.le_scan_enable = BT_HCI_DISABLE;
        disenable.filter_duplicates = BT_HCI_DISABLE;
        status = bt_gap_le_set_scan(&disenable, NULL);
    } else if (cmd[0] == 's') {
        bt_hci_cmd_le_set_scan_enable_t enable;
        enable.le_scan_enable = BT_HCI_ENABLE;
        enable.filter_duplicates = BT_HCI_ENABLE;
        status = bt_gattc_set_scan(&enable);
    } else if (cmd[0] == 'a') {
        status = gattc_set_adv();
    }
    return status;
}

#if 0 //MTK_BLE_GPIO_SERVICE
bt_hci_cmd_le_set_advertising_parameters_t adv_para = {
    .advertising_interval_min = 0x0800,
    .advertising_interval_max = 0x0800,
    .advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED,
    .advertising_channel_map = 7,
    .advertising_filter_policy = 0
};
#endif

static bt_status_t gattc_set_adv(void)
{
    bt_hci_cmd_le_set_advertising_enable_t enable;
    bt_hci_cmd_le_set_advertising_data_t adv_data = {
        .advertising_data_length = 31,
        .advertising_data = "DDDDDHR_ADV_DATA",
    };

	char gatts_device_name[256] = { "MTKHB" };

	//extern char gatts_device_name[256];


    memset(gatts_device_name, 0x00, sizeof(gatts_device_name));
    memcpy(gatts_device_name, &adv_data.advertising_data[5], 2);
    adv_data.advertising_data[0] = 2; //adv_length
    adv_data.advertising_data[1] = BT_GAP_LE_AD_TYPE_FLAG;
    adv_data.advertising_data[2] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED | BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE;
    adv_data.advertising_data[3] = 21; //adv_length
    adv_data.advertising_data[4] = 0x09;
    enable.advertising_enable = BT_HCI_ENABLE;
	
   return bt_gap_le_set_advertising(&enable, &adv_para, &adv_data, NULL);
}

#if 0 //MTK_BLE_GPIO_SERVICE

bt_status_t bt_app_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    BT_LOGI("GATTC", "bt_app_event_callback msg :  =%x" , msg);
    //bt_msg_type_t msg_id = msg;
    switch (msg) {
        /* GAP */
        case BT_POWER_ON_CNF: {
                BT_LOGI("GATTC", "BT_GAP_POWER_ON_CNF %s",
                        (status == BT_STATUS_SUCCESS) ? "Success" : "Failed");

                //gattc_set_adv();
                gattc_service_init();
                heart_rate_init();
            }
            break;
        case BT_GAP_LE_SET_ADVERTISING_CNF: {
            BT_LOGI("GATTC", "BT_GAP_SET_ADVERTISING_CNF %s",
                    (status == BT_STATUS_SUCCESS) ? "Success" : "Failed");
            BT_LOGI("GATTC", "bt_app_event_callback:adv cnf");
            }
            break;
        case BT_GAP_LE_ADVERTISING_REPORT_IND:{
            BT_LOGI("GATTC", "BT_GAP_ADVERTISING_REPORT_IND %s",
                    (status == BT_STATUS_SUCCESS) ? "Success" : "Failed");
            gattc_notify_scan_info_to_all_user((bt_gap_le_advertising_report_ind_t *)buff);

            }
        break;
        case BT_GAP_LE_CONNECT_IND:
        {
            BT_LOGI("GATTC", "BT_GAP_CONNECT_IND %s",
                    (status == BT_STATUS_SUCCESS) ? "Success" : "Failed");

            bt_handle_t handle;
            app_bt_connection_cb_t *conn_info = NULL;
            const bt_gap_le_connection_ind_t *connect_ind = (bt_gap_le_connection_ind_t *)buff;
            if (connect_ind == NULL) {
                BT_LOGI("GATTC", "connection ind is null\r\n");
                return status;
            }
            handle = connect_ind->connection_handle;
            BT_LOGI("GATTC", "connection handle=0x%04x", handle);
            BT_LOGI("GATTC", "role=%s", (connect_ind->role == BT_ROLE_MASTER) ? "Master" : "Slave");
            BT_LOGI("GATTC", "peer address:%s (%s)\r\n ", bt_debug_bd_addr2str(connect_ind->peer_addr.addr),
                    connect_ind->peer_addr.type ? "Random Device Address" : "Public Device Address");

            if (status == BT_STATUS_SUCCESS) {
                add_connection_info(buff);

                BT_LOGI("GATTC", "connection role=0x%04x", connect_ind->role);
                if (connect_ind->role == BT_ROLE_MASTER) {
                    conn_info = find_conneciton_info_by_handle(handle);
                    gattc_connect_cb(status, conn_info);
                }
            }
        }
            break;
        case BT_GAP_LE_DISCONNECT_IND: {
            BT_LOGI("GATTC", "BT_GAP_DISCONNECT_IND %s",
                    (status == BT_STATUS_SUCCESS) ? "Success" : "Failed");
            gattc_disconnect_cb(status, (bt_hci_evt_disconnect_complete_t *)buff);
            delete_connection_info(buff);
            }
            break;
        case BT_GAP_LE_SET_SCAN_CNF:{
            BT_LOGI("GATTC", "BT_GAP_SET_SCAN_CNF %s",
                    (status == BT_STATUS_SUCCESS) ? "Success" : "Failed");
            }
            break;
        default:
            break;
    }
    status = bt_gattc_event_callback(msg, status, buff);

    return status;
}
#endif

void gattc_disconnect_cb(bt_status_t status, bt_hci_evt_disconnect_complete_t *param)
{
    gattc_conn_t *gattc_conn;

    BT_LOGI("GATTC", "gattc_disconnect_cb: -- start: con_id = %d, err_code = %d", param->connection_handle, status);
    gattc_conn = bt_gattc_link_info_by_id(param->connection_handle);

    if (gattc_conn) {
        BT_LOGI("GATTC", "gattc_disconnect_cb: --gattc_conn = %x ", gattc_conn);
        gattc_notify_conn_info_to_all_user(gattc_conn, status, NULL, 0);
        memset(gattc_conn, 0, sizeof(gattc_conn_t));
    }
    BT_LOGI("GATTC", "gattc_disconnect_cb: -- end");
}

void gattc_connect_cb(bt_status_t status, app_bt_connection_cb_t *param)
{
    gattc_conn_t *gattc_conn = NULL;

    if (param) {
        BT_LOGI("GATTC", "gattc_connect_cb: -- start: connect_handle = %d, err_code = %d", param->connection_handle, status);
        gattc_conn = bt_gattc_link_info_by_id(param->connection_handle);

        if (gattc_conn == NULL) {
            BT_LOGI("GATTC", "gattc_connect_cb: -- conn is not exist");
            gattc_conn = gattc_create_conn(param);
        }
        if (gattc_conn) {

            BT_LOGI("GATTC", "gattc_connect_cb: -- conn link is valid");
            gattc_conn->state = GATTC_CONNECTED;
            gattc_notify_conn_info_to_all_user(gattc_conn, status, param, 1);
        } else {
            BT_LOGI("GATTC", "gattc_connect_cb: -- create conn is fail");
        }
    }

}


void gattc_service_init(void)
{
    /*initition*/
    BT_LOGI("GATTC", "gattc service init  start...");
    memset(&g_gattc_ctx, 0, sizeof(gattc_context_t));
    BT_LOGI("GATTC", "gattc service init  end...");
}


/**
*need listern when bluetooth power on , it should do scan
*/
bt_status_t bt_gattc_set_scan(bt_hci_cmd_le_set_scan_enable_t *is_enable)
{
    bt_status_t status;
    bt_hci_cmd_le_set_scan_parameters_t para;

    BT_LOGI("GATTC", "gattc_start_scan: -- start");
    para.le_scan_type = BT_HCI_SCAN_TYPE_ACTIVE;
    para.le_scan_interval = 0x0024; //0x0030;
    para.le_scan_window = 0x0011;//0x0030;
    para.own_address_type = BT_HCI_SCAN_ADDR_RANDOM;
    para.scanning_filter_policy = 0x00;

    status = bt_gap_le_set_scan(is_enable, &para);

    BT_LOGI("GATTC", "gattc_start_scan: -- end: status = %d", status);
	return status;
}


bt_status_t bt_gattc_connect(uint8_t *addr, uint8_t type)
{
    bt_status_t status;
    BT_LOGI("GATTC", "[GATTC]gattc_connect: -- end: start");

    bt_hci_cmd_le_create_connection_t conn_para;
    conn_para.le_scan_interval = 0x0010;
    conn_para.le_scan_window = 0x0010;
    conn_para.initiator_filter_policy = BT_HCI_CONN_FILTER_ASSIGNED_ADDRESS;
    if (type == 0) {
        conn_para.peer_address.type = BT_ADDR_PUBLIC;
    } else {
        conn_para.peer_address.type = BT_ADDR_RANDOM;
    }

    memcpy(conn_para.peer_address.addr, addr, BT_BD_ADDR_LEN);

    conn_para.own_address_type = BT_ADDR_RANDOM;
    conn_para.conn_interval_min = 0x0006;
    conn_para.conn_interval_max = 0x0080;
    conn_para.conn_latency = 0x0000;
    conn_para.supervision_timeout = 0x07d0;
    conn_para.minimum_ce_length = 0x0000;
    conn_para.maximum_ce_length = 0x0050;
    status = bt_gap_le_connect(&conn_para);

    BT_LOGI("GATTC", "gattc_connect: -- end: status = %d", status);
    return status;
}



bt_status_t  bt_gattc_disconnect(uint16_t conn_id)
{
    bt_status_t status;
    bt_hci_cmd_disconnect_t disconnect;
    disconnect.connection_handle = conn_id;
    BT_LOGI("GATTC", "connection_handle(0x%04x)", disconnect.connection_handle);
    disconnect.reason = BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION;
    status = bt_gap_le_disconnect(&disconnect);
    BT_LOGI("GATTC", "bt_gattc_disconnect: -- status = %d", status);
	return status;
}


static gattc_conn_t *gattc_create_conn(app_bt_connection_cb_t *conn_info)
{
    gattc_conn_t *gattc_conn;
    for (uint8_t i = 0 ; i < SRV_MAX_DEV; i++) {
        gattc_conn = g_gattc_ctx.conntext + i;
        if (!g_gattc_ctx.conntext[i].flag) {
            memset(gattc_conn, 0, sizeof(gattc_conn_t));
            gattc_conn->flag = 1;
            gattc_conn->state = GATTC_CONNECTED;
            gattc_conn->conn_id = conn_info->connection_handle;
            return gattc_conn;
        }
    }
    return NULL;
}


gattc_conn_t *bt_gattc_link_info_by_id(uint16_t conn_id)
{
    uint8_t i;
    gattc_conn_t *gattc_conn;
    BT_LOGI("GATTC", "bt_gattc_link_info_by_id--start: con_id = %d", conn_id);
    for (i = 0 ; i < SRV_MAX_DEV; i++) {
        gattc_conn = g_gattc_ctx.conntext + i;
        if (gattc_conn->flag) {
            if (gattc_conn->conn_id == conn_id ) {
                return gattc_conn;
            }
        }
    }
    BT_LOGI("Gattc", "bt_gattc_link_info_by_id--NOT Find conn info ");
    return NULL;
}

#ifdef __BT_HB_DUO__
const bt_gap_config_t bt_custom_config = {
	.inquiry_mode = 2,
	.io_capability = BT_GAP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
	.cod = 0x240404,
	.device_name = { "HB Duo dev" },
};

const bt_gap_config_t* bt_gap_get_local_configuration(void)
{
	return &bt_custom_config;
}
#endif

#if 0 //MTK_BLE_GPIO_SERVICE
bt_gap_le_local_config_req_ind_t *bt_gap_le_get_local_config(void)
{
	g_hr_app_local_config.local_key_req = &hr_local_key_req;
	g_hr_app_local_config.sc_only_mode_req = hr_sc_only;
	return &g_hr_app_local_config;
}

bt_gap_le_bonding_info_t le_bond;
bt_gap_le_bonding_info_t *bt_gap_le_get_bonding_info(const bt_addr_t remote_addr)
{
	BT_LOGI("GATTC", "BT_GAP_BONDING_INFO_REQ_IND %s");

	return &le_bond;//&(get_bonded_info(&remote_addr, 1)->info);
}

bt_status_t bt_gap_le_get_pairing_config(bt_gap_le_bonding_start_ind_t *ind)
{
	ind->pairing_config_req = hr_pairing_config_req;
	return BT_STATUS_SUCCESS;
}
#endif

#ifdef  __TS_WIN32__
#define BT_A2DP_MAKE_CODEC_SBC(role, min_bit_pool, max_bit_pool, block_length, subband_num, alloc_method, sample_rate, channel_mode) { \
                    BT_A2DP_CODEC_SBC, role, sizeof(bt_a2dp_sbc_codec_t), {\
                    (channel_mode&0x0F) | (sample_rate&0x0F)<<4, \
					(alloc_method&0x03) | (subband_num&0x03)<<2 | (block_length&0x0F)<<4, \
					(min_bit_pool & 0xF) ,(min_bit_pool & 0xF)>>2,((min_bit_pool>>4) & 0xF), (max_bit_pool & 0xFF)}}

#define BT_A2DP_MAKE_CODEC_AAC(role, vbr, object_type, channels, sample_rate, bit_rate)  { \
                    BT_A2DP_CODEC_AAC, role, sizeof(bt_a2dp_aac_codec_t), {\
                    object_type, ((sample_rate>>4)&0xFF), 0, (channels),(sample_rate&0x0F), \
                    ((bit_rate>>16)&0x7F), (vbr), ((bit_rate>>8)&0xFF), (bit_rate&0xFF)}}

const static bt_a2dp_codec_capability_t init_codec[] = {
	//{ 0, 1, 4, { 0xff, 0xff, 0x19, 0x4d } },
	//{ 2, 1, 6, { 0xc0, 0xff, 0x8c, 0xe0, 0x00, 0x00 } }
	BT_A2DP_MAKE_CODEC_SBC(BT_A2DP_SINK, 2, 75, 0x0f, 0x0f, 0x03, 0x0f, 0x0f),
	BT_A2DP_MAKE_CODEC_AAC(BT_A2DP_SINK, 1, 0xC0, 0x03, 0x0ff8, 0x60000)
};
bt_status_t bt_a2dp_get_init_params(bt_a2dp_init_params_t *params)
{
	int32_t idx = 0;
	bt_a2dp_codec_capability_t * codec = init_codec;
	if (params == NULL) {
		return BT_STATUS_FAIL;
	}

	params->codec_number = 2;
	params->codec_list = (bt_a2dp_codec_capability_t *)init_codec;
	BT_LOGI("avtdp", "[Music_APP] codec, type:%d, 0x%08x", init_codec[0].type, *(uint32_t *)&init_codec[0].codec);
	BT_LOGI("avtdp", "[Music_APP] codec, type:%d, 0x%08x", init_codec[1].type, *(uint32_t *)&init_codec[1].codec);

	BT_LOGI("avtdp", "[Music_APP] A2DP init. OK.");

	return BT_STATUS_SUCCESS;
}
#endif
