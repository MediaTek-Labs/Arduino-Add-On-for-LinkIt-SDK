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

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "nvdm.h"
#include "syslog.h"
#include "hal_efuse.h"
#ifdef MTK_HOMEKIT_ENABLE
#include "hal_sys.h"

#define MAX_KEY_PAIRS   16
#define CFG_KEY_HAP_CTRL_LTPK             "HAP_CTRL_LTPK"
#define NVDM_GROUP_HOMEKIT                "homekit"
#endif
#include "lwip/sockets.h"
#include "connsys_profile.h"
#include "connsys_util.h"
#include "get_profile_string.h"
#include "wifi_nvdm_config.h"
#include "type_def.h"
#include "syslog.h"
#include "wpa_supplicant_task.h"
#include "wifi_init.h"
#include "ethernetif.h"
#include "dhcpd.h"
#include "dhcp.h"
#ifdef MTK_WIFI_CONFIGURE_FREE_ENABLE
#include "wifi_profile.h"
#include <inband_queue.h>
#include "wifi_scan.h"
#endif
#include "wifi_inband.h"

typedef struct {
    char *item_name;
    nvdm_data_item_type_t data_type;
    char *item_default_value;
    uint32_t item_size;
} group_data_item_t;

/* common config */
static const group_data_item_t g_common_data_item_array[] = {
    {
        "OpMode",
        NVDM_DATA_ITEM_TYPE_STRING,
#ifdef MTK_HOMEKIT_ENABLE
        "2",
        sizeof("2")
#else
        "1",
        sizeof("1")
#endif
    },
    {
        "CountryRegion",
        NVDM_DATA_ITEM_TYPE_STRING,
        "5",
        sizeof("5")
    },
    {
        "CountryCode",
        NVDM_DATA_ITEM_TYPE_STRING,
        "TW",
        sizeof("TW")
    },
    {
        "CountryRegionABand",
        NVDM_DATA_ITEM_TYPE_STRING,
        "3",
        sizeof("3")
    },
    {
        "IpAddr",
        NVDM_DATA_ITEM_TYPE_STRING,
        "192.168.1.1",
        sizeof("192.168.1.1")
    },
    {
        "IpNetmask",
        NVDM_DATA_ITEM_TYPE_STRING,
        "255.255.255.0",
        sizeof("255.255.255.0")
    },
    {
        "IpGateway",
        NVDM_DATA_ITEM_TYPE_STRING,
        "192.168.1.254",
        sizeof("192.168.1.254")
    },
    {
        "RadioOff",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "DbgLevel",
        NVDM_DATA_ITEM_TYPE_STRING,
        "3",
        sizeof("3")
    },
    {
        "RTSThreshold",
        NVDM_DATA_ITEM_TYPE_STRING,
        "2347",
        sizeof("2347")
    },
    {
        "FragThreshold",
        NVDM_DATA_ITEM_TYPE_STRING,
        "2346",
        sizeof("2346")
    },
    {
        "BGChannelTable",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1,14,0|",
        sizeof("1,14,0|")
    },
    {
        "AChannelTable",
        NVDM_DATA_ITEM_TYPE_STRING,
        "36,8,0|100,11,0|149,4,0|",
        sizeof("36,8,0|100,11,0|149,4,0|")
    },
    {
        "syslog_filters",
        NVDM_DATA_ITEM_TYPE_STRING,
        "",
        sizeof("")
    },
#if defined(MTK_MINISUPP_ENABLE)
    {
        "Manufacturer",
        NVDM_DATA_ITEM_TYPE_STRING,
        "MTK",
        sizeof("MTK")
    },
    {
        "ModelName",
        NVDM_DATA_ITEM_TYPE_STRING,
        "MTK Wireless Device",
        sizeof("MTK Wireless Device")
    },
    {
        "ModelNumber",
        NVDM_DATA_ITEM_TYPE_STRING,
        "MT7687",
        sizeof("MT7687")
    },
    {
        "SerialNumber",
        NVDM_DATA_ITEM_TYPE_STRING,
        "12345678",
        sizeof("12345678")
    },
    {
        "DeviceName",
        NVDM_DATA_ITEM_TYPE_STRING,
        "MTK IoT",
        sizeof("MTK IoT")
    },
#endif
    {
        "ConfigFree_Ready",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "ConfigFree_Enable",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "StaFastLink",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "WiFiPrivilegeEnable",
        NVDM_DATA_ITEM_TYPE_STRING,
#ifdef MTK_WIFI_PRIVILEGE_ENABLE
        "1",
        sizeof("1")
#else
        "0",
        sizeof("0")
#endif
    },
};

/* STA config */
static const group_data_item_t g_sta_data_item_array[] = {
    {
        "LocalAdminMAC",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "MacAddr",
        NVDM_DATA_ITEM_TYPE_STRING,
        "00:0c:43:76:87:22",
        sizeof("00:0c:43:76:87:22")
    },
    {
        "Ssid",
        NVDM_DATA_ITEM_TYPE_STRING,
        "MTK_SOFT_AP",
        sizeof("MTK_SOFT_AP")
    },
    {
        "SsidLen",
        NVDM_DATA_ITEM_TYPE_STRING,
        "11",
        sizeof("11")
    },
    {
        "BssType",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "Channel",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "BW",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "WirelessMode",
        NVDM_DATA_ITEM_TYPE_STRING,
        "9",
        sizeof("9")
    },
    {
        "BADecline",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "AutoBA",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HT_MCS",
        NVDM_DATA_ITEM_TYPE_STRING,
        "33",
        sizeof("33")
    },
    {
        "HT_BAWinSize",
        NVDM_DATA_ITEM_TYPE_STRING,
        "64",
        sizeof("64")
    },
    {
        "HT_GI",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HT_PROTECT",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HT_EXTCHA",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "WmmCapable",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "ListenInterval",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "AuthMode",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "EncrypType",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "WpaPsk",
        NVDM_DATA_ITEM_TYPE_STRING,
        "12345678",
        sizeof("12345678")
    },
    {
        "WpaPskLen",
        NVDM_DATA_ITEM_TYPE_STRING,
        "8",
        sizeof("8")
    },
    {
        "Password",
        NVDM_DATA_ITEM_TYPE_STRING,
        "12345678",
        sizeof("12345678")
    },
    {
        "PMK",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "PMK_INFO",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "PairCipher",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "GroupCipher",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "DefaultKeyId",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "SharedKey",
        NVDM_DATA_ITEM_TYPE_STRING,
        "aaaaaaaaaaaaa,bbbbbbbbbbbbb,ccccccccccccc,ddddddddddddd",
        sizeof("aaaaaaaaaaaaa,bbbbbbbbbbbbb,ccccccccccccc,ddddddddddddd")
    },
    {
        "SharedKeyLen",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0,0,0,0",
        sizeof("0,0,0,0")
    },
    {
        "PSMode",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "KeepAlivePeriod",
        NVDM_DATA_ITEM_TYPE_STRING,
        "10",
        sizeof("10")
    },
    {
        "IpMode",
        NVDM_DATA_ITEM_TYPE_STRING,
        "dhcp",
        sizeof("dhcp")
    },
    {
        "BeaconLostTime",
        NVDM_DATA_ITEM_TYPE_STRING,
        "2",
        sizeof("2")
    },
    {
        "ApcliBWAutoUpBelow",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    }
};

