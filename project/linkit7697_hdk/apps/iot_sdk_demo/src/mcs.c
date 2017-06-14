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

//#include "network_init.h"
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
//#include "MQTTClient.h"
#include "nvdm.h"
#include "hal_md5.h"
#include "hal_gpio.h"
#include "hal_eint.h"
#include "wifi_lwip_helper.h"
#include "hal_gpt.h"
#include "ut_app.h"
#ifdef MTK_BLE_GPIO_SERVICE
#include "ble_smtcn.h"
#endif
#ifdef MTK_FOTA_ENABLE
#include "fota_download_interface.h"
#endif

extern int g_supplicant_ready;

/* mcs setting */
#define MAX_DATA_SIZE 1024
#define ENCODE_MD5_CHANNEL "encodeByMD5"
#define DECODE_MD5_CHANNEL "decodeByMD5"
#define DEVICE_ID  "DNftuC0n"
#define DEVICE_KEY "0K4SOnOZaLuvuEC2"
#define HOST       "com"
#define CTRL_LED   "id_led_ctrl"
#define STATUS_LED "id_led_sta"
#define CTRL_BLE   "id_bt_ctrl"
#define STATUS_BLE "id_bt_sta"
#define CTRL_SCAN  "id_scan_ctrl"
#define BLE_SCAN_TAB "id_show_bt_scan_tab"

/* button setting */
QueueSetHandle_t g_btn_queue = NULL;
#define BTN_QUEUE_LEN 2


/* tcp config */
#define SOCK_TCP_SRV_PORT 443

#define MAX_STRING_SIZE 200
TimerHandle_t heartbeat_timer;

/* RESTful config */
#define BUF_SIZE   (1024 * 1)
/* Now only .com , must do for china */
#define HTTPS_MTK_CLOUD_URL_COM "http://api.mediatek.com/mcs/v2/devices/"
#define HTTPS_MTK_CLOUD_URL_CN "http://api.mediatek.cn/mcs/v2/devices/"

/* MQTT HOST */
#define MQTT_HOST_COM "mqtt.mcs.mediatek.com"
#define MQTT_HOST_CN "mqtt.mcs.mediatek.cn"

char TCP_ip [20] = {0};
char g_device_id[20] = {0};
char g_device_key[20] = {0};
char g_host[20] = {0};
char g_id_ctrl_led[20] = {0};
char g_id_status_led[20] = {0};
char g_id_ctrl_ble[20] = {0};
char g_id_status_ble[20] = {0};
char g_id_ctrl_scan[20] = {0};
char g_id_ble_scan_tab[20] = {0};

log_create_module(MCS, PRINT_LEVEL_INFO);


/* utils */
void mcs_split(char **arr, char *str, const char *del) {
  char *s = strtok(str, del);
  while(s != NULL) {
    *arr++ = s;
    s = strtok(NULL, del);
  }
}
/**
 * @brief Split MCS response into limited splits
 * @details There two difference between mcs_split:
 *          1. This function can avoid burst of MCS data
 *          (for now, two MCS response data concatnates sometimes when sending requests in high frequency)
 *          2. This function is reentrant version of mcs_split
 *          (use strtok_r instead of strtok)
 *
 * @param dst output buffer
 * @param src input buffer
 * @param delimiter
 * @param max_split max number of splits
 */
void mcs_splitn(char ** dst, char * src, const char * delimiter, uint32_t max_split)
{
    uint32_t split_cnt = 0;
    char *saveptr = NULL;
    char *s = strtok_r(src, delimiter, &saveptr);
    while (s != NULL && split_cnt < max_split) {
        *dst++ = s;
        s = strtok_r(NULL, delimiter, &saveptr);
        split_cnt++;
    }
}

char *mcs_replace(char *st, char *orig, char *repl) {
  static char buffer[1024];
  char *ch = strstr(st, orig);
  if (!(ch))
   return st;
  strncpy(buffer, st, ch-st);
  buffer[ch-st] = 0;
  sprintf(buffer+(ch-st), "%s%s", repl, ch+strlen(orig));
  return buffer;
}

