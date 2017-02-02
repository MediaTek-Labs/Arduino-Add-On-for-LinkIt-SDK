;/* Copyright Statement:
; *
; * (C) 2005-2016  MediaTek Inc. All rights reserved.
; *
; * This software/firmware and related documentation ("MediaTek Software") are
; * protected under relevant copyright laws. The information contained herein
; * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
; * Without the prior written permission of MediaTek and/or its licensors,
; * any reproduction, modification, use or disclosure of MediaTek Software,
; * and information contained herein, in whole or in part, shall be strictly prohibited.
; * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
; * if you have agreed to and been bound by the applicable license agreement with
; * MediaTek ("License Agreement") and been granted explicit permission to do so within
; * the License Agreement ("Permitted User").  If you are not a Permitted User,
; * please cease any access or use of MediaTek Software immediately.
; * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
; * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
; * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
; * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
; * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
; * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
; * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
; * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
; * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
; * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
; * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
; * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
; * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
; * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
; * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
; * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
; * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
; */

; Macro defines
#define Cache_Ctrl_Base    0x01530000
#define Cache_Ctrl_Op      0x01530004
#define Cache_Channel_En   0x0153002C
#define Cache_Entry_Start  0x01540000
#define Cache_Entry_End    0x01540040
#define Cache_Disable      0x0
#define Cache_Enable       0x30D
#define Cache_Start_Addr   0x10000100
#define Cache_End_Addr     0x11000000
#define Cache_Invalidate_All     0x03

        MODULE  ?cstartup

        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .isr_vector:CODE:REORDER:ROOT(2)

        EXTERN  __iar_program_start
        EXTERN  SystemInit
        EXPORT  Jump_Cmd
        ;PUBLIC  __vector_table
        PUBLIC  __isr_vector

;__iar_init$$done:               ; The vector table is not needed
                                ; until after copy initialization is done

;        DATA
;__vector_table
__isr_vector

        DCD     SFE(CSTACK)                ; Top of Stack
        DCD     Reset_Handler              ; Reset Handler
        DCD     NMI_Handler                ; NMI Handler
        DCD     HardFault_Handler          ; Hard Fault Handler
        DCD     MemManage_Handler          ; MPU Fault Handler
        DCD     BusFault_Handler           ; Bus Fault Handler
        DCD     UsageFault_Handler         ; Usage Fault Handler
        DCD     0                          ; Reserved
        DCD     0                          ; Reserved
        DCD     0                          ; Reserved
        DCD     0                          ; Reserved
        DCD     SvcCall_Main                ; SVCall Handler
        DCD     DebugMon_Handler           ; Debug Monitor Handler
        DCD     0                          ; Reserved
        DCD     PendSV_Main             ; PendSV Handler
        DCD     SysTick_Main            ; SysTick Handler

        ; External Interrupts
        DCD     isrC_main        ;16:  UART1
        DCD     isrC_main     	 ;17:  DMA
        DCD     isrC_main        ;18:  HIF
        DCD     isrC_main        ;19:  I2C1
        DCD     isrC_main        ;20:  I2C2
        DCD     isrC_main        ;21:  UART2
        DCD     isrC_main        ;22:  MTK_CRYPTO
        DCD     isrC_main        ;23:  SF
        DCD     isrC_main        ;24:  EINT
        DCD     isrC_main        ;25:  BTIF
        DCD     isrC_main        ;26:  WDT
        DCD     isrC_main        ;27:  reserved
        DCD     isrC_main        ;28:  SPI_SLAVE
        DCD     isrC_main        ;29:  WDT_N9
        DCD     isrC_main        ;30:  ADC
        DCD     isrC_main        ;31:  IRDA_TX
        DCD     isrC_main        ;32:  IRDA_RX
        DCD     isrC_main        ;33:  USB_VBUS_ON
        DCD     isrC_main        ;34:  USB_VBUS_OFF
        DCD     isrC_main        ;35:  timer_hit
        DCD     isrC_main        ;36:  GPT3
        DCD     isrC_main        ;37:  alarm_hit
        DCD     isrC_main        ;38:  AUDIO
        DCD     isrC_main        ;39:  n9_cm4_sw_irq
        DCD     isrC_main        ;40:  GPT4
        DCD     isrC_main        ;41:  adc_comp_irq
        DCD     isrC_main        ;42:  reserved
        DCD     isrC_main        ;43:  SPIM
        DCD     isrC_main        ;44:  USB
        DCD     isrC_main        ;45:  UDMA
        DCD     isrC_main        ;46:  TRNG
        DCD     isrC_main        ;47:  reserved
        DCD     isrC_main        ;48:  configurable
        DCD     isrC_main        ;49:  configurable
        DCD     isrC_main        ;50:  configurable
        DCD     isrC_main        ;51:  configurable
        DCD     isrC_main        ;52:  configurable
        DCD     isrC_main        ;53:  configurable
        DCD     isrC_main        ;54:  configurable
        DCD     isrC_main        ;55:  configurable
        DCD     isrC_main        ;56:  configurable
        DCD     isrC_main        ;57:  configurable
        DCD     isrC_main        ;58:  configurable
        DCD     isrC_main        ;59:  configurable
        DCD     isrC_main        ;60:  configurable
        DCD     isrC_main        ;61:  configurable
        DCD     isrC_main        ;62:  configurable
        DCD     isrC_main        ;63:  configurable
        DCD     isrC_main        ;64:  configurable
        DCD     isrC_main        ;65:  configurable
        DCD     isrC_main        ;66:  configurable
        DCD     isrC_main        ;67:  configurable
        DCD     isrC_main        ;68:  configurable
        DCD     isrC_main        ;69:  configurable
        DCD     isrC_main        ;70:  configurable
        DCD     isrC_main        ;71:  configurable
        DCD     isrC_main        ;72:  configurable
        DCD     isrC_main        ;73:  configurable
        DCD     isrC_main        ;74:  configurable
        DCD     isrC_main        ;75:  configurable
        DCD     isrC_main        ;76:  configurable
        DCD     isrC_main        ;77:  configurable
        DCD     isrC_main        ;78:  configurable
        DCD     isrC_main        ;79:  configurable
        DCD     isrC_main        ;80:  configurable
        DCD     isrC_main        ;81:  configurable
        DCD     isrC_main        ;82:  configurable
        DCD     isrC_main        ;83:  configurable
        DCD     isrC_main        ;84:  configurable
        DCD     isrC_main        ;85:  configurable
        DCD     isrC_main        ;86:  configurable
        DCD     isrC_main        ;87:  configurable
        DCD     isrC_main        ;88:  configurable
        DCD     isrC_main        ;89:  configurable
        DCD     isrC_main        ;90:  configurable
        DCD     isrC_main        ;91:  configurable
        DCD     isrC_main        ;92:  configurable
        DCD     isrC_main        ;93:  configurable
        DCD     isrC_main        ;94:  configurable
        DCD     isrC_main        ;95:  configurable
        DCD     isrC_main        ;96:  configurable
        DCD     isrC_main        ;97:  configurable
        DCD     isrC_main        ;98:  configurable
        DCD     isrC_main        ;99:  configurable
        DCD     isrC_main        ;100: configurable
        DCD     isrC_main        ;101: configurable
        DCD     isrC_main        ;102: configurable
        DCD     isrC_main        ;103: configurable
        DCD     isrC_main        ;104: configurable
        DCD     isrC_main        ;105: configurable
        DCD     isrC_main        ;106: configurable
        DCD     isrC_main        ;107: configurable
        DCD     isrC_main        ;108: configurable
        DCD     isrC_main        ;109: configurable
        DCD     isrC_main        ;110: configurable
        DCD     isrC_main        ;111: configurable

        THUMB
        PUBWEAK Reset_Handler
        SECTION .reset_handler:CODE:REORDER:ROOT(2)
