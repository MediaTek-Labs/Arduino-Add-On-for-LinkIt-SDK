# Copyright Statement:
#
# (C) 2005-2016  MediaTek Inc. All rights reserved.
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
# Without the prior written permission of MediaTek and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
# You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
# if you have agreed to and been bound by the applicable license agreement with
# MediaTek ("License Agreement") and been granted explicit permission to do so within
# the License Agreement ("Permitted User").  If you are not a Permitted User,
# please cease any access or use of MediaTek Software immediately.
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
# ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
# WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#

##############################################################################
#
# Project settings
#
##############################################################################


SOURCE_DIR  = ../../../../..
PWD         = $(shell pwd)
SDK_PATH    = $(abspath $(PWD)/$(SOURCE_DIR))
DATIME      = $(shell date --iso=seconds)
V          ?= 0

##############################################################################
#
# build log output
#
##############################################################################


ifeq ($(V),0)
Q           = @
else
Q           =
endif


##############################################################################
#
# config files
#
##############################################################################


FEATURE    ?= feature.mk
include $(FEATURE)

# Gloabl Config
-include $(SOURCE_DIR)/.config
# IC Config
-include $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk
# Board Config
-include $(SOURCE_DIR)/config/board/$(BOARD_CONFIG)/board.mk
# HAL driver files
include $(SOURCE_DIR)/driver/chip/mt7687/module.mk
# EPT Config
-include $(SOURCE_DIR)/driver/board/mt76x7_hdk/ept/module.mk
# Minisupp Config
-include $(SOURCE_DIR)/middleware/MTK/minisupp/module.mk

ifeq ($(MTK_SMTCN_ENABLE),y)
-include $(SOURCE_DIR)/middleware/MTK/smtcn/module.mk
endif

ifeq ($(MTK_CLI_EXAMPLE_MODE_ENABLE),y)
CFLAGS         += -DMTK_CLI_EXAMPLE_MODE_ENABLE
endif


ifeq ($(MTK_FOTA_DUAL_IMAGE_ENABLE),y)
export MTK_FOTA_DUAL_IMAGE_ENABLE
FOTA_PARTITION_B_BUILD    ?= n
ifeq ($(FOTA_PARTITION_B_BUILD),y)

CFLAGS += -DFOTA_PARTITION_B_BUILD
endif
PARTITION_B_EXTRA ?= FOTA_PARTITION_B_BUILD=y BUILD_DIR=$(BUILD_DIR)/binary_B OUTPATH=$(OUTPATH)/binary_B PROJ_PATH=$(PROJ_PATH)
endif
RAM_BOOTING=0

DEBUG = 0
FLOAT_TYPE = hard
BUILD_DIR = $(PWD)/Build

# Project name
PROJ_NAME       = $(notdir $(abspath $(dir $(PWD))))
PROJ_PATH = $(PWD)
OUTPATH = $(PWD)/Build

# Main APP files
APP_PATH        = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))
APP_PATH_SRC = $(APP_PATH)/src
BQB_PATH_SRC = $(SOURCE_DIR)/$(APP_PATH)/src_protected/bqb

# bqb files
ifneq ($(wildcard $(BQB_PATH_SRC)),)
include $(BQB_PATH_SRC)/module.mk
endif

#include cjson
include $(SOURCE_DIR)/middleware/third_party/cjson/module.mk

#include xml
include $(SOURCE_DIR)/middleware/third_party/xml/module.mk

#include lwip
include $(SOURCE_DIR)/middleware/third_party/lwip/module.mk

#include ping
include $(SOURCE_DIR)/middleware/third_party/ping/module.mk

#include iperf
include $(SOURCE_DIR)/middleware/MTK/iperf/module.mk

# FOTA files
include $(SOURCE_DIR)/middleware/MTK/fota/module.mk

# lzma decodder files
include $(SOURCE_DIR)/middleware/third_party/lzma_decoder/module.mk

# httpclient
include $(SOURCE_DIR)/middleware/third_party/httpclient/module.mk

#include mbedtls
include $(SOURCE_DIR)/middleware/third_party/mbedtls/module.mk

#include dhcpd
include $(SOURCE_DIR)/middleware/third_party/dhcpd/module.mk

#include tftp
include $(SOURCE_DIR)/middleware/MTK/tftp/module.mk

# wifi src
include $(SOURCE_DIR)/driver/board/mt76x7_hdk/wifi/module.mk

