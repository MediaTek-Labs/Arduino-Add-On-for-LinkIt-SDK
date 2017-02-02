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

#include "ut_app.h"
#include "bt_uuid.h"
#include "bt_lwip.h"
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "syslog.h"

bt_status_t bt_app_gattc_io_callback(void *input, void *output);
bt_status_t bt_app_gatts_io_callback(void *input, void *output);
const bt_gatts_service_t** bqb_get_gatt_server(void);
bt_status_t bt_gatt_service_execute_write(uint16_t handle, uint8_t flag);

// Weak symbol declaration
#if _MSC_VER >= 1500
    #pragma comment(linker, "/alternatename:_bqb_get_gatt_server=_default_bqb_get_gatt_server")
    #pragma comment(linker, "/alternatename:_bt_app_gattc_io_callback=_default_bt_app_gattc_io_callback")
    #pragma comment(linker, "/alternatename:_bt_app_gatts_io_callback=_default_bt_app_gatts_io_callback")
    #pragma comment(linker, "/alternatename:_bt_gatt_service_execute_write=_default_bt_gatt_service_execute_write")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
    #pragma weak bqb_get_gatt_server = default_bqb_get_gatt_server
    #pragma weak bt_app_gattc_io_callback = default_bt_app_gattc_io_callback
    #pragma weak bt_app_gatts_io_callback = default_bt_app_gatts_io_callback
    #pragma weak bt_gatt_service_execute_write = default_bt_gatt_service_execute_write
#else
    #error "Unsupported Platform"
#endif

const bt_gatts_service_t** default_bqb_get_gatt_server()
{
    return NULL;
}

bt_status_t default_bt_app_gattc_io_callback(void *input, void *output)
{
   return BT_STATUS_SUCCESS;
}

bt_status_t default_bt_app_gatts_io_callback(void *input, void *output)
{
   return BT_STATUS_SUCCESS;
}

bt_status_t default_bt_gatt_service_execute_write(uint16_t handle, uint8_t flag)
{
   return BT_STATUS_SUCCESS;
}


typedef struct _list_entry
{
    struct _list_entry *prev;
    struct _list_entry *next;
} list_entry;

typedef struct _gatt_service
{
    uint16_t start_handle;
    uint16_t end_handle;
    bt_uuid_t uuid;
} gatt_service;

typedef struct _gatt_include_service
{
    uint16_t included_service_handle;
    uint16_t end_group_handle;
    bt_uuid_t uuid;
    uint16_t handle;
} gatt_include_service;

typedef struct _gatt_char
{
    uint16_t handle;
    uint8_t  properties;
    uint16_t value_handle;
    bt_uuid_t charc_uuid;
} gatt_char;

typedef struct _gatt_desc
{
    uint16_t handle;
    bt_uuid_t desc_uuid;
} gatt_desc;

typedef struct _gatt_service_node
{
    list_entry node;
    gatt_service service;
    list_entry include_list;	// gatt_include_service_node is defined for each node
    list_entry char_list;	    // gatt_char_node is defined for each node
} gatt_service_node;

typedef struct _gatt_include_service_node
{
    list_entry  node;
    gatt_include_service include_service;
} gatt_include_service_node;

typedef struct _gatt_char_node
{
    list_entry node;
    gatt_char  characteristic;
    list_entry char_desc_list;     // gatt_desc_node is defined for each node
} gatt_char_node;

typedef struct _gatt_desc_node
{
    list_entry node;
    gatt_desc  descriptor;
} gatt_desc_node;

#define TEST_DISCOVER_ALL_PRIMARY_SERVICES      0x0001
#define TEST_DISCOVER_ALL_INCLUDE_SERVICES_STEP 0x0002
#define TEST_DISCOVER_ALL_CHAR_STEP             0x0004
#define TEST_DISCOVER_ALL_DESC_STEP             0x0008
#define TEST_READ_CHAR_DESCRIPTOR               0x0010
#define TEST_WRITE_CHAR_DESCRIPTOR              0x0020

static uint16_t test_conn_id;
#ifdef BLE_THROUGHPUT
static uint32_t test_index;
static uint32_t test_show_interval;
static uint32_t test_next_show;
#endif
static uint32_t test_start_time;
static uint32_t test_finish_time;
static uint8_t  test_started;
static uint32_t cur_tick;
static uint16_t mtu = 23;
uint16_t conn_interval = 1;
//static uint16_t expected_mtu = 244;    // This is for BLE4.2
static uint8_t test_data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13 };

static uint16_t conn_id;
static list_entry g_service_list;
static uint16_t gatt_test_flag = 0;
static uint16_t srv_count = 0;
static uint16_t cur_srv_index = 0;
#define MAX_SERVICE_NO (50)
static gatt_service service_table[MAX_SERVICE_NO];
static gatt_service_node *cur_srv_node = NULL;
static gatt_char_node *cur_char_node = NULL;

static gatt_service_node *find_service_node_by_handle_range(list_entry *service_list, uint16_t handle)
{
    gatt_service_node *service_node;

    service_node = (gatt_service_node *)(service_list->next);
    while ((list_entry *)service_node != service_list)
    {
        if ((handle >= service_node->service.start_handle) &&
            (handle <= service_node->service.end_handle))
        {
            return service_node;
        }
        /* try the next node in the list */
        service_node = (gatt_service_node *)(service_node->node.next);
    }
    return NULL;
}

static gatt_char_node *find_char_node_by_handle_range(gatt_service_node *service_node, uint16_t handle)
{
    gatt_char_node *char_node;
    gatt_char_node *char_next_node;

    char_node = (gatt_char_node *)(service_node->char_list.next);
    while ((list_entry *)char_node != &service_node->char_list)
    {
        char_next_node = (gatt_char_node *)(char_node->node.next);

        if (((list_entry *)char_next_node == &service_node->char_list) &&
            (handle >= char_node->characteristic.handle) &&
            (handle <= service_node->service.end_handle))
        {
            return char_node;
        }

        if ((handle >= char_node->characteristic.handle) &&
            (handle < char_next_node->characteristic.handle))
        {
            return char_node;
        }
        /* try the next node in the list */
        char_node = char_next_node;
    }
    return NULL;
}

static void *gatt_malloc(unsigned int size)
{
    return pvPortMalloc(size);
}

static void gatt_free(void *pv)
{
    vPortFree(pv);
}

void initialize_list_head(list_entry *list)
{
    list->prev = list;
    list->next = list;
}

