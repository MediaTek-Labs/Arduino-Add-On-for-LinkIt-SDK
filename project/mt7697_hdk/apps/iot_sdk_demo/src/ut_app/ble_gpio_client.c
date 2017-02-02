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
#include "hrc.h"
#include <stdlib.h>

#define APP_QUEUE_SIZE      10
#if 1
QueueHandle_t app_queue = NULL;
#endif
hear_rate_message_struct app_queue_data;

static void heart_rate_connect_cb(gattc_user_connect_struct *conn, uint16_t connected, bt_addr_t *bd_addr);

static void heart_rate_update_notify_cb(gattc_user_connect_struct *conn,
                                        uint16_t handle, gattc_value_t *value);

static void heart_rate_callback_init(app_callback_struct *app_gattc_cb);

static	void heart_rate_write_descr(uint16_t conn_id, uint16_t handle);
static app_hrp_context_struct *heart_rate_get_cntx(void);
static void bt_hrc_app_show_data(bt_msg_type_t event_id, const void *param);
void bt_hrc_app_callback(hear_rate_message_struct * msg);

app_callback_struct g_hrcp_gatt_cb;
app_hrp_context_struct g_hrcp_cntx;


uint8_t  g_hr_srv_uuid[] =  {
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0x0D, 0x18, 0x00, 0x00,
};


uint16_t  g_hr_char_uuid[HRART_RATE_TYPE_TOTAL] = {
    HRM_CHAR_UUID, HBL_CHAR_UUID, HCP_CHAR_UUID
};

uint16_t g_desc_uuid[] = {
    CLIENT_CONFI_DESCRI
};


void heart_rate_init(void)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    //bt_hci_cmd_le_set_scan_enable_t enable;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    app_hrp_context_struct *hrc_cntx;
    gattc_register_req_struct req;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    hrc_cntx = heart_rate_get_cntx();
    BT_LOGI("hrc", "[HRC]heartrate app init: -- start");
    heart_rate_callback_init(&g_hrcp_gatt_cb);
    memset(&req, 0, sizeof(gattc_register_req_struct));
    req.uuid_count = 1;
    req.uuid = g_hr_srv_uuid;
    hrc_cntx->reg_ctx = gattc_register(&req, &g_hrcp_gatt_cb);
    if (!hrc_cntx->reg_ctx) {
        /*reg is  null*/
        BT_LOGI("APPS", "[GATTC]heart_rate_init: -- register failed");
    }
    BT_LOGI("hrc", "[HRC]heartrate app init: -- end");
}


void heart_rate_deinit()
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    app_hrp_context_struct *hrc_cntx;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    hrc_cntx = heart_rate_get_cntx();
    gattc_deregister((gattc_user_context_t *)hrc_cntx->reg_ctx);
}


static void heart_rate_scan_cb(void *reg_cntx, bt_gap_le_advertising_report_ind_t *param)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    uint8_t scan_data[31] = {0};
    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    //uint8_t srv_addr[6] = { 0xe6, 0xfd, 0x8b, 0x19, 0xa6, 0xeb};
    //uint8_t srv_addr[6] = {0x5d, 0xfc, 0x2b, 0x37, 0x9f, 0xee};
    BT_LOGI("hrc", "[HRAPP]heart_rate_scan_cb: -- start: type = %d", param->event_type);
    BT_LOGI("hrc", "[HRAPP]heart_rate_scan_cb: -- start: addr[0] = %x, addr[1] = %x, addr[2] = %x, addr[3] = %x, addr[4] = %x, addr[5] = %x\r\n",
            param->address.addr[0], param->address.addr[1],
            param->address.addr[2], param->address.addr[3], param->address.addr[4], param->address.addr[5]);
    memcpy(scan_data, param->data, param->data_length);
    printf("scan data : %s\n\r", scan_data);

}


static void heart_rate_connect_cb(gattc_user_connect_struct * conn, uint16_t connected, bt_addr_t *bd_addr) 
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    app_hrp_context_struct *hrc_cntx;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    hrc_cntx = heart_rate_get_cntx();
    BT_LOGI("hrc", "[HRAPP]heart_rate_connect_cb:start--reg =%x, con_reg = %x", hrc_cntx->reg_ctx, conn->reg_cntx);
    BT_LOGI("hrc", "[HRAPP]heart_rate_connect_cb:connected =%d", connected);
    if (hrc_cntx->reg_ctx == conn->reg_cntx) {

        BT_LOGI("hrc", "[HRAPP]heart_rate_connect_cb: dev_info = %x");
        if (connected) {
            bt_gattc_start_discover_service(conn->conn_id);/*start search service remote*/
        }
    }
}


