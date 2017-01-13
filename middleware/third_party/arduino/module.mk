###################################################
# Sources
ARDUINO_SRC  = middleware/third_party/arduino

ARDUINO_CORE = hardware/arduino/$(IC_CONFIG)/cores/arduino
ARDUINO_VARS = hardware/arduino/$(IC_CONFIG)/variants/$(BOARD_CONFIG)
ARDUINO_LIBS = hardware/arduino/$(IC_CONFIG)/libraries

S-FILES      = $(wildcard $(ARDUINO_VARS)/*.s)

C-FILES      = $(wildcard $(ARDUINO_CORE)/*.c)
C-FILES     += $(wildcard $(ARDUINO_VARS)/*.c)
C-FILES     += $(wildcard $(ARDUINO_LIBS)/Wire/src/*.c)
C-FILES     += $(wildcard $(ARDUINO_LIBS)/WiFi/src/*.c)
C-FILES     += $(wildcard $(ARDUINO_LIBS)/WiFi/src/utility/*.c)

CXX-FILES    = $(wildcard $(ARDUINO_CORE)/*.cpp)
CXX-FILES   += $(wildcard $(ARDUINO_VARS)/*.cpp)
CXX-FILES   += $(wildcard $(ARDUINO_LIBS)/Wire/src/*.cpp)
CXX-FILES   += $(wildcard $(ARDUINO_LIBS)/WiFi/src/*.cpp)
CXX-FILES   += $(wildcard $(ARDUINO_LIBS)/WiFi/src/utility/*.cpp)

S_FILES      = $(addprefix middleware/third_party/arduino/,$(S-FILES))
C_FILES      = $(addprefix middleware/third_party/arduino/,$(C-FILES))
CXX_FILES    = $(addprefix middleware/third_party/arduino/,$(CXX-FILES))

###################################################
# include path
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_CORE)
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_VARS)
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_LIBS)/Wire/src
INC_FLAGS += -I$(SOURCE_DIR)/$(ARDUINO_SRC)/$(ARDUINO_LIBS)/WiFi/src

CFLAGS    += $(INC_FLAGS)
CPPFLAGS  += $(INC_FLAGS)

# FIXME: In the config/board/mt7687_hdk/board.mk, the following include
# directories only have been exported to CFLAGS. So, we export them to CPPFLAGS
# here. The following could be removed after this issue has been fixed.
CPPFLAGS  += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/util/inc
CPPFLAGS  += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/wifi/inc
