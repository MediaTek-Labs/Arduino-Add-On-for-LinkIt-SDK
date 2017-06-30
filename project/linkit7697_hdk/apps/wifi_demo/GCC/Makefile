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
##############################################################################
#
# Project specific features
#
##############################################################################


ifeq ($(MTK_CLI_EXAMPLE_MODE_ENABLE),y)
CFLAGS         += -DMTK_CLI_EXAMPLE_MODE_ENABLE
endif


##############################################################################
#
# config
#
##############################################################################


RAM_BOOTING     = 0

DEBUG           = 0
FLOAT_TYPE      = hard
BUILD_DIR       = $(PWD)/Build

# Project name
PROJ_NAME       = $(notdir $(abspath $(dir $(PWD))))
PROJ_PATH       = $(PWD)
OUTPATH         = $(PWD)/Build

# Main APP files
APP_PATH        = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))
APP_PATH_SRC    = $(APP_PATH)/src


##############################################################################
#
# SDK source files
#
##############################################################################
#include lwip
include $(SOURCE_DIR)/middleware/third_party/lwip/module.mk

#include ping
include $(SOURCE_DIR)/middleware/third_party/ping/module.mk

#include iperf
include $(SOURCE_DIR)/middleware/MTK/iperf/module.mk

#include dhcpd
include $(SOURCE_DIR)/middleware/third_party/dhcpd/module.mk

# wifi src
include $(SOURCE_DIR)/driver/board/mt76x7_hdk/wifi/module.mk

APP_FILES       = $(APP_PATH_SRC)/main.c     \
                  $(APP_PATH_SRC)/wifi_lwip_helper.c \
                  $(APP_PATH)/GCC/syscalls.c \
                  $(APP_PATH_SRC)/sys_init.c \

ifeq ($(MTK_MINICLI_ENABLE),y)
APP_FILES       += driver/board/mt76x7_hdk/util/src/io_def.c
endif

APP_FILES       += $(APP_PATH_SRC)/ept_gpio_var.c
APP_FILES       += $(APP_PATH_SRC)/ept_eint_var.c

SYS_FILES       = $(APP_PATH_SRC)/system_mt7687.c

ifeq ($(MTK_MINICLI_ENABLE),y)
APP_FILES       += $(APP_PATH_SRC)/cli_def.c
APP_FILES       += $(APP_PATH_SRC)/cli_cmds.c
endif

C_FILES         += $(APP_FILES) $(SYS_FILES)


S_FILES         += $(APP_PATH)/GCC/startup_mt7687.s

##############################################################################
#
# SDK object files
#
##############################################################################


C_OBJS          = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)
S_OBJS          = $(S_FILES:%.s=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)


##############################################################################
#
# CFLAGS
#
##############################################################################


CFLAGS += -DMTK_FW_VERSION=\"$(MTK_FW_VERSION)\"


###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/$(IC_CONFIG)/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/inc
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/wifi/inc
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/ept/inc

##############################################################################
#
# LDFLAGS
#
##############################################################################


LDFLAGS += $(ALLFLAGS) $(FPUFLAGS) --specs=nano.specs -lnosys -nostartfiles
LDFLAGS += -Wl,-wrap=malloc -Wl,-wrap=calloc -Wl,-wrap=realloc -Wl,-wrap=free
LDFLAGS += -u _printf_float -u _scanf_float

ifeq ($(RAM_BOOTING), 1)
LDFLAGS += -Wl,-Tmt7687_sram.ld -Wl,--gc-sections
else
LDFLAGS += -Wl,-Tmt7687_flash.ld -Wl,--gc-sections
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
include $(SOURCE_DIR)/kernel/rtos/FreeRTOS/module.mk
include $(SOURCE_DIR)/kernel/service/module.mk

##############################################################################
#
# Target Rules
#
##############################################################################


.PHONY: proj clean $(MODULE_PATH)

all: cleanlog proj
	$(Q)mkdir -p $(BUILD_DIR)
	$(Q)$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf
	$(Q)echo "Generate Assembly from elf:"
	$(Q)$(OBJDUMP) -S $(OUTPATH)/$(PROJ_NAME).elf > $(OUTPATH)/$(PROJ_NAME).elf.s
	$(Q)$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin $(PWD)
	$(Q)cp *.cmm $(OUTPATH)/

MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_PATH)

$(LIBS): $(MODULE_PATH)

$(MODULE_PATH):
	$(Q)+make -C $@ $(MOD_EXTRA) $($@_EXTRA)


proj: $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(C_OBJS) $(CXX_OBJS) $(S_OBJS) $(LIBS)
	$(Q)echo Linking...
	$(Q)if [ -e "$@" ]; then rm -f "$@"; fi
	$(Q)if [ -e "$(OUTPATH)/$(PROJ_NAME).map" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).map"; fi
	$(Q)if [ -e "$(OUTPATH)/$(PROJ_NAME).hex" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).hex"; fi
	$(Q)if [ -e "$(OUTPATH)/$(PROJ_NAME).bin" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).bin"; fi
	$(Q)echo $(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%, $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group -Wl,-Map=$(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%,$(OUTPATH)/$(PROJ_NAME).map) -lm -o $@) > $(OUTPATH)/link_option.tmp
	$(Q)$(CXX) @$(OUTPATH)/link_option.tmp 2>>$(ERR_LOG)
	$(Q)rm -f $(OUTPATH)/link_option.tmp
	$(Q)$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	$(Q)$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin
	$(Q)echo Done


include $(SOURCE_DIR)/.rule.mk

clean:
	$(Q)rm -rf $(OUTPATH)

.PHONY: options

options:
	@for opt in `sed 's/[()]/\n/g' $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk|sed 's/-D/ /'|sort|uniq|grep ^MTK_|grep _ENABLE$$`; do echo $$opt; done
print-%  : ; @echo $* = $($*)