/* AP config */
static const group_data_item_t g_ap_data_item_array[] = {
    {
        "LocalAdminMAC",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "MacAddr",
        NVDM_DATA_ITEM_TYPE_STRING,
        "00:0c:43:76:62:12",
        sizeof("00:0c:43:76:62:12")
    },
    {
        "Ssid",
        NVDM_DATA_ITEM_TYPE_STRING,
        "MTK_SOFT_AP",
        sizeof("MTK_SOFT_AP")
    },
    {
        "SsidLen",
        NVDM_DATA_ITEM_TYPE_STRING,
        "11",
        sizeof("11")
    },
    {
        "Channel",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "BW",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "WirelessMode",
        NVDM_DATA_ITEM_TYPE_STRING,
        "9",
        sizeof("9")
    },
    {
        "AutoBA",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HT_MCS",
        NVDM_DATA_ITEM_TYPE_STRING,
        "33",
        sizeof("33")
    },
    {
        "HT_BAWinSize",
        NVDM_DATA_ITEM_TYPE_STRING,
        "64",
        sizeof("64")
    },
    {
        "HT_GI",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HT_PROTECT",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HT_EXTCHA",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "WmmCapable",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "DtimPeriod",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "AuthMode",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "EncrypType",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "WpaPsk",
        NVDM_DATA_ITEM_TYPE_STRING,
        "12345678",
        sizeof("12345678")
    },
    {
        "WpaPskLen",
        NVDM_DATA_ITEM_TYPE_STRING,
        "8",
        sizeof("8")
    },
    {
        "Password",
        NVDM_DATA_ITEM_TYPE_STRING,
        "12345678",
        sizeof("12345678")
    },
    {
        "PMK",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "PairCipher",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "GroupCipher",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "DefaultKeyId",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "SharedKey",
        NVDM_DATA_ITEM_TYPE_STRING,
        "11111,22222,33333,44444",
        sizeof("11111,22222,33333,44444")
    },
    {
        "SharedKeyLen",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0,0,0,0",
        sizeof("0,0,0,0")
    },
    {
        "HideSSID",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "RekeyInterval",
        NVDM_DATA_ITEM_TYPE_STRING,
        "3600",
        sizeof("3600")
    },
    {
        "BcnDisEn",
        NVDM_DATA_ITEM_TYPE_STRING,
#ifdef MTK_HOMEKIT_ENABLE
        "1",
        sizeof("1")
#else
        "0",
        sizeof("0")
#endif
    }
};

#ifdef MTK_HOMEKIT_ENABLE
/* homekit config */
static const group_data_item_t g_homekit_data_item_array[] = {
    {
        "WACDONE",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "WAC_ACS_NAME",
        NVDM_DATA_ITEM_TYPE_STRING,
        "New Wi-Fi Device",
        sizeof("New Wi-Fi Device")
    },
    {
        "HOMEKIT_AUTO_START",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HOMEKIT_DELAY",
        NVDM_DATA_ITEM_TYPE_STRING,
        "3",
        sizeof("3")
    },
    {
        "HAP_MODEL_NAME",
        NVDM_DATA_ITEM_TYPE_STRING,
        "MT7687E2",
        sizeof("MT7687E2")
    },
    {
        "HAP_ACS_NAME",
        NVDM_DATA_ITEM_TYPE_STRING,
        "MTK_Aces",
        sizeof("MTK_Aces")
    },
    {
        "HAP_SERIAL_NUM",
        NVDM_DATA_ITEM_TYPE_STRING,
        "123456789",
        sizeof("123456789")
    },
    {
        "HAP_SETUP_CODE",
        NVDM_DATA_ITEM_TYPE_STRING,
        "482-11-763",
        sizeof("482-11-763")
    },
    {
        "HAP_CONFIG_NUM",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HAP_MFR",
        NVDM_DATA_ITEM_TYPE_STRING,
        "MTK",
        sizeof("MTK")
    },
    {
        "HAP_ACS_LTPK",
        NVDM_DATA_ITEM_TYPE_STRING,
        "",
        sizeof("")
    },
    {
        "HAP_ACS_LTSK",
        NVDM_DATA_ITEM_TYPE_STRING,
        "",
        sizeof("")
    },
    {
        "HAP_CTRL_LTPK",
        NVDM_DATA_ITEM_TYPE_STRING,
        "",
        sizeof("")
    },
    {
        "HAP_MFI_ENABLE",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HAP_PAIR_SETUP_DISABLE",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "HAP_BIND_INF",
        NVDM_DATA_ITEM_TYPE_STRING,
        "apcli0",
        sizeof("apcli0")
    },
    {
        "HAP_DEBUG_LEVEL",
        NVDM_DATA_ITEM_TYPE_STRING,
        "4",
        sizeof("4")
    },
    {
        "HAP_ACS_CONF",
        NVDM_DATA_ITEM_TYPE_STRING,
        "3E,4A",
        sizeof("3E,4A")
    },
    {
        "HAP_ACS_CI",
        NVDM_DATA_ITEM_TYPE_STRING,
        "9",
        sizeof("9")
    },
    {
        "HAP_APP_THERMO_HC_CURRENT",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HAP_APP_THERMO_HC_TARGET",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HAP_APP_THERMO_TP_CURRENT",
        NVDM_DATA_ITEM_TYPE_STRING,
        "50",
        sizeof("50")
    },
    {
        "HAP_APP_THERMO_TP_TARGET",
        NVDM_DATA_ITEM_TYPE_STRING,
        "25",
        sizeof("25")
    },
    {
        "HAP_APP_THERMO_TP_UNITS",
        NVDM_DATA_ITEM_TYPE_STRING,
        "1",
        sizeof("1")
    },
    {
        "HAP_APP_THERMO_NAME",
        NVDM_DATA_ITEM_TYPE_STRING,
        "Thermostat_Service",
        sizeof("Thermostat_Service")
    },
    {
        "HAP_APP_THERMO_RH_CURRENT",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "HAP_APP_THERMO_RH_TARGET",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    },
    {
        "HAP_APP_THERMO_TC_THRESHOLD",
        NVDM_DATA_ITEM_TYPE_STRING,
        "10",
        sizeof("10")
    },
    {
        "HAP_APP_THERMO_TH_THRESHOLD",
        NVDM_DATA_ITEM_TYPE_STRING,
        "0",
        sizeof("0")
    }
};
#endif

