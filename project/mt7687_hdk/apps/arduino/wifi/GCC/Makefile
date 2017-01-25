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

SOURCE_DIR = ../../../../../..

PWD           = $(shell pwd)
SDK_ABS_PATH  = $(shell cd $(SOURCE_DIR) && pwd)

FEATURE    = feature.mk
include $(FEATURE)

# Gloabl Config
-include $(SOURCE_DIR)/.config
# IC Config
-include $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk
# Board Config
-include $(SOURCE_DIR)/config/board/$(BOARD_CONFIG)/board.mk
# Minisupp Config
-include $(SOURCE_DIR)/middleware/MTK/minisupp/module.mk

-include $(SOURCE_DIR)/driver/board/mt76x7_hdk/wifi/module.mk


RAM_BOOTING =0
DEBUG       =0
FLOAT_TYPE  =hard
FULL_TEST   =0

################################################################################
# Define some necessary variables indicated some path
################################################################################

# BUILD_DIR: The direction is to store the object files (*.o) during the
#            compiling
# OUTPATH:   The direction is to store these type files, such as *.elf/*.bin/*.a
################################################################################
BUILD_DIR  = $(PWD)/Build
OUTPATH    = $(PWD)/Build

# Some relative path related with Arduino
################################################################################
ARDUINO_REL_PATH      =middleware/third_party/arduino
ARDUINO_CORE_REL_PATH =$(ARDUINO_REL_PATH)/hardware/arduino/$(IC_CONFIG)/cores/arduino
ARDUINO_VARS_REL_PATH =$(ARDUINO_REL_PATH)/hardware/arduino/$(IC_CONFIG)/variants/$(BOARD_CONFIG)
ARDUINO_LIBS_REL_PATH =$(ARDUINO_REL_PATH)/hardware/arduino/$(IC_CONFIG)/libraries

# Some relative path related with arduino app project
# PROJ_ABS_PATH : such as '/xxx/.../project/mtxxxx_hdk/apps/arduino/xxxxx/GCC'
# PROJ_REL_PATH : such as 'project/mtxxxx_hdk/apps/arduino/xxxxx'
################################################################################
PROJ_ABS_PATH     =$(PWD)
PROJ_REL_PATH     =$(strip $(subst $(SDK_ABS_PATH)/,,$(PWD)))/..
PROJ_SRC_REL_PATH =$(PROJ_REL_PATH)/src
PROJ_NAME         =$(strip $(notdir $(shell cd $(CURDIR)/.. && pwd)))
# NONE_STRING    :=
# SPACE          :=$(NONE_STRING) #END
# PROJ_NAME       =$(subst $(SPACE),_,$(strip $(notdir $(shell cd $(CURDIR)/.. && pwd))))

