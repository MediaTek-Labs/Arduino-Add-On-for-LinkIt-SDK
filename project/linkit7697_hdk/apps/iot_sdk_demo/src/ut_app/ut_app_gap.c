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

#define __BT_MULTI_ADV__
#include "ut_app.h"
#include <string.h>


bt_status_t bqb_gap_io_callback(void *input, void *output);

// Weak symbol declaration
#if _MSC_VER >= 1500
    #pragma comment(linker, "/alternatename:_bqb_gap_io_callback=_default_bqb_gap_io_callback")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
    #pragma weak bqb_gap_io_callback = default_bqb_gap_io_callback
#else
    #error "Unsupported Platform"
#endif

extern bt_hci_cmd_read_rssi_t read_rssi;
extern bt_bd_addr_t local_public_addr;
extern bool bt_app_advertising;
extern bool bt_app_scanning;
extern bool bt_app_scanning;

bt_status_t default_bqb_gap_io_callback(void *input, void *output)
{
   return BT_STATUS_SUCCESS;
}

static void ut_app_gap_convert_hex_str(const char *str, uint8_t *output, uint8_t len)
{
    uint8_t i = 0;
    char tempbuf[2];

    while (len)
    {
        memcpy(tempbuf, (str + (i*2)), 2);
        output[i] = (uint8_t)strtoul(tempbuf, NULL, 16);
        len = len - 2;
        i++;
    }
}