void user_data_item_check_default_value(void);

#ifdef MTK_LOAD_MAC_ADDR_FROM_EFUSE
int32_t nvdm_get_mac_addr_from_efuse(const char *group_name, char *mac_addr)
{
    uint8_t buf[16] = {0};//efuse is 16 byte aligned
    uint16_t mac_offset = 0x00;//mac addr offset in efuse
    if (HAL_EFUSE_OK != hal_efuse_read(mac_offset, buf, sizeof(buf))) {
        LOG_W(common, "efuse read mac addr fail, default mac will be applied");
        return -1;
    }
    if (0 == strcmp("STA", group_name)) {
        /* original efuse MAC address for STA */
        sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);
    } else {
        /* original efuse MAC address with byte[5]+1 for AP */
        sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", buf[4], buf[5], buf[6], buf[7], buf[8], buf[9] + 1);
    }
    LOG_I(common, "efuse mac addr: %s", mac_addr);
    return 0;
}
#endif

static void check_default_value(const char *group_name, const group_data_item_t *group_data_array, uint32_t count)
{
    uint8_t buffer[1024] = {0};

    for (uint32_t index = 0; index < count; index++) {
        uint32_t buffer_size = 1024;
        nvdm_status_t status = nvdm_read_data_item(group_name,
                               group_data_array[index].item_name,
                               buffer,
                               &buffer_size);
        if (NVDM_STATUS_OK != status) {
#ifdef MTK_LOAD_MAC_ADDR_FROM_EFUSE
            char mac_addr[18] = {0};
            if ((0 == strcmp("MacAddr", group_data_array[index].item_name)
                    && (0 == nvdm_get_mac_addr_from_efuse(group_name, mac_addr)))) {
                status = nvdm_write_data_item(group_name,
                                              group_data_array[index].item_name,
                                              group_data_array[index].data_type,
                                              (uint8_t *)mac_addr,
                                              strlen(mac_addr));
                if (status != NVDM_STATUS_OK) {
                    LOG_I(common, "nvdm_write_data_item error");
                }
            } else
#endif
            {
                status = nvdm_write_data_item(group_name,
                                              group_data_array[index].item_name,
                                              group_data_array[index].data_type,
                                              (uint8_t *)group_data_array[index].item_default_value,
                                              group_data_array[index].item_size);
                if (NVDM_STATUS_OK != status) {
                    LOG_I(common, "write to [%s]%s error", group_name, group_data_array[index].item_name);
                }
            }
        }
    }
}

static void reset_to_default(const char *group_name, const group_data_item_t *group_data_array, uint32_t count)
{
    for (uint32_t index = 0; index < count; index++) {
        nvdm_status_t status;
#ifdef MTK_LOAD_MAC_ADDR_FROM_EFUSE
        char mac_addr[18] = {0};
        if ((0 == strcmp("MacAddr", group_data_array[index].item_name)
                && (0 == nvdm_get_mac_addr_from_efuse(group_name, mac_addr)))) {
            status = nvdm_write_data_item(group_name,
                                          group_data_array[index].item_name,
                                          NVDM_DATA_ITEM_TYPE_STRING,
                                          (uint8_t *)mac_addr,
                                          strlen(mac_addr));
            if (status != NVDM_STATUS_OK) {
                LOG_I(common, "nvdm_write_data_item error");
            }
        } else
#endif
        {
            status = nvdm_write_data_item(group_name,
                                          group_data_array[index].item_name,
                                          group_data_array[index].data_type,
                                          (uint8_t *)group_data_array[index].item_default_value,
                                          group_data_array[index].item_size);
            if (NVDM_STATUS_OK != status) {
                LOG_I(common, "write to [%s]%s error", group_name, group_data_array[index].item_name);
            }
        }
    }
}

static void show_group_value(const char *group_name, const group_data_item_t *group_data_array, uint32_t count)
{
    uint8_t buffer[1024] = {0};
    for (uint32_t index = 0; index < count; index++) {
        uint32_t buffer_size = 1024;
        nvdm_status_t status = nvdm_read_data_item(group_name,
                               group_data_array[index].item_name,
                               buffer,
                               &buffer_size);
        if (NVDM_STATUS_OK == status) {
            printf("[%s]%s: %s\r\n", group_name, group_data_array[index].item_name, (char *)buffer);
        } else {
            printf("read from [%s]%s error.\r\n", group_name, group_data_array[index].item_name);
        }
    }
}

/* user defined callback functions for each group */
static void common_check_default_value(void)
{
    check_default_value("common",
                        g_common_data_item_array,
                        sizeof(g_common_data_item_array) / sizeof(g_common_data_item_array[0]));
}

static void common_reset_to_default(void)
{
    reset_to_default("common",
                     g_common_data_item_array,
                     sizeof(g_common_data_item_array) / sizeof(g_common_data_item_array[0]));
}

static void common_show_value(void)
{
    show_group_value("common",
                     g_common_data_item_array,
                     sizeof(g_common_data_item_array) / sizeof(g_common_data_item_array[0]));
}

static void sta_check_default_value(void)
{
    check_default_value("STA",
                        g_sta_data_item_array,
                        sizeof(g_sta_data_item_array) / sizeof(g_sta_data_item_array[0]));
}

static void sta_reset_to_default(void)
{
    reset_to_default("STA",
                     g_sta_data_item_array,
                     sizeof(g_sta_data_item_array) / sizeof(g_sta_data_item_array[0]));
}

static void sta_show_value(void)
{
    show_group_value("STA",
                     g_sta_data_item_array,
                     sizeof(g_sta_data_item_array) / sizeof(g_sta_data_item_array[0]));
}

