/* Copyright Statement:
 * (C) 2005-2016 MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute MediaTek Software
 * if you have agreed to and been granted explicit permission within
 * the "License Agreement" that is available on MediaTek's website ("Website") before
 * downloading the relevant MediaTek Software from the Website ("Permitted User").
 * If you are not a Permitted User, please cease any access or use of MediaTek Software immediately.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

/**
  * @page freertos thread creation example project
  * @{

@par Overview
 - Application description
   - This application creates 4 tasks. Each task sleep a different period of OC Tick,and execute the same action when it wakes up.
 - Features of the application
	 - This sample project shows how to setup multi-tasks enviroment.
 - Process / procedure of the sample
   - 4-tasks will be created at initialize stage, then will be regulerly scheduled in different interval.
 - Input of the sample
	 - N.A.
 - Output of the sample
  - It will show log message after system runs up.
 - Limitation of the sample
   - N.A.

@par Hardware and Software environment
  - Supported Chipset/HW
    - MT7687 HDK

  - Any special environment needed
    - N.A.
  - Donwload any special SW
    - N.A.

@par Directory contents
  - List fils
   - freertos_thread_creation/src/main.c  			Main program
   - freertos_thread_creation/src/system_mt7687.c		MT7687 system clock configuration file
   - freertos_thread_creation/inc/FreeRTOSConfig.h 		FreeRTOS feature configuration file
   - freertos_thread_creation/inc/hal_feature_config.h 		MT7687 feature configuration file
   - freertos_thread_creation/inc/flash_map.h		        MT7687 memory layout symbol file
   - freertos_thread_creation/GCC/startup_mt7687.s		MT7687 start up file of GCC version
   - freertos_thread_creation/GCC/syscalls.c			MT7687 syscalls of GCC version
   - freertos_thread_creation/MDK-ARM/startup_mt7687.s	        MT7687 start up file of keil version

@par Run the demo
  - HDK swithes, pin config
    - N.A.

  - PC/environment config
   - Need a serial tool(such as hyper terminal) for UART logging .
   - Com port setting : baudrate: 115200, data bits: 8, stop bit: 1, parity: none and flow control: off.

@par Run the demo
  - Connect board with PC with serial port cable.
  - Build the sample project and download the bin file to the target.
  - Run the app, the log will show "hello world" message

@par Configuration
  - GCC version project configuration files
   - freertos_thread_creation/GCC/feature.mk		    	          Feature configuration file of GCC version project
   - freertos_thread_creation/GCC/mt7687_flash.ld			  Linker script of GCC version project
  - Keil version project configuration files
   - freertos_thread_creation/MDK-ARM/freertos_thread_creation.uvprojx	  uVision5 Project File. Contains the project structure in XML format.
   - freertos_thread_creation/MDK-ARM/freertos_thread_creation.uvoptx     uVision5 project options. Contains the settings for the debugger, trace configuration, breakpoints, currently open files, etc.
   - freertos_thread_creation/MDK-ARM/flash.sct		                  Linker script of Keil version project

* @}
*/
