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

/*mian for  gattc operation*/
#include "bt_gattc_handle_op.h"
#include "bt_gattc.h"
#include "bt_debug.h"
#include "bt_gattc_connect.h"
#include "bt_gattc_srv.h"
#include "hrc.h"
#include <stdlib.h>
#ifdef MTK_MCS_ENABLE
#include "mcs.h"
#endif


gattc_conn_t g_gattc_conn;
static void bt_gattc_explore_service(uint16_t conn_id, bt_gatt_service_t *srv);

static void bt_gattc_discover_primary_service_cnf(bt_status_t status, bt_gattc_find_by_type_value_rsp_t *p_event_data);

static void bt_gattc_discover_include_service_cnf(bt_status_t status, bt_gattc_read_by_type_rsp_t *p_event_data);

static void bt_gattc_discover_charactiristics_cnf(bt_status_t status, bt_gattc_read_by_type_rsp_t *p_event_data);

static void bt_gattc_discover_descriptors_cnf(bt_status_t status, bt_gattc_find_info_rsp_t *p_event_data);

static void bt_gattc_write_char_cnf(bt_status_t status, bt_gattc_write_rsp_t *param);

static void bt_gattc_discover_complete(uint16_t conn_id, bt_status_t status, bt_gatt_service_t *service);

static bt_status_t bt_gattc_find_all_descriptor_of_char(uint16_t conn_id,  bt_gatt_service_t *service, uint8_t index);

static bt_gatt_service_t *bt_gattc_get_service(uint16_t conn_id);

static void bt_gattc_char_val_notify_and_ind(bt_status_t status, bt_gatt_handle_value_notification_t *rsp);

static void bt_gattc_notify_error(uint16_t conn_id, bt_status_t status);

static void bt_gattc_parse_characteristic(bt_gattc_read_by_type_rsp_t *data, bt_gatt_service_t *service_node);

static void bt_gattc_parse_descriptor(bt_gattc_find_info_rsp_t *data, bt_gatt_char_t *char_node);

static bt_status_t bt_gattc_discover_charateristic(uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle);

static void bt_gattc_parse_primary_service(bt_gattc_find_by_type_value_rsp_t *rsp, gatt_data_t *data);

static bt_status_t bt_gattc_discover_include_service(uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle);

static void bt_gattc_parse_include_service(bt_gattc_read_by_type_rsp_t *rsp, bt_gatt_service_t *service);


gatt_data_t g_data;


#define BLE_GPIO_ADDRESS               {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}
#define BLE_GPIO_ADV_DATA              "DDDDDDDDDHRG"
#define BLE_GPIO_ADV_DATA_LEN          12
#define BLE_GPIO_DEVICE_NAME           "HRG"
#define BLE_GPIO_SERVICE_UUID          (0x18BB)
#define BLE_GPIO_MAX_INTERVAL          0x00C0    /*The range is from 0x0020 to 0x4000.*/
#define BLE_GPIO_MIN_INTERVAL          0x00C0    /*The range is from 0x0020 to 0x4000.*/
#define BLE_GPIO_CHANNEL_NUM           7
#define BLE_GPIO_FILTER_POLICY         0
#define BLE_GPIO_AD_FLAG_LEN           2
#define BLE_GPIO_AD_UUID_LEN           3
#define BLE_GPIO_AD_NAME_LEN           4

#if 0
#define BLE_SMTCN_ADDRESS               {0x0C, 0x01, 0x02, 0x03, 0x04, 0x05}
#define BLE_SMTCN_ADV_DATA              "DDDDDDDDDBLE_SMTCN"
#define BLE_SMTCN_ADV_DATA_LEN          18
#define BLE_SMTCN_DEVICE_NAME           "BLE_SMTCN"
#define BLE_SMTCN_SERVICE_UUID          (0x18AA)
#define BLE_SMTCN_MAX_INTERVAL          0x00C0    /*The range is from 0x0020 to 0x4000.*/
#define BLE_SMTCN_MIN_INTERVAL          0x00C0    /*The range is from 0x0020 to 0x4000.*/
#define BLE_SMTCN_CHANNEL_NUM           7
#define BLE_SMTCN_FILTER_POLICY         0
#define BLE_SMTCN_AD_FLAG_LEN           2
#define BLE_SMTCN_AD_UUID_LEN           3
#define BLE_SMTCN_AD_NAME_LEN           10
#endif

bool gpio_started;