void initialize_list_entry(list_entry *list)
{
    list->prev = 0;
    list->next = 0;
}

uint8_t is_list_empty(list_entry *list)
{
    return ((list)->next == (list));
}
#define is_node_connected(n) (((n)->next->prev == (n)) && ((n)->prev->next == (n)))

uint8_t is_list_circular(list_entry *list)
{
    list_entry *tmp = list;
    if (!is_node_connected(list)) {
        return 0;
    }
    for (tmp = tmp->next; tmp != list; tmp = tmp->next) {
        if (!is_node_connected(tmp)) {
            return 0;
        }
    }
    return 1;
}

uint8_t is_node_on_list(list_entry *head, list_entry *node)
{
    list_entry *tmp;
    assert(is_list_circular(head));
    tmp = head->next;
    while (tmp != head) {
        if (tmp == node) {
            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}

void insert_tail_list(list_entry *head, list_entry *entry)
{
    assert(is_list_circular(head));
    entry->next = head;
    entry->prev = head->prev;
    head->prev->next = entry;
    head->prev = entry;
    assert(is_node_connected(entry));
    assert(is_list_circular(head));
}
/*  Remove the first entry on the list specified by head. */
list_entry *remove_head(list_entry *head)
{
    list_entry *first;
    assert(is_list_circular(head));
    first = head->next;
    first->next->prev = head;
    head->next = first->next;
    assert(is_list_circular(head));
    return (first);
}

/* Remove the given entry from the list. */
void remove_entry(list_entry *entry)
{
    assert(is_list_circular(entry));
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    assert(is_list_circular(entry->prev));
    initialize_list_entry(entry);
}

static void dump_uuid128(bt_uuid_t *uuid128)
{
    uint8_t buf[34] = { 0 };
    int i, j;
    for (j = 15, i = 0; j >= 0; j--, i += 2) {
        snprintf((char *)(buf + i), 3, "%02x", uuid128->uuid[j]);
    }
    LOG_I(common, "[GATT]uuid = %s", buf);
}

static uint8_t tmp_uuid_buf[34] = { 0 };
static uint8_t*  get_uuid128(bt_uuid_t *uuid128)
{
    int i, j;
    memset(tmp_uuid_buf, 0, 34);
    for (j = 15, i = 0; j >= 0; j--, i += 2) {
        snprintf((char *)(tmp_uuid_buf + i), 3, "%02x", uuid128->uuid[j]);
    }
    return tmp_uuid_buf;
}

static void print_include_service(gatt_include_service *inc_srv)
{
    if (!inc_srv) return;
    LOG_I(common, "  [GATT]Include Service Handle = 0x%04x", inc_srv->handle);
    if (bt_uuid_is_uuid16(&inc_srv->uuid)) {
        LOG_I(common, "    [GATT]Start Handle = 0x%04x End Handle = 0x%04x uuid = %04x", inc_srv->included_service_handle, inc_srv->end_group_handle, inc_srv->uuid.uuid16);
    } else {
        LOG_I(common, "    [GATT]Start Handle = 0x%04x End Handle = 0x%04x", inc_srv->included_service_handle, inc_srv->end_group_handle);
        LOG_I(common, "      [GATT]uuid = %s", get_uuid128(&inc_srv->uuid));
    }
}

static void print_all_include_service(list_entry *include_list)
{
    gatt_include_service_node *inc_srv_node;
    inc_srv_node = (gatt_include_service_node *)include_list->next;
    while ((NULL != inc_srv_node) && ((list_entry *)inc_srv_node != include_list)) {
        print_include_service(&inc_srv_node->include_service);
        inc_srv_node = (gatt_include_service_node *)inc_srv_node->node.next;
    }
}

static void print_descriptor(gatt_desc *desc)
{
    if (!desc) return;
    if (bt_uuid_is_uuid16(&desc->desc_uuid)) {
        LOG_I(common, "      [GATT]Descriptor: Handle = 0x%04x, uuid = %04x", desc->handle, desc->desc_uuid.uuid16);
    } else {
        LOG_I(common, "      [GATT]Descriptor: Handle = 0x%04x uuid = %s", desc->handle, get_uuid128(&desc->desc_uuid));
    }
}

static void print_all_descriptor(list_entry *desc_list)
{
    gatt_desc_node *desc_node;
    desc_node = (gatt_desc_node *)desc_list->next;
    while ((NULL != desc_node) && ((list_entry *)desc_node != desc_list)) {
        print_descriptor(&desc_node->descriptor);
        desc_node = (gatt_desc_node *)desc_node->node.next;
    }
}

static void dump_char_properties(uint8_t prop)
{
    char buf[80] = { 0 };
    char *ptr = buf;
    if (prop & 0x01) {
        const char *prop_str = "Broadcast|";
        snprintf(ptr, strlen(prop_str) + 1, prop_str);
        ptr += strlen(ptr);
    }
    if (prop & 0x02) {
        const char *prop_str = "Read|";
        snprintf(ptr, strlen(prop_str) + 1, prop_str);
        ptr += strlen(ptr);
    }
    if (prop & 0x04) {
        const char *prop_str = "Write Without Response|";
        snprintf(ptr, strlen(prop_str) + 1, prop_str);
        ptr += strlen(ptr);
    }
    if (prop & 0x08) {
        const char *prop_str = "Write|";
        snprintf(ptr, strlen(prop_str) + 1, prop_str);
        ptr += strlen(ptr);
    }
    if (prop & 0x10) {
        const char *prop_str = "Notify|";
        snprintf(ptr, strlen(prop_str) + 1, prop_str);
        ptr += strlen(ptr);
    }
    if (prop & 0x20) {
        const char *prop_str = "Indicate|";
        snprintf(ptr, strlen(prop_str) + 1, prop_str);
        ptr += strlen(ptr);
    }
    if (prop & 0x40) {
        const char *prop_str = "Authenticated Signed Writes|";
        snprintf(ptr, strlen(prop_str) + 1, prop_str);
        ptr += strlen(ptr);
    }
    if (prop & 0x80) {
        const char *prop_str = "Extend Properties|";
        snprintf(ptr, strlen(prop_str) + 1, prop_str);
        ptr += strlen(ptr);
    }
    if (ptr != buf) ptr--;
    *ptr = 0;
    LOG_I(common, "      [GATT]Properties:%s", buf);
}

static void print_char_info(gatt_char *char_info)
{
    if (!char_info) return;
    if (bt_uuid_is_uuid16(&char_info->charc_uuid)) {
        LOG_I(common, "    [GATT]Characteristic: ValueHandle = 0x%04x uuid = %04x", char_info->value_handle, char_info->charc_uuid.uuid16);
    } else {
        LOG_I(common, "    [GATT]Characteristic: ValueHandle = 0x%04x", char_info->value_handle);
        LOG_I(common, "      [GATT]uuid = %s", get_uuid128(&char_info->charc_uuid));
    }
    dump_char_properties(char_info->properties);
}

static void print_all_character(list_entry *char_list)
{
    gatt_char_node *char_node;
    char_node = (gatt_char_node *)char_list->next;
    while ((NULL != char_node) && ((list_entry *)char_node != char_list)) {
        print_char_info(&char_node->characteristic);
        print_all_descriptor(&char_node->char_desc_list);
        char_node = (gatt_char_node *)char_node->node.next;
    }
}

static void print_service(gatt_service *srv)
{
    if (!srv) return;
    if (bt_uuid_is_uuid16(&srv->uuid)) {
        LOG_I(common, "[GATT]Primary Service: [0x%04x-0x%04x] uuid = %04x", srv->start_handle, srv->end_handle, srv->uuid.uuid16);
    }
    else {
        LOG_I(common, "[GATT]Primary Service: [0x%04x-0x%04x] ", srv->start_handle, srv->end_handle);
        LOG_I(common, "  [GATT]uuid = %s", get_uuid128(&srv->uuid));
    }
}

static void gatt_free_service_node(gatt_service_node *service_node)
{
    gatt_include_service_node *include_node;
    gatt_char_node *char_node;
    gatt_desc_node *char_desc_node;

    while (!is_list_empty(&service_node->include_list))
    {
        include_node = (gatt_include_service_node *)remove_head(&service_node->include_list);
        gatt_free(include_node);
    }

    while (!is_list_empty(&service_node->char_list))
    {
        char_node = (gatt_char_node *)remove_head(&service_node->char_list);
        while (!is_list_empty(&char_node->char_desc_list))
        {
            char_desc_node = (gatt_desc_node *)remove_head(&char_node->char_desc_list);
            gatt_free(char_desc_node);
        }
        gatt_free(char_node);
    }

    gatt_free(service_node);
}

static void gatt_free_all_service()
{
    gatt_service_node *service_node;

    while (!is_list_empty(&g_service_list)) {
        service_node = (gatt_service_node *)remove_head(&g_service_list);
        gatt_free_service_node(service_node);
    }
}

static void print_all_service(list_entry *service_list)
{
    gatt_service_node *srv_node;
    gatt_service *srv;
    if (!service_list || is_list_empty(service_list)) return;
    srv_node = (gatt_service_node *)service_list->next;
    while ((list_entry *)srv_node != service_list) {
        srv = &srv_node->service;
        print_service(srv);
        print_all_include_service(&srv_node->include_list);
        print_all_character(&srv_node->char_list);
        srv_node = (gatt_service_node *)(srv_node->node.next);
    }
    /* free all service_list */
    gatt_free_all_service();
}

static gatt_service_node *gatt_new_service(uint16_t start_handle, uint16_t end_handle, bt_uuid_t *uuid)
{
    gatt_service_node *service_node = NULL;
    service_node = (gatt_service_node *)gatt_malloc(sizeof(gatt_service_node));
    if (service_node) {
        memset((uint8_t *)service_node, 0, sizeof(gatt_service_node));
        initialize_list_head(&service_node->include_list);
        initialize_list_head(&service_node->char_list);

        service_node->service.start_handle = start_handle;
        service_node->service.end_handle = end_handle;
        bt_uuid_copy(&service_node->service.uuid, uuid);
    } else {
        LOG_I(common, "[GATT]Memory alloc fail for service");
    }

    return service_node;
}

static gatt_include_service_node *gatt_new_included_service(uint16_t attribute_handle, uint16_t start_handle, uint16_t end_handle, bt_uuid_t *uuid)
{
    gatt_include_service_node *inc_srv_node = NULL;
    inc_srv_node = (gatt_include_service_node *)gatt_malloc(sizeof(gatt_include_service_node));
    if (inc_srv_node) {
        memset((uint8_t *)inc_srv_node, 0, sizeof(gatt_include_service_node));
        inc_srv_node->include_service.handle = attribute_handle;
        inc_srv_node->include_service.included_service_handle = start_handle;
        inc_srv_node->include_service.end_group_handle = end_handle;
        bt_uuid_copy(&inc_srv_node->include_service.uuid, uuid);
    } else {
        LOG_I(common, "[GATT]Memory alloc fail for included service");
    }

    return inc_srv_node;
}

static gatt_char_node *gatt_new_char(uint16_t handle, uint16_t value_handle, uint8_t properties, bt_uuid_t *uuid)
{
    gatt_char_node *char_node = NULL;
    char_node = (gatt_char_node *)gatt_malloc(sizeof(gatt_char_node));
    if (char_node) {
        memset((uint8_t *)char_node, 0, sizeof(gatt_char_node));
        initialize_list_head(&char_node->char_desc_list);
        char_node->characteristic.handle = handle;
        char_node->characteristic.value_handle = value_handle;
        char_node->characteristic.properties = properties;
        bt_uuid_copy(&char_node->characteristic.charc_uuid, uuid);
    }
    return char_node;
}

static gatt_desc_node *gatt_new_desc(uint16_t handle, bt_uuid_t *uuid)
{
    gatt_desc_node *desc_node = NULL;
    desc_node = (gatt_desc_node *)gatt_malloc(sizeof(gatt_desc_node));
    if (desc_node) {
        memset((uint8_t *)desc_node, 0, sizeof(gatt_desc_node));
        desc_node->descriptor.handle = handle;
        bt_uuid_copy(&desc_node->descriptor.desc_uuid, uuid);
    }
    return desc_node;
}

static void ble_gattc_find_included_service(uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle)
{
    bt_gattc_find_included_services_req_t req;

    req.opcode = BT_ATT_OPCODE_READ_BY_TYPE_REQUEST;
    req.starting_handle = start_handle;
    req.ending_handle = end_handle;
    req.type16 = BT_GATT_UUID16_INCLUDE;

    bt_gattc_find_included_services(conn_handle, &req);
}

static void ble_gattc_discover_all_charc(uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle)
{
    bt_gattc_discover_charc_req_t req;
    uint16_t uuid_16 = BT_GATT_UUID16_CHARC;

    req.opcode = BT_ATT_OPCODE_READ_BY_TYPE_REQUEST;
    req.starting_handle = start_handle;
    req.ending_handle = end_handle;

    bt_uuid_load(&(req.type), (void *)&uuid_16, 2);
    bt_gattc_discover_charc(conn_handle, &req);
}

static void ble_gattc_discover_descriptors(uint16_t conn_handle, uint16_t start_handle, uint16_t end_handle)
{
    bt_gattc_discover_charc_descriptor_req_t req;

    req.opcode = BT_ATT_OPCODE_FIND_INFORMATION_REQUEST;
    req.starting_handle = start_handle;
    req.ending_handle = end_handle;

    bt_gattc_discover_charc_descriptor(conn_handle, &req);
}

static void real_begin_discover_next_charc()
{
    gatt_test_flag = TEST_DISCOVER_ALL_CHAR_STEP;
    ble_gattc_discover_all_charc(conn_id, service_table[cur_srv_index].start_handle, service_table[cur_srv_index].end_handle);
}

static void real_begin_discover_next_include_service()
{
    cur_srv_index++;
    if (cur_srv_index < srv_count) {
        gatt_test_flag = TEST_DISCOVER_ALL_INCLUDE_SERVICES_STEP;
        ble_gattc_find_included_service(conn_id, service_table[cur_srv_index].start_handle, service_table[cur_srv_index].end_handle);
    } else {
        gatt_test_flag = 0;
        cur_srv_index = 0;
        print_all_service(&g_service_list);
    }
}

static void begin_discover_desc()
{
    gatt_test_flag = TEST_DISCOVER_ALL_DESC_STEP;
    cur_srv_node = find_service_node_by_handle_range(&g_service_list, service_table[cur_srv_index].start_handle);
    if (!cur_srv_node) return;
    cur_char_node = (gatt_char_node *)cur_srv_node->char_list.next;
    if (is_list_empty((list_entry *)&cur_srv_node->char_list)) {
        /* should not come here, there should be at least 1 character */
        real_begin_discover_next_include_service();
        return;
    }
    if (cur_char_node->characteristic.value_handle == cur_srv_node->service.end_handle) {
        /* no descriptor, begin to discover next primary service's include service */
        real_begin_discover_next_include_service();
    }
    else {
        uint16_t start_handle;
        uint16_t end_handle;
        if ((list_entry *)cur_char_node->node.next != &cur_srv_node->char_list) {
            /* there are more than one character in service */
            while ((list_entry *)cur_char_node->node.next != &cur_srv_node->char_list) {
                start_handle = cur_char_node->characteristic.value_handle + 1;
                end_handle = ((gatt_char_node *)cur_char_node->node.next)->characteristic.handle - 1;
                if (start_handle <= end_handle) {
                    ble_gattc_discover_descriptors(conn_id, start_handle, end_handle);
                    return;
                }
                cur_char_node = (gatt_char_node *)cur_char_node->node.next;
            }
            /* come here means all character have no descriptor, now it goes to the last character */
            if (cur_char_node->characteristic.value_handle == cur_srv_node->service.end_handle) {
                real_begin_discover_next_include_service();
            }
            else {
                start_handle = cur_char_node->characteristic.value_handle + 1;
                end_handle = cur_srv_node->service.end_handle;
                ble_gattc_discover_descriptors(conn_id, start_handle, end_handle);
            }
        }
        else {
            /* only one character in service */
            start_handle = cur_char_node->characteristic.value_handle + 1;
            end_handle = cur_srv_node->service.end_handle;
            ble_gattc_discover_descriptors(conn_id, start_handle, end_handle);
        }
    }
}

/* find char_node by handle, then get descriptor's start and end search handle */
static void discover_next_desc(uint8_t skip, uint16_t cur_handle)
{
    gatt_char_node *char_node;
    uint16_t start_handle;
    uint16_t end_handle;
    static uint16_t cur_end_handle = 0;
    if ((gatt_test_flag & TEST_DISCOVER_ALL_DESC_STEP) == 0) {
        return;
    }
    if (!cur_srv_node) return;

    if (cur_handle >= cur_srv_node->service.end_handle || ((cur_handle + 1) >= cur_srv_node->service.end_handle)) {
        real_begin_discover_next_include_service();
        return;
    }

    /* only find char node under current service node */
    char_node = find_char_node_by_handle_range(cur_srv_node, cur_handle);
    if (!char_node) {
        /* begin discover next include service */
        real_begin_discover_next_include_service();
    }
    else {
        if ((list_entry *)char_node->node.next != &cur_srv_node->char_list) {
            /* there are more than one character in service */
            while ((list_entry *)char_node->node.next != &cur_srv_node->char_list) {
                start_handle = char_node->characteristic.value_handle + 1;
                end_handle = ((gatt_char_node *)char_node->node.next)->characteristic.handle - 1;

                if (start_handle > end_handle) {
                    char_node = (gatt_char_node *)char_node->node.next;
                }
                else {
                    if (start_handle <= cur_handle && cur_handle <= end_handle) {
                        if (0 == skip) {
                            cur_end_handle = end_handle;
                            ble_gattc_discover_descriptors(conn_id, cur_handle, end_handle);
                        }
                        else {
                            discover_next_desc(0, cur_end_handle + 1);
                        }
                        return;
                    }
                    else {
                        cur_end_handle = end_handle;
                        ble_gattc_discover_descriptors(conn_id, start_handle, end_handle);
                        return;
                    }
                }
            }
            /* come here means all character have no descriptor, now it goes to the last character */
            if (char_node->characteristic.value_handle == cur_srv_node->service.end_handle) {
                real_begin_discover_next_include_service();
            }
            else {
                start_handle = char_node->characteristic.value_handle + 1;
                end_handle = cur_srv_node->service.end_handle;
                cur_end_handle = end_handle;
                ble_gattc_discover_descriptors(conn_id, start_handle, end_handle);
            }
        }
        else {
            /* last character in service */
            start_handle = char_node->characteristic.value_handle + 1;
            if (start_handle < cur_handle) start_handle = cur_handle;
            end_handle = cur_srv_node->service.end_handle;
            if (0 == skip) {
                ble_gattc_discover_descriptors(conn_id, start_handle, end_handle);
            }
            else {
                real_begin_discover_next_include_service();
            }
        }
    }
}

static bt_status_t bt_gattc_write_data_test()
{
    uint8_t buffer[300] = { 0 };
    bt_gattc_write_charc_req_t req;

    req.attribute_value_length = mtu - 3;
    req.att_req = (bt_att_write_req_t *)buffer;
    req.att_req->opcode = BT_ATT_OPCODE_WRITE_REQUEST;
    req.att_req->attribute_handle = 1;
    memcpy(req.att_req->attribute_value, test_data, mtu - 3);

#if defined(MTK_BT_LWIP_ENABLE)
    bt_lwip_send(test_data, 18);
#endif
    return bt_gattc_write_charc(test_conn_id, &req);
}

#ifdef BLE_THROUGHPUT
extern QueueHandle_t ble_tp_queue;

static bt_status_t ble_gattc_write_data_test()
{
    uint8_t buf[260] = { 0 };
    bt_gattc_write_without_rsp_req_t req;

    req.attribute_value_length = mtu - 3;
    req.att_req = (bt_att_write_req_t *)buf;
    req.att_req->opcode = BT_ATT_OPCODE_WRITE_COMMAND;
    req.att_req->attribute_handle = 1;
    //memcpy(req.att_req->attribute_value, test_data, mtu - 3);
    memset(req.att_req->attribute_value, test_index, mtu - 3);
    return bt_gattc_write_without_rsp(test_conn_id, 0, &req);
}

void ble_gatt_send_data()
{
    bt_status_t status;
    uint32_t speed = 1;
    if (test_started == 0) return;
    do {
        cur_tick = xTaskGetTickCount();
        if (cur_tick > test_finish_time) {
            printf("Throughput Finished: %d.\n", cur_tick);
            test_started = 0;
            speed = test_index * (mtu - 3) * 1000 / ((cur_tick - test_start_time) * portTICK_PERIOD_MS);
            printf("Throughput: finish send data test start_time:%u end_time:%u, test_index:%d\n", test_start_time, test_finish_time, test_index);
            printf("Throughput: %d bytes/sec\n", speed);
            break;
        }
        if (cur_tick > test_next_show) {
            speed = test_index * (mtu - 3) * 1000 / ((cur_tick - test_start_time) * portTICK_PERIOD_MS);
            test_next_show += (test_show_interval);
            printf("Throughput: test start_time:%u end_time:%u, test_index:%d\n", test_start_time, cur_tick, test_index);
            printf("Throughput: %d bytes/sec\n", speed);
        }
        status = ble_gattc_write_data_test();
        if (status != BT_STATUS_SUCCESS) {
            uint8_t msg = 1;
            printf("index %d fail:%x\n", test_index, status);
            xQueueSend(ble_tp_queue, (void *)&msg, 0);
            break;
        } else {
            test_index++;
            /*if ((test_index % 5) == 0) {
                uint8_t msg = 1;
                xQueueSend(ble_tp_queue, (void *)&msg, 0);
                break;
            }*/
        }
    } while (1);
}
#endif

static void show_throughput()
{
    if (test_started)
    {
        cur_tick = xTaskGetTickCount();
        if (cur_tick > test_finish_time)
        {
            test_started = 0;
            test_conn_id = 0;
            LOG_I(common, "send to NB-- END");
        }

        if (test_started)
        {
            bt_gattc_write_data_test();
        }
    }
}

static bt_status_t bt_app_cmd_gattc_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
{
    switch (msg) {
        case BT_GATTC_EXCHANGE_MTU:
        {
            LOG_I(common, "[GATT]BT_GATTC_EXCHANGE_MTU status = %d\n", status);
            if (BT_STATUS_SUCCESS != status) break;
            bt_gatt_exchange_mtu_rsp_t *rsp = (bt_gatt_exchange_mtu_rsp_t *)buff;
            mtu = rsp->server_rx_mtu;
            LOG_I(common, "[GATT] exchange mtu rsp: %d\n", mtu);
        #ifdef BLE_THROUGHPUT
            printf("[GATT] exchange mtu rsp: %d\n", mtu);
        #endif
        }
        break;
        case BT_GATTC_DISCOVER_PRIMARY_SERVICE:
        {
            LOG_I(common, "[GATT]BT_GATTC_DISCOVER_PRIMARY_SERVICE status = %d", status);

            if (status != BT_STATUS_SUCCESS && status != BT_ATT_ERRCODE_CONTINUE) {
                LOG_I(common, "BT_GATTC_DISCOVER_PRIMARY_SERVICE FINISHED!!");
                if (gatt_test_flag & TEST_DISCOVER_ALL_PRIMARY_SERVICES) {
                    gatt_test_flag = TEST_DISCOVER_ALL_INCLUDE_SERVICES_STEP;
                    cur_srv_index = 0;
                    ble_gattc_find_included_service(conn_id, service_table[cur_srv_index].start_handle, service_table[cur_srv_index].end_handle);
                }
                break;
            }

            bt_gattc_read_by_group_type_rsp_t *rsp = (bt_gattc_read_by_group_type_rsp_t *)buff;
            uint16_t end_group_handle = 0, starting_handle = 0, uuid = 0;
            bt_uuid_t uuid128;
            uint8_t *attribute_data_list = rsp->att_rsp->attribute_data_list;
            uint8_t num_of_data = (rsp->length - 2) / rsp->att_rsp->length;
            int i;

            for (i = 0; i < num_of_data; i++){
                memcpy(&starting_handle, attribute_data_list + i * rsp->att_rsp->length, 2);
                memcpy(&end_group_handle, attribute_data_list + i * rsp->att_rsp->length + 2, 2);
                LOG_I(common, "[GATT]data : %d", i);
                LOG_I(common, "[GATT]starting_handle = 0x%04x, end_group_handle = 0x%04x", starting_handle, end_group_handle);
                if (rsp->att_rsp->length == 6) {
                    memcpy(&uuid, attribute_data_list + i * rsp->att_rsp->length + 4, rsp->att_rsp->length - 4);
                    LOG_I(common, "[GATT]uuid = 0x%04x", uuid);
                    bt_uuid_from_uuid16(&uuid128, uuid);
                }
                else {
                    memcpy(&uuid128.uuid, attribute_data_list + i * rsp->att_rsp->length + 4, rsp->att_rsp->length - 4);
                    dump_uuid128(&uuid128);
                }
                if (gatt_test_flag & TEST_DISCOVER_ALL_PRIMARY_SERVICES) {
                    gatt_service_node *srv_node = NULL;
                    srv_node = gatt_new_service(starting_handle, end_group_handle, &uuid128);

                    service_table[srv_count].start_handle = starting_handle;
                    service_table[srv_count].end_handle = end_group_handle;
                    /* Only need to store start and end handle */
                    srv_count++;
                    if (srv_count >= MAX_SERVICE_NO) {
                        LOG_I(common, "[GATT]There are more than %d primary services.", MAX_SERVICE_NO);
                        assert(0);
                    }
                    if (srv_node) {
                        insert_tail_list(&g_service_list, &srv_node->node);
                    }
                }
            }
            if (status == BT_STATUS_SUCCESS) {
                if (gatt_test_flag & TEST_DISCOVER_ALL_PRIMARY_SERVICES) {
                    gatt_test_flag = TEST_DISCOVER_ALL_INCLUDE_SERVICES_STEP;
                    cur_srv_index = 0;
                    ble_gattc_find_included_service(conn_id, service_table[cur_srv_index].start_handle, service_table[cur_srv_index].end_handle);
                }
            }
        }
        break;
        case BT_GATTC_FIND_INCLUDED_SERVICES:
        {
            LOG_I(common, "[GATT]BT_GATTC_FIND_INCLUDED_SERVICES status = %d", status);

            if (status != BT_STATUS_SUCCESS && status != BT_ATT_ERRCODE_CONTINUE) {
                LOG_I(common, "[GATT]BT_GATTC_FIND_INCLUDED_SERVICES FINISHED!!");
                if (gatt_test_flag & TEST_DISCOVER_ALL_INCLUDE_SERVICES_STEP) {
                    /* will discover characteristic actually */
                    real_begin_discover_next_charc();
                }
                break;
            }

            bt_gattc_read_by_type_rsp_t *rsp = (bt_gattc_read_by_type_rsp_t *)buff;
            uint8_t *attribute_data_list = rsp->att_rsp->attribute_data_list;
            uint16_t attribute_handle = 0, starting_handle = 0, ending_handle = 0, uuid = 0;
            bt_uuid_t uuid128;
            int i;
            uint8_t num_of_data = (rsp->length - 2) / rsp->att_rsp->length;

            for (i = 0; i < num_of_data; i++) {
                LOG_I(common, "[GATT]data : %d", i);
                memcpy(&attribute_handle, attribute_data_list + i * rsp->att_rsp->length, 2);
                memcpy(&starting_handle, attribute_data_list + i * rsp->att_rsp->length + 2, 2);
                memcpy(&ending_handle, attribute_data_list + i * rsp->att_rsp->length + 4, 2);
                LOG_I(common, "[GATT]attribute_handle = 0x%04x, starting_handle = 0x%04x, end_group_handle = 0x%04x",
                    attribute_handle, starting_handle, ending_handle);
                if (rsp->att_rsp->length <= 8) {
                    memcpy(&uuid, attribute_data_list + i * rsp->att_rsp->length + 6, 2);
                    LOG_I(common, "[GATT]uuid = 0x%04x", uuid);
                    bt_uuid_from_uuid16(&uuid128, uuid);
                } else {
                    memcpy(&uuid128.uuid, attribute_data_list + i * rsp->att_rsp->length + 6, 16);
                    dump_uuid128(&uuid128);
                }
                if (gatt_test_flag & TEST_DISCOVER_ALL_INCLUDE_SERVICES_STEP) {
                    /* CFTBD: find position and add include service to service_list */
                    gatt_service_node *srv_node = NULL;
                    gatt_include_service_node *inc_srv_node = NULL;

                    srv_node = find_service_node_by_handle_range(&g_service_list, attribute_handle);
                    if (srv_node) {
                        inc_srv_node = gatt_new_included_service(attribute_handle, starting_handle, ending_handle, &uuid128);
                        if (inc_srv_node) {
                            insert_tail_list(&srv_node->include_list, &inc_srv_node->node);
                        }
                    }
                }
            }

            if (status == BT_STATUS_SUCCESS) {
                if (gatt_test_flag & TEST_DISCOVER_ALL_INCLUDE_SERVICES_STEP) {
                    /* will discover characteristic actually */
                    real_begin_discover_next_charc();
                }
            }
        }
        break;
        case BT_GATTC_DISCOVER_CHARC:
        {
            LOG_I(common, "[GATT]BT_GATTC_DISCOVER_CHARC status = %d", status);

            if (status != BT_STATUS_SUCCESS && status != BT_ATT_ERRCODE_CONTINUE) {
                LOG_I(common, "[GATT]BT_GATTC_DISCOVER_CHARC FINISHED!!");
                if (gatt_test_flag & TEST_DISCOVER_ALL_CHAR_STEP) {
                    begin_discover_desc();
                }
                break;
            }

            bt_gattc_read_by_type_rsp_t *rsp = (bt_gattc_read_by_type_rsp_t *)buff;
            uint8_t *attribute_data_list = rsp->att_rsp->attribute_data_list;
            uint16_t attribute_handle = 0, value_handle = 0, uuid = 0;
            uint8_t properties = 0;
            uint8_t num_of_data = (rsp->length - 2) / rsp->att_rsp->length;
            bt_uuid_t uuid128;
            int i;

            for (i = 0; i < num_of_data; i++) {
                memcpy(&attribute_handle, attribute_data_list + i * rsp->att_rsp->length, 2);
                memcpy(&properties, attribute_data_list + i * rsp->att_rsp->length + 2, 1);
                memcpy(&value_handle, attribute_data_list + i * rsp->att_rsp->length + 3, 2);
                LOG_I(common, "[GATT]num_of_data = %d", i);
                LOG_I(common, "[GATT]attribute handle = 0x%04x value_handle = 0x%04x, properties = 0x%02x", attribute_handle, value_handle, properties);
                if (rsp->att_rsp->length < 20) {
                    memcpy(&uuid, attribute_data_list + i * rsp->att_rsp->length + 5, 2);
                    LOG_I(common, "[GATT]uuid = 0x%04x", uuid);
                    bt_uuid_from_uuid16(&uuid128, uuid);
                } else {
                    memcpy(&uuid128.uuid, attribute_data_list + i * rsp->att_rsp->length + 5, 16);
                    dump_uuid128(&uuid128);
                }

                if (gatt_test_flag & TEST_DISCOVER_ALL_CHAR_STEP) {
                    /* CFTBD: find position and add char to service_list */
                    gatt_service_node *srv_node = NULL;
                    gatt_char_node *char_node = NULL;

                    srv_node = find_service_node_by_handle_range(&g_service_list, attribute_handle);
                    if (srv_node) {
                        char_node = gatt_new_char(attribute_handle, value_handle, properties, &uuid128);
                        if (char_node) {
                            insert_tail_list(&srv_node->char_list, &char_node->node);
                        }
                    }
                }
            }

            if (BT_STATUS_SUCCESS == status) {
                if (gatt_test_flag & TEST_DISCOVER_ALL_CHAR_STEP) {
                    begin_discover_desc();
                }
            }
        }
        break;
        case BT_GATTC_DISCOVER_CHARC_DESCRIPTOR:
        {
            LOG_I(common, "[GATT]BT_GATTC_DISCOVER_CHARC_DESCRIPTOR status = %d", status);

            if (status != BT_STATUS_SUCCESS && status != BT_ATT_ERRCODE_CONTINUE) {
                bt_att_error_rsp_t *rsp = (bt_att_error_rsp_t*)buff;
                LOG_I(common, "[GATT]BT_GATTC_DISCOVER_CHARC_DESCRIPTOR FINISHED!!");
                discover_next_desc(1, rsp->error_handle + 1);
                break;
            }

            bt_gattc_find_info_rsp_t *rsp = (bt_gattc_find_info_rsp_t *)buff;
            uint8_t format = rsp->att_rsp->format;
            uint16_t attribute_handle = 0, attribute_value = 0;
            uint8_t attribute_length = 0;
            uint8_t num_of_attribute;
            bt_uuid_t uuid128;
            int i;

            if (format == 0x02) {
                attribute_length = 18;
            }
            else {
                attribute_length = 4;
            }
            num_of_attribute = (rsp->length - 2) / attribute_length;
            for (i = 0; i < num_of_attribute; ++i) {
                LOG_I(common, "[GATT]num_of_data = %d", i);
                if (format == 0x02) {
                    /* uuid 128 */
                    memcpy(&attribute_handle, rsp->att_rsp->info_data + i * attribute_length, 2);
                    memcpy(&uuid128, rsp->att_rsp->info_data + i * attribute_length + 2, 16);
                    LOG_I(common, "[GATT]attribute handle = 0x%04x", attribute_handle);
                    dump_uuid128(&uuid128);
                } else {
                    /* uuid 16 */
                    memcpy(&attribute_handle, rsp->att_rsp->info_data + i * attribute_length, 2);
                    memcpy(&attribute_value, rsp->att_rsp->info_data + i * attribute_length + 2, 2);
                    LOG_I(common, "[GATT]attribute handle = 0x%04x, uuid = 0x%04x", attribute_handle, attribute_value);
                    bt_uuid_from_uuid16(&uuid128, attribute_value);
                }
                if (gatt_test_flag & TEST_DISCOVER_ALL_DESC_STEP) {
                    /* CFTBD: find position and add desc to service_list */
                    gatt_service_node *srv_node = NULL;
                    gatt_char_node *char_node = NULL;
                    gatt_desc_node *desc_node = NULL;

                    srv_node = find_service_node_by_handle_range(&g_service_list, attribute_handle);
                    if (srv_node) {
                        char_node = find_char_node_by_handle_range(srv_node, attribute_handle);
                        if (char_node) {
                            desc_node = gatt_new_desc(attribute_handle, &uuid128);
                            if (desc_node) {
                                insert_tail_list(&char_node->char_desc_list, &desc_node->node);
                            }
                        }
                    }
                }
            }
            if (BT_STATUS_SUCCESS == status) {
                if (gatt_test_flag & TEST_DISCOVER_ALL_DESC_STEP) {
                    discover_next_desc(0, attribute_handle + 1);
                }
            }
        }
        break;
        case BT_GATTC_READ_CHARC:
        {
            LOG_I(common, "[GATT]BT_GATTC_READ_CHARC status = %d", status);

            if (status != BT_STATUS_SUCCESS) {
                LOG_I(common, "[GATT]BT_GATTC_READ_CHARC FINISHED with error.");
                break;
            }

            bt_gattc_read_rsp_t *rsp = (bt_gattc_read_rsp_t *)buff;
            uint8_t length = rsp->length - 1;
            int i;

            for (i = 0; i < length; i++)
                printf("%02x", rsp->att_rsp->attribute_value[i]);
            printf("\n");
            if (status == BT_STATUS_SUCCESS) {
                LOG_I(common, "[GATT]bt_gattc_read_charc FINISHED!!");
            }
        }
        break;
        case BT_GATTC_WRITE_CHARC:
        {
            LOG_I(common, "[GATT]BT_GATTC_WRITE_CHARC status = %d", status);
            if (status != BT_STATUS_SUCCESS) {
                bt_gattc_error_rsp_t * err_rsp = (bt_gattc_error_rsp_t *)buff;
                if (err_rsp->connection_handle == test_conn_id) {
                    show_throughput();
                } else {
                    LOG_I(common, "[GATT]BT_GATTC_WRITE_CHARC FINISHED with error.");
                }
                break;
            }
            bt_gattc_write_rsp_t *rsp = (bt_gattc_write_rsp_t *)buff;
            if (rsp->connection_handle == test_conn_id) {
                show_throughput();
            }
        }
        break;
        case BT_GATTC_CHARC_VALUE_NOTIFICATION:
        {
            LOG_I(common, "BT_GATTC_CHARC_VALUE_NOTIFICATION status = %d", status);
            if (status != BT_STATUS_SUCCESS) {
                LOG_I(common, "BT_GATTC_CHARC_VALUE_NOTIFICATION FINISHED.");
                break;
            }

            bt_gatt_handle_value_notification_t *rsp = (bt_gatt_handle_value_notification_t *)buff;
            uint16_t attribute_handle = rsp->att_rsp->handle;

            LOG_I(common, "attribute_handle = 0x%04x", attribute_handle);

            if (status == BT_STATUS_SUCCESS) {
#if defined(MTK_BT_LWIP_ENABLE)
                uint8_t length = rsp->length - 3;
                LOG_I(common, "length = %d", length);
                bt_lwip_send(rsp->att_rsp->attribute_value, length);
                bt_lwip_send("\r\n", 5);
#endif
                LOG_I(common, "BT_GATTC_CHARC_VALUE_NOTIFICATION FINISHED!!");
            }
        }
        break;
        case BT_GATTC_CHARC_VALUE_INDICATION:
        {
            LOG_I(common, "BT_GATTC_CHARC_VALUE_INDICATION");

            if (status != BT_STATUS_SUCCESS) {
                LOG_I(common, "BT_GATTC_CHARC_VALUE_INDICATION FINISHED.");
                break;
            }

            bt_gatt_handle_value_notification_t *rsp = (bt_gatt_handle_value_notification_t *)buff;
            uint16_t attribute_handle = rsp->att_rsp->handle;

            LOG_I(common, "attribute_handle = 0x%04x", attribute_handle);

            if (status == BT_STATUS_SUCCESS) {
                LOG_I(common, "BT_GATTC_CHARC_VALUE_INDICATION FINISHED!!");
            }
        }
        break;
#ifdef BLE_THROUGHPUT
        case BT_GAP_LE_DISCONNECT_IND:
        {
            bt_gap_le_disconnect_ind_t *ind = (bt_gap_le_disconnect_ind_t *)buff;
            printf("Connection handle:%04x disconnected.\n", ind->connection_handle);
            if (ind->connection_handle == test_conn_id) {
                test_started = 0;
            }
        }
        break;
#endif
        default:
            break;
    }

    return BT_STATUS_SUCCESS;
}

bt_status_t bt_cmd_gattc_io_callback(void *input, void *output)
{
    const char *cmd = input;
    if (UT_APP_CMP("gatt discover_all")) {
        const char *handle = cmd + 18;
        bt_gattc_discover_primary_service_req_t req;

        req.opcode = BT_ATT_OPCODE_READ_BY_GROUP_TYPE_REQUEST;
        req.starting_handle = 0x0001;
        req.ending_handle = 0xFFFF;
        req.type16 = BT_GATT_UUID16_PRIMARY_SERVICE;

        conn_id = (uint16_t)strtoul(handle, NULL, 16);
	    LOG_I(common, "[GATT]Start to discover all services.");
        ut_app_callback = bt_app_cmd_gattc_event_callback;

        gatt_test_flag |= TEST_DISCOVER_ALL_PRIMARY_SERVICES;
        srv_count = 0;
        memset(service_table, 0, sizeof(service_table));
        initialize_list_head(&g_service_list);
        bt_gattc_discover_primary_service(conn_id, &req);
    } else if (UT_APP_CMP("gatt read")) {
        const char *handle = cmd + 10;
        const char *attribute_handle = cmd + 15;

        bt_gattc_read_charc_req_t req;

        req.opcode = BT_ATT_OPCODE_READ_REQUEST;
        req.attribute_handle = (uint16_t)strtoul(attribute_handle, NULL, 16);
        conn_id = (uint16_t)strtoul(handle, NULL, 16);
        LOG_I(common, "[GATT]Start to read charc.");
        ut_app_callback = bt_app_cmd_gattc_event_callback;
        bt_gattc_read_charc(conn_id, &req);
    } else if (UT_APP_CMP("gatt write")) {
        const char *handle = cmd + 11;
        const char *attribute_handle = cmd + 16;
        const char *attribute_value = cmd + 21;
        uint8_t buffer[158] = { 0 };
        uint8_t valuehex[158] = { 0 };
        uint8_t len;
        int i, j;
        bt_gattc_write_charc_req_t req;

        len = strlen(attribute_value);
        for (i = len - 1, j = 0; i >= 0; --i, ++j) {
            if (attribute_value[i] >= '0' && attribute_value[i] <= '9') {
                valuehex[j] = attribute_value[i] - '0';
            }
            else if (attribute_value[i] >= 'A' && attribute_value[i] <= 'F') {
                valuehex[j] = attribute_value[i] - 'A' + 10;
            }
            else if (attribute_value[i] >= 'a' && attribute_value[i] <= 'f') {
                valuehex[j] = attribute_value[i] - 'a' + 10;
            }
        }
        req.attribute_value_length = strlen(attribute_value);
        req.att_req = (bt_att_write_req_t *)buffer;
        req.att_req->opcode = BT_ATT_OPCODE_WRITE_REQUEST;
        conn_id = (uint16_t)strtoul(handle, NULL, 16);
        req.att_req->attribute_handle = (uint16_t)strtoul(attribute_handle, NULL, 16);
        //memcpy(req.att_req->attribute_value, attribute_value, strlen(attribute_value));
        memcpy(req.att_req->attribute_value, valuehex, len);

        LOG_I(common, "[GATT]Start to write charc.");
        ut_app_callback = bt_app_cmd_gattc_event_callback;
         bt_gattc_write_charc(conn_id, &req);
    } else if (UT_APP_CMP("gatt send_data_test")) {
        uint32_t time_duration;
        const char *handle = cmd + 20;
        const char *duration = cmd + 25;
#ifdef BLE_THROUGHPUT
        const char *interval = cmd + 30;
        test_show_interval = (uint16_t)strtoul(interval, NULL, 10) * 1000 / portTICK_PERIOD_MS;
#endif
        test_conn_id = (uint16_t)strtoul(handle, NULL, 16);
        time_duration = (uint16_t)strtoul(duration, NULL, 10) * 1000 / portTICK_PERIOD_MS;
        test_start_time = xTaskGetTickCount();
        test_finish_time = time_duration + test_start_time;
        test_started = 1;
        ut_app_callback = bt_app_cmd_gattc_event_callback;
#ifdef BLE_THROUGHPUT
        test_next_show = test_start_time + (test_show_interval);
        test_index = 1;
        LOG_I(common, "start send data test");
        ble_gatt_send_data();
#else
        LOG_I(common, "send to NB-- Start");
        bt_gattc_write_data_test();
#endif
    } else if (UT_APP_CMP("gatt mtu")) {
        const char *handle = cmd + 9;
        const char *cmtu = cmd + 14;
        test_conn_id = (uint16_t)strtoul(handle, NULL, 16);
        BT_GATTC_NEW_EXCHANGE_MTU_REQ(req, (uint16_t)strtoul(cmtu, NULL, 10));
        //BT_GATTC_NEW_EXCHANGE_MTU_REQ(req, expected_mtu);
        bt_gattc_exchange_mtu(test_conn_id, &req);
        ut_app_callback = bt_app_cmd_gattc_event_callback;
    }

    return BT_STATUS_SUCCESS;
}
