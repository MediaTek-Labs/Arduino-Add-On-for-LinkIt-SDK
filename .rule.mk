OS_VERSION := $(shell uname)
ifneq ($(filter MINGW%,$(OS_VERSION)),)
  $(DRV_CHIP_PATH)_EXTRA    := -j1
  $(MID_MBEDTLS_PATH)_EXTRA := -j1
endif

ifeq ($(MAKELEVEL),0)
M :=
else
M := -
endif

include $(SOURCE_DIR)/middleware/MTK/verno/module.mk

.PHONY: merge_lib copy_lib cleanlog

cleanlog:
ifeq ($(TARGET_PATH),)
	rm -f $(OUTPATH)/*.log
else
	@echo "trigger by build.sh, skip cleanlog"
endif


copy_lib:
	@if [ -e $(RELEASE_TARGET_LIB_PATH) ]; then \
	rm -rf $(OUTPATH)/$(TARGET_LIB).a; \
	mkdir -p $(OUTPATH); \
	cp $(RELEASE_TARGET_LIB_PATH) $(OUTPATH)/$(TARGET_LIB).a; \
	if [ "$$?" != "0" ]; then \
		echo "COPY LIB $(TARGET_LIB).a FAIL"; \
		echo "COPY LIB $(TARGET_LIB).a FAIL" >> $(OUTPATH)/build.log; \
	else \
		echo "COPY LIB $(TARGET_LIB).a PASS"; \
		echo "COPY LIB $(TARGET_LIB).a PASS" >> $(OUTPATH)/build.log; \
		fi; \
	fi

merge_lib:
	@if [ -e $(RELEASE_TARGET_LIB_PATH) ] && [ -e $(OUTPATH)/$(TARGET_LIB).a ]; then \
		mkdir -p $(OUTPATH)/merge_$(TARGET_LIB); \
		cp $(RELEASE_TARGET_LIB_PATH) $(OUTPATH)/merge_$(TARGET_LIB)/$(TARGET_LIB).a; \
		cd $(OUTPATH)/merge_$(TARGET_LIB); \
		$(CURDIR)/$(AR) -x ../$(TARGET_LIB).a; \
		$(CURDIR)/$(AR) -r $(TARGET_LIB).a *.o; \
		if [ "$$?" != "0" ]; then \
			echo "MERGE LIB $(TARGET_LIB).a FAIL"; \
			echo "MERGE LIB $(TARGET_LIB).a FAIL" >> ../build.log; \
			rm ../$(TARGET_LIB).a; \
		else \
			echo "MERGE LIB $(TARGET_LIB).a PASS"; \
			echo "MERGE LIB $(TARGET_LIB).a PASS" >> ../build.log; \
			mv -f $(TARGET_LIB).a ../; \
		fi; \
			cd $(PWD); \
			rm -r $(OUTPATH)/merge_$(TARGET_LIB); \
		fi

$(TARGET_LIB).a: $(C_OBJS) $(CXX_OBJS) $(S_OBJS)
	@echo Gen $(TARGET_LIB).a
	@echo Gen $(TARGET_LIB).a >>$(BUILD_LOG)
	$(Q)if [ -e "$(OUTPATH)/$@" ]; then rm -f "$(OUTPATH)/$@"; fi
	$(Q)if [ -e "$(OUTPATH)/lib/$@" ]; then rm -f "$(OUTPATH)/lib/$@"; fi
	$(Q)$(M)$(AR) -r $(OUTPATH)/$@ $(C_OBJS) $(CXX_OBJS) $(S_OBJS) >>$(BUILD_LOG) 2>>$(ERR_LOG); \
	if [ "$$?" != "0" ]; then \
		echo "MODULE BUILD $@ FAIL"; \
		echo "MODULE BUILD $@ FAIL" >> $(BUILD_LOG); \
		exit 1;\
	else \
		echo "MODULE BUILD $@ PASS"; \
		echo "MODULE BUILD $@ PASS" >> $(BUILD_LOG); \
	fi;

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo Build... $$(basename $@)
	@echo Build... $@ >> $(BUILD_LOG)
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@echo $(CC) $(CFLAGS) $@ >> $(BUILD_LOG)
	@-$(CC) $(CFLAGS) -c $< -o $@ 2>>$(ERR_LOG); \
	if [ "$$?" != "0" ]; then \
		echo "Build... $$(basename $@) FAIL"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
		echo "Build... $$(basename $@) PASS"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

$(BUILD_DIR)/%.d: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	@set -e; rm -f $@; \
	export D_FILE="$@"; \
	export B_NAME=`echo $$D_FILE | sed 's/\.d//g'`; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's@\(.*\)\.o@'"$$B_NAME\.o $$B_NAME\.d"'@g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo Build... $$(basename $@)
	@echo Build... $@ >> $(BUILD_LOG)
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@echo $(CXX) $(CXXFLAGS) $@ >> $(BUILD_LOG)
	@-$(CXX) $(CXXFLAGS) -c $< -o $@ 2>>$(ERR_LOG); \
	if [ "$$?" != "0" ]; then \
		echo "Build... $$(basename $@) FAIL"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
		echo "Build... $$(basename $@) PASS"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

$(BUILD_DIR)/%.d: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@set -e; rm -f $@; \
	export D_FILE="$@"; \
	export B_NAME=`echo $$D_FILE | sed 's/\.d//g'`; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's@\(.*\)\.o@'"$$B_NAME\.o $$B_NAME\.d"'@g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.s
	@mkdir -p $(dir $@)
	@echo Build... $$(basename $@)
	@echo Build... $@ >> $(BUILD_LOG)
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@-$(CC) $(CFLAGS) -c $< -o $@; \
	if [ "$$?" != "0" ]; then \
		echo "Build... $$(basename $@) FAIL"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
		echo "Build... $$(basename $@) PASS"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

ifneq ($(MAKECMDGOALS),clean)
-include $(C_OBJS:.o=.d)
-include $(CXX_OBJS:.o=.d)
endif