/* to get TCP IP */
HTTPCLIENT_RESULT getInitialTCPIP () {
    HTTPCLIENT_RESULT ret = HTTPCLIENT_ERROR_CONN;
    httpclient_t client = {0};
    char *buf = NULL;

    httpclient_data_t client_data = {0};

    /* deviceKey */
    //char deviceKey[20];
    //int nvdm_deviceKey_len = sizeof(deviceKey);
    //nvdm_read_data_item("mcs", "deviceKey", (uint8_t *)deviceKey, (uint32_t *)&nvdm_deviceKey_len);

    /* deviceId */
    //char deviceId[20];
    //int nvdm_deviceId_len = sizeof(deviceId);
    //nvdm_read_data_item("mcs", "deviceId", (uint8_t *)deviceId, (uint32_t *)&nvdm_deviceId_len);

    /* set Url */
    char get_url[70] ={0};

    //char host[5];
    //int nvdm_host_len = sizeof(host);
    //nvdm_read_data_item("mcs", "host", (uint8_t *)host, (uint32_t *)&nvdm_host_len);

    if (strcmp(g_host, "com") == 0) {
        strcat(get_url, HTTPS_MTK_CLOUD_URL_COM);
    } else {
        strcat(get_url, HTTPS_MTK_CLOUD_URL_CN);
    }

    strcat(get_url, g_device_id);
    strcat(get_url, "/connections.csv");

    /* Set header */
    char header[40] = {0};
    strcat(header, "deviceKey:");
    strcat(header, g_device_key);
    strcat(header, "\r\n");

    buf = pvPortMalloc(BUF_SIZE);
    if (buf == NULL) {
        return ret;
    }
    buf[0] = '\0';
    //sky modified for sdk 4.0.0
    //ret = httpclient_connect(&client, get_url, HTTPS_PORT);
    ret = httpclient_connect(&client, get_url);

    client_data.response_buf = buf;
    client_data.response_buf_len = BUF_SIZE;
    httpclient_set_custom_header(&client, header);

    //sky modified for sdk 4.0.0
    //ret = httpclient_get(&client, get_url, HTTP_PORT, &client_data);
    ret = httpclient_get(&client, get_url, &client_data);
    if (ret < 0) {
        return ret;
    }

    LOG_I(MCS, "content:%s", client_data.response_buf);

    if (200 == httpclient_get_response_code(&client)) {
        char split_buf[MAX_STRING_SIZE] = {0};
        strcpy(split_buf, client_data.response_buf);

        char *arr[1];
        char *del = ",";
        mcs_split(arr, split_buf, del);
        strcpy(TCP_ip, arr[0]);
    }
    vPortFree(buf);
    //sky modified for sdk 4.0.0
    //httpclient_close(&client, HTTPS_PORT);
    httpclient_close(&client);
    return ret;
}

void mcs_upload_datapoint(char *value)
{
    /* upload mcs datapoint */
    httpclient_t client = {0};
    char *buf = NULL;

    int ret = HTTPCLIENT_ERROR_CONN;
    httpclient_data_t client_data = {0};
    char *content_type = "text/csv";
    // char post_data[32];

    /* deviceKey */
    //char deviceKey[20] = {0};
    //int nvdm_deviceKey_len = sizeof(deviceKey);
    //nvdm_read_data_item("mcs", "deviceKey", (uint8_t *)deviceKey, (uint32_t *)&nvdm_deviceKey_len);

    /* deviceId */
    //char deviceId[20] = {0};
    //int nvdm_deviceId_len = sizeof(deviceId);
    //nvdm_read_data_item("mcs", "deviceId", (uint8_t *)deviceId, (uint32_t *)&nvdm_deviceId_len);

    /* Set post_url */
    char post_url[70] ={0};

    //char host[5] = {0};
    //int nvdm_host_len = sizeof(host);
    //nvdm_read_data_item("mcs", "host", (uint8_t *)host, (uint32_t *)&nvdm_host_len);

    if (strcmp(g_host, "com") == 0) {
        strcat(post_url, HTTPS_MTK_CLOUD_URL_COM);
    } else {
        strcat(post_url, HTTPS_MTK_CLOUD_URL_CN);
    }

    strcat(post_url, g_device_id);
    strcat(post_url, "/datapoints.csv");

    /* Set header */
    char header[40] = {0};
    strcat(header, "deviceKey:");
    strcat(header, g_device_key);
    strcat(header, "\r\n");

    LOG_I(MCS, "header: %s", header);
    LOG_I(MCS, "url: %s", post_url);
    LOG_I(MCS, "data: %s", value);

    buf = pvPortMalloc(BUF_SIZE);
    if (buf == NULL) {
        LOG_E(MCS, "buf malloc failed.");
        return;// ret;
    }
    buf[0] = '\0';
    //sky modified for sdk 4.0.0
    //ret = httpclient_connect(&client, post_url, HTTPS_PORT);
    ret = httpclient_connect(&client, post_url);
    client_data.response_buf = buf;
    client_data.response_buf_len = BUF_SIZE;
    client_data.post_content_type = content_type;
    // sprintf(post_data, data);
    client_data.post_buf = value;
    client_data.post_buf_len = strlen(value);
    httpclient_set_custom_header(&client, header);
    ret = httpclient_send_request(&client, post_url, HTTPCLIENT_POST, &client_data);
    if (ret < 0) {
        return;// ret;
    }
    ret = httpclient_recv_response(&client, &client_data);
    if (ret < 0) {
        return;// ret;
    }
    LOG_I(MCS, "************************");
    LOG_I(MCS, "httpclient_test_keepalive post data every 5 sec, http status:%d, response data: %s", httpclient_get_response_code(&client), client_data.response_buf);
    LOG_I(MCS, "************************");
    vPortFree(buf);
    //sky modified for sdk 4.0.0
    //httpclient_close(&client, HTTPS_PORT);
    httpclient_close(&client);
    return;// ret;
}