# Indicate source files list related with current project
################################################################################
# Indicate these files that you want to compile by manual
# S_FILES =
# C_FILES =
# CXX_FILES = $(PROJ_SRC_REL_PATH)/xxx.cpp
CXX_FILES = $(foreach src,$(wildcard $(PROJ_ABS_PATH)/../src/*.cpp), $(PROJ_SRC_REL_PATH)/$(notdir $(src)))

# Indicate some compile flags
################################################################################
__FLAGS  = -DMTK_FW_VERSION=\"$(MTK_FW_VERSION)\"
__FLAGS  += -I$(SOURCE_DIR)/$(ARDUINO_CORE_REL_PATH)
__FLAGS  += -I$(SOURCE_DIR)/$(ARDUINO_VARS_REL_PATH)
__FLAGS  += -I$(SOURCE_DIR)/$(ARDUINO_LIBS_REL_PATH)/Wire/src
__FLAGS  += -I$(SOURCE_DIR)/$(ARDUINO_LIBS_REL_PATH)/WiFi/src

ifeq ($(DEBUG), 1)
	__FLAGS += -DDEBUG_IO
endif

CPPFLAGS += -fno-rtti $(__FLAGS)
CFLAGS   += $(__FLAGS)

# FIXME: In the config/board/mt7687_hdk/board.mk, the following include
# directories only have been exported to CFLAGS. So, we export them to CPPFLAGS
# here. The following could be removed after this issue has been fixed.
CPPFLAGS  += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/util/inc
CPPFLAGS  += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/wifi/inc

# Indicate some link flags
################################################################################
LDFLAGS  = $(ALLFLAGS) $(FPUFLAGS) --specs=nano.specs -lnosys -nostartfiles
LDFLAGS += -Wl,-wrap=malloc -Wl,-wrap=calloc -Wl,-wrap=realloc -Wl,-wrap=free
LDFLAGS += -lsupc++

LD_SCRIPT_PATH = $(strip $(SDK_ABS_PATH))/$(strip $(ARDUINO_VARS_REL_PATH))/linkscripts
ifeq ($(RAM_BOOTING), 1)
	LDFLAGS += -Wl,-T$(LD_SCRIPT_PATH)/$(IC_CONFIG)_sram.ld -Wl,--check-sections -Wl,--gc-sections
else
	LDFLAGS += -Wl,-T$(LD_SCRIPT_PATH)/$(IC_CONFIG)_flash.ld -Wl,--check-sections -Wl,--gc-sections
endif

ifeq ($(DEBUG), 1)
	LDFLAGS += --specs=rdimon.specs -lrdimon
endif

################################################################################
# Dependency Graph
################################################################################
# *.elf: $(CXX_OBJS) $(C_OBJS) $(S_OBJS) $(LIBS)

# $(CXX_OBJS) $(C_OBJS) $(S_OBJS)
################################################################################
S_OBJS   = $(S_FILES:%.s=$(BUILD_DIR)/%.o)
C_OBJS   = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
CXX_OBJS = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)

# $(LIBS)
################################################################################
# For HAL Layer:
# Necessary:	libhal.a	MTxxxx peripheral operation
# Unnecessary:	libhal_core.a	efuse operation
# Unnecessary:	libhal_sec.a	crypt operation
LIBS += $(OUTPATH)/libhal.a
# For FreeRTOS
LIBS += $(OUTPATH)/librtos.a
LIBS += $(OUTPATH)/libarduino.o
LIBS += $(OUTPATH)/libservice.a
#LIBS += $(OUTPATH)/libbsp.a
LIBS += $(OUTPATH)/liblwip.a
LIBS += $(OUTPATH)/libnvdm_CM4_GCC.a
LIBS += $(SOURCE_DIR)/driver/chip/mt7687/lib/libhal_core_CM4_GCC.a

MODULE_PATH += $(DRV_CHIP_PATH)
MODULE_PATH += $(KRL_OS_PATH)
MODULE_PATH += $(MID_ARDUINO_PATH)
MODULE_PATH += $(KRL_SRV_PATH)
#MODULE_PATH += $(DRV_BSP_PATH)
MODULE_PATH += $(MID_LWIP_PATH)
MODULE_PATH += $(MID_NVDM_PATH)

MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_ABS_PATH) CONFIG_PATH=$(ARDUINO_VARS_REL_PATH)
$(MID_LWIP_PATH)_EXTRA += MINISUPP=1 SUPPORT_LP=1

################################################################################
# Rules
################################################################################
.PHONY: proj clean $(MODULE_PATH)

all: cleanlog proj
	@mkdir -p $(BUILD_DIR)
	@$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf
	@$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) \
		$(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin \
		$(LD_SCRIPT_PATH)


$(LIBS): $(MODULE_PATH)

$(MODULE_PATH):
	@+make -C $@ $(MOD_EXTRA) $($@_EXTRA)

$(OUTPATH)/libarduino.o:
	@+make -C $(MID_ARDUINO_PATH) $(MOD_EXTRA) $($@_EXTRA)

proj: $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(CXX_OBJS) $(C_OBJS) $(S_OBJS) $(LIBS)
	@echo Linking...
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).map" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).map"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).dis" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).dis"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).hex" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).hex"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).bin" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).bin"; fi
	@$(CXX) $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group -Wl,-Map=$(OUTPATH)/$(PROJ_NAME).map -lm -o $@ 2>>$(ERR_LOG)
	@$(OBJDUMP) -D $(OUTPATH)/$(PROJ_NAME).elf > $(OUTPATH)/$(PROJ_NAME).dis
	@$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	@$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin
	@echo Done

# Include compile rules
################################################################################
include $(SOURCE_DIR)/.rule.mk

################################################################################
# Misc Options
################################################################################
clean:
	rm -rf $(OUTPATH)

print-%  : ; @echo $* = $($*)