APP_FILES       = $(APP_PATH_SRC)/main.c     \
                  $(APP_PATH_SRC)/sys_init.c \
                  $(APP_PATH_SRC)/wifi_lwip_helper.c \
                  $(APP_PATH_SRC)/wifi_nvdm_config.c  \
                  $(APP_PATH_SRC)/hci_log.c  \
                  $(APP_PATH_SRC)/bt_init.c \
                  $(APP_PATH_SRC)/ut_app/ut_app.c \
                  $(APP_PATH_SRC)/ut_app/ut_app_gap.c \
                  $(APP_PATH_SRC)/ut_app/connection_info.c \
                  $(APP_PATH_SRC)/ut_app/gatt_service.c \
                  $(APP_PATH_SRC)/ut_app/gatt_command.c \
                  $(APP_PATH)/GCC/syscalls.c \
                  driver/board/mt76x7_hdk/util/src/io_def.c

CFLAGS += -D__BT_DEBUG__

SYS_FILES       = $(APP_PATH_SRC)/system_mt7687.c

# sky added for mcs support
ifeq ($(MTK_MCS_ENABLE),y)
APP_FILES       += $(APP_PATH_SRC)/mcs.c
endif

ifeq ($(MTK_MINICLI_ENABLE),y)
APP_FILES       += $(APP_PATH_SRC)/cli_def.c
APP_FILES       += $(APP_PATH_SRC)/cli_cmds.c
endif

ifeq ($(MTK_HOMEKIT_ENABLE),y)
CFLAGS          += -I$(SOURCE_DIR)/middleware/MTK/homekit/inc
endif

C_FILES += $(APP_FILES)
C_FILES += $(APP_PATH_SRC)/system_mt7687.c

# add startup file to build
S_FILES += $(APP_PATH)/GCC/startup_mt7687.s

#CFLAGS += -DMTK_BT_LWIP_ENABLE
#C_FILES += $(APP_PATH_SRC)/ut_app/bt_lwip.c

ifeq ($(MTK_BLE_SMTCN_ENABLE),y)
C_FILES += $(APP_PATH_SRC)/ble_smtcn.c
endif

# add ept file to build
C_FILES += $(APP_PATH_SRC)/ept_gpio_var.c
C_FILES += $(APP_PATH_SRC)/ept_eint_var.c

#include BLE DEMO files

CFLAGS += -DMTK_FW_VERSION=\"$(MTK_FW_VERSION)\"

C_OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)
S_OBJS = $(S_FILES:%.s=$(BUILD_DIR)/%.o)



###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/mt7687/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/inc
# BLE include path
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/src/ut_app
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/bluetooth/inc

ifeq ($(MTK_BLE_SMTCN_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/ble_smtcn/inc
endif

## It should be in config\chip\mt7697 chip.mk but we want to seperate it from there to project.
## We can just only release project to customer and do not release other file.
##
## MTK_MCS_ENABLE
## Brief:       This option is to enable MCS feature.
## Usage:       Enable the feature by configuring it as y.
##              project/mt7697_hdk/apps/iot_sdk_demo/inc/mcs.h
##              project/mt7697_hdk/apps/iot_sdk_demo/inc/mcs.c
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(MTK_MCS_ENABLE),y)
CFLAGS += -DMTK_MCS_ENABLE
endif

##
## MTK_MCS_ENABLE
## Brief:       This option is to enable BLE GPIO service feature.
## Usage:       Enable the feature by configuring it as y.
##              project/mt7697_hdk/apps/iot_sdk_demo/ut_app/bt_gattc_connect.c
##              project/mt7697_hdk/apps/iot_sdk_demo/ut_app/bt_gattc_handle_op.c
##              project/mt7697_hdk/apps/iot_sdk_demo/ut_app/bt_gattc_srv.c
##              project/mt7697_hdk/apps/iot_sdk_demo/ut_app/ble_gpio_service.c
##              project/mt7697_hdk/apps/iot_sdk_demo/ut_app/ble_gpio_client.c
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(MTK_BLE_GPIO_SERVICE),y)
C_FILES += $(APP_PATH_SRC)/ut_app/bt_gattc_connect.c
C_FILES += $(APP_PATH_SRC)/ut_app/bt_gattc_handle_op.c
C_FILES += $(APP_PATH_SRC)/ut_app/bt_gattc_srv.c
C_FILES += $(APP_PATH_SRC)/ut_app/ble_gpio_service.c
C_FILES += $(APP_PATH_SRC)/ut_app/ble_gpio_client.c
CFLAGS += -DMTK_BLE_GPIO_SERVICE
endif

CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/lwip/src/ports
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/lwip/src/ports/include
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/lwip/src/lwip-master/src/include

CFLAGS += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/ept/inc

###################################################
# LD Flags
LDFLAGS = $(ALLFLAGS) $(FPUFLAGS) --specs=nano.specs -lnosys -nostartfiles
LDFLAGS += -Wl,-wrap=malloc -Wl,-wrap=calloc -Wl,-wrap=realloc -Wl,-wrap=free
ifeq ($(RAM_BOOTING), 1)
LDFLAGS += -Wl,-Tmt7687_sram.ld -Wl,--gc-sections
else

