###################################################
# Sources
MID_ARDUINO_PATH = $(SOURCE_DIR)/middleware/third_party/arduino
ARDUINO_SRC  = middleware/third_party/arduino
ARDUINO_CORE = hardware/arduino/mt7697/cores/arduino
ARDUINO_VARS = hardware/arduino/mt7697/variants/linkit_7697
ARDUINO_LIBS = hardware/arduino/mt7697/libraries

###################################################
# include path
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_CORE)
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_VARS)
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_LIBS)/MCS/src
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_LIBS)/LTimer/src
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_LIBS)/LFlash/src
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_LIBS)/SPI/src
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_LIBS)/Wire/src
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_LIBS)/LWiFi/src
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_LIBS)/LBLE/src
INC_FLAGS += -I$(SOURCE_DIR)/middleware/MTK/fota/inc
INC_FLAGS += -I$(SOURCE_DIR)/middleware/MTK/fota/inc/76x7
INC_FLAGS += -I$(SOURCE_DIR)/middleware/MTK/nvdm/inc
INC_FLAGS += -I$(SOURCE_DIR)/middleware/MTK/bluetooth/inc
INC_FLAGS += -I$(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/inc
INC_FLAGS += -I$(SOURCE_DIR)/middleware/MTK/connsys/inc
INC_FLAGS += -I$(SOURCE_DIR)/middleware/MTK/fota/inc
INC_FLAGS += -I$(SOURCE_DIR)/middleware/third_party/mbedtls/include
INC_FLAGS += -I$(SOURCE_DIR)/middleware/third_party/mbedtls/configs
INC_FLAGS += -I$(SOURCE_DIR)/middleware/third_party/lwip/src/include
INC_FLAGS += -I$(SOURCE_DIR)/middleware/third_party/lwip/ports/include

CFLAGS    += $(INC_FLAGS)
CXXFLAGS  += $(INC_FLAGS)

# FIXME: In the config/board/mt76x7_hdk/board.mk, the following include
# directories only have been exported to CFLAGS. So, we export them to CPPFLAGS
# here. The following could be removed after this issue has been fixed.
CXXFLAGS  += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/util/inc
CXXFLAGS  += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/wifi/inc
CXXFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/nvdm/inc
CXXFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/fota/inc