static void heart_rate_search_complete_cb(gattc_user_connect_struct * conn, int32_t result, bt_gatt_service_t *service)
{

    uint32_t i, j;
    bt_gatt_char_t  *chara;
    bt_gatt_descriptor_t *descr;
    app_hrp_context_struct *hrc_cntx;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    hrc_cntx = heart_rate_get_cntx();

    BT_LOGI("hrc", "[HRAPP]heart_rate_search_complete_cb:start--dev = %x, result = %d", result);
    if (hrc_cntx->reg_ctx == conn->reg_cntx) {
        if (result == BT_STATUS_SUCCESS) {
            if (service->uuid == SRV_HRM_SER_UUID) {
                for (i = 0; i < service->num_of_char; i++) {
                    chara = service->chara + i;
                    if (chara->uuid == g_hr_char_uuid[0]) {
                        for (j = 0; j < 3; j++) {
                            descr = chara->descr + j;
                            if (descr->uuid == CLIENT_CONFI_DESCRI) {
                                heart_rate_write_descr(conn->conn_id, descr->handle);
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        } else {

            BT_LOGI("hrc", "[HRAPP]heart_rate_search_complete_cb:fail");
        }
    }

}


static void heart_rate_write_descr(uint16_t conn_id, uint16_t handle) 
{
    bt_gattc_write_descr_req_t req;
    uint8_t buffer[16];
    uint16_t notify;

    memset(&req, 0, sizeof(bt_gattc_write_descr_req_t));
    req.handle = handle;
    req.value = buffer;
    req.size = 2;

    notify = 0x0001;
    memcpy(req.value, &notify, 2);
    bt_gattc_write_descr(conn_id, &req);

}


static void heart_rate_callback_init(app_callback_struct * app_gattc_cb) 
{
    app_gattc_cb->scan_cb = heart_rate_scan_cb;
    app_gattc_cb->connect_cb = heart_rate_connect_cb;
    app_gattc_cb->search_complete_cb = heart_rate_search_complete_cb;
    app_gattc_cb->notify_cb = heart_rate_update_notify_cb;
}



static app_hrp_context_struct *heart_rate_get_cntx(void) 
{
    return &g_hrcp_cntx;
}


static void heart_rate_update_notify_cb(gattc_user_connect_struct * conn,
                                            uint16_t handle, gattc_value_t *value) 
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    app_hrp_context_struct *hrc_cntx;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    hrc_cntx = heart_rate_get_cntx();


    BT_LOGI("hrc", "[HRAPP]heart_rate_update_notify_cb:start--handle= %x", handle);

    if (conn->reg_cntx == hrc_cntx->reg_ctx) {
        bt_gatt_char_t *chara = bt_gattc_get_char_by_handle(conn->conn_id, handle);
        if (chara->uuid == HRM_CHAR_UUID) {
            hear_rate_message_struct msg;
            msg.event_id = BT_GATTC_CHARC_VALUE_NOTIFICATION;
            memcpy(msg.param, value, sizeof(gattc_value_t));
            bt_hrc_app_callback(&msg);
        }

    }
}

void bt_hrc_app_callback(hear_rate_message_struct * msg) 
{

    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
#if 0 //MTK_BLE_GPIO_SERVICE
    BaseType_t ret = 0;
    ret = xQueueSend(app_queue, (void *)msg, 0);  //if queue is full ,the new notifications will be dropped

    BT_LOGI("hrc", "[HRapp]send ret = %d\r\n", ret);
    //bt_hrc_app_show_data(msg->event_id, msg->param);
#else
    bt_hrc_app_show_data(msg->event_id, msg->param);

#endif
}

extern QueueHandle_t g_mcs_status_xQueue;
static void bt_hrc_app_show_data(bt_msg_type_t event_id, const void *param) 
{
    BT_LOGI("hrc", "[HRAPP]bt_hrc_app_show_data:start--event_id = %d", event_id);

    if (event_id == BT_GATTC_CHARC_VALUE_NOTIFICATION) {
        gattc_value_t *value;
        hr_data_t data;
        value = (gattc_value_t *)param;
        BT_LOGI("hrc", "[HRAPP]bt_hrc_app_show_data:start--len = %d", value->len);
        gattc_decode_char_data(value, &data);
#ifdef MTK_MCS_ENABLE
        if (value->len) {
            BT_LOGI("hrc", "[HRAPP]heart_rate_update_notify_cb:start--heart_value = %d", data.val);
            int on_off = data.val;
            xQueueSend(g_mcs_status_xQueue, &on_off, 0);
        }
#else
        if (data.val) {
            BT_LOGI("hrc", "[HRAPP]heart_rate_update_notify_cb:start--heart_value = %d", data.val);
        }
#endif
    }
}


void heart_rate_task(void *arg) 
{

    BT_LOGI("hrc", "[HRAPP]App test task begin\r\n");
    /*queue ring buffer*/
    gattc_service_init();
    heart_rate_init();

    app_queue = xQueueCreate(APP_QUEUE_SIZE, sizeof(hear_rate_message_struct));
    if ( app_queue == NULL ) {
        BT_LOGI("hrc", "[HRAPP]create queue failed!\r\n");
        return;
    }
    memset((void *)&app_queue_data, 0, sizeof(hear_rate_message_struct));

    while (1) {
        BT_LOGI("hrc", "[HRAPP]try to get one msg\r\n");
        if (xQueueReceive(app_queue, (void *)&app_queue_data, portMAX_DELAY)) {
            bt_hrc_app_show_data(app_queue_data.event_id, app_queue_data.param);
        }
    }
}


void heart_rate_task_init(void) 
{
    TaskHandle_t xCreatedTask;
    BT_LOGI("hrc", "[HRAPP]create task!\r\n");
    xTaskCreate(heart_rate_task, "heart_rate_test_task", 1024, NULL, 1, &xCreatedTask);
}