static void ap_check_default_value(void)
{
    check_default_value("AP",
                        g_ap_data_item_array,
                        sizeof(g_ap_data_item_array) / sizeof(g_ap_data_item_array[0]));
}

static void ap_reset_to_default(void)
{
    reset_to_default("AP",
                     g_ap_data_item_array,
                     sizeof(g_ap_data_item_array) / sizeof(g_ap_data_item_array[0]));
}

static void ap_show_value(void)
{
    show_group_value("AP",
                     g_ap_data_item_array,
                     sizeof(g_ap_data_item_array) / sizeof(g_ap_data_item_array[0]));
}

#ifdef MTK_HOMEKIT_ENABLE
static void homekit_check_default_value(void)
{
    check_default_value("homekit",
                        (group_data_item_t *)g_homekit_data_item_array,
                        sizeof(g_homekit_data_item_array) / sizeof(g_homekit_data_item_array[0]));
    LOG_I(common, "homekit_check start %d", sizeof(g_homekit_data_item_array) / sizeof(group_data_item_t));

    uint8_t *buffer = NULL;
    buffer = (uint8_t *)pvPortMalloc(1024 * sizeof(uint8_t));
    if (buffer == NULL) {
        LOG_E(common, "homekit_check_value, failed to malloc");
        return;
    }
    memset(buffer, 0, 1024 * sizeof(uint8_t));

    for (uint32_t idx = 0 ; idx < MAX_KEY_PAIRS; idx++) {
        char keyName[64] = {0};
        uint32_t buffer_size = 1024;
        snprintf(keyName, sizeof(keyName), "%s_%d", CFG_KEY_HAP_CTRL_LTPK, (int16_t)idx);
        nvdm_status_t status = nvdm_read_data_item(NVDM_GROUP_HOMEKIT, keyName, buffer, &buffer_size);
        LOG_I(common, "HAP cli: homekit_check start CTRL_LTPK %d", status);
        if (status != NVDM_STATUS_OK) {
            nvdm_status_t status = nvdm_write_data_item(NVDM_GROUP_HOMEKIT,
                                   keyName,
                                   NVDM_DATA_ITEM_TYPE_STRING,
                                   (const uint8_t *)(""),
                                   sizeof(""));
            LOG_I(common, "HAP cli: homekit_check start CTRL_LTPK %d", status);
            if (status != NVDM_STATUS_OK) {
                LOG_I(common, "homekit_check CTRL_LTPK error: %s", g_homekit_data_item_array[idx].item_name);
            }
        }
    }
    vPortFree(buffer);
    LOG_I(common, "homekit_check end");
}

static void homekit_reset_to_default(void)
{
    reset_to_default("homekit",
                     (group_data_item_t *)g_homekit_data_item_array,
                     sizeof(g_homekit_data_item_array) / sizeof(g_homekit_data_item_array[0]));

    for (uint32_t idx = 0 ; idx < MAX_KEY_PAIRS; idx++) {
        char keyName[64] = {0};
        snprintf(keyName, sizeof(keyName), "%s_%d", CFG_KEY_HAP_CTRL_LTPK, (int16_t)idx);
        nvdm_status_t status = nvdm_write_data_item(NVDM_GROUP_HOMEKIT,
                               keyName,
                               NVDM_DATA_ITEM_TYPE_STRING,
                               (const uint8_t *)(""),
                               sizeof(""));
        if (status != NVDM_STATUS_OK) {
            LOG_I(common, "          error: %s", keyName);
        }
    }
    LOG_I(common, "homekit_reset end.         System will reboot now... ...");
    hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);
}

static void homekit_show_value(void)
{
    show_group_value("homekit",
                     (group_data_item_t *)g_homekit_data_item_array,
                     sizeof(g_homekit_data_item_array) / sizeof(g_homekit_data_item_array[0]));

    LOG_I(common, "homekit_show_value start");

    uint8_t *buffer = NULL;
    buffer = (uint8_t *)pvPortMalloc(1024 * sizeof(uint8_t));
    if (buffer == NULL) {
        LOG_E(common, "homekit_show_value, failed to malloc");
        return;
    }
    memset(buffer, 0, 1024 * sizeof(uint8_t));

    for (uint32_t idx = 0 ; idx < MAX_KEY_PAIRS; idx++) {
        char keyName[64] = {0};
        uint32_t buffer_size = 1024;
        snprintf(keyName, sizeof(keyName), "%s_%d", CFG_KEY_HAP_CTRL_LTPK, (int16_t)idx);
        nvdm_status_t status = nvdm_read_data_item(NVDM_GROUP_HOMEKIT, keyName, buffer, &buffer_size);
        if (status == NVDM_STATUS_OK) {
            LOG_I(common, "          %s: %s", keyName, buffer);
        } else {
            LOG_I(common, "          error: %s %d", keyName, status);
        }
    }

    vPortFree(buffer);
    LOG_I(common, "homekit_show_value end");
}
#endif

typedef struct {
    const char *group_name;
    void (*check_default_value)(void);
    void (*reset_default_value)(void);
    void (*show_value)(void);
} user_data_item_operate_t;

static const user_data_item_operate_t user_data_item_operate_array[] = {
    {
        "common",
        common_check_default_value,
        common_reset_to_default,
        common_show_value,
    },
    {
        "STA",
        sta_check_default_value,
        sta_reset_to_default,
        sta_show_value,
    },
    {
        "AP",
        ap_check_default_value,
        ap_reset_to_default,
        ap_show_value,
    },
#ifdef MTK_HOMEKIT_ENABLE
    {
        "homekit",
        homekit_check_default_value,
        homekit_reset_to_default,
        homekit_show_value,
    },
#endif
};

/* This function is used to check whether data is exist in NVDM region,
 * and write default value to NVDM region if no value can be found in NVDM region.
 */
void user_check_default_value(void)
{
    uint32_t index;
    uint32_t max = sizeof(user_data_item_operate_array) / sizeof(user_data_item_operate_t);

    for (index = 0; index < max; index++) {
        user_data_item_operate_array[index].check_default_value();
    }
}

