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
#if 1
#include "gatt_service.h"
#include "bt_gatts.h"
#include "bt_gattc.h"
#include "bt_uuid.h"
#ifndef __TS_WIN32__
#include <FreeRTOS.h>
#include "timers.h"
#endif
#include "connection_info.h"
#include "os.h"
#ifdef MTK_MCS_ENABLE
#include "mcs.h"
#endif

typedef struct {
    uint16_t conn_handle;               /**< connect handle */
    uint16_t is_notify;          /**< client config is notify or not*/
    uint8_t hr_value[2];         /**< HeartRate Measurement vaule */

    TimerHandle_t timer;
} hr_app_cntx_t;

hr_app_cntx_t g_hr_app;

#define BT_SIG_UUID16_HEARTRATE_MEASUREMENT 0x2A37
//static uint16_t bt_if_hr_notify = 0;//Client Characteristic Configuration
const bt_uuid_t BT_SIG_UUID_HEARTRATE_MEASUREMENT = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_HEARTRATE_MEASUREMENT);

//added for GPIO control
#define BT_GATT_UUID16_GPIO_SERVICE        (0x18BB) 
#define BT_SIG_UUID16_GPIO_CONTROL 0x3ABB
const bt_uuid_t BT_SIG_UUID_GPIO_CONTROL = BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_GPIO_CONTROL);


/* heart rate start ...*/
static void bt_hr_notify()
{
    bt_status_t status;
    uint16_t conn_handle;
    TimerHandle_t timer;
    uint8_t buff[6] = {0};
    bt_gattc_charc_value_notification_indication_t *notify;
    notify = (bt_gattc_charc_value_notification_indication_t *)buff;
    BT_LOGD("HRS", "bt_hr_notify : start... ");

    conn_handle = g_hr_app.conn_handle;
    timer = g_hr_app.timer;
    app_bt_connection_cb_t *conn_cb = find_conneciton_info_by_handle(conn_handle);

    if (conn_cb == NULL) {/*MAYBE link disconnect*/
#ifndef __TS_WIN32__
        xTimerStop(timer, 0);
        xTimerDelete(timer, 0);
        //bt_timer_cancel(timer);
        os_memset(&g_hr_app, 0 , sizeof(hr_app_cntx_t));
        BT_LOGD("HRS", "connection link is invalid bt_hr_notify : stop... ");
#endif
    } else {
#ifndef __TS_WIN32__
        if (timer) {
           xTimerChangePeriod(timer, 3000 / portTICK_PERIOD_MS, 0);
           xTimerReset(timer, 0);
           //bt_timer_cancel(timer);
        }
#endif
        notify->attribute_value_length = 5;
        notify->att_req.opcode = BT_ATT_OPCODE_HANDLE_VALUE_NOTIFICATION;;
        notify->att_req.handle = 0x0602;
#ifdef MTK_MCS_ENABLE
        int ledstatus = 0;
        ledstatus = mcs_get_gpio33_led();
        g_hr_app.hr_value[0] = 0;
        g_hr_app.hr_value[1] = ledstatus; //led statsus
#else
        g_hr_app.hr_value[0] = 0;
        g_hr_app.hr_value[1] = 30;
#endif        
        os_memcpy(notify->att_req.attribute_value, &g_hr_app.hr_value, 2);
        BT_LOGD("HRS", "bt_hr_notify : notify data = %s", notify->att_req.attribute_value);
        status =  bt_gatts_send_charc_value_notification_indication(conn_handle, notify);
        BT_LOGD("HRS", "bt_hr_notify : notificaiton status = %d", status);
    }
}

static uint32_t bt_if_heartrate_client_config_callback(const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    BT_LOGD("HRS", "client_config_callback : RW= %d, size = %d", rw, size);

    if (rw == BT_GATTS_CALLBACK_WRITE) {
        if (size != sizeof(g_hr_app.is_notify)) { //Size check
            return 0;
        }
        g_hr_app.is_notify = *(uint16_t *)data;
        BT_LOGD("HRS", "client_config_callback :notify= %d, data = %d", g_hr_app.is_notify, *(uint16_t *)data);

        if (g_hr_app.is_notify ==
                CLIENT_CHARC_CONFIGURATION_NOTIFICATION) {
            g_hr_app.conn_handle = handle;
#ifndef __TS_WIN32__
            g_hr_app.timer = xTimerCreate("HRS Timer", 0xffff, pdFALSE, ( void *) 0,
                                          (TimerCallbackFunction_t)bt_hr_notify);
            xTimerChangePeriod(g_hr_app.timer, 3000 / portTICK_PERIOD_MS, 0);
            xTimerReset(g_hr_app.timer, 0);
           // bt_timer_start(g_hr_app.timer, 0, 3000, bt_hr_notify)
#else
            bt_hr_notify();
#endif
        } else {
#ifndef  __TS_WIN32__
            if (g_hr_app.timer) {
            xTimerStop(g_hr_app.timer, 0);
            xTimerDelete(g_hr_app.timer, 0);
            //bt_timer_cancel(g_hr_app.timer);
            BT_LOGD("HRS", "bt_hr_notify : stop... ");
            }

#endif
        }
    } else {
        if (size != 0) {
            uint16_t *buf = (uint16_t *) data;
            *buf = g_hr_app.is_notify;
        }
    }
    return sizeof(g_hr_app.is_notify);
}