#if 0
void mqttMessageArrived(MessageData *md) {
    char rcv_buf_old[100] = {0};

    MQTTMessage *message = md->message;
    char rcv_buf[100] = {0};
    strcpy(rcv_buf, message->payload);

    char split_buf[MAX_STRING_SIZE] = {0};
    strcpy(split_buf, rcv_buf);

    char *arr[7];
    char *del = ",";
    mcs_split(arr, split_buf, del);

    if (0 == strncmp (arr[3], "FOTA", 4)) {
        char *s = mcs_replace(arr[6], "https", "http");
        LOG_E(MCS, "fota url: %s", s);
        fota_download_by_http(s);
    } else {
        if (strcmp(rcv_buf_old, rcv_buf) != 0) {
            rcv_buf[(size_t)(message->payloadlen)] = '\0';
            * rcv_buf_old = "";
            strcpy(*rcv_buf_old, rcv_buf);
            mcs_mqtt_callback(rcv_buf);
        }
    }

}

void mcs_mqtt_init(void (*mcs_mqtt_callback)(char *)) {
    //static int arrivedcount = 0;
    Client c;   //MQTT client
    //MQTTMessage message;
    int rc = 0;

    char topic[50];
    int nvdm_topic_len = sizeof(topic);
    nvdm_read_data_item("mcs", "topic", (uint8_t *)topic, (uint32_t *)&nvdm_topic_len);

    char clientId[50];
    int nvdm_clientId_len = sizeof(clientId);
    nvdm_read_data_item("mcs", "clientId", (uint8_t *)clientId, (uint32_t *)&nvdm_clientId_len);

    char port[5];
    int nvdm_port_len = sizeof(port);
    nvdm_read_data_item("mcs", "port", (uint8_t *)port, (uint32_t *)&nvdm_port_len);

    // char qos_method[1] = {0};
    // int nvdm_qos_method_len = sizeof(qos_method);
    // nvdm_read_data_item("common", "qos", (uint8_t *)qos_method, (uint32_t *)&nvdm_qos_method_len);

    LOG_I(MCS, "topic: %s !", topic);
    LOG_I(MCS, "clientId: %s !", clientId);
    LOG_I(MCS, "port: %s !", port);
    // LOG_I(MCS, "qos: %s\n", qos_method);

    //arrivedcount = 0;

    unsigned char msg_buf[100];     //generate messages such as unsubscrube
    unsigned char msg_readbuf[100]; //receive messages such as unsubscrube ack

    Network n;  //TCP network
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

    //init mqtt network structure
    NewNetwork(&n);

    char host[5];
    int nvdm_host_len = sizeof(host);
    nvdm_read_data_item("mcs", "host", (uint8_t *)host, (uint32_t *)&nvdm_host_len);

    if (strcmp(host, "com") == 0) {
        rc = ConnectNetwork(&n, MQTT_HOST_COM, port);
    } else {
        rc = ConnectNetwork(&n, MQTT_HOST_CN, port);
    }

    if (rc != 0) {
        LOG_E(MCS, "TCP connect fail,status -%4X !", -rc);
        return true;
    }

    //init mqtt client structure
    MQTTClient(&c, &n, 12000, msg_buf, 100, msg_readbuf, 100);

    //mqtt connect req packet header
    data.willFlag = 0;
    data.MQTTVersion = 3;
    data.clientID.cstring = clientId;
    data.username.cstring = NULL;
    data.password.cstring = NULL;
    data.keepAliveInterval = 10;
    data.cleansession = 1;

    //send mqtt connect req to remote mqtt server
    rc = MQTTConnect(&c, &data);

    if (rc != 0) {
        LOG_E(MCS, "MQTT connect fail,status%d !", rc);
    }

    LOG_I(MCS, "Subscribing to %s !", topic);

    // if (strcmp(qos_method, "0") == 0) {
        rc = MQTTSubscribe(&c, topic, QOS0, mqttMessageArrived);
    // } else if (strcmp(qos_method, "1") == 0) {
    //     rc = MQTTSubscribe(&c, topic, QOS1, mqttMessageArrived);
    // } else if (strcmp(qos_method, "2") == 0) {
    //     rc = MQTTSubscribe(&c, topic, QOS2, mqttMessageArrived);
    // }

    LOG_I(MCS, "Client Subscribed %d !", rc);

    for(;;) {
        MQTTYield(&c, 1000);
    }
    return true;
}
#endif

int g_socket_id = -1;
char g_cmd_buf [50]= {0};
    /* timer */
void tcpTimerCallback( TimerHandle_t pxTimer )
{
  if ( g_socket_id >= 0 )
    lwip_write(g_socket_id, g_cmd_buf, sizeof(g_cmd_buf));
}