void user_data_item_reset_to_default(char *group_name)
{
    uint32_t index;
    uint32_t max = sizeof(user_data_item_operate_array) / sizeof(user_data_item_operate_t);

    if (group_name == NULL) {
        for (index = 0; index < max; index++) {
            user_data_item_operate_array[index].reset_default_value();
        }
    } else {
        for (index = 0; index < max; index++) {
            if (memcmp(user_data_item_operate_array[index].group_name, group_name,
                       strlen(user_data_item_operate_array[index].group_name)) == 0) {
                user_data_item_operate_array[index].reset_default_value();
                break;
            }
        }
    }
}

void user_data_item_show_value(char *group_name)
{
    uint32_t index;
    uint32_t max = sizeof(user_data_item_operate_array) / sizeof(user_data_item_operate_t);

    if (group_name == NULL) {
        for (index = 0; index < max; index++) {
            user_data_item_operate_array[index].show_value();
        }
    } else {
        for (index = 0; index < max; index++) {
            if (memcmp(user_data_item_operate_array[index].group_name, group_name,
                       strlen(user_data_item_operate_array[index].group_name)) == 0) {
                user_data_item_operate_array[index].show_value();
                break;
            }
        }
    }
}

#ifdef __ICCARM__
#define STRCPY strncpy
#else
#define STRCPY strlcpy
#endif

static void save_wep_key_length(uint8_t *length, char *wep_key_len, uint8_t key_id)
{
    uint8_t id = 0;
    uint8_t index = 0;

    do {
        if ('\0' == wep_key_len[index]) {
            LOG_E(wifi, "index not found");
            return;
        }
        if (key_id == id) {
            *length = (uint8_t)atoi(&wep_key_len[index]);
            return;
        }
        if (',' == wep_key_len[index++]) {
            id++;
        }
    } while (id < 4);
    LOG_E(wifi, "index not found: %d", key_id);
}

static void save_shared_key(uint8_t *wep_key, uint8_t *raw_wep_key, uint8_t length, uint8_t key_id)
{
    uint8_t id = 0;
    uint8_t index = 0;

    do {
        if ('\0' == raw_wep_key[index]) {
            LOG_E(wifi, "index not found");
            return;
        }
        if (key_id == id) {
            memcpy(wep_key, &raw_wep_key[index], length);
            wep_key[length] = '\0';
            LOG_E(wifi, "obtained wep key: %s", wep_key);
            return;
        }
        if (',' == raw_wep_key[index++]) {
            id++;
        }
    } while (id < 4);
    LOG_E(wifi, "index not found: %d", key_id);
}

int32_t wifi_config_init(wifi_cfg_t *wifi_config)
{
#ifdef MTK_WIFI_PROFILE_ENABLE

    // init wifi profile
    uint8_t buff[PROFILE_BUF_LEN];
    uint32_t len = sizeof(buff);

    // common
    len = sizeof(buff);
    nvdm_read_data_item("common", "OpMode", buff, &len);
    wifi_config->opmode = (uint8_t)atoi((char *)buff);
    len = sizeof(buff);
    nvdm_read_data_item("common", "CountryCode", buff, &len);
    memcpy(wifi_config->country_code, buff, len);

    // STA
    len = sizeof(buff);
    nvdm_read_data_item("STA", "SsidLen", buff, &len);
    wifi_config->sta_ssid_len = (uint8_t)atoi((char *)buff);
    len = sizeof(buff);
    nvdm_read_data_item("STA", "Ssid", buff, &len);
    memcpy(wifi_config->sta_ssid, buff, wifi_config->sta_ssid_len);

    len = sizeof(buff);
    nvdm_read_data_item("STA", "EncrypType", buff, &len);
    if (WIFI_ENCRYPT_TYPE_WEP_ENABLED == (uint8_t)atoi((char *)buff)) {
        len = sizeof(buff);
        nvdm_read_data_item("STA", "DefaultKeyId", buff, &len);
        wifi_config->sta_default_key_id = (uint8_t)atoi((char *)buff);

        len = sizeof(buff);
        nvdm_read_data_item("STA", "SharedKeyLen", buff, &len);
        save_wep_key_length(&wifi_config->sta_wpa_psk_len, (char *)buff, wifi_config->sta_default_key_id);

        len = sizeof(buff);
        nvdm_read_data_item("STA", "SharedKey", buff, &len);
        save_shared_key(wifi_config->sta_wpa_psk, buff, wifi_config->sta_wpa_psk_len, wifi_config->sta_default_key_id);
    } else {
        len = sizeof(buff);
        nvdm_read_data_item("STA", "WpaPskLen", buff, &len);
        wifi_config->sta_wpa_psk_len = (uint8_t)atoi((char *)buff);
        len = sizeof(buff);
        nvdm_read_data_item("STA", "WpaPsk", buff, &len);
        memcpy(wifi_config->sta_wpa_psk, buff, wifi_config->sta_wpa_psk_len);
    }
    len = sizeof(buff);
    nvdm_read_data_item("STA", "BW", buff, &len);
    wifi_config->sta_bandwidth = (uint8_t)atoi((char *)buff);
    len = sizeof(buff);
    nvdm_read_data_item("STA", "WirelessMode", buff, &len);
    wifi_config->sta_wireless_mode = (uint8_t)atoi((char *)buff);
    len = sizeof(buff);
    nvdm_read_data_item("STA", "ListenInterval", buff, &len);
    wifi_config->sta_listen_interval = (uint8_t)atoi((char *)buff);
    len = sizeof(buff);
    nvdm_read_data_item("STA", "PSMode", buff, &len);
    wifi_config->sta_power_save_mode = (uint8_t)atoi((char *)buff);

    // AP
#ifdef MTK_WIFI_REPEATER_ENABLE
    if (wifi_config->opmode == WIFI_MODE_REPEATER) {
        len = sizeof(buff);
        nvdm_read_data_item("STA", "Channel", buff, &len);
        wifi_config->ap_channel = (uint8_t)atoi((char *)buff);
        len = sizeof(buff);
        nvdm_read_data_item("STA", "BW", buff, &len);
        wifi_config->ap_bw = (uint8_t)atoi((char *)buff);
    } else {
#endif
        /* Use STA MAC/IP as AP MAC/IP for the time being, due to N9 dual interface not ready yet */
        len = sizeof(buff);
        nvdm_read_data_item("AP", "Channel", buff, &len);
        wifi_config->ap_channel = (uint8_t)atoi((char *)buff);
        len = sizeof(buff);
        nvdm_read_data_item("AP", "BW", buff, &len);
        wifi_config->ap_bw = (uint8_t)atoi((char *)buff);
#ifdef MTK_WIFI_REPEATER_ENABLE
    }
#endif /* MTK_WIFI_REPEATER_ENABLE */
    len = sizeof(buff);
    nvdm_read_data_item("AP", "SsidLen", buff, &len);
    wifi_config->ap_ssid_len = (uint8_t)atoi((char *)buff);
    len = sizeof(buff);
    nvdm_read_data_item("AP", "Ssid", buff, &len);
    memcpy(wifi_config->ap_ssid, buff, wifi_config->ap_ssid_len);
    len = sizeof(buff);
    nvdm_read_data_item("AP", "HideSSID", buff, &len);
    wifi_config->ap_hide_ssid = (uint8_t)atoi((char *)buff);
    len = sizeof(buff);
    nvdm_read_data_item("AP", "AuthMode", buff, &len);
    wifi_config->ap_auth_mode = (uint8_t)atoi((char *)buff);
    len = sizeof(buff);
    nvdm_read_data_item("AP", "EncrypType", buff, &len);
    wifi_config->ap_encryp_type = (uint8_t)atoi((char *)buff);

    if (WIFI_ENCRYPT_TYPE_WEP_ENABLED == wifi_config->ap_encryp_type) {
        len = sizeof(buff);
        nvdm_read_data_item("AP", "DefaultKeyId", buff, &len);
        wifi_config->ap_default_key_id = (uint8_t)atoi((char *)buff);

        len = sizeof(buff);
        nvdm_read_data_item("AP", "SharedKeyLen", buff, &len);
        save_wep_key_length(&wifi_config->ap_wpa_psk_len, (char *)buff, wifi_config->ap_default_key_id);

        len = sizeof(buff);
        nvdm_read_data_item("AP", "SharedKey", buff, &len);
        save_shared_key(wifi_config->ap_wpa_psk, buff, wifi_config->ap_wpa_psk_len, wifi_config->ap_default_key_id);
    } else {
        len = sizeof(buff);
        nvdm_read_data_item("AP", "WpaPskLen", buff, &len);
        wifi_config->ap_wpa_psk_len = (uint8_t)atoi((char *)buff);
        len = sizeof(buff);
        nvdm_read_data_item("AP", "WpaPsk", buff, &len);
        memcpy(wifi_config->ap_wpa_psk, buff, wifi_config->ap_wpa_psk_len);
    }
    len = sizeof(buff);
    nvdm_read_data_item("AP", "WirelessMode", buff, &len);
    wifi_config->ap_wireless_mode = (uint8_t)atoi((char *)buff);
    len = sizeof(buff);
    nvdm_read_data_item("AP", "DtimPeriod", buff, &len);
    wifi_config->ap_dtim_interval = (uint8_t)atoi((char *)buff);

#else
    //wifi profile is disabled, take the user

#endif
    return 0;
}