void ble_gpio_set_adv(void)
{
    bt_bd_addr_t addr = BLE_GPIO_ADDRESS;
    bt_bd_addr_ptr_t random_addr = addr;    
    extern char gatts_device_name[256];
    
    memset(gatts_device_name, 0x00, sizeof(gatts_device_name));
    strcpy(gatts_device_name, BLE_GPIO_DEVICE_NAME);
    
    bt_gap_le_set_random_address(random_addr);

    gpio_started = true;
}


/* For Profile API only for connect id*/
bt_status_t bt_gattc_start_discover_service(uint16_t conn_id)
{
    bt_status_t status = 0;
    uint8_t buff[40];
    uint8_t uuid[2] = {0x0D, 0x18
                      };
    bt_gattc_discover_primary_service_by_uuid_req_t req;
    gattc_conn_t *gattc_conn = bt_gattc_link_info_by_id(conn_id);
    if (gattc_conn) {
        if (gattc_conn->state == GATTC_CONNECTED) {
            gattc_conn->state = GATTC_IN_SEARCHING;
            req.attribute_value_length = 2;
            req.att_req = (bt_att_find_by_type_value_req_t *)buff;
            req.att_req->opcode = BT_ATT_OPCODE_FIND_BY_TYPE_VALUE_REQUEST;
            req.att_req->starting_handle = 0x0001;
            req.att_req->ending_handle = 0xFFFF;
            memcpy(req.att_req->attribute_value, uuid, 2);
            req.att_req->uuid = BT_GATT_UUID16_PRIMARY_SERVICE;

            status = bt_gattc_discover_primary_service_by_uuid(conn_id, &req);

            BT_LOGI("GATTC", "bt_gattc_start_discover_service: status = %d", status);
        } else {
            status = GATTC_IN_SEARCHING;
        }
    } else {

        status = GATTC_IN_IDLE;
        BT_LOGI("GATTC", "bt_gattc_start_discover_service--not found conn info");
    }

    BT_LOGI("GATTC", "bt_gattc_start_discover_service--end: status = %d", status);
    return status;
}


static void bt_gattc_discover_primary_service_cnf(bt_status_t status, bt_gattc_find_by_type_value_rsp_t *p_event_data)
{
    BT_LOGI("GATTC", "primary_service_cnf: status = %d", status);
    uint16_t conn_id = 0;
    gattc_conn_t *gattc_conn;
    gatt_data_t *data;

    if (p_event_data) {
        conn_id = p_event_data->connection_handle;
        gattc_conn = bt_gattc_link_info_by_id(conn_id);
        if (gattc_conn == NULL) {

            return;
        }
        data = &gattc_conn->data;
        if (status == BT_ATT_ERRCODE_CONTINUE) {
            bt_gattc_parse_primary_service(p_event_data, data);
        } else if (status == BT_STATUS_SUCCESS) {
            if (p_event_data->att_rsp) {
                bt_gattc_parse_primary_service(p_event_data, data);
            }
            BT_LOGI("GATTC", "discover finished :success.. t_ser_num = %d", data->service_number);
            bt_gattc_explore_service(conn_id, &data->att_data);
        } else if (status == BT_ATT_ERRCODE_ATTRIBUTE_NOT_FOUND) {
            BT_LOGI("GATTC", "discover finished :not found .. t_ser_num = %d", data->service_number);
            bt_gattc_explore_service(conn_id, &data->att_data);

        } else {
            BT_LOGI("GATTC", "error happend");
            bt_gattc_notify_error(conn_id, status);
        }
    }
}


static void bt_gattc_explore_service(uint16_t conn_id, bt_gatt_service_t *srv)
{
    uint16_t start_handle, end_handle;

    if (srv) {
        start_handle = srv->start_handle;
        end_handle = srv->end_handle;
        bt_gattc_discover_include_service(conn_id, start_handle, end_handle);
    } else {

        BT_LOGI("GATTC", "srv node error happend");
    }
}


static bt_status_t bt_gattc_discover_include_service(uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle)
{
    bt_status_t status;
    bt_gattc_find_included_services_req_t req;
    BT_LOGI("GATTC", "include_service start: start_handle = %d, end_handle = %d", start_handle, end_handle);
    req.opcode = BT_ATT_OPCODE_READ_BY_TYPE_REQUEST;
    req.starting_handle = start_handle;
    req.ending_handle = end_handle;
    req.type16 =  BT_GATT_UUID16_INCLUDE;
    status = bt_gattc_find_included_services(conn_handle, &req);
    BT_LOGI("GATTC", "include_service end: status = %d", status);
    return status;
}