/* tcp connection */
int32_t mcs_tcp_init(void (*mcs_tcp_callback)(char *))
{
    //int s;
    //int c;
    int ret;
    struct sockaddr_in addr;
    int count = 0;
    int rcv_len, rlen;

    //int32_t mcs_ret = MCS_TCP_DISCONNECT;

    /* Setting the TCP ip */
    if (HTTPCLIENT_OK != getInitialTCPIP()) {
        return MCS_TCP_INIT_ERROR;
    }

    /* deviceId */
    //char deviceId[20] = {0};
    //int nvdm_deviceId_len = sizeof(deviceId);
    //nvdm_read_data_item("mcs", "deviceId", (uint8_t *)deviceId, (uint32_t *)&nvdm_deviceId_len);

    /* deviceKey */
    //char deviceKey[20] = {0};
    //int nvdm_deviceKey_len = sizeof(deviceKey);
    //nvdm_read_data_item("mcs", "deviceKey", (uint8_t *)deviceKey, (uint32_t *)&nvdm_deviceKey_len);

    /* command buffer */
    //char cmd_buf [50]= {0};
    strcat(g_cmd_buf, g_device_id);
    strcat(g_cmd_buf, ",");
    strcat(g_cmd_buf, g_device_key);
    strcat(g_cmd_buf, ",0");

mcs_tcp_connect:
    os_memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SOCK_TCP_SRV_PORT);
    addr.sin_addr.s_addr =inet_addr(TCP_ip);

    /* create the socket */
    g_socket_id = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (g_socket_id < 0) {
        //mcs_ret = MCS_TCP_SOCKET_INIT_ERROR;
        LOG_E(MCS, "tcp client create fail 0 !");
        goto idle;
    }

    ret = lwip_connect(g_socket_id, (struct sockaddr *)&addr, sizeof(addr));

    if (ret < 0) {
        lwip_close(g_socket_id);
        LOG_E(MCS, "tcp client connect fail 1 !");
        goto mcs_tcp_connect;
    }

    heartbeat_timer = xTimerCreate("TimerMain", (30*1000 / portTICK_RATE_MS), pdTRUE, (void *)0, tcpTimerCallback);
    xTimerStart( heartbeat_timer, 0 );

    for (;;) {
        char rcv_buf[MAX_STRING_SIZE] = {0};

        if (0 == count) {
            ret = lwip_write(g_socket_id, g_cmd_buf, sizeof(g_cmd_buf));
        }

        LOG_I(MCS, "MCS tcp-client waiting for data...");
        rcv_len = 0;
        rlen = lwip_recv(g_socket_id, &rcv_buf[rcv_len], sizeof(rcv_buf) - 1 - rcv_len, 0);
        rcv_len += rlen;

        if ( 0 == rcv_len ) {
            return MCS_TCP_DISCONNECT;
        }

        LOG_I(MCS, "MCS tcp-client received data:%s", rcv_buf);

        /* split the string of rcv_buffer */
        char split_buf[MAX_STRING_SIZE] = {0};
        strcpy(split_buf, rcv_buf);

        char *arr[7];
        char *del = ",";
        mcs_splitn(arr, split_buf, del, 7);
#ifdef MTK_FOTA_ENABLE           
        if (0 == strncmp (arr[3], "FOTA", 4)) {
            char *s = mcs_replace(arr[6], "https", "http");
            LOG_I(MCS, "fota url: %s\n", s);
            char data_buf [100] = {0};
            strcat(data_buf, "status");
            strcat(data_buf, ",,fotaing");
            mcs_upload_datapoint(data_buf);
            fota_download_by_http(s);
        } else {
#endif
          mcs_tcp_callback(rcv_buf);
#ifdef MTK_FOTA_ENABLE           
        }
#endif
        count ++;
    }

idle:
    LOG_I(MCS, "MCS tcp-client end");
    return MCS_TCP_DISCONNECT;
}

/* 0 : off, 1: on */
int get_gpio33_led(void)
{
    hal_gpio_data_t data_up_down = HAL_GPIO_DATA_LOW;
    hal_gpio_status_t ret;
    ret = hal_gpio_init(HAL_GPIO_33);

    if (HAL_GPIO_STATUS_OK != ret) {
        LOG_E(MCS, "hal_gpio_init failed !");
        hal_gpio_deinit(HAL_GPIO_33);
        return 0;
    }

    ret = hal_gpio_get_input(HAL_GPIO_33, &data_up_down);
    if (HAL_GPIO_STATUS_OK != ret) {
        LOG_E(MCS, "hal_gpio_get_input failed !");
        hal_gpio_deinit(HAL_GPIO_33);
        return 0;
    }

    hal_gpio_deinit(HAL_GPIO_33);

    if (data_up_down == HAL_GPIO_DATA_HIGH)//on
        return 1;
    else
        return 0;
}
    
