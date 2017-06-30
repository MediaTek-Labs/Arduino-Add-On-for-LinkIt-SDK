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

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* device.h includes */
#include "mt7687.h"
#include "system_mt7687.h"

#include "sys_init.h"
#include "task_def.h"

#include "wifi_lwip_helper.h"
#include "wifi_api.h"

#include "bt_init.h"

/* Create the log control block as user wishes. Here we use 'template' as module name.
 * User needs to define their own log control blocks as project needs.
 * Please refer to the log dev guide under /doc folder for more details.
 */
log_create_module(template, PRINT_LEVEL_INFO);

/**
* @brief       Main function
* @param[in]   None.
* @return      None.
*/
int main(void)
{
    /* Do system initialization, eg: hardware, nvdm and random seed. */
    system_init();

    /* system log initialization.
     * This is the simplest way to initialize system log, that just inputs three NULLs
     * as input arguments. User can use advanved feature of system log along with NVDM.
     * For more details, please refer to the log dev guide under /doc folder or projects
     * under project/mtxxxx_hdk/apps/.
     */
    log_init(NULL, NULL, NULL);

    LOG_I(template, "start to create task.\n");

    /* User initial the parameters for wifi initial process,  system will determin which wifi operation mode
     * will be started , and adopt which settings for the specific mode while wifi initial process is running*/
    wifi_config_t config = {0};
    config.opmode = WIFI_MODE_STA_ONLY;
    strcpy((char *)config.sta_config.ssid, (const char *)"MTK_STA");
    strcpy((char *)config.sta_config.password, (const char *)"12345678");
    config.sta_config.ssid_length = strlen((const char *)config.sta_config.ssid);
    config.sta_config.password_length = strlen((const char *)config.sta_config.password);


    /* Initialize wifi stack and register wifi init complete event handler,
     * notes:  the wifi initial process will be implemented and finished while system task scheduler is running.*/
    wifi_init(&config, NULL);

    /* Tcpip stack and net interface initialization,  dhcp client, dhcp server process initialization*/
    lwip_network_init(config.opmode);
    lwip_net_start(config.opmode);

    bt_create_task();
	
	/* As for generic HAL init APIs like: hal_uart_init(), hal_gpio_init() and hal_spi_master_init() etc,
     * user can call them when they need, which means user can call them here or in user task at runtime.
     */

    /* Create a user task for demo when and how to use wifi config API to change WiFI settings,
    Most WiFi APIs must be called in task scheduler, the system will work wrong if called in main(),
    For which API must be called in task, please refer to wifi_api.h or WiFi API reference.
    xTaskCreate(user_wifi_app_entry,
                UNIFY_USR_DEMO_TASK_NAME,
                UNIFY_USR_DEMO_TASK_STACKSIZE / 4,
                NULL, UNIFY_USR_DEMO_TASK_PRIO, NULL);
    user_wifi_app_entry is user's task entry function, which may be defined in another C file to do application job.
    UNIFY_USR_DEMO_TASK_NAME, UNIFY_USR_DEMO_TASK_STACKSIZE and UNIFY_USR_DEMO_TASK_PRIO should be defined
    in task_def.h. User needs to refer to example in task_def.h, then makes own task MACROs defined.
    */

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for( ;; );
}

