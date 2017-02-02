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

#include "bt_gattc_srv.h"
#include "bt_gap_le.h"

#include "bt_gattc_connect.h"
#include <FreeRTOS.h>

/*for all op callback*/
static gattc_user_context_t *gattc_service_alloc_user(void);



static void gattc_init_user_context(gattc_user_context_t *user);

gattc_context_t g_gattc_ctx;


void  *gattc_register(gattc_register_req_struct *req, app_callback_struct *cb)
{
    gattc_user_context_t *user;
    uint8_t count, i = 0, size = 0;

    count = req->uuid_count;
    BT_LOGI("GATT_SRV", "Gattc_Register: -- start: count = %d", count);

    user = gattc_service_alloc_user();
    BT_LOGI("GATT_SRV", "Gattc_Register: --user = %x", user);
    if (user != NULL) {
        user->appCb = cb; /*need  transfer gattc_callback_struct*/
        user->uuid_count = count;
        while (i < count) {

#ifndef WIN32
            user->uuid =  pvPortMalloc(16);
#else
            user->uuid = malloc(16);
#endif /*alloc momery*/
            BT_LOGI("GATT_SRV", "Gattc_Register: --  useruuid = %x", user->uuid);
            if (user->uuid != NULL) {
                memcpy((user->uuid + size), req->uuid, 16);
                size += 16;
            }
            i++;
        }
    }
    BT_LOGI("GATT_SRV", "Gattc_Register: -- start: end = %x", user);
    return (void *)user;
}


void gattc_deregister(gattc_user_context_t *user)
{
    if (user) {
        if (user->flag) {
            if (user->uuid) {
#ifndef WIN32
                vPortFree(user->uuid);
#else
                free(user->uuid);
#endif
            }
            gattc_init_user_context(user);
        }
    }
}


void gattc_notify_scan_info_to_all_user(bt_gap_le_advertising_report_ind_t *param)
{
    gattc_user_context_t *user;
    app_callback_struct *cb = NULL;
    BT_LOGI("GATT_SRV", "scan_info_to_all_user: --start");


    if (g_gattc_ctx.userContext.flag) {/*notify all user*/
        BT_LOGI("GATT_SRV", "scan_info_to_all_user: user is not null");
        user = &g_gattc_ctx.userContext;
        if (user) {
            cb = (app_callback_struct *)user->appCb;
        }
        if (cb && cb->scan_cb) {
            cb->scan_cb((void *)user, param);
        }
    }
    BT_LOGI("GATT_SRV", "scan_info_to_all_user: --end");
}


void gattc_notify_conn_info_to_all_user(gattc_conn_t *gattc_conn, bt_status_t error_code, app_bt_connection_cb_t *conn_info, uint16_t type)
{
    app_callback_struct *cb;
    gattc_user_context_t *user;
    uint16_t  connected = 0;
    gattc_user_connect_struct conn;

    conn.conn_id = gattc_conn->conn_id;
    BT_LOGI("GATT_SRV", "gattc_notify_conn_info_to_all_user: --start:con_id = %d", conn.conn_id);
    if (type) {
        if (error_code == BT_STATUS_SUCCESS) {
            connected = 1;
        }
    }

    BT_LOGI("GATT_SRV", "gattc_notify_conn_info_to_all_user: --conn = %x, error_code = %d", gattc_conn, error_code);
    if (g_gattc_ctx.userContext.flag) {/*notify all user*/
        user = &g_gattc_ctx.userContext;
        if (user) {
            cb = (app_callback_struct *)user->appCb;
        }
        if (cb && cb->connect_cb) {
            conn.reg_cntx = (void *)user;
            if (type) {
                cb->connect_cb(&conn, connected, &conn_info->peer_addr);
            } else {
                cb->connect_cb(&conn, connected, NULL);
            }
        }
    }
    BT_LOGI("GATT_SRV", "gattc_notify_conn_info_to_all_user: --end");
}


static gattc_user_context_t *gattc_service_alloc_user(void)
{
    BT_LOGI("GATT_SRV", "[GATTC]gattc_service_alloc_user: -- start:");
    if (!g_gattc_ctx.userContext.flag) {
        BT_LOGI("GATT_SRV", "[GATTC]gattc_service_alloc_user: -- success:");
        g_gattc_ctx.userContext.flag = 1;
        return &g_gattc_ctx.userContext;

    } else {
        BT_LOGI("GATT_SRV", "[GATTC]gattc_service_alloc_user: -- fail:");
        return NULL;
    }

}

static void gattc_init_user_context(gattc_user_context_t *user)
{
    user->appCb = NULL;
    user->uuid = NULL;
    user->uuid_count = 0;
    user->flag = 0;
}

void gattc_decode_char_data(gattc_value_t *value, hr_data_t *data)
{
    uint8_t flag;
    uint8_t start_index;
    app_uuid_t uuid, uuid1, uuid2;

    flag = value->value[0];

    uuid.len = 2;
    uuid.uuid[0] = value->value[1];
    if (value->len < 2) {/*the min len is 2*/
        return;
    }
    if (flag << (8 - 1) & 1) { /*value is uint16*/

        uuid.uuid[1] = value->value[2];
        start_index = 3;
    } else { /*value is uint8*/
        uuid.uuid[1] = 0;
        start_index = 2;

    }
    data->val = gattc_convert_array_to_uuid16(&uuid);/*hr value*/
    uuid1.len = 2;
    if ((flag >> 3) & 1) {/*check energy_expend*/
        if (value->len >= 5) {
            uuid1.uuid[0] = value->value[start_index];
            uuid1.uuid[1] = value->value[start_index + 1];
            data->en_expend = gattc_convert_array_to_uuid16(&uuid1);
        } else {
            /*there has some error*/
            data->en_expend = 0;
        }
    }
    start_index = start_index + 1;
    uuid2.len = 2;
    if ((flag >> 4) & 1) {/*check RR_interval*/
        if (value->len >= 7) {
            uuid2.uuid[0] = value->value[start_index];
            uuid2.uuid[1] = value->value[start_index + 1];
            data->RR_inteval = gattc_convert_array_to_uuid16(&uuid2);
        } else {
            /*there has some error*/
            data->RR_inteval = 0;
        }
    }
}

uint16_t gattc_convert_array_to_uuid16(app_uuid_t *uu)
{
    uint16_t uuid = 0;
    if (uu) {
        if (uu->len == 2) {
            uuid = ((uint16_t)uu->uuid[1]) << 8 | uu->uuid[0];

        } else if (uu->len == 16) {
            uuid = ((uint16_t)uu->uuid[13]) << 8 | uu->uuid[12];
        }
    }
    return uuid;
}
