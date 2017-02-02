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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include <lwip/tcpip.h>
#include <ethernetif.h>
#include <lwip/sockets.h>

#include "os.h"
#include "ut_app.h"
#include "task_def.h"
#if defined(MTK_BT_LWIP_ENABLE)

#define SOCK_TCP_SRV_PORT        23
int wifi_console = 0;
static void tcp_server_console(void)
{
    int s;

    int ret;
    int rlen;
    int i;
    int index = 0;
    int rev_cmd = 0;
    struct sockaddr_in addr;
    char srv_buf[32] = {0};
    char cmd_buf[100] = {0};
    LOG_I(common, "tcp_server_test start\n");

    os_memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(SOCK_TCP_SRV_PORT);
    addr.sin_addr.s_addr = lwip_htonl(IPADDR_ANY);

    /* create the socket */
    s = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        LOG_I(common, "tcp server create fail\n");
        goto done;
    }

    ret = lwip_bind(s, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_I(common, "tcp server bind fail\n");
        goto clean;
    }

    ret = lwip_listen(s, 0);
    if (ret < 0) {
        LOG_I(common, "tcp server listen fail\n");
        goto clean;
    }

    while ( 1 ) {
        socklen_t sockaddr_len = sizeof(addr);
        wifi_console = 0;
        LOG_I(common, "tcp server waiting for accept connection...\n");
        wifi_console = lwip_accept(s, (struct sockaddr *)&addr, &sockaddr_len);
        if (wifi_console < 0) {
            LOG_I(common, "tcp server accept error\n");
            break;   // or go on waiting for connect requestion?
        }

        LOG_I(common, "tcp server waiting for data...\n");
        while ((rlen = lwip_read(wifi_console, srv_buf, sizeof(srv_buf) - 1)) != 0) {
            if (rlen < 0) {
                LOG_I(common, "read error.\n");
                break;
            }
            srv_buf[rlen] = 0; //for the next statement - printf string.
            //printf("tcp server received data:%s\n", srv_buf);
            for (i = 0; i < rlen; i++) {
                if (srv_buf[i] == '\n' || srv_buf[i] == 13) {
                    rev_cmd = 1;
                    break;
                }

                if ((srv_buf[i] & 0xf0) == 0xf0) {
                    continue;
                }
                if (i < 100) {
                    cmd_buf[index++] = srv_buf[i];
                }
            }

            if (rev_cmd) {
                printf("tcp server received cmd_buf :%s\n", cmd_buf);
                printf("tcp server received transfer data:%s\n", srv_buf);

                index = 0 ;
                rev_cmd = 0;
                bt_app_io_callback(cmd_buf, NULL);
            }
            //lwip_write(wifi_console, srv_buf, rlen);      // sonar server
        }

        lwip_close(wifi_console);
    }

clean:
    lwip_close(s);

done:
    LOG_I(common, "tcp server test done\n");

}

void tcp_server_thread(void *not_used)
{

    tcp_server_console();

    //keep the task alive
    for (;;) {
    }
}

void bt_lwip_send(const void *data, size_t size)
{
    lwip_send(wifi_console, data, size, MSG_DONTWAIT);
}

void bt_lwip_init(void)
{
    LOG_I(common, "begin to create tcp server task\n");
    if (pdPASS != xTaskCreate(tcp_server_thread, TCP_TASK_NAME, TCP_TASK_STACKSIZE / sizeof(portSTACK_TYPE), NULL, TCP_TASK_PRIO, NULL)) {
        LOG_E(common, "cannot create tcp_server_thread.");
    }

}
#endif