static void bt_gattc_discover_include_service_cnf(bt_status_t status , bt_gattc_read_by_type_rsp_t *p_event_data)
{
    uint16_t conn_id;
    uint16_t start_handle, end_handle;
    BT_LOGI("GATTC", "discover_include_service_cn: status = %d", status);
    if (p_event_data == NULL) {
        return;
    }
    conn_id = p_event_data->connection_handle;
    bt_gatt_service_t *service =  bt_gattc_get_service(conn_id);


    if (status == BT_ATT_ERRCODE_CONTINUE) {
        bt_gattc_parse_include_service(p_event_data, service);
    } else if (status == BT_STATUS_SUCCESS) {

        if (p_event_data->att_rsp) {
            bt_gattc_parse_include_service(p_event_data, service);
        }
        start_handle = service->start_handle;
        end_handle = service->end_handle;

        bt_gattc_discover_charateristic(conn_id, start_handle, end_handle);


        BT_LOGI("GATTC", "success :starting_handle = 0x%08x, end_group_handle = 0x%08x", start_handle, end_handle);
    } else if (status == BT_ATT_ERRCODE_ATTRIBUTE_NOT_FOUND) {

        start_handle = service->start_handle;
        end_handle = service->end_handle;
        BT_LOGI("GATTC", "not found :starting_handle = 0x%08x, end_group_handle = 0x%08x", start_handle, end_handle);
        bt_gattc_discover_charateristic(conn_id, start_handle, end_handle);

    } else {
        BT_LOGI("GATTC", "error happend");
        bt_gattc_notify_error(conn_id, status);
    }
}


static bt_status_t bt_gattc_discover_charateristic(uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle)
{
    bt_status_t  status;
    bt_gattc_discover_charc_req_t req;
    BT_LOGI("GATTC", "dis_char start: start_handle = %d, end_handle = %d", start_handle, end_handle);
    uint16_t uuid_16 = BT_GATT_UUID16_CHARC;
    req.opcode = BT_ATT_OPCODE_READ_BY_TYPE_REQUEST;
    req.starting_handle = start_handle;
    req.ending_handle = end_handle;
    bt_uuid_load(&(req.type), (void *)&uuid_16, 2);
    status = bt_gattc_discover_charc(conn_handle, &req);
    BT_LOGI("GATTC", "dis_char end: status = %d", status);
    return status;
}


static void bt_gattc_discover_charactiristics_cnf(bt_status_t status, bt_gattc_read_by_type_rsp_t *p_event_data)
{
    uint16_t conn_id;
    bt_gatt_service_t *service;

    BT_LOGI("GATTC", "discover_charactiristics_cnf: status = %d", status);
    if (p_event_data == NULL) {

        return;
    }
    conn_id = p_event_data->connection_handle;
    service = bt_gattc_get_service(conn_id);

    if (status == BT_ATT_ERRCODE_CONTINUE) {
        bt_gattc_parse_characteristic(p_event_data, service);
    } else if (status == BT_STATUS_SUCCESS) {
        if (p_event_data->att_rsp) {
            bt_gattc_parse_characteristic(p_event_data, service);
        }
        BT_LOGI("GATTC", "disc_char_cnf: success char_num = %d", service->num_of_char);

        if (service->num_of_char) {
            bt_gattc_find_all_descriptor_of_char(conn_id, service, 0);
        }
    } else if (status == BT_ATT_ERRCODE_ATTRIBUTE_NOT_FOUND) {
        BT_LOGI("GATTC", "disc_char_cnf: not found char_num = %d", service->num_of_char);
        if (service->num_of_char) {
            bt_gattc_find_all_descriptor_of_char(conn_id, service, 0);
        }
    } else {
        BT_LOGI("GATTC", "error happend");
    }
}


bt_status_t bt_gattc_discover_descriptor_of_characteristic(uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle)
{
    bt_status_t status;
    bt_gattc_discover_charc_descriptor_req_t req;
    BT_LOGI("GATTC", "dis_descr start: start_handle = %d, end_handle = %d", start_handle, end_handle);
    req.opcode = BT_ATT_OPCODE_FIND_INFORMATION_REQUEST;
    req.starting_handle = start_handle;
    req.ending_handle = end_handle;

    status = bt_gattc_discover_charc_descriptor(conn_handle, &req);
    BT_LOGI("GATTC", "dis_descr end: status = %d", status);
    return status;
}