static uint32_t bt_if_gpio_control_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
#ifndef MTK_MCS_ENABLE
    char gpio_status_buf[64]={"ON"};
    uint32_t str_size = strlen(gpio_status_buf);
    uint32_t buf_size = sizeof(gpio_status_buf);
    uint32_t copy_size;
#endif
    
    BT_LOGD("GPIO", "bt_if_gpio_control_callback : rw = %d ",rw);
    
    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            BT_LOGD("GPIO", "bt_if_gpio_control_callback : BT_GATTS_CALLBACK_READ ");
#ifdef MTK_MCS_ENABLE
            //do something here
            break;
#else
            copy_size = (str_size> offset)?(str_size-offset):0;
            if (size==0){
                return str_size;
            }
            copy_size = (size > copy_size)? copy_size:size;
            memcpy(data, gpio_status_buf+offset, copy_size);
            return copy_size;
#endif  
        case BT_GATTS_CALLBACK_WRITE:
            BT_LOGD("GPIO", "bt_if_gpio_control_callback : BT_GATTS_CALLBACK_WRITE : %s", (char*)data);
#ifdef MTK_MCS_ENABLE
            if ( os_strncmp((char *)data , "ON", 2) == 0 )//turn on led
                mcs_set_gpio33_led(1);
            else if ( os_strncmp((char *)data , "OFF", 3) == 0 )// turn off led 
                mcs_set_gpio33_led(0);
            else
                BT_LOGD("GPIO", "bt_if_gpio_control_callback : ERROR OPERATION!");
                                
            break;
#else
           copy_size = (size > buf_size)? buf_size:size;
            memset(gpio_status_buf, 0 , buf_size);
            memcpy(gpio_status_buf, data, copy_size);
            return copy_size;
#endif
        default:
            //nothing here
            break;
    }
    return BT_STATUS_SUCCESS;
}


BT_GATTS_NEW_PRIMARY_SERVICE_16(bt_if_hearrate_primary_service, BT_GATT_UUID16_HEART_RATE_SERVICE);

BT_GATTS_NEW_CHARC_16(bt_if_heartrate_measurement,
                      BT_GATT_CHARC_PROP_NOTIFY, 0x0602, BT_SIG_UUID16_HEARTRATE_MEASUREMENT);

BT_GATTS_NEW_CHARC_VALUE_UINT16(bt_if_heartrate_measurement_value, BT_SIG_UUID_HEARTRATE_MEASUREMENT,
                                BT_GATTS_REC_PERM_READABLE,
                                0);


BT_GATTS_NEW_CLIENT_CHARC_CONFIG(bt_if_heartrate_client_config,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 bt_if_heartrate_client_config_callback);
                                 
static const bt_gatts_service_rec_t *bt_if_hr_service_rec[] = {
    (const bt_gatts_service_rec_t *) &bt_if_hearrate_primary_service,
    (const bt_gatts_service_rec_t *) &bt_if_heartrate_measurement,
    (const bt_gatts_service_rec_t *) &bt_if_heartrate_measurement_value,
    (const bt_gatts_service_rec_t *) &bt_if_heartrate_client_config
};

const bt_gatts_service_t bt_if_hr_service = {
    .starting_handle = 0x0600,
    .ending_handle = 0x0603,
    .required_encryption_key_size = 0,
    .records = bt_if_hr_service_rec
};
                                 
//added for GPIO control
BT_GATTS_NEW_PRIMARY_SERVICE_16(bt_if_gpio_primary_service, BT_GATT_UUID16_GPIO_SERVICE);

BT_GATTS_NEW_CHARC_16_WRITABLE(bt_if_gpio_control, BT_GATT_CHARC_PROP_READ, 0x0702, BT_SIG_UUID16_GPIO_CONTROL);


BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_gpio_control_value, BT_SIG_UUID_GPIO_CONTROL,
                BT_GATTS_REC_PERM_READABLE|BT_GATTS_REC_PERM_WRITABLE, bt_if_gpio_control_callback);


#if 0  
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(bt_if_gpio_control_client_config,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 bt_if_gpio_control_callback);
#else
BT_GATTS_NEW_CHARC_USER_DESCRIPTION(bt_if_gpio_control_client_config,
                                 BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 bt_if_gpio_control_callback);
#endif


static const bt_gatts_service_rec_t *bt_if_gpio_service_rec[] = {
    (const bt_gatts_service_rec_t *) &bt_if_gpio_primary_service,
    (const bt_gatts_service_rec_t *) &bt_if_gpio_control,
    (const bt_gatts_service_rec_t *) &bt_if_gpio_control_value,
    (const bt_gatts_service_rec_t *) &bt_if_gpio_control_client_config
};


const bt_gatts_service_t bt_if_gpio_service = {
    .starting_handle = 0x0700,
    .ending_handle = 0x0703,
    .required_encryption_key_size = 0,
    .records = bt_if_gpio_service_rec
};

/* heart rate end ...*/
#endif