ifeq ($(MTK_FOTA_DUAL_IMAGE_ENABLE),y)
ifeq ($(FOTA_PARTITION_B_BUILD),y)
LDFLAGS += -Wl,-Tmt7687_flash_B.ld -Wl,--gc-sections
else
LDFLAGS += -Wl,-Tmt7687_flash_A.ld -Wl,--gc-sections
endif

else
LDFLAGS += -Wl,-Tmt7687_flash.ld -Wl,--gc-sections
endif

endif

ifneq ($(MTK_DEBUG_LEVEL), none)
LDFLAGS += -Wl,-wrap=printf
endif


##############################################################################
#
# Libraries
#
##############################################################################

include $(SOURCE_DIR)/project/common/bsp_ex/module.mk
include $(SOURCE_DIR)/middleware/MTK/minicli/module.mk
# NVDM files
include $(SOURCE_DIR)/middleware/MTK/nvdm/module.mk
include $(SOURCE_DIR)/middleware/third_party/sntp/module.mk
include $(SOURCE_DIR)/middleware/third_party/mqtt/module.mk
include $(SOURCE_DIR)/middleware/third_party/nghttp2/module.mk
# bluetooth files
include $(SOURCE_DIR)/middleware/MTK/bluetooth/module.mk

include $(SOURCE_DIR)/kernel/rtos/FreeRTOS/module.mk
include $(SOURCE_DIR)/kernel/service/module.mk

ifeq ($(MTK_HOMEKIT_ENABLE),y)
include $(SOURCE_DIR)/middleware/MTK/homekit/module.mk
include $(SOURCE_DIR)/middleware/third_party/mDNSResponder/module.mk
include $(SOURCE_DIR)/middleware/third_party/curve25519/module.mk
include $(SOURCE_DIR)/middleware/third_party/ed25519/module.mk
include $(SOURCE_DIR)/middleware/third_party/chacha20poly1305/module.mk
include $(SOURCE_DIR)/middleware/third_party/srp/module.mk
endif

###################################################
# Rules
.PHONY: proj clean $(MODULE_PATH)

all: cleanlog proj
	$(Q)echo $(OUTPATH)
	$(Q)mkdir -p $(BUILD_DIR)
	$(Q)$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf
	$(Q)echo "Generate Assembly from elf:"
	#$(Q)$(OBJDUMP) -S $(OUTPATH)/$(PROJ_NAME).elf > $(OUTPATH)/$(PROJ_NAME).elf.s
ifneq ($(MTK_FOTA_DUAL_IMAGE_ENABLE),y)
	$(Q)$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin $(PWD)
else
	$(Q)$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin $(PWD)
ifneq ($(FOTA_PARTITION_B_BUILD),y)
else
	export FOTA_PARTITION_B_BUILD=y
	$(Q)$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin $(PWD)
endif
endif
	$(Q)cp *.cmm $(OUTPATH)/
ifeq ($(MTK_FOTA_DUAL_IMAGE_ENABLE),y)
ifneq ($(FOTA_PARTITION_B_BUILD),y)
	$(Q)echo "build partition B binary."
	$(Q)mkdir -p $(BUILD_DIR)/binary_B
	$(Q)mkdir -p $(OUTPATH)/binary_B
	$(Q)make -C $(PWD) $(PARTITION_B_EXTRA) $($@_EXTRA)
	$(Q)./hash_gen.sh $(OUTPATH) $(PROJ_NAME).bin
endif
endif


MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_PATH)

$(LIBS): $(MODULE_PATH)

$(MODULE_PATH):
ifeq ($(MTK_HOMEKIT_ENABLE),y)
	@if [ ! -d "$(MID_HOMEKIT_PATH)" ]; then echo "\n####################################################################################################\nBuild Error: there isn't homekit folder in your codebase.\nYou should apply to the MTK for the related files of homekit. \n####################################################################################################\n"; exit 1;fi
endif
	@+make -C $@ $(MOD_EXTRA) $($@_EXTRA)


proj: $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(C_OBJS) $(CXX_OBJS) $(S_OBJS) $(LIBS)
	@echo Linking...
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).map" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).map"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).hex" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).hex"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).bin" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).bin"; fi
	@echo $(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%, $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group -Wl,-Map=$(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%,$(OUTPATH)/$(PROJ_NAME).map) -lm -o $@) > $(OUTPATH)/link_option.tmp
	@$(CC) @$(OUTPATH)/link_option.tmp 2>>$(ERR_LOG)
	@rm -f $(OUTPATH)/link_option.tmp
	@$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	@$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin
	@echo Done

include $(SOURCE_DIR)/.rule.mk

clean:
	rm -rf $(OUTPATH)


print-%  : ; @echo $* = $($*)