static bt_status_t bt_gattc_find_all_descriptor_of_char(uint16_t conn_id, bt_gatt_service_t *service, uint8_t index)
{
    bt_gatt_char_t *cur_char, *next_char;
    bt_status_t status = 0;
    BT_LOGI("GATTC", "[Gattc]ble_gattc_find_all_descriptor_of_char--start:n_id= %d", conn_id);
    uint8_t char_num;
    char_num = service->num_of_char;
    service->explore_index = index;
    if (index <= (char_num - 1)) {
        uint16_t start_handle;
        uint16_t end_handle;
        cur_char = service->chara + index;
        start_handle = cur_char->value_handle + 1;

        BT_LOGI("GATTC", "[Gattc]ble_gattc_find_all_descriptor_of_char--cha_hd = %d, num = %d", index, char_num);

        /* This one might be the last characteristic in service */
        if (index == char_num - 1) {
            end_handle = service->end_handle;

        } else {
            next_char = service->chara + index + 1;
            end_handle = next_char->handle - 1;
        }

        BT_LOGI("GATTC", "[Gattc]ble_gattc_find_all_descriptor_of_char--s_hd = %x, e_hd =%x", start_handle, end_handle);
        if (start_handle <= end_handle) {
            bt_gattc_discover_descriptor_of_characteristic(conn_id, start_handle, end_handle);
        } else {
            bt_gattc_find_all_descriptor_of_char(conn_id, service, index + 1);
        }
    } else {/*all descriptor is complete done */

        BT_LOGI("GATTC", "[Gattc]ble_gattc_find_all_descriptor_of_char--next service");
        bt_gattc_discover_complete(conn_id, status, service);
    }
    BT_LOGI("GATTC", "[Gattc]ble_gattc_find_all_descriptor_of_char--end");
    return status;
}


static void bt_gattc_discover_descriptors_cnf(bt_status_t status, bt_gattc_find_info_rsp_t *p_event_data)
{
    uint16_t conn_id;
    bt_gatt_char_t *chara = NULL;
    bt_gatt_service_t *service;
    if (p_event_data == NULL) {
        return;
    }
    conn_id = p_event_data->connection_handle;
    BT_LOGI("GATTC", "disc_desc_cnf: status = %d", status);
    service = bt_gattc_get_service(conn_id);
    if (service == NULL) {
        return;
    }
    if (service->explore_index < service->num_of_char) {
        chara = service->chara + service->explore_index;
    }

    service->explore_index++;

    if (status == BT_ATT_ERRCODE_CONTINUE) {
        bt_gattc_parse_descriptor(p_event_data, chara);

    } else if (status == BT_STATUS_SUCCESS) {
        if (p_event_data->att_rsp) {
            bt_gattc_parse_descriptor(p_event_data, chara);
        }
        BT_LOGI("GATTC", "disc_desc_cnf: succsee:index = %d", service->explore_index);
        bt_gattc_find_all_descriptor_of_char(conn_id, service, service->explore_index);
    } else if (status == BT_ATT_ERRCODE_ATTRIBUTE_NOT_FOUND) {
        BT_LOGI("GATTC", "disc_desc_cnf: not found :index = %d", service->explore_index);
        bt_gattc_find_all_descriptor_of_char(conn_id, service, service->explore_index);;
    } else {
        BT_LOGI("GATTC", "error happend");
    }
}


static void bt_gattc_parse_primary_service(bt_gattc_find_by_type_value_rsp_t *rsp, gatt_data_t *data)
{
    uint8_t i;
    uint16_t uuid = 0;
    uint8_t *attribute_data_list = rsp->att_rsp->handles_info_list;
    uint8_t num_of_data = (rsp->length - 1) / 4;
    bt_gatt_service_t *srv_data;

    for (i = 0 ; i < num_of_data; i++) {
        srv_data = &data->att_data;
        memcpy(&srv_data->start_handle, attribute_data_list + i * 4, 2);
        memcpy(&srv_data->end_handle, attribute_data_list +  i * 4 + 2, 2);

        BT_LOGI("GATTC", "starting_handle = 0x%08x, end_group_handle = 0x%08x, uuid = 0x%08x", srv_data->start_handle, srv_data->end_handle, uuid);
        srv_data->uuid = 0x180d;
        data->service_number++;
    }
}


static bt_gatt_service_t *bt_gattc_get_service(uint16_t conn_id)
{
    gattc_conn_t *gattc_conn;
    gatt_data_t *data;
    bt_gatt_service_t *srv_data;
    gattc_conn = bt_gattc_link_info_by_id(conn_id);
    if (gattc_conn == NULL) {
        BT_LOGI("GATTC", "bt_gattc_get_service--conn link is null");
        return NULL;
    }
    data = &gattc_conn->data;
    srv_data = &data->att_data;

    BT_LOGI("GATTC", "bt_gattc_get_service--start_h = %x", srv_data->start_handle);
    return srv_data;
}


