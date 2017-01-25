#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "ethernetif.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"
#include "timers.h"
#include "os.h"
#include "httpclient.h"
#include "mcs.h"

#define BUF_SIZE   (1024 * 1)

/* Now only .com , must do for china */
#define HTTPS_MTK_CLOUD_URL_COM "http://api.mediatek.com/mcs/v2/devices/"
#define HTTPS_MTK_CLOUD_URL_CN "http://api.mediatek.cn/mcs/v2/devices/"

#include "hal_sys.h"
#include "fota.h"
#include "fota_config.h"

char *TCP_ip [20] = {0};

/* tcp config */
#define SOCK_TCP_SRV_PORT 443

#define MAX_STRING_SIZE 200
TimerHandle_t heartbeat_timer;

/* to get TCP IP */
HTTPCLIENT_RESULT getInitialTCPIP () {
    int ret = HTTPCLIENT_ERROR_CONN;
    httpclient_t client = {0};
    char *buf = NULL;

    httpclient_data_t client_data = {0};

    /* set Url */
    char get_url[70] ={0};

    //printf("deviceId: %s\n", DEVICEID);
    //printf("deviceKey: %s\n", DEVICEKEY);
    //printf("host0: %s\n", HOST);

    if (strcmp(HOST, "com") == 0) {
        strcat(get_url, HTTPS_MTK_CLOUD_URL_COM);
    } else {
        strcat(get_url, HTTPS_MTK_CLOUD_URL_CN);
    }
    //printf("host1: %s\n", get_url);
    strcat(get_url, DEVICEID);
    strcat(get_url, "/connections.csv");
    //printf("host2: %s\n", get_url);

    /* Set header */
    char header[40] = {0};
    strcat(header, "deviceKey:");
    strcat(header, DEVICEKEY);
    strcat(header, "\r\n");

    buf = pvPortMalloc(BUF_SIZE);
    if (buf == NULL) {
        return ret;
    }
    buf[0] = '\0';
    ret = httpclient_connect(&client, get_url);

    client_data.response_buf = buf;
    client_data.response_buf_len = BUF_SIZE;
    httpclient_set_custom_header(&client, header);

    ret = httpclient_get(&client, get_url, &client_data);
    if (ret < 0) {
        return ret;
    }

    //printf("content:%s\n", client_data.response_buf);

    if (200 == httpclient_get_response_code(&client)) {
        char split_buf[MAX_STRING_SIZE] = {0};
        strcpy(split_buf, client_data.response_buf);

        char *arr[1];
        char *del = ",";
        mcs_splitn(arr, split_buf, del, 2);
        strncpy(TCP_ip, arr[0], 20);
    }
    vPortFree(buf);
    httpclient_close(&client);
    return ret;
}

int32_t mcs_tcp_init(void (*mcs_tcp_callback)(char *))
{
    int s;
    int c;
    int ret;
    struct sockaddr_in addr;
    int count = 0;
    int rcv_len, rlen;

    int32_t mcs_ret = MCS_TCP_DISCONNECT;

    /* Setting the TCP ip */
    if (HTTPCLIENT_OK != getInitialTCPIP()) {
        return MCS_TCP_INIT_ERROR;
    }

    /* command buffer */
    char cmd_buf [50]= {0};
    strcat(cmd_buf, DEVICEID);
    strcat(cmd_buf, ",");
    strcat(cmd_buf, DEVICEKEY);
    strcat(cmd_buf, ",0");

mcs_tcp_connect:
    os_memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SOCK_TCP_SRV_PORT);
    addr.sin_addr.s_addr =inet_addr(TCP_ip);

    /* create the socket */
    s = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        mcs_ret = MCS_TCP_SOCKET_INIT_ERROR;
        //printf("tcp client create fail 0\n");
        goto idle;
    }

    ret = lwip_connect(s, (struct sockaddr *)&addr, sizeof(addr));

    if (ret < 0) {
        lwip_close(s);
        //printf("tcp client connect fail 1\n");
        goto mcs_tcp_connect;
    }

    /* timer */
    void tcpTimerCallback( TimerHandle_t pxTimer ) {
        ret = lwip_write(s, cmd_buf, sizeof(cmd_buf));
    }

    heartbeat_timer = xTimerCreate("TimerMain", (30*1000 / portTICK_RATE_MS), pdTRUE, (void *)0, tcpTimerCallback);
    xTimerStart( heartbeat_timer, 0 );

    for (;;) {
        char rcv_buf[MAX_STRING_SIZE] = {0};

        if (0 == count) {
            ret = lwip_write(s, cmd_buf, sizeof(cmd_buf));
        }

        //LOG_I(common, "MCS tcp-client waiting for data...");
        rcv_len = 0;
        rlen = lwip_recv(s, &rcv_buf[rcv_len], sizeof(rcv_buf) - 1 - rcv_len, 0);
        rcv_len += rlen;

        if ( 0 == rcv_len ) {
            return MCS_TCP_DISCONNECT;
        }

        //LOG_I(common, "MCS tcp-client received data:%s", rcv_buf);

        /* split the string of rcv_buffer */
        char split_buf[MAX_STRING_SIZE] = {0};
        strcpy(split_buf, rcv_buf);

        char *arr[7];
        char *del = ",";
        mcs_splitn(arr, split_buf, del, 7);
        if (0 == strncmp (arr[3], "FOTA", 4)) {
            char *s = mcs_replace(arr[6], "https", "http");
            //printf("fota url: %s\n", s);
            char data_buf [100] = {0};
            strcat(data_buf, "status");
            strcat(data_buf, ",,fotaing");
            mcs_upload_datapoint(data_buf);
            fota_download_by_http(s);
            fota_trigger_update();

            fota_ret_t err;
            err = fota_trigger_update();
            if (0 == err ){
                hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
                return 0;
            } else {
                return -1;
            }
        } else {
          mcs_tcp_callback(rcv_buf);
        }

        count ++;
    }

idle:
    //LOG_I(common, "MCS tcp-client end");
    return MCS_TCP_DISCONNECT;
}