void set_gpio33_led(int on_off)
{
    hal_gpio_data_t data_pull_up;
    hal_gpio_data_t data_pull_down;
    hal_gpio_status_t ret;
    hal_pinmux_status_t ret_pinmux_status;

    ret = hal_gpio_init(HAL_GPIO_33);
    if (HAL_GPIO_STATUS_OK != ret) {
        LOG_E(MCS, "hal_gpio_init failed !");
        hal_gpio_deinit(HAL_GPIO_33);
        return;
    }

    /* Set pin as GPIO mode.*/
    ret_pinmux_status = hal_pinmux_set_function(HAL_GPIO_33, HAL_GPIO_33_GPIO33);
    if (HAL_PINMUX_STATUS_OK != ret_pinmux_status) {
        LOG_E(MCS, "hal_pinmux_set_function failed !");
        hal_gpio_deinit(HAL_GPIO_33);
        return;
    }

    /* Set GPIO as input.*/
    ret = hal_gpio_set_direction(HAL_GPIO_33, HAL_GPIO_DIRECTION_INPUT);
    if (HAL_GPIO_STATUS_OK != ret) {
        LOG_E(MCS, "hal_gpio_set_direction failed !");
        hal_gpio_deinit(HAL_GPIO_33);
        return;
    }


    if ( on_off == 1 )
    {
        /* Configure the pull state to pull-up. */
        ret = hal_gpio_pull_up(HAL_GPIO_33);
        if (HAL_GPIO_STATUS_OK != ret) {
            LOG_E(MCS, "hal_gpio_pull_up failed !");
            hal_gpio_deinit(HAL_GPIO_33);
            return;
        }

        /* Read the input data of the pin for further validation.*/
        ret = hal_gpio_get_input(HAL_GPIO_33, &data_pull_up);
        if (HAL_GPIO_STATUS_OK != ret) {
            LOG_E(MCS, "hal_gpio_get_input failed !");
            hal_gpio_deinit(HAL_GPIO_33);
            return;
        }
    }
    else
    {
        /* Configure the pull state to pull-down.*/
        ret = hal_gpio_pull_down(HAL_GPIO_33);
        if (HAL_GPIO_STATUS_OK != ret) {
            LOG_E(MCS, "hal_gpio_pull_down failed !");
            hal_gpio_deinit(HAL_GPIO_33);
            return;
        }

        /* Read the input data of the pin for further validation.*/
        ret = hal_gpio_get_input(HAL_GPIO_33, &data_pull_down);
        if (HAL_GPIO_STATUS_OK != ret) {
            LOG_E(MCS, "hal_gpio_get_input failed !");
            hal_gpio_deinit(HAL_GPIO_33);
            return;
        }
    }
    
    /* Verify whether the configuration of pull state is susccessful.*/
    /*if ((data_pull_down == HAL_GPIO_DATA_LOW) && (data_pull_up == HAL_GPIO_DATA_HIGH)) {
        printf("GPIO pull state configuration is successful\r\n");
    } else {
        printf("GPIO pull state configuration failed\r\n");
    }*/

    ret = hal_gpio_deinit(HAL_GPIO_33);
    if (HAL_GPIO_STATUS_OK != ret) {
        LOG_E(MCS, "hal_gpio_deinit failed !");
        return;
    }
}

void mcs_set_gpio33_led(int on_off)
{
    set_gpio33_led(on_off);
}

int mcs_get_gpio33_led(void)
{
    return get_gpio33_led();
}

/*
1 : led status
2 : ble status
3 : ble scan table
*/
void mcs_update(int status_id, int onoff, char* displaystr)
{
    if ( is_lwip_net_ready() == 0 )
        return; //ip is not ready
        
    char statusbuf[1024] = {0};
    switch(status_id)
    {
        case 1://led status
        {
            strcat(statusbuf, g_id_status_led);
            strcat(statusbuf,",,");
            if ( onoff == 1 )
                strcat(statusbuf,"1");
            else
                strcat(statusbuf,"0");
            strcat(statusbuf,"\n");           
        }break;

        case 2://ble status
        {
            strcat(statusbuf, g_id_status_ble);
            strcat(statusbuf,",,");
            if ( onoff == 1 )
                strcat(statusbuf,"1");
            else
                strcat(statusbuf,"0");
            strcat(statusbuf,"\n");           
        }break;

        case 3://scan table
        //just update string to cloud by using g_id_ble_scan_tab
        break;

        default:
        break;
    }

    //update string to string list 
    strcat(statusbuf,g_id_ble_scan_tab);
    strcat(statusbuf,",,");
    strcat(statusbuf,displaystr);
    
    mcs_upload_datapoint(statusbuf);
}

void eint_irq_handle(void *user_data)
{
    static uint32_t btn_down_count = 0;
    static uint32_t btn_up_count = 0;
    static uint32_t press_step = 0;

    press_step++;
    if(press_step%2 != 0) {
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &btn_down_count);
    }
    else {
        portBASE_TYPE time_monitor_wake = pdFALSE;
        uint32_t press_period;

        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &btn_up_count);
        if(btn_up_count > btn_down_count) {
            press_period = (uint32_t)(((uint64_t)(btn_up_count - btn_down_count)*1000) / 32768);
        }
        else {
            press_period = (uint32_t)((((uint64_t)(btn_up_count + (0xFFFFFFFF - btn_down_count))) * 1000) / 32768);
        }

        if(!g_supplicant_ready)
            return;

        xQueueSendToBackFromISR(g_btn_queue, &press_period, &time_monitor_wake);
    }
}

