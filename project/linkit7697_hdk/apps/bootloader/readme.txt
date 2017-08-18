/* Copyright Statement:
 *
 * (C) 2005-2017 MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its
 * licensors. Without the prior written permission of MediaTek and/or its
 * licensors, any reproduction, modification, use or disclosure of MediaTek
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute
 * (as applicable) MediaTek Software if you have agreed to and been bound by
 * the applicable license agreement with MediaTek ("License Agreement") and
 * been granted explicit permission to do so within the License Agreement
 * ("Permitted User"). If you are not a Permitted User, please cease any
 * access or use of MediaTek Software immediately.
 */

/**
 * @addtogroup linkit7697_hdk linkit7697_hdk
 * @{
 * @addtogroup linkit7697_hdk_apps apps
 * @{
 * @addtogroup linkit7697_hdk_apps_bootloader_lite bootloader_lite
 * @{

@par Overview
  - This is not a example project. This project is a bootloader for LinkIt
    7697 HDK (without Firmware Over The Air update capability). The developer
    can refer this project to understand how to build a bootloader.

@par Hardware and software environment
  - Supported platform
    - LinkIt 7697 HDK.
  - PC/environment configuration
    - A serial tool is required, such as hyper terminal, for UART logging.
    - COM port settings. baudrate: 115200, data bits: 8, stop bit: 1, parity:
      none and flow control: off.
  - Customization Option
    - The \b BL_FOTA_DEBUG definition determines the bootloader accesses the
      UART driver directly to pring logs (overwrites printf(.)).

@par Directory contents
  - Source and header files
    - \b \<sdk_root\>/driver/board/linkit7697_hdk/bootloader/src/mt7687/main.c:
                                  Main program
    - \b inc/hal_feature_config.h:
                                  MT7687x's feature configuration file.
    - \b inc/flash_map.h:         MT7687x's memory layout symbol file
    - \b GCC/syscalls.c:          MT7687x's syscalls for GCC.
    - \b MDK-ARM/startup_mt7687.s:
                                  MT7687x's startup file for Keil.
    - \b EWARM/startup_mt7687.s:  MT7687x's startup file for IAR.
    - \b src/system_mt7687.c:     MT7687x's system clock configuration file
  - Project configuration files using GCC
    - \b GCC/feature.mk:     Feature configuration.
    - \b GCC/Makefile:       Makefile.
    - \b GCC/bootloader.ld:  Linker script.
  - Project configuration files using Keil IDE
    - \b MDK-ARM/bootloader.uvprojx:
                             uVision5 project File. Contains the project
                             structure in XML format.
    - \b MDK-ARM/flash.sct:  Scatter file.
  - Project configuration files using IAR IDE
    - \b EWARM/bootloader.ewd:     IAR project options. Contains the settings
                                   for the debugger.
    - \b EWARM/bootloader.ewp:     IAR project file. Contains the project
                                   structure in XML format.
    - \b EWARM/bootloader.eww:     IAR workspace file. Contains project
                                   information.
    - \b EWARM/mtk7697_flash.icf:  Linker script.

@par Run the example
  - Connect board to the PC with serial port cable.
  - Build bootloader with the command, "./build.sh linkit7697_hdk bootloader"
    under the SDK root folder and download the binary file to LinkIt 7697
    HDK.
  - Power on LinkIt 7697 HDK and bootloader log are showen on serial tool.
*/
/**
 * @}
 * @}
 * @}
 */
*/