void bt_gattc_parse_include_command_data(bt_gatt_included_service_t *include_service, uint8_t *attribute_data_list)
{
    memset(include_service, 0, sizeof(bt_gatt_included_service_t));
    memcpy(&(include_service->handle), attribute_data_list, 2);
    memcpy(&(include_service->start_handle), attribute_data_list + 2, 2);
    memcpy(&(include_service->end_handle), attribute_data_list + 4, 2);
}

static void bt_gattc_parse_include_service(bt_gattc_read_by_type_rsp_t *rsp, bt_gatt_service_t *service)
{
    uint16_t uuid;
    bt_gatt_included_service_t include_service;
    uint8_t *attribute_data_list;
    bt_att_read_by_type_rsp_t *att_rsp = rsp->att_rsp;

    if (att_rsp == NULL) {
        return;
    }
    attribute_data_list = att_rsp->attribute_data_list;
    if (rsp->att_rsp->length <= 8) {
        uint8_t num_of_data = (rsp->length - 2) / rsp->att_rsp->length;
        for (int i = 0 ; i < num_of_data; i++) {
            bt_gattc_parse_include_command_data(&include_service, (attribute_data_list + rsp->att_rsp->length * i));
            memcpy(&(include_service.uuid), attribute_data_list + rsp->att_rsp->length * i + 6, 2);
            memcpy((void*)(service->incl_srv + i), &include_service, sizeof(bt_gatt_included_service_t));
        }
    } else { /*when it is 128 , it must only one uuid not a list*/
        bt_uuid_t uuid128;
        bt_gattc_parse_include_command_data(&include_service, attribute_data_list);

        memcpy((void*)&uuid128.uuid, attribute_data_list + 6, 16);

        BT_LOGI("GATTC", "attribute_handle = 0x%08x, starting_handle = 0x%08x, end_group_handle = 0x%08x",
                include_service.handle, include_service.start_handle, include_service.end_handle);
        uuid = uuid128.uuid16;
        include_service.uuid = uuid;
        memcpy((void*)service->incl_srv, &include_service, sizeof(bt_gatt_included_service_t));
        BT_LOGI("GATTC", "  uuid = 0x%08x,", uuid);
    }
}


void bt_gattc_parse_char_command_data(bt_gatt_char_t *char_info, uint8_t *attribute_data_list)
{
    if (attribute_data_list) {
        memset(char_info, 0, sizeof(bt_gatt_char_t));
        memcpy(&char_info->handle, attribute_data_list, 2);
        memcpy(&char_info->property, attribute_data_list + 2, 1);
        memcpy(&char_info->value_handle, attribute_data_list + 3, 2);
        BT_LOGI("GATTC", "handle = %d, value_handle = 0x%08x", char_info->value_handle, char_info->handle);
    }

}


static void bt_gattc_parse_characteristic(bt_gattc_read_by_type_rsp_t *data, bt_gatt_service_t *service)
{
    bt_gattc_read_by_type_rsp_t rsp = *data;
    bt_gatt_char_t char_info;
    uint8_t *attribute_data_list = rsp.att_rsp->attribute_data_list;
    uint16_t attribute_handle = 0, uuid;

    uint8_t num_of_data = (rsp.length - 2) / rsp.att_rsp->length;
    service->num_of_char = num_of_data;
    BT_LOGI("GATTC", "ble_gattc_parse_characteristic : char_num = %d", service->num_of_char);
    if (rsp.att_rsp->length < 20) {/*16+2 + 1 +1*/

        for (int i = 0 ; i < num_of_data; i++) {

            bt_gattc_parse_char_command_data(&char_info, (rsp.att_rsp->attribute_data_list + i * rsp.att_rsp->length));

            memcpy((void*)&char_info.uuid, attribute_data_list + i * rsp.att_rsp->length + 5, 2);

            BT_LOGI("GATTC", "num_of_data = %d, attribute handle = 0x%08x", i, char_info.uuid);
            memcpy((void*)(service->chara + i), &char_info, sizeof(bt_gatt_char_t));
        }
    } else {
        bt_uuid_t uuid128;
        bt_gattc_parse_char_command_data(&char_info, rsp.att_rsp->attribute_data_list);
        memcpy(&uuid128.uuid, attribute_data_list + rsp.att_rsp->length + 5, 16);
        uuid = uuid128.uuid16;
        char_info.uuid = uuid;
        BT_LOGI("GATTC", "attribute handle = 0x%08x, value_handle = 0x%08x", attribute_handle, uuid);
        memcpy((void*)service->chara, &char_info, sizeof(bt_gatt_char_t));
    }
}