void press_time_monitor(void *args)
{
    for(;;) {
        uint32_t press_period;
        xQueueReceive(g_btn_queue, (void *)&press_period, portMAX_DELAY);

        if(press_period < 1000) {
            // <1000ms reboot and aws start
            LOG_I(MCS, "Button period:%ud ----> Turn on/off GPIO 33 LED <----", (unsigned int)press_period);
            int led_status = get_gpio33_led();
            if ( led_status )//on
            {
                set_gpio33_led(0);
                mcs_update(1,0,"Turn off GPIO 33 LED !");
            }
            else
            {
                set_gpio33_led(1);
                mcs_update(1,1,"Turn on GPIO 33 LED !");
            }
            
        } else {
            // >=1000ms only factory reset alink
            /* Does not use this, it just registers wifi callback event function on BT smart connection.
                       We need connection event and ip ready so we do not call this init function */
            //ble_smtcn_stop_adv();
            ble_smtcn_init2();
            ble_smtcn_set_adv();
            LOG_I(MCS, "Button period:%ud ----> Start BLE smart connection <----", (unsigned int)press_period);            
        }
    }
}

void register_action_btn_proc(void)
{
    hal_eint_number_t irq_num = HAL_EINT_NUMBER_0;
    hal_eint_config_t eint_config;

    LOG_I(MCS, "Register BLE and LED button");

    eint_config.debounce_time = 5;
    eint_config.trigger_mode = HAL_EINT_EDGE_FALLING_AND_RISING;

    hal_eint_init(irq_num, &eint_config);
    hal_eint_register_callback(irq_num, eint_irq_handle, NULL);

    g_btn_queue = xQueueCreate(BTN_QUEUE_LEN, sizeof(uint32_t));
    if(g_btn_queue == NULL)
        return;

    if(xTaskCreate(press_time_monitor, "press_time_monitor", 4096/sizeof(portSTACK_TYPE), NULL, 3, NULL) != pdPASS){
        vQueueDelete(g_btn_queue);
        g_btn_queue = NULL;
        return;
    }
}

#ifdef MTK_BLE_GPIO_SERVICE
#include "bt_type.h"
extern bt_handle_t g_bt_conn_handle;
#endif
void mcs_tcp_callback(char *rcv_buf) {
    char *arr[7];
    char *del = ",";

    mcs_split(arr, rcv_buf, del);
    if (0 == strncmp (arr[3], g_id_ctrl_led, strlen(g_id_ctrl_led))) {
        if (0 == strncmp (arr[4], "1", 1))
        {
            LOG_I(MCS, "Turn on GPIO 33 LED !");
#ifdef MTK_BLE_GPIO_SERVICE
            if (g_bt_conn_handle)
            {
                char op_buf[32] = {0};
                char bt_handle_id[8] = {0};
                os_snprintf(bt_handle_id, sizeof(bt_handle_id),"%04x", g_bt_conn_handle);
                strcat(op_buf,"gpio client w ");
                strcat(op_buf,bt_handle_id);                
                strcat(op_buf," ");                    
                strcat(op_buf,"0703 ON");
                LOG_I(MCS, "BT operation : %s", op_buf);
                bt_app_io_callback(op_buf, NULL);
            }
#else
            set_gpio33_led(1);
            mcs_update(1,1,"Turn on GPIO 33 LED !");
#endif
        }
        else
        {
            LOG_I(MCS, "Turn off GPIO 33 LED !");
#ifdef MTK_BLE_GPIO_SERVICE
        if (g_bt_conn_handle)
        {
            char op_buf[32] = {0};
            char bt_handle_id[8] = {0};
            os_snprintf(bt_handle_id, sizeof(bt_handle_id),"%04x", g_bt_conn_handle);
            strcat(op_buf,"gpio client w ");
            strcat(op_buf,bt_handle_id);                
            strcat(op_buf," ");                    
            strcat(op_buf,"0703 OFF");
            LOG_I(MCS, "BT operation : %s", op_buf);
            bt_app_io_callback(op_buf, NULL);
        }
#else
            set_gpio33_led(0);
            mcs_update(1,0,"Turn off GPIO 33 LED !");
#endif      
        }          
    }
    else if (0 == strncmp (arr[3], g_id_ctrl_ble, strlen(g_id_ctrl_ble))) {
        if (0 == strncmp (arr[4], "1", 1))
        {
            LOG_I(MCS, "Connect to BLE !");
#ifdef MTK_BLE_GPIO_SERVICE            
            bt_app_io_callback("gpio client g", NULL);
            bt_app_io_callback("gpio client c 1 AAAAAAAAAAAA", NULL);
#else
            mcs_update(2,1,"Connect to BLE !");
#endif
        }
        else
        {
            LOG_I(MCS, "Disconnect to BLE !");
#ifdef MTK_BLE_GPIO_SERVICE
            char op_buf[32] = {0};
            char bt_handle_id[8] = {0};
            os_snprintf(bt_handle_id, sizeof(bt_handle_id),"%04x", g_bt_conn_handle);
            strcat(op_buf,"gpio client d ");
            strcat(op_buf,bt_handle_id);
            LOG_I(MCS, "BT operation : %s", op_buf);
            bt_app_io_callback(op_buf, NULL);
#else
            mcs_update(2,0,"Disconnect to BLE !");
#endif
        }          
    }
    else if (0 == strncmp (arr[3], g_id_ctrl_scan, strlen(g_id_ctrl_scan))) {
        if (0 == strncmp (arr[4], "1", 1))
        {
            LOG_I(MCS, "Start to scan !");
#ifdef MTK_BLE_GPIO_SERVICE            
            bt_app_io_callback("gpio client s", NULL);
#endif
            mcs_update(3,1,"Start to scan !");//just update string to list
        }
        else
        {
            LOG_I(MCS, "Stop to scan !");
#ifdef MTK_BLE_GPIO_SERVICE            
            bt_app_io_callback("gpio client e", NULL);
#endif 
            mcs_update(3,0,"Stop to scan !");//just update string to list
        }          
    }

#if 0
    // Dln7lL0G,zLfxhiabFnCEZZJc,1459307476444,encodeByMD5,test
    if (0 == strncmp (arr[3], ENCODE_MD5_CHANNEL, strlen(ENCODE_MD5_CHANNEL))) {
        /* encode BY MD5 */
        uint8_t digest[HAL_MD5_DIGEST_SIZE] = {0};
        printf("User give: %s \n", arr[4]);
        hal_md5_context_t context = {0};
        hal_md5_init(&context);
        hal_md5_append(&context, arr[4], strlen(arr[4]));
        hal_md5_end(&context, digest);

        uint8_t i;
        char str_buffer [50] = {0};
        strcpy(str_buffer, "");
        for (i = 0; i < sizeof(digest); i++) {
          if (i % 16 == 0) {
              printf("\r\n");
          }
          char buffer [2];
          sprintf (buffer, "%02x", digest[i]);
          strcat(str_buffer, buffer);
        }

        /* send to MCS */
        char data_buf [MAX_DATA_SIZE] = {0};
        strcat(data_buf, DECODE_MD5_CHANNEL);
        strcat(data_buf, ",,");
        strcat(data_buf, str_buffer);
        mcs_upload_datapoint(data_buf);
    }
#endif
}

