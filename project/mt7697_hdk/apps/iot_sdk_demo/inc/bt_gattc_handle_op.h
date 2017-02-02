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

#ifndef __BT_GATT_HANDLE_OP_H__
#define __BT_GATT_HANDLE_OP_H__

#include "bt_gattc.h"

#include "stdint.h"
#include "bt_uuid.h"

#define __BT_MAX_LE_LINK_NUM__ 4
#define SRV_MAX_DEV  __BT_MAX_LE_LINK_NUM__

/*gattc current state*/
#define GATTC_IN_IDLE 0
#define GATTC_CONNECTED 1
#define GATTC_SEARCH_DONE   2
#define GATTC_IN_SEARCHING 3

typedef struct {
    uint8_t addr[16];
} ble_address_t;

typedef struct {
    uint8_t uuid[16];
} gattc_uuid_t;

typedef struct {
    uint16_t handle;
    uint16_t uuid;
} bt_gatt_descriptor_t;

typedef struct {
    uint16_t handle;
    uint16_t value_handle;
    uint8_t property;
    uint16_t uuid;
    bt_gatt_descriptor_t descr[3];
} bt_gatt_char_t;

typedef struct {
    uint16_t handle;
    uint16_t start_handle;
    uint16_t end_handle;
    uint16_t uuid;
} bt_gatt_included_service_t;

typedef struct {
    uint16_t start_handle;
    uint16_t end_handle;
    uint16_t uuid;
    uint8_t num_of_char;
    uint8_t explore_index;
    bt_gatt_included_service_t incl_srv[3];
    bt_gatt_char_t chara[3];
} bt_gatt_service_t;


typedef struct {
    uint16_t service_number; /*current primary service number*/
    uint32_t conn_id;
    bt_gatt_service_t att_data;     /* Stores all the supported services found on the Server */
} gatt_data_t;


typedef struct {
    uint8_t flag;
    uint16_t state;
    uint16_t conn_id;
    gatt_data_t data;
} gattc_conn_t;

typedef struct {
    uint8_t *value;
    uint16_t handle;
    uint16_t size;
} bt_gattc_write_descr_req_t;


/*start discovery service from remote device*/
int32_t bt_gattc_start_discover_service(uint16_t conn_id);
bt_gatt_char_t *bt_gattc_get_char_by_handle(uint16_t conn_id, uint16_t handle);
/*write descriptor's value*/
bt_status_t bt_gattc_write_descr(uint16_t conn_id, bt_gattc_write_descr_req_t *req);
bt_status_t bt_gattc_event_callback(bt_msg_type_t msg, bt_status_t status, void *param);

void ble_gpio_set_adv(void);
#endif