static void bt_gattc_parse_descriptor(bt_gattc_find_info_rsp_t *data, bt_gatt_char_t *chara)
{
    bt_gattc_find_info_rsp_t rsp = *data;

    uint8_t format = 0;
    uint16_t attribute_handle = 0, attribute_value = 0;
    uint8_t attribute_length = 0;
    uint8_t i;

    if (rsp.att_rsp) {
        format = rsp.att_rsp->format;
    }
    if (format == 0x02) {
        attribute_length = 18;
    } else {
        attribute_length = 4;
    }
    uint8_t num_of_attribute = (rsp.length - 2) / attribute_length;
    for (i = 0; i < num_of_attribute; i++) {
        if (format == 0x02) {
            /* uuid 128 */
            bt_uuid_t uuid128;
            memcpy(&attribute_handle, rsp.att_rsp->info_data + i * attribute_length, 2);

            memcpy(&uuid128, rsp.att_rsp->info_data + i * attribute_length + 2, 16);
            attribute_value = uuid128.uuid16;

        } else {
            /* uuid 16 */
            memcpy(&attribute_handle, rsp.att_rsp->info_data + i * attribute_length, 2);
            memcpy(&attribute_value, rsp.att_rsp->info_data + i * attribute_length + 2, 2);
        }
        BT_LOGI("GATTC", "attribute handle = 0x%08x, attribute value = 0x%08x", attribute_handle, attribute_value);
        bt_gatt_descriptor_t descr_data;
        memset(&descr_data, 0, sizeof(bt_gatt_descriptor_t));
        descr_data.handle = attribute_handle;

        descr_data.uuid = attribute_value;
        memcpy(chara->descr + i, &descr_data, sizeof(bt_gatt_descriptor_t));
    }
}


bt_status_t bt_gattc_write_descr(uint16_t conn_id, bt_gattc_write_descr_req_t *req)
{
    bt_status_t status;
    bt_gattc_write_charc_req_t write_req;
    uint8_t buffer[256];
    memset(&write_req, 0, sizeof(bt_gattc_write_charc_req_t));
    write_req.attribute_value_length = req->size;

    write_req.att_req = (bt_att_write_req_t*)buffer;
	  write_req.att_req->opcode = BT_ATT_OPCODE_WRITE_COMMAND;
    write_req.att_req->attribute_handle = req->handle;

    memcpy(write_req.att_req->attribute_value, req->value, req->size);
    status = bt_gattc_write_charc(conn_id, &write_req);
    BT_LOGI("GATTC", "bt_gattc_write_descr: status = %d\r\n", status);
    return status;
}


static void bt_gattc_write_char_cnf(bt_status_t status, bt_gattc_write_rsp_t *param)/*write char vaule */
{
    BT_LOGI("GATTC", "status = 0x%08x", status);
}


static void bt_gattc_char_val_notify_and_ind(bt_status_t status, bt_gatt_handle_value_notification_t *rsp)
{
    uint16_t conn_id = 0;
    gattc_conn_t *gattc_conn = NULL;
    app_callback_struct *cb;
    gattc_value_t value;
    if (rsp) {
        conn_id = rsp->connection_handle;
        gattc_conn = bt_gattc_link_info_by_id(conn_id);
    }
    if (gattc_conn) {
        gattc_user_context_t *user;
        gattc_user_connect_struct conn;
        if (g_gattc_ctx.userContext.flag) { /*service node 's all userlist*/
            user = &g_gattc_ctx.userContext;
            conn.reg_cntx = (void *)user;
            conn.conn_id = conn_id;
            cb = (app_callback_struct *)user->appCb;
            if (cb) {
                if (cb->notify_cb) {
                    if (rsp->att_rsp) {
                        memcpy(value.value, rsp->att_rsp->attribute_value, rsp->length - 3);
                    }
                    value.len = rsp->length - 3;
                    cb->notify_cb(&conn, rsp->att_rsp->handle, &value);
                }
            }
        }
    }
}

