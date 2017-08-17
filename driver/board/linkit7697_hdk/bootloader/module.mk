
BL_SRC = driver/board/linkit7697_hdk/bootloader/src/mt7687


C_FILES  += $(BL_SRC)/main.c
C_FILES  += $(BL_SRC)/hw_uart.c
C_FILES  += $(BL_SRC)/xmodem.c
C_FILES  += $(BL_SRC)/crc16.c

#################################################################################
#include path
CFLAGS += -I$(SOURCE_DIR)/driver/chip/mt7687/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/inc
ifeq ($(MTK_FOTA_ENABLE),y)
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/fota/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/fota/inc/76x7
endif