void mcs_setting_print(void)
{
    LOG_I(MCS, "********** mcs setting **********");
    LOG_I(MCS, "Device ID         : %s", g_device_id);
    LOG_I(MCS, "Device Key        : %s", g_device_key);
    LOG_I(MCS, "Control LED ID    : %s", g_id_ctrl_led);
    LOG_I(MCS, "LED Status ID     : %s", g_id_status_led);
    LOG_I(MCS, "Control BLE ID    : %s", g_id_ctrl_ble);
    LOG_I(MCS, "BLE Status ID     : %s", g_id_status_ble);
    LOG_I(MCS, "Control Scan ID   : %s", g_id_ctrl_scan);
    LOG_I(MCS, "BLE Scan Table ID : %s", g_id_ble_scan_tab);
    LOG_I(MCS, "************** End **************");
}
void mcs_initial_task(void * arg) {
  //check connection and ip is ready then go
  lwip_net_ready();
  LOG_I(MCS, "mcs_initial_task : get ip ready !");
  mcs_tcp_init(mcs_tcp_callback);
  vTaskDelete(NULL);
}

void mcs_nvdm_initial(void)
{
    int nvdm_deviceKey_len = sizeof(g_device_key);
    int nvdm_deviceId_len = sizeof(g_device_id);
    int nvdm_host_len = sizeof(g_host);
    int nvdm_ctrlLed_len = sizeof(g_id_ctrl_led);
    int nvdm_statusLed_len = sizeof(g_id_status_led);
    int nvdm_ctrlBle_len = sizeof(g_id_ctrl_ble);
    int nvdm_statusBle_len = sizeof(g_id_status_ble);
    int nvdm_ctrlscan_len = sizeof(g_id_ctrl_scan);
    int nvdm_ble_scan_tab_len = sizeof(g_id_ble_scan_tab);

    nvdm_status_t isfound = NVDM_STATUS_OK;
    //deviceID
    isfound = nvdm_read_data_item("mcs", "deviceId", (uint8_t *)g_device_id, (uint32_t *)&nvdm_deviceId_len);
    if ( isfound != NVDM_STATUS_OK )
    {
        LOG_I(MCS, "cannot find item name **deviceId** in nvdm, write it first !\n");
        nvdm_write_data_item("mcs", "deviceId", NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)DEVICE_ID, sizeof(DEVICE_ID));
        strcpy(g_device_id, DEVICE_ID);
    }

    //deviceKey
    isfound = nvdm_read_data_item("mcs", "deviceKey", (uint8_t *)g_device_key, (uint32_t *)&nvdm_deviceKey_len);
    if ( isfound != NVDM_STATUS_OK )
    {
        LOG_I(MCS, "cannot find item name **deviceKey** in nvdm, write it first !\n");
        nvdm_write_data_item("mcs", "deviceKey", NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)DEVICE_KEY, sizeof(DEVICE_KEY));
        strcpy(g_device_key, DEVICE_KEY);        
    }

    //host
    isfound = nvdm_read_data_item("mcs", "host", (uint8_t *)g_host, (uint32_t *)&nvdm_host_len);
    if ( isfound != NVDM_STATUS_OK )
    {
        LOG_I(MCS, "cannot find item name **host** in nvdm, write it first !\n");
        nvdm_write_data_item("mcs", "host", NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)HOST, sizeof(HOST));
        strcpy(g_host, HOST);
    }

    //led control
    isfound = nvdm_read_data_item("mcs", "id_led", (uint8_t *)g_id_ctrl_led, (uint32_t *)&nvdm_ctrlLed_len);
    if ( isfound != NVDM_STATUS_OK )
    {
        LOG_I(MCS, "cannot find item name **id_led** in nvdm, write it first !\n");
        nvdm_write_data_item("mcs", "id_led", NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)CTRL_LED, sizeof(CTRL_LED));
        strcpy(g_id_ctrl_led, CTRL_LED);
    }

    //led status
    isfound = nvdm_read_data_item("mcs", "id_status_led", (uint8_t *)g_id_status_led, (uint32_t *)&nvdm_statusLed_len);
    if ( isfound != NVDM_STATUS_OK )
    {
        LOG_I(MCS, "cannot find item name **id_status_led** in nvdm, write it first !\n");
        nvdm_write_data_item("mcs", "id_status_led", NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)STATUS_LED, sizeof(STATUS_LED));
        strcpy(g_id_status_led, STATUS_LED);
    }

    //ble control
    isfound = nvdm_read_data_item("mcs", "id_ble", (uint8_t *)g_id_ctrl_ble, (uint32_t *)&nvdm_ctrlBle_len);
    if ( isfound != NVDM_STATUS_OK )
    {
        LOG_I(MCS, "cannot find item name **id_ble** in nvdm, write it first !\n");
        nvdm_write_data_item("mcs", "id_ble", NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)CTRL_BLE, sizeof(CTRL_BLE));
        strcpy(g_id_ctrl_ble, CTRL_BLE);
    }

    //ble status
    isfound = nvdm_read_data_item("mcs", "id_status_ble", (uint8_t *)g_id_status_ble, (uint32_t *)&nvdm_statusBle_len);
    if ( isfound != NVDM_STATUS_OK )
    {
        LOG_I(MCS, "cannot find item name **id_status_ble** in nvdm, write it first !\n");
        nvdm_write_data_item("mcs", "id_status_ble", NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)STATUS_BLE, sizeof(STATUS_BLE));
        strcpy(g_id_status_ble, STATUS_BLE);
    }

    //ble scan control
    isfound = nvdm_read_data_item("mcs", "id_scan", (uint8_t *)g_id_ctrl_scan, (uint32_t *)&nvdm_ctrlscan_len);
    if ( isfound != NVDM_STATUS_OK )
    {
        LOG_I(MCS, "cannot find item name **ble_scan_table** in nvdm, write it first !\n");
        nvdm_write_data_item("mcs", "id_scan", NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)CTRL_SCAN, sizeof(CTRL_SCAN));
        strcpy(g_id_ctrl_scan, CTRL_SCAN);
    }

   
    //ble scan table id
    isfound = nvdm_read_data_item("mcs", "ble_scan_table", (uint8_t *)g_id_ble_scan_tab, (uint32_t *)&nvdm_ble_scan_tab_len);
    if ( isfound != NVDM_STATUS_OK )
    {
        LOG_I(MCS, "cannot find item name **ble_scan_table** in nvdm, write it first !\n");
        nvdm_write_data_item("mcs", "ble_scan_table", NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)BLE_SCAN_TAB, sizeof(BLE_SCAN_TAB));
        strcpy(g_id_ble_scan_tab, BLE_SCAN_TAB);
    }
    mcs_setting_print();
}

void mcs_init(void)
{
    register_action_btn_proc();
    mcs_nvdm_initial();
    xTaskCreate(mcs_initial_task, "MCS initial", 2048, NULL, 4, NULL);
}

#ifdef MTK_BLE_GPIO_SERVICE
QueueHandle_t g_mcs_status_xQueue = NULL;
void mcs_update_task(void * arg) 
{
    int mcs_data;
    while (1) {
        LOG_I(MCS, "123Update BT device status to MCS");
        #if 1
        if (xQueueReceive(g_mcs_status_xQueue, &mcs_data, portMAX_DELAY)) {
            if (mcs_data)
                mcs_update(1,1,"LED is on !");
            else
                mcs_update(1,0,"LED is off !");
        }
        #endif
    }

}

void mcs_status_updata_init(void)
{
    /* Create a queue capable of containing 2 unsigned long values. */
    g_mcs_status_xQueue = xQueueCreate(2, sizeof(int));
    xTaskCreate(mcs_update_task, "mcs status update", 2048, NULL, 4, NULL);
}
#endif