bt_handle_t g_bt_conn_handle = 0;
bt_status_t bt_gattc_event_callback(bt_msg_type_t msg, bt_status_t status, void *param)
{
    switch (msg) { 
        case BT_GAP_LE_SET_RANDOM_ADDRESS_CNF:
        {
            if (gpio_started == true) {
                bt_hci_cmd_le_set_advertising_enable_t enable;
                bt_hci_cmd_le_set_advertising_parameters_t adv_param = {
                        .advertising_interval_min = BLE_GPIO_MIN_INTERVAL,
                        .advertising_interval_max = BLE_GPIO_MAX_INTERVAL,
                        .advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED,
                        .own_address_type = BT_ADDR_RANDOM,
                        .advertising_channel_map = BLE_GPIO_CHANNEL_NUM,
                        .advertising_filter_policy = BLE_GPIO_FILTER_POLICY
                    };

                    
                bt_hci_cmd_le_set_advertising_data_t adv_data = {
                   .advertising_data_length = BLE_GPIO_ADV_DATA_LEN,
                    .advertising_data = BLE_GPIO_ADV_DATA
                };

                adv_data.advertising_data[0] = BLE_GPIO_AD_FLAG_LEN;
                adv_data.advertising_data[1] = BT_GAP_LE_AD_TYPE_FLAG;
                adv_data.advertising_data[2] = BT_GAP_LE_AD_FLAG_BR_EDR_NOT_SUPPORTED | BT_GAP_LE_AD_FLAG_GENERAL_DISCOVERABLE;
                adv_data.advertising_data[3] = BLE_GPIO_AD_UUID_LEN;
                adv_data.advertising_data[4] = BT_GAP_LE_AD_TYPE_16_BIT_UUID_COMPLETE;
                adv_data.advertising_data[5] = BLE_GPIO_SERVICE_UUID & 0x00FF;
                adv_data.advertising_data[6] = (BLE_GPIO_SERVICE_UUID & 0xFF00)>>8;
                adv_data.advertising_data[7] = BLE_GPIO_AD_NAME_LEN;
                adv_data.advertising_data[8] = BT_GAP_LE_AD_TYPE_NAME_COMPLETE;

                enable.advertising_enable = BT_HCI_ENABLE;
                bt_gap_le_set_advertising(&enable, &adv_param, &adv_data, NULL);

                gpio_started = false;
            }
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
            gattc_notify_scan_info_to_all_user((bt_gap_le_advertising_report_ind_t *)param);

            }
            break;
        case BT_GAP_LE_CONNECT_IND:
        {
            BT_LOGI("GATTC", "BT_GAP_CONNECT_IND %s",
                    (status == BT_STATUS_SUCCESS) ? "Success" : "Failed");

            bt_handle_t handle;
            app_bt_connection_cb_t *conn_info = NULL;
            const bt_gap_le_connection_ind_t *connect_ind = (bt_gap_le_connection_ind_t *)param;
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
                add_connection_info(param);

                BT_LOGI("GATTC", "connection role=0x%04x", connect_ind->role);
                if (connect_ind->role == BT_ROLE_MASTER) {
                    conn_info = find_conneciton_info_by_handle(handle);
                    gattc_connect_cb(status, conn_info);
                }
                g_bt_conn_handle = handle;
            }
#ifdef MTK_MCS_ENABLE
            mcs_update(2,1,"BLE is connected !");
#endif
        }
            break;
        case BT_GAP_LE_DISCONNECT_IND: {
            BT_LOGI("GATTC", "BT_GAP_DISCONNECT_IND %s",
                    (status == BT_STATUS_SUCCESS) ? "Success" : "Failed");
            gattc_disconnect_cb(status, (bt_hci_evt_disconnect_complete_t *)param);
            delete_connection_info(param);
            g_bt_conn_handle = 0;
#ifdef MTK_MCS_ENABLE
            mcs_update(2,0,"BLE is disconnected !");
#endif
            }
            break;
        case BT_GAP_LE_SET_SCAN_CNF:{
            BT_LOGI("GATTC", "BT_GAP_SET_SCAN_CNF %s",
                    (status == BT_STATUS_SUCCESS) ? "Success" : "Failed");
            }
            break;

        case BT_GATTC_DISCOVER_PRIMARY_SERVICE_BY_UUID: {
            BT_LOGI("GATTC", "Primary service: msg_id = %d,status = %d", msg, status);
            bt_gattc_discover_primary_service_cnf(status, (bt_gattc_find_by_type_value_rsp_t *)param);
            break;
        }
        case BT_GATTC_FIND_INCLUDED_SERVICES: {
            BT_LOGI("GATTC", "inlcuded service: msg_id = %d,status = %d", msg, status);
            bt_gattc_discover_include_service_cnf(status, (bt_gattc_read_by_type_rsp_t *)param);
            break;
        }
        case BT_GATTC_DISCOVER_CHARC: {
            BT_LOGI("GATTC", "discover char: msg_id = %d,status = %d", msg, status);
            bt_gattc_discover_charactiristics_cnf(status, (bt_gattc_read_by_type_rsp_t *)param);
            break;
        }
        case BT_GATTC_DISCOVER_CHARC_DESCRIPTOR: {
            BT_LOGI("GATTC", "discover discriptor: msg_id = %d,status = %d", msg, status);
            bt_gattc_discover_descriptors_cnf(status, (bt_gattc_find_info_rsp_t *)param);
            break;
        }
        case BT_GATTC_WRITE_CHARC: {
            BT_LOGI("GATTC", "BT_GATTC_WRITE_CHARC: msg_id = %d,status = %d", msg, status);
            bt_gattc_write_char_cnf(status, (bt_gattc_write_rsp_t *)param);
            break;
        }
        case BT_GATTC_READ_CHARC:
        {
            BT_LOGI("GATTC", "BT_GATTC_READ_CHARC: msg_id = %d,status = %d", msg, status);
            
            bt_gattc_read_rsp_t rsp = *((bt_gattc_read_rsp_t *)param);
        
            if (status == BT_STATUS_SUCCESS && rsp.att_rsp == NULL) {
                BT_LOGI("GATTC", "BT_GATTC_READ_CHARC FINISHED!!");
                break;
            }
            
            if (rsp.att_rsp == NULL) {
                BT_LOGI("GATTC", "status = %d", status);
                break;
            }
            
            uint8_t op_code = rsp.att_rsp->opcode;
            BT_LOGI("GATTC", "op_code = 0x%08x", op_code);
                        
            uint8_t length = rsp.length - 1;
            BT_LOGI("GATTC", "length = 0x%08x", length);
            
            for (int i = 0; i < length ; i++) {
                BT_LOGI("GATTC", "0x%02x ", rsp.att_rsp->attribute_value[i]);
            }
            

            if (status == BT_STATUS_SUCCESS) {
                BT_LOGI("APP", "bt_gattc_read_charc FINISHED!!!");
            }
            
            break;
        }
        case BT_GATTC_CHARC_VALUE_NOTIFICATION: {
            bt_gattc_char_val_notify_and_ind(status, (bt_gatt_handle_value_notification_t *)param);
            break;
        }
    }
    return BT_STATUS_SUCCESS;
}


