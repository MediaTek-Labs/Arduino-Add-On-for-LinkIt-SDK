# Copyright Statement:
# (C) 2005-2016 MediaTek Inc. All rights reserved.
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
# Without the prior written permission of MediaTek and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
# You may only use, reproduce, modify, or distribute MediaTek Software
# if you have agreed to and been granted explicit permission within
# the "License Agreement" that is available on MediaTek's website ("Website") before
# downloading the relevant MediaTek Software from the Website ("Permitted User").
# If you are not a Permitted User, please cease any access or use of MediaTek Software immediately.
#
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

# Middleware Compilation Parameter, such as:
# MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_PATH) CONFIG_PATH=$(ARDUINO_VARS_PATH)
#
# PROJ_PATH   : The App project GCC/ absolute path, such as:
#               /.../project/mt7687_hdk/apps/arduino/blink/GCC
#
# CONFIG_PATH : In general, it indicates the relative path of a App project inc/, such as:
#               project/mt7687_hdk/apps/lwip_socket/inc
#               However, it is a trick in the arduino project. Its value is:
#               middleware/third_party/arduino/hardware/arduino/mt7687/variants/mt7687_hdk
#               This parameter is used for driver hal, and some other module, but it not used in the
#               arduino middleware.

SOURCE_DIR   = ../../..
BINPATH      = ~/gcc-arm-none-eabi/bin
CONFIG_PATH ?= .

FEATURE ?= feature.mk
include $(PROJ_PATH)/$(FEATURE)

# Gloabl Config
-include $(SOURCE_DIR)/.config
# IC Config
-include $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk
# Board Config
-include $(SOURCE_DIR)/config/board/$(BOARD_CONFIG)/board.mk

CPPFLAGS += -fno-rtti

# Project name
TARGET_LIB=libarduino

BUILD_DIR ?= Build
OUTPATH   ?= Build

# Sources
include module.mk

CXX_OBJS = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)
C_OBJS   = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
S_OBJS   = $(S_FILES:%.s=$(BUILD_DIR)/%.o)


.PHONY: $(TARGET_LIB).o

all: $(TARGET_LIB).o
	@echo Build $< Done

# For avoid to change the global .rule.mk, the following rule have been
# appended at here.
$(TARGET_LIB).o: $(CXX_OBJS) $(C_OBJS) $(S_OBJS)
	@echo Gen $(TARGET_LIB).o
	@echo Gen $(TARGET_LIB).o >>$(BUILD_LOG)
	@$(LD) -r -o $(OUTPATH)/$@ $^ >>$(BUILD_LOG) 2>/dev/null; \
	 if [ "$$?" != "0" ]; then \
		echo "MODULE BUILD $@ FAIL" >> $(BUILD_LOG); \
	 else \
		echo "MODULE BUILD $@ PASS" >> $(BUILD_LOG); \
	 fi;

include $(SOURCE_DIR)/.rule.mk

clean:
	rm -rf $(OUTPATH)/$(TARGET_LIB).o
	rm -rf $(BUILD_DIR)

print-%  : ; @echo $* = $($*)
