#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "MQTTClient.h"
//#include "mqtt.h"
#include "os.h"
#include "mcs.h"

#define MQTT_HOST_COM "mqtt.mcs.mediatek.com"
#define MQTT_HOST_CN "mqtt.mcs.mediatek.cn"

#include "hal_sys.h"
#include "fota.h"
#include "fota_config.h"

#define MIN(a,b) ((a) < (b) ? a : b)

Client c;
char *topic_buf [20] = {0};
char *value_buf [100] = {0};

void mqttMessageArrived(MessageData *md)
{
    char rcv_buf_old[200] = {0};
    char rcv_buf[200] = {0};

    MQTTMessage *message = md->message;

    const size_t write_len = MIN((size_t)(message->payloadlen), 200 - 1);
    strncpy(rcv_buf, message->payload, write_len);
    rcv_buf[write_len] = 0;
    //printf("rcv1: %s\n", rcv_buf);

    char split_buf[MCS_MAX_STRING_SIZE] = {0};
    strncpy(split_buf, rcv_buf, MCS_MAX_STRING_SIZE);

    char *arr[5];
    char *del = ",";
    mcs_splitn(arr, split_buf, del, 5);

    if (0 == strncmp (arr[1], "FOTA", 4)) {
        char *s = mcs_replace(arr[4], "https", "http");
        fota_download_by_http(s);
        fota_ret_t err;
        err = fota_trigger_update();
        if (0 == err){
            hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
            return 0;
        } else {
            return -1;
        }
    } else {
        if (strcmp(rcv_buf_old, rcv_buf) != 0) {
            * rcv_buf_old = "";
            strncpy(*rcv_buf_old, rcv_buf, 200);
            mcs_mqtt_callback(rcv_buf);
        }
    }

}

void mcs_mqtt_upload_datapoint(char* channel, char *value)
{
    MQTTMessage message1;
    message1.qos = QOS0;
    message1.retained = false;
    message1.dup = false;

    topic_buf[0] = '\0';
    value_buf[0] = '\0';

    // char topic_buf [20] = {0};
    strcat(topic_buf, "mcs/");
    strcat(topic_buf, DEVICEID);
    strcat(topic_buf, "/");
    strcat(topic_buf, DEVICEKEY);
    strcat(topic_buf, "/");
    strcat(topic_buf, channel);

    strcat(value_buf, ",");
    strcat(value_buf, channel);
    strcat(value_buf, ",");
    strcat(value_buf, value);

    message1.payload = value_buf;
    message1.payloadlen = strlen(value_buf) + 1;

    return MQTTPublish(&c, topic_buf, &message1);
}

void mcs_mqtt_init(void (*mcs_mqtt_callback)(char *))
{
    Network n;  //TCP network
    int rc = 0;

    unsigned char msg_buf[100];     //generate messages such as unsubscrube
    unsigned char msg_readbuf[100]; //receive messages such as unsubscrube ack

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

    //init mqtt network structure
    NewNetwork(&n);

    if (strcmp(HOST, "com") == 0) {
        rc = ConnectNetwork(&n, MQTT_HOST_COM, PORT);
    } else {
        rc = ConnectNetwork(&n, MQTT_HOST_CN, PORT);
    }

    if (rc != 0) {
        //printf("TCP connect fail,status -%4X\n", -rc);
        return true;
    }

    //init mqtt client structure
    MQTTClient(&c, &n, 12000, msg_buf, 100, msg_readbuf, 100);

    //mqtt connect req packet header
    data.willFlag = 0;
    data.MQTTVersion = 3;
    data.clientID.cstring = CLIENTID;
    data.username.cstring = NULL;
    data.password.cstring = NULL;
    data.keepAliveInterval = 10;
    data.cleansession = 1;

    //send mqtt connect req to remote mqtt server
    rc = MQTTConnect(&c, &data);

    if (rc != 0) {
        //printf("MQTT connect fail,status%d\n", rc);
    }

    char *device_topic_buf [20] = {0};
    strcat(device_topic_buf, "mcs/");
    strcat(device_topic_buf, DEVICEID);
    strcat(device_topic_buf, "/");
    strcat(device_topic_buf, DEVICEKEY);
    strcat(device_topic_buf, "/+");

    rc = MQTTSubscribe(&c, device_topic_buf, QOS0, mqttMessageArrived);
    //printf("rc(%d):", rc);
    for(;;) {
        MQTTYield(&c, 1000);
    }

    return true;
}