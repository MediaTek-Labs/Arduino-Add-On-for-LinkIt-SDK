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
 * @addtogroup mt7697_hdk mt7697_hdk
 * @{
 * @addtogroup mt7697_hdk_apps apps
 * @{
 * @addtogroup mt7697_hdk_apps_wifi_demo wifi_demo
 * @{

@par Overview
  - Appliation description
    - This appliation demonstrates the Wi-Fi connectivity features of the
      LinkIt 7697 HDK through the following:
      - How to use the Wi-Fi API to initialize the Wi-Fi for different
        operation mode.
  - Application features
    - Act as a Wi-Fi station to connect to a Wi-Fi network.
    - Act as a Wi-Fi AP to accept connection to the LinkIt 7697 HDK from a
      handheld device or a laptop computer.
    - Ping out or into the device.
    - Wi-Fi Command Line Interface (CLI). For more details about Wi-Fi
      related CLI commands, please refer to the "Supported commands" section
      of the "cli_readme.txt" document.

@par Hardware and software environment
  - Supported platform
    - LinkIt 7697 HDK.
  - Environment configuration
    - The output logs are communicated through a micro-USB cable to the PC
      from USB (CON5) connector on the HDK.
      - Install the mbed serial driver according to the instructions at
        https://developer.mbed.org/handbook/Windows-serial-configuration. For
        more information, please refer to section "Installing the LinkIt 7697
        HDK drivers on Microsoft Windows" on the "LinkIt 7697 HDK User Guide"
        in [sdk_root]/doc folder.
      - Use a type-A to micro-B USB cable to connect type-A USB of the PC and
        MK20 micro-B USB connector on the LinkIt 7697 HDK. For more
        information about the connector cable, please refer to
        https://en.wikipedia.org/wiki/USB#Mini_and_micro_connectors.
      - Launch a terminal emulator program, such as Tera terminal on your PC
        for data logging through UART. For the installation details, please
        refer to section "Installing Tera terminal on Microsoft Windows" on
        the "LinkIt for RTOS Get Started Guide" in [sdk_root]/doc folder.
      - COM port settings. baudrate: 115200, data bits: 8, stop bit: 1,
        parity: none and flow control: off.

@par Directory contents
  - Source and header files
    - \b src/main.c:              Main program.
    - \b src/sys_init.c:          Aggregated initialization routines.
    - \b src/system_mt7687.c:     MT76x7 clock configuration file.
    - \b src/cli_cmds.c:          CLI table.
    - \b src/wifi_lwip_helper.c:  lwIP configuration.
    - \b src/cli_def.c:           CLI definition.
    - \b inc/flash_map.h:         MT76x7 memory layout symbol file.
    - \b inc/task_def.h:          Define the task stack size, queue length,
                                  project name, and priority for the
                                  application to create tasks.
    - \b inc/hal_feature_config.h:
                                  MT76x7 feature configuration file.
    - \b inc/FreeRTOSConfig.h:    MT76x7 FreeRTOS configuration file.
    - \b inc/lwipopts.h:          lwIP configuration.
    - \b inc/sys_init.h:          Prototype declaration for
                                  wifi_demo/src/sys_init.c.
    - \b inc/app_cli_table.h:     CLI table entry.
    - \b inc/cli_cmds.h:          Prototype declaration for
                                  wifi_demo/src/cli_cmds.c.
    - \b inc/wifi_lwip_helper.h:  Prototype declaration for
                                  wifi_demo/src/wifi_lwip_helper.c.
    - \b GCC/syscalls.c:          MT76x7 syscalls for GCC.
    - \b GCC/startup_mt7687.s:    MT76x7 startup file for GCC.
    - \b MDK-ARM/startup_mt7687.s:
                                  MT76x7 startup file for Keil IDE.
  - Project configuration files using GCC
    - \b GCC/feature.mk:       Feature configuration.
    - \b GCC/Makefile:         Makefile.
    - \b GCC/mt7687_flash.ld:  Linker script.
  - Project configuration files using Keil
    - \b MDK-ARM/wifi_demo.uvprojx:
                             uVision5 Project File. Contains the project
                             structure in XML format.
    - \b MDK-ARM/flash.sct:  Linker script.
  - Project configuration files using IAR
    - \b EWARM/wifi_demo.ewd:  IAR project options. Contains the settings
                                  for the debugger.
    - \b EWARM/wifi_demo.ewp:  IAR project file. Contains the project
                                  structure in XML format.
    - \b EWARM/wifi_demo.eww:  IAR workspace file. Contains project
                                  information.
    - \b EWARM/flash.icf:         Linker script.

@par Run the examples
  - Below are two examples to demonstrate the Wi-Fi station and Wi-Fi access
    point modes of the LinkIt 7697 HDK.
  - Example 1. Wi-Fi station mode.
    - Find your Wi-Fi access point settings: Before connecting to a Wi-Fi
      access point, the following information needs to be collected:
      - The SSID of your Wi-Fi access point.
      - The password of your Wi-Fi access point.
    - Apply the following settings in the main.c source file to initialize
      the HDK. In this example the access point SSID is 'myhome' (length 6)
      and the password is '12345678' (length 8).
      - User-defined settings.
        \code
        wifi_config_t config = {0};
        config.opmode = WIFI_MODE_STA_ONLY;
        strcpy((char *)config.sta_config.ssid, "myhome");
        config.sta_config.ssid_length = strlen("myhome");
        strcpy((char *)config.sta_config.password, "12345678");
        config.sta_config.password_length = strlen("12345678");
        \endcode
      - Wi-Fi initialization.
        \code
        wifi_init(&config, NULL);
        \endcode
  - Build the example project with the command "./build.sh mt7697_hdk
    wifi_demo" from the SDK root folder and download the binary file to
    the LinkIt 7697 HDK.
  - Connect your board to the PC with a micro-USB cable.
  - Reboot the HDK, the console will show "FreeRTOS Running" message to
    indicate the HDK is booting up. If everything is correct, similar
    messages "DHCP got IP:10.10.10.101" will be shown in the console to
    notify your HDK has received an IP address.
    - PING from the LinkIt 7697 HDK. If the IP address is fetched and the
      network is operating, the LinkIt 7697 HDK can ping other
      devices/computer on the network with the following command in the
      console. ping 10.10.10.254 3 64 The ping stops after sending three
      packets to 10.10.10.254. The ping usage is: ping <ip_address>
      <ping_count> <ping_packet_length>.
  - Example 2. Wi-Fi access point mode.
    - Apply the Wi-Fi access point settings before proceeding:
      - SSID
      - Authentication Mode
      - Encryption Type
      - Password
      - Channel
    - Once the information is collected, use the following commands to
      configure the LinkIt 7697 HDK. The example code in main.c assumes WPA2
      PSK is used for authentication, AES for encryption, 'iot_ap' (length 6)
      for the SSID, the password of WPA2 as '87654321' (length 8) and the
      channel 6 is assigned to the channel.
      - User-defined settings.
        \code
        wifi_config_t config = {0};
        wifi_config.opmode = WIFI_MODE_AP_ONLY;
        strcpy((char *)wifi_config.ap_config.ssid, "iot_ap");
        wifi_config.ap_config.ssid_length = strlen("iot_ap");
        wifi_config.ap_config.auth_mode = WIFI_AUTH_MODE_WPA2_PSK;
        wifi_config.ap_config.encrypt_type = WIFI_ENCRYPT_TYPE_AES_ENABLED;
        strcpy((char *)wifi_config.ap_config.password, "87654321");
        wifi_config.ap_config.password_length = strlen("87654321");
        wifi_config.ap_config.channel = 6;
        \endcode
      - Wi-Fi initialization.
        \code
        wifi_init(&config, NULL);
        \endcode
  - Build the example project with the command "./build.sh mt7697_hdk
    wifi_demo" from the SDK root folder and download the binary file to
    the LinkIt 7697 HDK.
  - Connect your board to the PC with a micro-USB cable.
  - Reboot the HDK, the console will show "FreeRTOS Running" message to
    indicate the HDK is booting up.
    - Use a handheld device or a laptop computer to connect to the access
      point 'iot_ap'. On the HDK console, the IP address assigned to the
      handheld device or laptop is: [DHCPD:DBG]lease_ip:10.10.10.2
*/
/**
 * @}
 * @}
 * @}
 */
