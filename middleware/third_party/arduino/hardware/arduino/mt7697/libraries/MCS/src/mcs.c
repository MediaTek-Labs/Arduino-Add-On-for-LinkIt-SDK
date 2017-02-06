#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "httpclient.h"
#include "mcs.h"

/* RESTful config */
#define BUF_SIZE   (1024 * 1)
/* Now only .com , must do for china */
#define HTTPS_MTK_CLOUD_URL_COM "http://api.mediatek.com/mcs/v2/devices/"
#define HTTPS_MTK_CLOUD_URL_CN "http://api.mediatek.cn/mcs/v2/devices/"

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
  char *ch;
  if (!(ch = strstr(st, orig)))
   return st;
  strncpy(buffer, st, ch-st);
  buffer[ch-st] = 0;
  sprintf(buffer+(ch-st), "%s%s", repl, ch+strlen(orig));
  return buffer;
}

void mcs_upload_datapoint(char *value)
{
    /* upload mcs datapoint */
    int ret = HTTPCLIENT_ERROR_CONN;
    httpclient_t client = {0};
    char *buf = NULL;

    httpclient_data_t client_data = {0};
    char *content_type = "text/csv";
    // char post_data[32];

    /* Set post_url */
    char post_url[70] ={0};

    if (strcmp(HOST, "com") == 0) {
        strcat(post_url, HTTPS_MTK_CLOUD_URL_COM);
    } else {
        strcat(post_url, HTTPS_MTK_CLOUD_URL_CN);
    }

    strcat(post_url, DEVICEID);
    strcat(post_url, "/datapoints.csv");

    /* Set header */
    char header[40] = {0};
    strcat(header, "deviceKey:");
    strcat(header, DEVICEKEY);
    strcat(header, "\r\n");

    printf("header: %s\n", header);
    printf("url: %s\n", post_url);
    printf("data: %s\n", value);

    buf = pvPortMalloc(BUF_SIZE);
    if (buf == NULL) {
        printf("buf malloc failed.\r\n");
        return ret;
    }
    buf[0] = '\0';
    ret = httpclient_connect(&client, post_url);

    client_data.response_buf = buf;
    client_data.response_buf_len = BUF_SIZE;
    client_data.post_content_type = content_type;
    // sprintf(post_data, data);
    client_data.post_buf = value;
    client_data.post_buf_len = strlen(value);
    httpclient_set_custom_header(&client, header);
    // ret = httpclient_post(&client, post_url, &client_data);
    ret = httpclient_send_request(&client, post_url, HTTPCLIENT_POST, &client_data);
    if (ret < 0) {
        return ret;
    }
    // if (200 != httpclient_get_response_code(&client)) {
    //     return -1;
    // }
    ret = httpclient_recv_response(&client, &client_data);
    if (ret < 0) {
        return ret;
    }
    vPortFree(buf);
    httpclient_close(&client);
    return ret;
}