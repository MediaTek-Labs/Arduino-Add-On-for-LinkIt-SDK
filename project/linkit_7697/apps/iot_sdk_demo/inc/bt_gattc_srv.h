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

#ifndef __BLE_GATTC_H__
#define __BLE_GATTC_H__

#include <stdint.h>
#include "bt_debug.h"
#include "bt_gattc.h"
#include "bt_gap_le.h"
#include "bt_gattc_handle_op.h"
#include "connection_info.h"
#include <stdbool.h>
#include <stdint.h>
/************************************************
*    GATTC request definition
*************************************************/
//#define SRV_MAX_DEV 1
typedef struct {
    uint8_t flag;
    uint16_t val;
    uint16_t en_expend;
    uint16_t RR_inteval;
} hr_data_t;

typedef struct {
    uint16_t len;
    uint8_t uuid[16];
} app_uuid_t;

/*WRITE A VALUT TO CHARACTERISTIC OR DESCRIPTOR*/
typedef struct {
    uint16_t len;
    uint8_t  value[512];/*this vaule length maybe change*/
} gattc_value_t;

/*USER UINT*/
typedef struct {
    void *reg_cntx; /*gattc srv alloc cntx to user by uuid*/
    uint16_t conn_id;
} gattc_user_connect_struct;


/* user uint of  in gattc srv*/
typedef struct {
    uint8_t flag; /*after connect, for connect list */
    uint8_t *uuid; /*app care's service uuid*/
    uint8_t uuid_count; /*app care's service uuid's  number*/
    void *appCb; /*app callbcak*/
} gattc_user_context_t;

typedef struct {
    gattc_user_context_t userContext;
    gattc_conn_t conntext[SRV_MAX_DEV];
} gattc_context_t;


/* GATTC_REISTER_REQ */
typedef struct {
    uint8_t *uuid; /*app care's service uuid*/
    uint8_t uuid_count; /*app care's service uuid's  number*/
} gattc_register_req_struct;

typedef void *gattc_reg_handle;
extern gattc_context_t g_gattc_ctx;

/*for callback done*/

/*this function will be callled when  client  has  scanned some remote devices information  */
typedef void(*app_scan_callback)(void *reg_cntx, bt_gap_le_advertising_report_ind_t *param);

/*this function will be called when client has  connected or disconnected with a remote device */
typedef void(*app_connect_callback)(gattc_user_connect_struct *conn, uint16_t connected, bt_addr_t *bd_addr);

/* this function will be called when client  recevieved  a nofity or indication from remote device*/
typedef void (*app_noti_callback)(gattc_user_connect_struct *conn, uint16_t handle, gattc_value_t *value);

/*this function will be called when  discovery service from remote device has been done*/
typedef void(*app_search_complete_callback)(gattc_user_connect_struct *conn, int32_t result, bt_gatt_service_t *service);

/*gattc srv notify to user app */
typedef struct {
    app_scan_callback scan_cb;
    app_connect_callback connect_cb;
    app_search_complete_callback search_complete_cb;
    app_noti_callback notify_cb;
} app_callback_struct;

/*Function defination*/

/*every user reigister in gattsrv*/
void  *gattc_register(gattc_register_req_struct *req, app_callback_struct *cb);

/*every user dereigister from  gattsrv*/
void gattc_deregister(gattc_user_context_t *user);

/*notify scan result to user*/
void gattc_notify_scan_info_to_all_user(bt_gap_le_advertising_report_ind_t *param);

/*notify connect result to user*/
void gattc_notify_conn_info_to_all_user(gattc_conn_t *gattc_conn, bt_status_t error_code, app_bt_connection_cb_t *conn_info, uint16_t type);

void gattc_decode_char_data(gattc_value_t *value, hr_data_t *data);

uint16_t gattc_convert_array_to_uuid16(app_uuid_t *uu);
#endif