int32_t dhcp_config_init(void)
{
    uint8_t buff[PROFILE_BUF_LEN] = {0};
    uint32_t sz = sizeof(buff);

    nvdm_read_data_item("STA", "IpMode", buff, &sz);
    return strcmp((char *)buff, "dhcp") ? STA_IP_MODE_STATIC : STA_IP_MODE_DHCP;
}

int32_t tcpip_config_init(lwip_tcpip_config_t *tcpip_config)
{
    uint8_t ip_addr[4] = {0};
    uint8_t buff[PROFILE_BUF_LEN] = {0};
    uint32_t sz = sizeof(buff);

    nvdm_read_data_item("common", "IpAddr", buff, &sz);
    wifi_conf_get_ip_from_str(ip_addr, (char *)buff);
    IP4_ADDR(&tcpip_config->sta_addr, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
    IP4_ADDR(&tcpip_config->ap_addr, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
    sz = sizeof(buff);
    nvdm_read_data_item("common", "IpNetmask", buff, &sz);
    wifi_conf_get_ip_from_str(ip_addr, (char *)buff);
    IP4_ADDR(&tcpip_config->sta_mask, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
    IP4_ADDR(&tcpip_config->ap_mask, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
    sz = sizeof(buff);
    nvdm_read_data_item("common", "IpGateway", buff, &sz);
    wifi_conf_get_ip_from_str(ip_addr, (char *)buff);
    IP4_ADDR(&tcpip_config->sta_gateway, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
    IP4_ADDR(&tcpip_config->ap_gateway, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
    return 0;
}

static uint32_t ip_number_to_big_endian(uint32_t ip_number)
{
    uint8_t *byte = (uint8_t *)&ip_number;
    return (uint32_t)((byte[0] << 24) | (byte[1] << 16) | (byte[2] << 8) | byte[3]);
}

static void ip_number_to_string(uint32_t ip_number, char ip_string[IP4ADDR_STRLEN_MAX])
{
    snprintf(ip_string,
                IP4ADDR_STRLEN_MAX,
                "%d.%d.%d.%d",
                (uint8_t)((ip_number >> 24) & 0xFF),
                (uint8_t)((ip_number >> 16) & 0xFF),
                (uint8_t)((ip_number >> 8) & 0xFF),
                (uint8_t)((ip_number >> 0) & 0xFF));
}

static void dhcpd_set_ip_pool(const ip4_addr_t *ap_ip_addr,
                              const ip4_addr_t *ap_net_mask,
                              char ip_pool_start[IP4ADDR_STRLEN_MAX],
                              char ip_pool_end[IP4ADDR_STRLEN_MAX])
{
    uint32_t ap_ip_number = ip_number_to_big_endian(ip4_addr_get_u32(ap_ip_addr));
    uint32_t ap_mask_number = ip_number_to_big_endian(ip4_addr_get_u32(ap_net_mask));
    uint32_t ip_range_min = ap_ip_number & ap_mask_number;
    uint32_t ip_range_max = ip_range_min | (~ap_mask_number);

    if ((ip_range_max - ap_ip_number) > (ap_ip_number - ip_range_min)) {
        ip_number_to_string(ap_ip_number + 1, ip_pool_start);
        ip_number_to_string(ip_range_max - 1, ip_pool_end);
    } else {
        ip_number_to_string(ip_range_min + 1, ip_pool_start);
        ip_number_to_string(ap_ip_number - 1, ip_pool_end);
    }
}

void dhcpd_settings_init(const lwip_tcpip_config_t *tcpip_config,
                                dhcpd_settings_t *dhcpd_settings)
{
    STRCPY(dhcpd_settings->dhcpd_server_address,
               ip4addr_ntoa(&tcpip_config->ap_addr),
               IP4ADDR_STRLEN_MAX);

    STRCPY(dhcpd_settings->dhcpd_netmask,
               ip4addr_ntoa(&tcpip_config->ap_mask),
               IP4ADDR_STRLEN_MAX);

    STRCPY(dhcpd_settings->dhcpd_gateway,
               (char *)dhcpd_settings->dhcpd_server_address,
               IP4ADDR_STRLEN_MAX);

    STRCPY(dhcpd_settings->dhcpd_primary_dns,
               (char *)dhcpd_settings->dhcpd_server_address,
               IP4ADDR_STRLEN_MAX);

    /* secondary DNS is not defined by default */
    STRCPY(dhcpd_settings->dhcpd_secondary_dns,
               "0.0.0.0",
               IP4ADDR_STRLEN_MAX);

    dhcpd_set_ip_pool(&tcpip_config->ap_addr,
                      &tcpip_config->ap_mask,
                      dhcpd_settings->dhcpd_ip_pool_start,
                      dhcpd_settings->dhcpd_ip_pool_end);
}

int32_t wifi_init_done_handler(wifi_event_t event,
                                      uint8_t *payload,
                                      uint32_t length)
{
    LOG_I(common, "WiFi Init Done: port = %d", payload[6]);
    return 1;
}

#if 0 /* WIFI_EVENT_IOT_CONNECTION_FAILED event is not ready yet, turn it off by default.*/
static int32_t wifi_station_connect_fail_event_handler(wifi_event_t event,
                                                       uint8_t *payload,
                                                       uint32_t length)
{
    uint8_t *port = payload;
    uint8_t *reason_code = payload + 1;
    LOG_E(wifi,"reason code[port %d]: %d", *port,reason_code[0] + reason_code[1]*256);
    return 0;
}
#endif


#ifdef MTK_WIFI_CONFIGURE_FREE_ENABLE
extern int32_t mtk_smart_connect(void);

int32_t cf_set_ssid(uint8_t port, uint8_t *ssid , uint8_t ssid_length)
{
    if (!wifi_is_port_valid(port)) {
        LOG_E(wifi, "port is invalid: %d", port);
        return WIFI_ERR_PARA_INVALID;
    }
    if (ssid_length > WIFI_MAX_LENGTH_OF_SSID) {
        LOG_I(wifi, "incorrect length(=%d)", ssid_length);
        return WIFI_ERR_PARA_INVALID;
    }
    if (NULL == ssid) {
        LOG_E(wifi, "ssid is null.");
        return WIFI_ERR_PARA_INVALID;
    }

    char ssid_buf[WIFI_PROFILE_BUFFER_LENGTH] = {0};
    char ssid_len_buf[WIFI_PROFILE_BUFFER_LENGTH] = {0};

    memcpy(ssid_buf, ssid, ssid_length);
    ssid_buf[ssid_length] = '\0';

    sprintf(ssid_len_buf, "%d", ssid_length);
    if (port == WIFI_PORT_AP) {
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_AP, "Ssid",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)ssid_buf, strlen(ssid_buf))) {
            return WIFI_FAIL;
        }
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_AP, "SsidLen",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)ssid_len_buf, strlen(ssid_len_buf))) {
            return WIFI_FAIL;
        }
    } else {
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_STA, "Ssid",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)ssid_buf, strlen(ssid_buf))) {
            return WIFI_FAIL;
        }
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_STA, "SsidLen",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)ssid_len_buf, strlen(ssid_len_buf))) {
            return WIFI_FAIL;
        }
    }
    return WIFI_SUCC;
}

