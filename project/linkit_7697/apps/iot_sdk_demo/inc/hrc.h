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

#ifndef __HR_APP_H__
#define __HR_APP_H__

#include <stdint.h>
#include <stdbool.h>
#ifndef WIN32
#include "FreeRTOS.h"

#include "queue.h"
#include "task.h"
#endif
#include "bt_gattc_connect.h"

#define SRV_HR_SERVICE_MAX_NUM  2

#define SRV_HRM_SER_UUID    0x180D

/*heartrate service's characteristics*/
typedef enum {
    HEART_RATE_MEASURE,
    BODY_SENSOR_LOCATION,
    HRART_RATE_CNT_POINT,
    HRART_RATE_TYPE_TOTAL
} srv_hrsp_char_type_enum;


/*device inforamtion saved in app*/
typedef struct {
    void *reg_ctx;
} app_hrp_context_struct;


/*characteristic */
typedef enum {
    HRM_CHAR_UUID = 0x2A37,/*only need change this char*/
    HBL_CHAR_UUID,
    HCP_CHAR_UUID,
    HR_CHAR_TOTAL
} char_uuid_enum;

typedef enum {
    CLIENT_CONFI_DESCRI = 0x2902,
    CLIENT_CONFI_TOTAL
} desc_uuid_enum;

/*heat rate's register*/
void heart_rate_init(void);

/*heat rate's deregister*/
void heart_rate_deinit(void);


typedef struct {
    bt_msg_type_t event_id;
    bt_status_t status;
    int8_t param[512];
} hear_rate_message_struct;
#ifndef WIN32
extern hear_rate_message_struct app_queue_data;
extern QueueHandle_t app_queue;
#endif


#endif /*__HR_APP_H__*/