Reset_Handler

        LDR     SP, =SFE(CSTACK)
        
        BL      PreCacheInit

        LDR     R0, =SystemInit
        BLX     R0

        LDR     R0, =__iar_program_start
        BX      R0

PreCacheInit:
        LDR     R0, =Cache_Ctrl_Base
        MOVS    R1, #Cache_Disable
        STR     R1, [R0, #0]

        LDR     R0, =Cache_Entry_Start
        LDR     R1, =Cache_Start_Addr
        STR     R1, [R0, #0]

        LDR     R0, =Cache_Entry_End
        LDR     R1, =Cache_End_Addr
        STR     R1, [R0, #0]

        LDR     R0, =Cache_Channel_En
        MOVS    R1, #0x1
        STR     R1, [R0, #0]

        LDR     R0, =Cache_Ctrl_Base
        LDR     R1, =Cache_Enable
        STR     R1, [R0, #0]

        LDR     R0, =Cache_Ctrl_Op
        LDR     R1, =Cache_Invalidate_All
        STR     R1, [R0, #0]

        BX      LR

Jump_Cmd
        ORR     R0,R0, #0x01
        BX      R0
		
        PUBWEAK SvcCall_Main
        SECTION .ramTEXT:CODE:REORDER:ROOT(2)
SvcCall_Main
                IMPORT  Flash_ReturnReady
                CPSID   I	
                PUSH    {LR}
                LDR     R0, =Flash_ReturnReady
                BLX     R0
                POP     {LR}
                CPSIE   I
                LDR     R0, =SVC_Handler
                BX      R0


        PUBWEAK PendSV_Main
        SECTION .ramTEXT:CODE:REORDER:ROOT(2)
PendSV_Main
                IMPORT  Flash_ReturnReady
                CPSID   I
                PUSH    {LR}
                LDR     R0, =Flash_ReturnReady
                BLX     R0
                POP     {LR}
                CPSIE   I
                LDR     R0, =PendSV_Handler
                BX      R0           

        PUBWEAK SysTick_Main
        SECTION .ramTEXT:CODE:REORDER:ROOT(2)
SysTick_Main
                IMPORT  Flash_ReturnReady
                CPSID   I
                PUSH    {LR}
                LDR     R0, =Flash_ReturnReady
                BLX     R0
                CPSIE   I
                LDR     R0, =SysTick_Handler
                BLX     R0
                POP     {LR}
                BX      LR
		
        PUBWEAK isrC_main
        SECTION .ramTEXT:CODE:REORDER:NOROOT(2)
isrC_main
       B       .


        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
HardFault_Handler
        B HardFault_Handler

        PUBWEAK MemManage_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
MemManage_Handler
        B MemManage_Handler

        PUBWEAK BusFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
BusFault_Handler
        B BusFault_Handler

        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
UsageFault_Handler
        B UsageFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SVC_Handler
        B SVC_Handler

        PUBWEAK DebugMon_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
DebugMon_Handler
        B DebugMon_Handler

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
PendSV_Handler
        B PendSV_Handler

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SysTick_Handler
        B SysTick_Handler

        PUBWEAK Default_IRQ_Handler
        SECTION .text:CODE:REORDER:NOROOT(2)
Default_IRQ_Handler
        B       .



        END
;************************ (C) COPYRIGHT MEDIATEK *****END OF FILE*****
