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

SOURCE_DIR = ../../../../..

PWD= $(shell pwd)
SDK_PATH    = $(abspath $(PWD)/$(SOURCE_DIR))
FEATURE ?= feature.mk
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

RAM_BOOTING=0

DEBUG = 0
FLOAT_TYPE = hard
BUILD_DIR = $(PWD)/Build

# Project name
PROJ_NAME = mt7697_bootloader
PROJ_PATH = $(PWD)
OUTPATH = $(PWD)/Build

Q ?= @

include $(SOURCE_DIR)/driver/board/mt76x7_hdk/bootloader/module.mk

# FOTA files
ifeq ($(MTK_FOTA_ENABLE),y)
include $(SOURCE_DIR)/middleware/MTK/fota/module.mk
endif

# Main APP files
APP_PATH        = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))
APP_PATH_SRC = $(APP_PATH)/src



C_FILES += $(APP_PATH_SRC)/system_mt7687.c
C_FILES += $(APP_PATH_SRC)/ept_gpio_var.c
C_FILES += $(APP_PATH_SRC)/ept_eint_var.c
C_FILES += $(APP_PATH)/GCC/syscalls.c

C_OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)
S_OBJS = $(S_FILES:%.s=$(BUILD_DIR)/%.o)


CFLAGS += -DMTK_FW_VERSION=\"$(MTK_FW_VERSION)\"

ifeq ($(MTK_FOTA_ENABLE),y)
CFLAGS += -DMOD_CFG_FOTA_DISABLE_CLI
CFLAGS += -DMOD_CFG_FOTA_DISABLE_OS
else
CFLAGS += -DMOD_CFG_LOADER_DISABLE_FOTA
endif

ifeq ($(MTK_BL_LOAD_ENABLE),y)
CFLAGS  += -DBL_FOTA_DEBUG
endif

ifeq ($(MTK_BL_DEBUG_ENABLE),y)
CFLAGS += -DBL_DEBUG
endif

ifeq ($(MTK_BL_BOOT_MENU_ENABLE),y)
CFLAGS += -DMTK_BOOTLOADER_MENU_ENABLE
endif

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc
ifeq ($(MTK_BL_LOAD_ENABLE),y)
CFLAGS  += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/bootloader/src/mt7687
endif
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/ept/inc

###################################################
# LD Flags
LDFLAGS  = $(ALLFLAGS) $(FPUFLAGS) --specs=nano.specs -lnosys -nostartfiles
LDFLAGS_FLASH = $(LDFLAGS) -Wl,-Tbootloader.ld -Wl,--gc-sections
LDFLAGS_RAM   = $(LDFLAGS) -Wl,-Tbootloader-ram.ld -Wl,--gc-sections

###################################################
# Libraries

ifeq ($(MTK_FOTA_ENABLE),y)
include $(SOURCE_DIR)/middleware/third_party/lzma_decoder/module.mk
endif




###################################################
# Rules
.PHONY: clean proj $(MODULE_PATH)

all: cleanlog proj
	@mkdir -p $(BUILD_DIR)
	@$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf
	@$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin $(PWD)


MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_PATH)

$(LIBS): $(MODULE_PATH)

$(MODULE_PATH):
	@+make -C $@ $(MOD_EXTRA) $($@_EXTRA)


proj: $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME)-ram.elf: $(C_OBJS) $(CXX_OBJS) $(S_OBJS) $(LIBS)
	@echo Linking...
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME)-ram.map" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME)-ram.map"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME)-ram.hex" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME)-ram.hex"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME)-ram.bin" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME)-ram.bin"; fi
	$(Q)echo $(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%, $(LDFLAGS_RAM) -Wl,--start-group $^ -Wl,--end-group -Wl,-Map=$(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%,$(OUTPATH)/$(PROJ_NAME)-ram.map) -lm -o $@) > $(OUTPATH)/link_option.tmp
	$(Q)$(CC) @$(OUTPATH)/link_option.tmp 2>>$(ERR_LOG)
	$(Q)rm -f $(OUTPATH)/link_option.tmp
	$(Q)$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME)-ram.elf $(OUTPATH)/$(PROJ_NAME)-ram.hex
	$(Q)$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME)-ram.elf $(OUTPATH)/$(PROJ_NAME)-ram.bin
	@echo Done

$(OUTPATH)/$(PROJ_NAME).elf: $(C_OBJS) $(CXX_OBJS) $(S_OBJS) $(LIBS)
	@echo Linking...
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).map" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).map"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).hex" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).hex"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).bin" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).bin"; fi
	$(Q)echo $(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%, $(LDFLAGS_FLASH) -Wl,--start-group $^ -Wl,--end-group -Wl,-Map=$(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%,$(OUTPATH)/$(PROJ_NAME).map) -lm -o $@) > $(OUTPATH)/link_option.tmp
	$(Q)$(CC) @$(OUTPATH)/link_option.tmp 2>>$(ERR_LOG)
	$(Q)rm -f $(OUTPATH)/link_option.tmp
	$(Q)$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	$(Q)$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin
	@echo Done

include $(SOURCE_DIR)/.rule.mk

clean:
	rm -rf $(OUTPATH)

print-%  : ; @echo $* = $($*)