static void bt_gattc_notify_error(uint16_t conn_id, bt_status_t status)
{
    bt_gattc_discover_complete(conn_id, status, NULL);
}


static void bt_gattc_discover_complete(uint16_t conn_id, bt_status_t status, bt_gatt_service_t *service)
{
    app_callback_struct *app_cb = NULL;
    gattc_conn_t *conn = bt_gattc_link_info_by_id(conn_id);
    BT_LOGI("GATTC", "bt_gattc_discover_complete: --start");
    if (conn) {
        BT_LOGI("GATTC", "bt_gattc_discover_complete: --state = %d, result = %d", conn->state, status);

        if (conn->state == GATTC_IN_SEARCHING) {
            if (status == BT_STATUS_SUCCESS) {
                conn->state = GATTC_SEARCH_DONE;
            } else {
                conn->state = GATTC_CONNECTED;
            }
        }

        if (g_gattc_ctx.userContext.flag) {
            /*for all user in userlist*/
            gattc_user_context_t *user = &g_gattc_ctx.userContext;
            gattc_user_connect_struct user_conn;
            user_conn.reg_cntx = (void *)user;
            user_conn.conn_id = conn->conn_id;
            app_cb = (app_callback_struct *)user->appCb;
            app_cb->search_complete_cb(&user_conn, status, service);
        }
    }
    BT_LOGI("GATTC", "bt_gattc_discover_complete: end");
}


bt_gatt_char_t *bt_gattc_get_char_by_handle(uint16_t conn_id, uint16_t handle)
{
    uint8_t i;
    bt_gatt_char_t *chara;
    bt_gatt_service_t *service = bt_gattc_get_service(conn_id);
    if (handle >= service->start_handle && handle <= service->end_handle) {

        for (i = 0; i < service->num_of_char; i++) {

            chara = service->chara + i;
            if (handle == chara->value_handle) {
                return chara;
            }
        }
    }

    return NULL;

}