int32_t cf_set_wpa_psk_key(uint8_t port, uint8_t *passphrase, uint8_t passphrase_length)
{
    char pass_buf[WIFI_PROFILE_BUFFER_LENGTH] = {0};
    char pass_len_buf[WIFI_PROFILE_BUFFER_LENGTH] = {0};

    if (!wifi_is_port_valid(port)) {
        LOG_E(wifi, "port is invalid: %d", port);
        return WIFI_ERR_PARA_INVALID;
    }
    if (passphrase == NULL) {
        LOG_E(wifi, "passphrase is null.");
        return WIFI_ERR_PARA_INVALID;
    }
    if ((passphrase_length < 8) || (passphrase_length > WIFI_LENGTH_PASSPHRASE)) {
        LOG_E(wifi, "incorrect length(=%d)", passphrase_length);
        return WIFI_ERR_PARA_INVALID;
    }
    if (passphrase_length == WIFI_LENGTH_PASSPHRASE) {
        for (uint8_t index = 0; index < WIFI_LENGTH_PASSPHRASE; index++) {
            if (!hex_isdigit(passphrase[index])) {
                LOG_E(wifi, "length(=%d) but the strings are not hex strings!", passphrase_length);
                return WIFI_ERR_PARA_INVALID;
            }
        }
    }

    sprintf(pass_len_buf, "%d", passphrase_length);
    memcpy(pass_buf, passphrase, passphrase_length);
    pass_buf[passphrase_length] = '\0';

    if (port == WIFI_PORT_AP) {
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_AP, "WpaPskLen",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)pass_len_buf, strlen(pass_len_buf))) {
            return WIFI_FAIL;
        }
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_AP, "WpaPsk",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)pass_buf, strlen(pass_buf))) {
            return WIFI_FAIL;
        }
    } else {
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_STA, "WpaPskLen",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)pass_len_buf, strlen(pass_len_buf))) {
            return WIFI_FAIL;
        }
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_STA, "WpaPsk",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)pass_buf, strlen(pass_buf))) {
            return WIFI_FAIL;
        }
    }
    return WIFI_SUCC;
}
int32_t cf_set_security_mode(uint8_t port, wifi_auth_mode_t auth_mode, wifi_encrypt_type_t encrypt_type)
{
    char auth_buf[WIFI_PROFILE_BUFFER_LENGTH] = {0};
    char encrypt_buf[WIFI_PROFILE_BUFFER_LENGTH] = {0};

    if (!wifi_is_port_valid(port)) {
        LOG_E(wifi, "port is invalid: %d", port);
        return WIFI_ERR_PARA_INVALID;
    }
    if (!wifi_is_auth_mode_valid(auth_mode)) {
        LOG_E(wifi, "auth_mode is invalid: %d", auth_mode);
        return WIFI_ERR_PARA_INVALID;
    }
    if (!wifi_is_encrypt_type_valid(encrypt_type)) {
        LOG_E(wifi, "encrypt_type is invalid: %d", encrypt_type);
        return WIFI_ERR_PARA_INVALID;
    }

    sprintf(auth_buf, "%d", auth_mode);
    sprintf(encrypt_buf, "%d", encrypt_type);

    if (port == WIFI_PORT_AP) {
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_AP, "AuthMode",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)auth_buf, strlen(auth_buf))) {
            return WIFI_FAIL;
        }
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_AP, "EncrypType",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)encrypt_buf, strlen(encrypt_buf))) {
            return WIFI_FAIL;
        }
    } else {
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_STA, "AuthMode",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)auth_buf, strlen(auth_buf))) {
            return WIFI_FAIL;
        }
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_STA, "EncrypType",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)encrypt_buf, strlen(encrypt_buf))) {
            return WIFI_FAIL;
        }
    }
    return WIFI_SUCC;
}