bt_status_t bt_app_gap_io_callback(void *input, void *output)
{
    const char *cmd = input;

    if (UT_APP_CMP("gap power_on")) {
        bt_power_on((bt_bd_addr_ptr_t)&local_public_addr, NULL);
        //bt_gatts_set_max_mtu(128); /* This value should consider with MM Tx/Rx buffer size. */
    }

    else if (UT_APP_CMP("gap power_off")) {
        bt_power_off();
    }
    
    /* Usage: advanced power_on [public address] [random address].
       Note:  Set N if you doesn't need it. */
    else if (UT_APP_CMP("advanced power_on")) {
        if (strlen(cmd) >= 18) {
            uint8_t public_addr[6]={0};
            uint8_t random_addr[6]={0};
            const char *addr_str = cmd + 18;

            /* Find public address */
            if (strncmp("N", addr_str, 1) != 0) {
                copy_str_to_addr(public_addr, addr_str);
            } else {
                public_addr[0] = 'N';
            }

            /* Jump to the start of the random address */
            uint32_t i=0;
            while (i<18) {
                if (strncmp(" ", addr_str, 1) == 0)
                    break;
                addr_str++;
                i++;
            }
            addr_str++;

            /* Find random address */
            if (strncmp("N", addr_str, 1) != 0) {
                copy_str_to_addr(random_addr, addr_str);
            } else {
                random_addr[0] = 'N';
            }

            bt_power_on((public_addr[0] == 'N'? NULL:public_addr),
                (random_addr[0] == 'N'? NULL:random_addr));
        } else {
            BT_COLOR_SET(BT_COLOR_RED);
            BT_LOGW("APP", "please input the specific public address and random address");
            BT_LOGW("APP", "format: advanced po [public address/N] [random address/N]");
            BT_COLOR_SET(BT_COLOR_WHITE);
        }
    }

    /* Usage: random_addr [random address].
       Note:  [random address] should be existed. */
    else if (UT_APP_CMP("gap random_addr")) {
        if (strlen(cmd) >= 16) {
            const char *addr_str = cmd + 16;
            uint8_t addr[6];
            copy_str_to_addr(addr, addr_str);

            bt_gap_le_set_random_address(addr);
        } else {
            BT_COLOR_SET(BT_COLOR_RED);
            BT_LOGW("APP", "please input the specific random address");
            BT_LOGW("APP", "gap random_addr [random address]");
            BT_COLOR_SET(BT_COLOR_WHITE);
        }
    }
    
    /* Usage: gap wl add [0:public / 1:random] [bt address].
       Note:  default use #lt_addr_type and #lt_addr */
    else if (UT_APP_CMP("gap wl add")) {
        bt_addr_t device;
        if (strlen(cmd) >= 11) {

            uint8_t addr_type = (uint8_t)strtoul(cmd + 11, NULL, 10);

            if (addr_type != 0 && addr_type!= 1) {
                BT_COLOR_SET(BT_COLOR_RED);
                BT_LOGW("APP", "please input the correct address type");
                BT_LOGW("APP", "gap wl add [0:public / 1:random] [bt address]");
                BT_COLOR_SET(BT_COLOR_WHITE);
            } else {
                const char *addr_str = cmd + 13;
                uint8_t addr[6];
                copy_str_to_addr(addr, addr_str);

                device.type = addr_type;
                memcpy(device.addr, addr, sizeof(addr));
                bt_gap_le_set_white_list(BT_GAP_LE_ADD_TO_WHITE_LIST, &device);
            }
        } else {
            //device.address_type = lt_addr_type;
            //memcpy(device, lt_addr, sizeof(lt_addr));
            //bt_gap_le_set_white_list(BT_ADD_TO_WHITE_LIST, &device);
            BT_COLOR_SET(BT_COLOR_RED);
            BT_LOGW("APP", "please input the correct cmd");
            BT_LOGW("APP", "gap wl add [0:public / 1:random] [bt address]");
            BT_COLOR_SET(BT_COLOR_WHITE);
        }
    }

    /* Usage: gap wl remove [0:public / 1:random] [bt address].
       Note:  default use #lt_addr_type and #lt_addr */
    else if (UT_APP_CMP("gap wl remove")) {
        bt_addr_t device;
        if (strlen(cmd) >= 14) {

            uint8_t addr_type = (uint8_t)strtoul(cmd + 14, NULL, 10);
            if (addr_type != 0 && addr_type!= 1) {
                BT_COLOR_SET(BT_COLOR_RED);
                BT_LOGW("APP", "please input the correct address type");
                BT_LOGW("APP", "gap wl remove [0:public / 1:random] [bt address]");
                BT_COLOR_SET(BT_COLOR_WHITE);
            } else {
                const char *addr_str = cmd + 16;
                uint8_t addr[6];
                copy_str_to_addr(addr, addr_str);

                device.type = addr_type;
                memcpy(device.addr, addr, sizeof(addr));
                bt_gap_le_set_white_list(BT_GAP_LE_REMOVE_FROM_WHITE_LIST, &device);
            }
        } else {
            //device.address_type = lt_addr_type;
            //memcpy(device.address, lt_addr, sizeof(lt_addr));
            //bt_gap_le_set_white_list(BT_REMOVE_FROM_WHITE_LIST, &device);
            BT_COLOR_SET(BT_COLOR_RED);
            BT_LOGW("APP", "please input the correct cmd");
            BT_LOGW("APP", "gap wl remove [0:public / 1:random] [bt address]");
            BT_COLOR_SET(BT_COLOR_WHITE);
        }
    }

    else if (UT_APP_CMP("gap wl clear")) {
        bt_gap_le_set_white_list(BT_GAP_LE_CLEAR_WHITE_LIST, NULL);
    }
    
    else if (UT_APP_CMP("gap set_adv_data")) {
        if (strlen(cmd) >= 17) {
            const char *adv_data_str = cmd + 17;
            uint8_t len = strlen(adv_data_str); 

            adv_data.advertising_data_length=31;
            ut_app_gap_convert_hex_str(adv_data_str, adv_data.advertising_data, len);
            BT_LOGW("APP", "[GAP] adv data:%x, %x\n", adv_data.advertising_data[0], adv_data.advertising_data[1]);
        }
    }

    else if (UT_APP_CMP("gap set_scan_rsp_data")) {
        if (strlen(cmd) >= 22) {
            const char *scan_data_str = cmd + 22;
            uint8_t len = strlen(scan_data_str); 

            scan_data.scan_response_data_length=31;
            ut_app_gap_convert_hex_str(scan_data_str, scan_data.scan_response_data, len);
        }
    }
    
    /*gap set_adv_params [min_interval] [max_interval] [adv type] [own addr type] [peer addr type] [peer BT addr] [channel map] [advertising_filter_policy]
      [adv type] : Chck src/hbif/bt_gap_le_spec.h BT_GAP_LE_AD_xxxx 0~4
      [own addr type] :0:public / 1:random/ 2: Gen RPA from resolving list or public address host provide/ 3: Gen RPA from resolving list or static random address host provide
      [peer addr type]:0:public / 1:random
      [advertising_filter_policy]: define in spec, 0~4
      [peer BT Addr] : peer BT address for BT_GAP_LE_AD_CONNECTABLE_DIRECTED_HIGH or BT_GAP_LE_AD_CONNECTABLE_DIRECTED_LOW
     */
      else if (UT_APP_CMP("gap set_adv_params")) {
          uint16_t min_interval = (uint16_t)strtoul(cmd+19, NULL, 16);
          uint16_t max_interval = (uint16_t)strtoul(cmd+24, NULL, 16);
          uint8_t adv_type = (uint8_t)strtoul(cmd+29, NULL, 10);
          uint8_t own_addr_type = (uint8_t)strtoul(cmd+31, NULL, 10);
          uint8_t peer_addr_type = (uint8_t)strtoul(cmd+33, NULL, 10);
          
          const char *addr_str = cmd + 35;
          uint8_t addr[6];
          copy_str_to_addr(addr, addr_str);

          uint8_t map = (uint8_t)strtoul(cmd+53, NULL, 10);
          uint8_t policy = (uint8_t)strtoul(cmd+55, NULL, 10);
          
          BT_COLOR_SET(BT_COLOR_BLUE);
          BT_LOGI("APP", "set advertising params");
          BT_LOGI("APP", "own_addr_type[%d] adv_type[%d] adv_policy[%d] peer_addr_type[%d]",
              own_addr_type,adv_type,policy,peer_addr_type);
          BT_LOGI("APP", "peer_addr(%02x:%02x:%02x:%02x:%02x:%02x)",
              addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
          BT_COLOR_SET(BT_COLOR_WHITE);
    
          adv_para.advertising_interval_min =min_interval;
          adv_para.advertising_interval_max =max_interval;
          adv_para.advertising_type = adv_type;
          adv_para.own_address_type = own_addr_type;
          adv_para.peer_address.type = peer_addr_type;
          memcpy(adv_para.peer_address.addr, addr, 6);
          adv_para.advertising_channel_map = map;
          adv_para.advertising_filter_policy = policy;
    }
#ifdef __BT_MULTI_ADV__
     /*gap start_multi_adv [instance] [tx_power] [address]
      [instance] : 01 ~ (max_adv - 1).
      [tx_power] : -70 ~ 020, default 005.
      [address]: ex. AA11223344CC
      [advertising_filter_policy]: define in spec, 0~4
      [peer BT Addr] : peer BT address for BT_GAP_LE_AD_CONNECTABLE_DIRECTED_HIGH or BT_GAP_LE_AD_CONNECTABLE_DIRECTED_LOW
     */
    else if (UT_APP_CMP("gap start_multi_adv")) {
        bt_status_t ret;
        BT_LOGI("APP", "start multi adv %d", __LINE__);
        if (strlen(cmd) >= sizeof("gap start_multi_adv xx xx")) {
            uint8_t instance = (uint8_t)strtoul(cmd + sizeof("gap start_multi_adv"), NULL, 10);
            int8_t tx_power = (int8_t)strtoul(cmd + sizeof("gap start_multi_adv xx"), NULL, 10);
            uint8_t addr[6];
            copy_str_to_addr(addr, cmd + sizeof("gap start_multi_adv xx xxx"));
            BT_LOGI("APP", "MADV(%d) min: %x, max: %x, adv_type %d, own_type %d, map %x, policy %d", instance, 
                                       adv_para.advertising_interval_min, 
                                       adv_para.advertising_interval_max, 
                                       adv_para.advertising_type, 
                                       adv_para.own_address_type, 
                                       adv_para.advertising_channel_map, 
                                       adv_para.advertising_filter_policy);
            ret = bt_gap_le_start_multiple_advertising(instance, tx_power, addr, &adv_para, &adv_data, &scan_data);
            BT_LOGI("APP", "start multi adv return %x", ret);
        }
    }
    else if (UT_APP_CMP("gap stop_multi_adv")) {
        if (strlen(cmd) >= sizeof("gap stop_multi_adv")) {
            uint8_t instance = (uint8_t)strtoul(cmd + sizeof("gap stop_multi_adv"), NULL, 10);
            bt_gap_le_stop_multiple_advertising(instance);
        }
    }
    else if (UT_APP_CMP("gap get_adv_instance")) {
            BT_LOGI("APP", "Max adv instance %d", bt_gap_le_get_max_multiple_advertising_instances());
    }
#endif
    else if (UT_APP_CMP("gap start_adv")) {
        bt_app_advertising = true;
        memset(gatts_device_name, 0x00, sizeof(gatts_device_name));
        memcpy(gatts_device_name, &adv_data.advertising_data[5], 3);

        BT_COLOR_SET(BT_COLOR_BLUE);
        BT_LOGI("APP", "start advertising");
        BT_COLOR_SET(BT_COLOR_WHITE);
          
        adv_enable.advertising_enable = BT_HCI_ENABLE;
        bt_gap_le_set_advertising(&adv_enable, &adv_para, &adv_data, &scan_data);
    }

    else if (UT_APP_CMP("gap stop_adv")) {
        bt_app_advertising = false;
        bt_hci_cmd_le_set_advertising_enable_t enable;
        enable.advertising_enable = BT_HCI_DISABLE;
        bt_gap_le_set_advertising(&enable, NULL, NULL, NULL);
    }
    /* gap start_scan [scan type] [scan interval] [scan window] [own address type] [scan filter policy]
       [scan type]: 0 is passive, 1 is active
    */
    else if (UT_APP_CMP("gap start_scan")) {
        scan_para.le_scan_type = (uint8_t)strtoul(cmd + 15, NULL, 10);
        scan_para.le_scan_interval = (uint16_t)strtoul(cmd + 17, NULL, 16);
        scan_para.le_scan_window = (uint16_t)strtoul(cmd + 22, NULL, 16);
        scan_para.own_address_type = (uint8_t)strtoul(cmd + 27, NULL, 10);
        scan_para.scanning_filter_policy = (uint8_t)strtoul(cmd + 29, NULL, 10);
        bt_app_scanning = true;
        bt_gap_le_set_scan(&scan_enable, &scan_para);
    }

    else if (UT_APP_CMP("gap stop_scan")) {
        bt_app_scanning = false;
        bt_gap_le_set_scan(&scan_disable, NULL);
    }

    /* Usage: gap connect [0:public / 1:random] [bt address].
        Note:  default use #lt_addr_type and #lt_addr */
     else if (UT_APP_CMP("gap connect")) {
         if (strlen(cmd) >= 12) {
             uint8_t peer_addr_type = (uint8_t)strtoul(cmd + 12, NULL, 10);
    
             const char *addr_str = cmd + 14;
             uint8_t addr[6];
             copy_str_to_addr(addr, addr_str);
    
             connect_para.peer_address.type = peer_addr_type;
             memcpy(connect_para.peer_address.addr, addr, sizeof(addr));
             bt_gap_le_connect(&connect_para);
         } else {
         }
     }

     /* Usage: gap advanced_conn [scan interval] [scan window] [initiator_filter_policy] [peer_address_type] [peer_address] [own_address_type]
               [conn_interval_min] [conn_interval_max] [conn_latency] [supervision_timeout] [minimum_ce_length] [maximum_ce_length].
     */
     else if (UT_APP_CMP("gap advanced_conn")) {
         if (strlen(cmd) >= 18) {
             connect_para.le_scan_interval = (uint16_t)strtoul(cmd + 18, NULL, 16);
             connect_para.le_scan_window = (uint16_t)strtoul(cmd + 23, NULL, 16);
             connect_para.initiator_filter_policy = (uint8_t)strtoul(cmd + 28, NULL, 10);
             connect_para.peer_address.type = (uint8_t)strtoul(cmd + 30, NULL, 10);

             const char *addr_str = cmd + 32;
             uint8_t addr[6];
             copy_str_to_addr(addr, addr_str);
             memcpy(connect_para.peer_address.addr, addr, sizeof(addr));
             connect_para.own_address_type = (uint8_t)strtoul(cmd + 50, NULL, 10);
             connect_para.conn_interval_min = (uint16_t)strtoul(cmd + 52, NULL, 16);
             connect_para.conn_interval_max = (uint16_t)strtoul(cmd + 57, NULL, 16);
             connect_para.conn_latency = (uint16_t)strtoul(cmd + 62, NULL, 16);
             connect_para.supervision_timeout = (uint16_t)strtoul(cmd + 67, NULL, 16);
             connect_para.minimum_ce_length = (uint16_t)strtoul(cmd + 72, NULL, 16);
             connect_para.maximum_ce_length = (uint16_t)strtoul(cmd + 77, NULL, 16);
             bt_gap_le_connect(&connect_para);
         } else {
         }
     }
    
     else if (UT_APP_CMP("gap cancel connect")) {
         bt_gap_le_cancel_connection();
     }
    
     /* Usage:   disconnect <handle in hex>
        Example: disconnect 0200 */
     else if (UT_APP_CMP("gap disconnect")) {
         const char *handle = cmd + strlen("gap disconnect ");
         disconnect_para.connection_handle = (uint16_t)strtoul(handle, NULL, 16);
         BT_LOGI("APP", "connection_handle(0x%04x)",disconnect_para.connection_handle);
         bt_gap_le_disconnect(&disconnect_para);
     }
    
     else if (UT_APP_CMP("gap read_rssi")) {
         read_rssi.handle = (uint16_t)strtoul(cmd + 14, NULL, 16);
         bt_gap_le_read_rssi(&read_rssi);
     }
    
     else if (UT_APP_CMP("gap update_conn")) {
         conn_update_para.connection_handle = (uint16_t)strtoul(cmd + 16, NULL, 16);
         conn_update_para.conn_interval_min = (uint16_t)strtoul(cmd + 21, NULL, 16);
         conn_update_para.conn_interval_max = (uint16_t)strtoul(cmd + 26, NULL, 16);
         conn_update_para.conn_latency = (uint16_t)strtoul(cmd + 31, NULL, 16);
         conn_update_para.supervision_timeout = (uint16_t)strtoul(cmd + 36, NULL, 16);
         conn_update_para.minimum_ce_length = (uint16_t)strtoul(cmd + 41, NULL, 16);
         conn_update_para.maximum_ce_length = (uint16_t)strtoul(cmd + 46, NULL, 16);
         bt_gap_le_update_connection_parameter(&conn_update_para);
     }
    
     /* Usage: update data length <handle in hex> <tx octets in hex> <tx time in hex>.
        Example: update data length 0200 0030 0500*/
     else if (UT_APP_CMP("gap update data length")) {
         bt_hci_cmd_le_set_data_length_t data_length;
         data_length.connection_handle = (uint16_t)strtoul(cmd + 23, NULL, 16);
         data_length.tx_octets = (uint16_t)strtoul(cmd + 28, NULL, 16);
         data_length.tx_time = (uint16_t)strtoul(cmd + 33, NULL, 16);
         if (data_length.connection_handle > 0x0f00 ||
            (data_length.tx_octets < 0x001B || data_length.tx_octets > 0x00FB) ||
            (data_length.tx_time < 0x0148 || data_length.tx_time > 0x0848)) {
             BT_LOGW("APP", "Usage: update data length <handle in hex> <tx octets in hex> <tx time in hex>.");
             BT_LOGW("APP", "The range of connection handle is 0x0000-0x0EFF");
             BT_LOGW("APP", "The range of tx octets is 0x001B-0x00FB");
             BT_LOGW("APP", "The range of tx time is 0x0148-0x0848");
         }
         else {
             BT_LOGI("APP", "update data length handle(%04x) tx_octets(%04x) tx_time(%04x)",
                 data_length.connection_handle,data_length.tx_octets,data_length.tx_time);
             bt_gap_le_update_data_length(&data_length);
         }
     }

     /* Usage: tx_power <handle in hex> <tx power level in integer>.
        [tx power level]: 0~7
        Example: tx_power 0200 1*/
     else if (UT_APP_CMP("gap tx_power")) {
         bt_hci_cmd_le_set_tx_power_t tx_power_t;
         tx_power_t.connection_handle = (uint16_t)strtoul(cmd + 13, NULL, 16);
         tx_power_t.tx_power_level = (uint16_t)strtoul(cmd + 18, NULL, 10);
         
         if (tx_power_t.connection_handle > 0x0f00 ||
             tx_power_t.tx_power_level > 7) {
             BT_LOGW("APP", "Usage: tx_power <handle in hex> <tx power level in integer>.");
             BT_LOGW("APP", "The range of connection handle is 0x0000-0x0EFF");
             BT_LOGW("APP", "The range of tx power level is 0-7");
         }
         else {
             BT_LOGI("APP", "tx power handle(%04x) tx_power_level(%d)",
                 tx_power_t.connection_handle, tx_power_t.tx_power_level);
             bt_gap_le_set_tx_power(&tx_power_t);
         }
     }
     
     /* Usage:   gap bond <handle in hex> [io capability] [oob data flag] [auth req]
                 [initiator_key_distribution] [responder_key_distribution]
        Example: gap bond 0200 3 0 1 0 0*/
     else if (UT_APP_CMP("gap bond")) {
         const char *handle = cmd + strlen("gap bond ");

         pairing_config_req.io_capability = (uint8_t)strtoul(cmd + 14, NULL, 10);
         pairing_config_req.oob_data_flag = (uint8_t)strtoul(cmd + 16, NULL, 10);
         pairing_config_req.auth_req = (uint8_t)strtoul(cmd + 18, NULL, 10);
         pairing_config_req.maximum_encryption_key_size = 16;
         pairing_config_req.initiator_key_distribution = (uint8_t)strtoul(cmd + 20, NULL, 10);
         pairing_config_req.responder_key_distribution = (uint8_t)strtoul(cmd + 22, NULL, 10);
            
         bt_gap_le_bond(strtoul(handle, NULL, 16), &pairing_config_req);
     }
     else {
        return bqb_gap_io_callback(input, output);
     }
    return BT_STATUS_SUCCESS;
}
