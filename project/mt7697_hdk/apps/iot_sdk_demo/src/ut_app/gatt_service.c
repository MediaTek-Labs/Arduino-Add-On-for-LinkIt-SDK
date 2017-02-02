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

#include "gatt_service.h"
#include <string.h>
const bt_uuid_t CLI_BT_SIG_UUID_SERVICE_CHANGED =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_SERVICE_CHANGED);
const bt_uuid_t CLI_BT_SIG_UUID_DEVICE_NAME =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_DEVICE_NAME);
const bt_uuid_t CLI_BT_SIG_UUID_APPEARANCE =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_APPEARANCE);
const bt_uuid_t CLI_BT_SIG_UUID_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS);
const bt_uuid_t CLI_BT_SIG_UUID_SERIAL_NUMBER =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_SERIAL_NUMBER);
const bt_uuid_t CLI_BT_SIG_UUID_CENTRAL_ADDRESS_RESOLUTION =
    BT_UUID_INIT_WITH_UUID16(BT_SIG_UUID16_CENTRAL_ADDRESS_RESOLUTION);

//Declare every record here
//service collects all bt_gatts_service_rec_t
//IMPORTAMT: handle:0x0000 is reserved, please start your handle from 0x0001
//GAP 0x0001
char gatts_device_name[256]={"MTKHB"};
static uint32_t bt_if_gap_dev_name_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    uint32_t str_size = strlen(gatts_device_name);
    uint32_t buf_size = sizeof(gatts_device_name);
    uint32_t copy_size;
#ifdef MTK_BLE_GPIO_SERVICE    
    BT_LOGD("GAP", "bt_if_gap_dev_name_callback : rw = %d ",rw);
#endif    
    switch (rw) {
        case BT_GATTS_CALLBACK_READ:
            copy_size = (str_size> offset)?(str_size-offset):0;
            if (size==0){
                return str_size;
            }
            copy_size = (size > copy_size)? copy_size:size;
            memcpy(data, gatts_device_name+offset, copy_size);
            return copy_size;
        case BT_GATTS_CALLBACK_WRITE:
            copy_size = (size > buf_size)? buf_size:size;
            memcpy(gatts_device_name, data, copy_size);
            return copy_size;
        default:
            return BT_STATUS_SUCCESS;
    }
}

uint16_t gap_appearance=0x1234;//GAP appearance
static uint32_t bt_if_gap_appearance_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
#ifdef MTK_BLE_GPIO_SERVICE
    BT_LOGD("GAP", "bt_if_gap_appearance_callback : rw = %d ",rw);
#endif
    if (rw == BT_GATTS_CALLBACK_WRITE){
        if (size != sizeof(gap_appearance)){ //Size check
            return 0;
        }
        gap_appearance = *(uint16_t*)data;
    }else {
        if (size!=0){
            uint16_t *buf = (uint16_t*) data;
            *buf = gap_appearance;
        }
    }
    return sizeof(gap_appearance);
}
BT_GATTS_NEW_PRIMARY_SERVICE_16(bt_if_gap_primary_service, BT_GATT_UUID16_GAP_SERVICE);
BT_GATTS_NEW_CHARC_16_WRITABLE(bt_if_gap_char4_dev_name, BT_GATT_CHARC_PROP_READ, 0x0003, BT_SIG_UUID16_DEVICE_NAME);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_gap_dev_name, CLI_BT_SIG_UUID_DEVICE_NAME,
                BT_GATTS_REC_PERM_READABLE|BT_GATTS_REC_PERM_WRITABLE, bt_if_gap_dev_name_callback);
/* For BQB test TC_GAR_SR_BV_07_C & TC_GAR_SR_BV_08_C*/
BT_GATTS_NEW_CHARC_USER_DESCRIPTION(bt_if_gap_dev_name_user_description,
                BT_GATTS_REC_PERM_READABLE, bt_if_gap_dev_name_callback);
/* For BQB test TC_GAR_SR_BI_01_C */
/* This test characteristic can not read and write */
BT_GATTS_NEW_CHARC_16(bt_if_gap_char4_serial_number, 0,
                0x0006, BT_SIG_UUID16_SERIAL_NUMBER);
BT_GATTS_NEW_CHARC_VALUE_STR16(bt_if_gap_serial_number, CLI_BT_SIG_UUID_SERIAL_NUMBER,
                0, 9, MY_VENDOR_SERIAL_NUMBER);
/* For BQB test TC_GAR_SR_BI_28_C */
//can not read and write.
BT_GATTS_NEW_CHARC_USER_DESCRIPTION_STR16(bt_if_gap_serial_number_user_description,
                0,
                8, "MediaTek");
BT_GATTS_NEW_CHARC_16_WRITABLE(bt_if_gap_char4_appearance, BT_GATT_CHARC_PROP_READ, 0x0009, BT_SIG_UUID16_APPEARANCE);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(bt_if_gap_appearance, CLI_BT_SIG_UUID_APPEARANCE,
                BT_GATTS_REC_PERM_READABLE|BT_GATTS_REC_PERM_WRITABLE, bt_if_gap_appearance_callback);