int32_t cf_set_channel(uint8_t port, uint8_t channel)
{
    char buf[WIFI_PROFILE_BUFFER_LENGTH] = {0};

    sprintf(buf, "%d", channel);

    if (port == WIFI_PORT_AP) {
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_AP, "Channel",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)buf, strlen(buf))) {
            return WIFI_FAIL;
        }
    } else if (port == WIFI_PORT_STA) {
        if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_STA, "Channel",
                NVDM_DATA_ITEM_TYPE_STRING,
                (uint8_t *)buf, strlen(buf))) {
            return WIFI_FAIL;
        }
    } else {
        LOG_E(wifi, "port is invalid: %d", port);
        return WIFI_ERR_PARA_INVALID;
    }
    return WIFI_SUCC;
}
int save_cf_credential_to_nvdm(P_IOT_CONFIG_FREE_IE cred)
{
    int status = 0;

    if (cred == NULL) {
        LOG_E(wifi, "ERROR! invalid cred pointer(NULL)");
        return -1;
    } else {
        LOG_E(wifi, "[ConfigFree] cred: Ssid = %s, SsidLen = %d, AuthMode = %d, EncrypType = %d, WpaPsk = %s, WpaPskLen = %d, Ch = %d",
              cred->Ssid,
              cred->SsidLen,
              cred->AuthMode,
              cred->EncrypType,
              cred->WpaPsk,
              cred->WpaPskLen,
              cred->Channel);
    }

    // Set NVRAM STA configuration by credential
    if (cf_set_ssid(WIFI_PORT_STA, cred->Ssid, cred->SsidLen) != 0) {
        LOG_E(wifi, "ERROR! [ConfigFree][STA] wifi_profile_set_ssid failed (Ssid=%s, Len=%d)", cred->Ssid, cred->SsidLen);
        status = -1;
    }

    if (cf_set_security_mode(WIFI_PORT_STA, cred->AuthMode, cred->EncrypType) != 0) {
        LOG_E(wifi, "ERROR! [ConfigFree][STA] wifi_profile_set_security_mode failed (Auth=%d, Encry=%d)", cred->AuthMode, cred->EncrypType);
        status = -1;
    }

    if (cf_set_wpa_psk_key(WIFI_PORT_STA, cred->WpaPsk, cred->WpaPskLen) != 0) {
        LOG_E(wifi, "ERROR! [ConfigFree][STA] wifi_profile_set_wpa_psk_key failed (WpaPsk=%s, Len=%d)", cred->WpaPsk, cred->WpaPskLen);
        status = -1;
    }

    // Set NVRAM AP configuration by credential
    if (cf_set_ssid(WIFI_PORT_AP, cred->Ssid, cred->SsidLen) != 0) {
        LOG_E(wifi, "ERROR! [ConfigFree][AP] wifi_profile_set_ssid failed (Ssid=%s, Len=%d)", cred->Ssid, cred->SsidLen);
        status = -1;
    }

    if (cf_set_security_mode(WIFI_PORT_AP, cred->AuthMode, cred->EncrypType) != 0) {
        LOG_E(wifi, "ERROR! [ConfigFree][AP] wifi_profile_set_security_mode failed (Auth=%d, Encry=%d)", cred->AuthMode, cred->EncrypType);
        status = -1;
    }

    if (cf_set_channel(WIFI_PORT_AP, cred->Channel) != 0) {
        LOG_E(wifi, "ERROR! [ConfigFree][AP] wifi_profile_set_channel failed (Ch=%d)", cred->Channel);
        status = -1;
    }

    return status;
}

int32_t save_cf_ready_to_nvdm(uint8_t config_ready)
{
    char buf[WIFI_PROFILE_BUFFER_LENGTH] = {0};

    if (0 != config_ready && 1 != config_ready) {
        LOG_E(wifi, "config_ready is invalid: %d", config_ready);
        return WIFI_ERR_PARA_INVALID;
    }

    sprintf(buf, "%d", WIFI_MODE_REPEATER);
    LOG_I(wifi, "wifi_profile_set_opmode: opmode=%s", buf);

    if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_COMMON, "OpMode",
            NVDM_DATA_ITEM_TYPE_STRING,
            (uint8_t *)buf, strlen(buf))) {
        return WIFI_FAIL;
    }

    memset(buf, 0x0, WIFI_PROFILE_BUFFER_LENGTH);

    sprintf(buf, "%d", config_ready);
    LOG_I(wifi, "ConfigFree ready: %s", buf);

    if (NVDM_STATUS_OK != nvdm_write_data_item(WIFI_PROFILE_BUFFER_COMMON, "ConfigFree_Ready",
            NVDM_DATA_ITEM_TYPE_STRING,
            (uint8_t *)buf, strlen(buf))) {
        return WIFI_FAIL;
    }

    return WIFI_SUCC;
}

int32_t get_cf_ready_to_nvdm(uint8_t *config_ready)
{
    char buf[WIFI_PROFILE_BUFFER_LENGTH] = {0};
    uint32_t len = sizeof(buf);

    if (NULL == config_ready) {
        LOG_E(wifi, "config_ready is null");
        return WIFI_ERR_PARA_INVALID;
    }

    if (NVDM_STATUS_OK != nvdm_read_data_item(WIFI_PROFILE_BUFFER_COMMON, "ConfigFree_Ready", (uint8_t *)buf, &len)) {
        return WIFI_FAIL;
    }

    *config_ready = atoi(buf);
    return WIFI_SUCC;
}
#endif /* MTK_WIFI_CONFIGURE_FREE_ENABLE */

