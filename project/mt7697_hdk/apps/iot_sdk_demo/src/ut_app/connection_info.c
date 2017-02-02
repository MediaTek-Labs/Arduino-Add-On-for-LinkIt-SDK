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

#include "connection_info.h"
#include "bt_debug.h"
#include <string.h>

app_bt_connection_cb_t connection_cb[BT_CONNECTION_MAX] = {{0}};
app_bt_bonded_info_t bonded_info_req[BT_CONNECTION_MAX] = {{{0}}};
static const bt_addr_t default_bt_addr = {
                       .type = BT_ADDR_TYPE_UNKNOW,
                       .addr = {0}
                       };

void add_connection_info(void *buff)
{
    uint8_t i;

    bt_gap_le_connection_ind_t *connection_ind = (bt_gap_le_connection_ind_t *)buff;
    for (i =0; i< BT_CONNECTION_MAX; i++) {
        if (connection_cb[i].connection_handle==0) {
            connection_cb[i].connection_handle = connection_ind->connection_handle;
            connection_cb[i].role = connection_ind->role;
            connection_cb[i].peer_addr = connection_ind->peer_addr;
            break;
        }
    }
    if (i == BT_CONNECTION_MAX) {
        BT_LOGW("APP", "Reach maximum connection\n");
    }
}

void dump_connection_info_list()
{
    uint8_t i;
    for (i = 0; i< BT_CONNECTION_MAX ; i++) {
        if (connection_cb[i].connection_handle) {
            BT_LOGD("APP", "Connection Info[%d]: [%s] [0x%04x] [%s]",i,bt_debug_addr2str(&(connection_cb[i].peer_addr)), connection_cb[i].connection_handle, connection_cb[i].role?"Slave":"Master");
        }
    }
}

void delete_connection_info(void *buff)
{
    bt_hci_evt_disconnect_complete_t *disconnect_complete;
    uint8_t i;

    disconnect_complete = (bt_hci_evt_disconnect_complete_t*) buff;
    for (i = 0; i< BT_CONNECTION_MAX ; i++) {
        if (disconnect_complete->connection_handle == connection_cb[i].connection_handle) {
            connection_cb[i].connection_handle = 0;//clear conneciton info.
            connection_cb[i].gatts_wait_att_rx_opcode = 0;
            break;
        }
    }
    if (i == BT_CONNECTION_MAX) {
        BT_LOGW("APP", "Don't know connection info for deleting.\n");
    }
}
app_bt_connection_cb_t* find_conneciton_info_by_handle(bt_handle_t target_handle)
{
    uint8_t i;
    for (i = 0; i< BT_CONNECTION_MAX; i++) {
        if (target_handle == connection_cb[i].connection_handle) {
            return &(connection_cb[i]);
        }
    }
    return NULL;
}
app_bt_bonded_info_t* get_bonded_info(const bt_addr_t *target_bt, uint8_t create)
{
    uint8_t i;
    //Check have we been bonded?
    for (i = 0; i< BT_CONNECTION_MAX ; i++) {
        if (0 == memcmp(target_bt,&(bonded_info_req[i].bt_addr), sizeof(default_bt_addr))) {
            return &(bonded_info_req[i]);
        }
    }
    //Give a new
    if (create) {
        for (i = 0; i< BT_CONNECTION_MAX ; i++) {
            if (0 == memcmp(&default_bt_addr,&(bonded_info_req[i].bt_addr), sizeof(default_bt_addr))) {
                bonded_info_req[i].info.identity_addr.address.type = BT_ADDR_TYPE_UNKNOW;
                memcpy(&(bonded_info_req[i].bt_addr), target_bt, sizeof(default_bt_addr));
                return &(bonded_info_req[i]);
            }
        }
    }
    //Out of memory
    return NULL;
}
app_bt_bonded_info_t* find_bonded_info_by_index(uint8_t idx)
{
    if (idx <BT_CONNECTION_MAX) {
        if (0 != memcmp(&default_bt_addr,&(bonded_info_req[idx].bt_addr), sizeof(default_bt_addr))) {
            return &(bonded_info_req[idx]);
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}
app_bt_bonded_info_t* find_bonded_info_by_handle(bt_handle_t target_handle)
{
    app_bt_connection_cb_t *con = find_conneciton_info_by_handle(target_handle);
    return get_bonded_info(&(con->peer_addr), 0);
}
void cancel_bonded_info(const bt_addr_t *target_bt)
{
    uint8_t i;
    for (i = 0; i< BT_CONNECTION_MAX ; i++) {
        if (0 == memcmp(target_bt,&(bonded_info_req[i].bt_addr), sizeof(default_bt_addr))) {
            memset(&(bonded_info_req[i]),0x00, sizeof(app_bt_bonded_info_t));
            bonded_info_req[i].info.identity_addr.address.type = BT_ADDR_TYPE_UNKNOW;
            BT_LOGD("APP", "Cancel bonded info for BT addr %s",bt_debug_addr2str(target_bt));
        }
    }
}
void clear_bonded_info(void)
{
    uint8_t i;
    for (i = 0; i< BT_CONNECTION_MAX ; i++) {
        memset(&(bonded_info_req[i]),0x00, sizeof(app_bt_bonded_info_t));
        bonded_info_req[i].bt_addr.type = BT_ADDR_TYPE_UNKNOW;
        bonded_info_req[i].info.identity_addr.address.type = BT_ADDR_TYPE_UNKNOW;
    }
}
void dump_bonded_info_list()
{
    uint8_t i;
    for (i = 0; i< BT_CONNECTION_MAX ; i++) {
        if (0 != memcmp(&default_bt_addr,&(bonded_info_req[i].bt_addr), sizeof(default_bt_addr))) {
            BT_LOGD("APP", "Bonded Info[%d]: [%s]",i,bt_debug_addr2str(&(bonded_info_req[i].bt_addr)));
        }
    }
}