BT_GATTS_NEW_CHARC_16(bt_if_gap_char4_ppcp, BT_GATT_CHARC_PROP_READ, 0x000B, BT_SIG_UUID16_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS);
BT_GATTS_NEW_CHARC_VALUE_HALFW8_WRITABLE(bt_if_gap_ppcp, CLI_BT_SIG_UUID_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS,
                BT_GATTS_REC_PERM_READABLE, 8, 0x0580,0x0c80,0x0010,0x0333);
BT_GATTS_NEW_CHARC_16(bt_if_gap_char4_central_address_resolution, BT_GATT_CHARC_PROP_READ, 0x000D, BT_SIG_UUID16_CENTRAL_ADDRESS_RESOLUTION);
BT_GATTS_NEW_CHARC_VALUE_UINT8_WRITABLE(bt_if_central_address_resolution, CLI_BT_SIG_UUID_CENTRAL_ADDRESS_RESOLUTION, BT_GATTS_REC_PERM_READABLE, 1);

static const bt_gatts_service_rec_t *bt_if_gap_service_rec[] = {
    (const bt_gatts_service_rec_t*) &bt_if_gap_primary_service,
    (const bt_gatts_service_rec_t*) &bt_if_gap_char4_dev_name,
    (const bt_gatts_service_rec_t*) &bt_if_gap_dev_name,
    (const bt_gatts_service_rec_t*) &bt_if_gap_dev_name_user_description,
    (const bt_gatts_service_rec_t*) &bt_if_gap_char4_serial_number,
    (const bt_gatts_service_rec_t*) &bt_if_gap_serial_number,
    (const bt_gatts_service_rec_t*) &bt_if_gap_serial_number_user_description,
    (const bt_gatts_service_rec_t*) &bt_if_gap_char4_appearance,
    (const bt_gatts_service_rec_t*) &bt_if_gap_appearance,
    (const bt_gatts_service_rec_t*) &bt_if_gap_char4_ppcp,
    (const bt_gatts_service_rec_t*) &bt_if_gap_ppcp,
    (const bt_gatts_service_rec_t*) &bt_if_gap_char4_central_address_resolution,
    (const bt_gatts_service_rec_t*) &bt_if_central_address_resolution
    };

static const bt_gatts_service_t bt_if_gap_service = {
    .starting_handle = 0x0001,
    .ending_handle = 0x000D,
    .required_encryption_key_size = 7,
    .records = bt_if_gap_service_rec
    };

//GATT 0x0011
/*---------------------------------------------*/
BT_GATTS_NEW_PRIMARY_SERVICE_16(gatt_primary_service, BT_GATT_UUID16_GATT_SERVICE);
BT_GATTS_NEW_CHARC_16(gatt_char4_service_changed, BT_GATT_CHARC_PROP_READ|BT_GATT_CHARC_PROP_NOTIFY|BT_GATT_CHARC_PROP_INDICATE,
                0x0013, BT_SIG_UUID16_SERVICE_CHANGED);
BT_GATTS_NEW_CHARC_VALUE_UINT32_WRITABLE(gatt_service_changed, CLI_BT_SIG_UUID_SERVICE_CHANGED,
                             0x2, 0x0001050F);
static const bt_gatts_service_rec_t *gatt_service_rec[] = {
    (const bt_gatts_service_rec_t*) &gatt_primary_service,
    (const bt_gatts_service_rec_t*) &gatt_char4_service_changed,
    (const bt_gatts_service_rec_t*) &gatt_service_changed
    };

static const bt_gatts_service_t bt_if_gatt_service_ro = {
    .starting_handle = 0x0011,
    .ending_handle = 0x0013,
    .required_encryption_key_size = 7,
    .records = gatt_service_rec
    };

#ifdef MTK_BLE_SMTCN_ENABLE
extern const bt_gatts_service_t bt_if_ble_smtcn_service;
#endif
#ifdef MTK_BLE_GPIO_SERVICE
extern const bt_gatts_service_t bt_if_hr_service;
extern const bt_gatts_service_t bt_if_gpio_service;
#endif
//server collects all service
const bt_gatts_service_t * bt_if_clm_gatt_server[] = {
    &bt_if_gap_service,//0x0001
    &bt_if_gatt_service_ro,//0x0011
#ifdef MTK_BLE_SMTCN_ENABLE
    &bt_if_ble_smtcn_service, //0x0014-0x0017
#endif
#ifdef MTK_BLE_GPIO_SERVICE
    &bt_if_hr_service, //0x0018-0x001B
    &bt_if_gpio_service,
#endif
    NULL
    };

extern bool is_cli_bqb_mode;
extern const bt_gatts_service_t** bqb_get_gatt_server(void);
//When GATTS get req from remote client, GATTS will call bt_get_gatt_server() to get application's gatt service DB.
//You have to return the DB(bt_gatts_service_t pointer) to gatts stack.
const bt_gatts_service_t** bt_get_gatt_server()
{
    if (is_cli_bqb_mode) {
        return bqb_get_gatt_server();
    } else {
        return bt_if_clm_gatt_server;
    }
}

extern bt_status_t bt_gatt_service_execute_write(uint16_t handle, uint8_t flag);
bt_status_t bt_gatts_get_execute_write_result(bt_gatts_execute_write_req_t *req)
{
    return bt_gatt_service_execute_write(req->handle, req->flag);
}
