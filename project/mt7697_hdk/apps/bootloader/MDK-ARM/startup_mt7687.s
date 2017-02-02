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

; main stack allocates at end of TCM, which is determined by scatter file
Stack_Size      EQU     0xC00

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

; fake Heap, to make compilation pass on KEIL MDK
; real heap allocates at ZI space
Heap_Size       EQU     0x8

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

; Macro defines
Cache_Ctrl_Base    EQU     0x01530000
Cache_Op_Base      EQU     0x01530004
Cache_Channel_En   EQU     0x0153002C
Cache_Entry_Start  EQU     0x01540000
Cache_Entry_End    EQU     0x01540040

Cache_Disable      EQU     0x0
Cache_Enable       EQU     0x30D
Cache_Start_Addr   EQU     0x10000100
Cache_End_Addr     EQU     0x11000000
Cache_Invalidate_All EQU   0x03

                PRESERVE8
                THUMB

; Vector Table Mapped to head of RAM
                AREA    |.isr_vector|, DATA, READONLY
                EXPORT  __isr_vector
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__isr_vector    DCD     __initial_sp               ; Top of Stack
                DCD     reset_isr                  ; Reset Handler
                DCD     NMIISR                     ; NMI Handler
                DCD     FaultISR                   ; Hard Fault Handler
                DCD     ExpDefHandler              ; MPU Fault Handler
                DCD     ExpDefHandler              ; Bus Fault Handler
                DCD     ExpDefHandler              ; Usage Fault Handler
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     0                          ; Reserved
                DCD     ExpDefHandler              ; SVCall Handler
                DCD     ExpDefHandler              ; Debug Monitor Handler
                DCD     0                          ; Reserved
                DCD     ExpDefHandler              ; PendSV Handler
                DCD     0                          ; SysTick Handler
                DCD     ExpDefHandler              ; irq0, UART1
                DCD     ExpDefHandler              ; irq1,
                                         
__Vectors_End

__Vectors_Size  EQU  __Vectors_End - __isr_vector 

                AREA    |.reset_isr|, CODE, READONLY


reset_isr   PROC
                EXPORT  reset_isr              [WEAK]
                EXPORT  Jump_Cmd
                ;BKPT    #0x3
                IMPORT  SystemInit
                IMPORT  main
                IMPORT  |Image$$STACK$$ZI$$Base|
                IMPORT  |Image$$STACK$$ZI$$Limit|
                IMPORT  |Load$$RAM$$Base|
                IMPORT  |Image$$RAM$$Base|
                IMPORT  |Image$$RAM$$Length|
                IMPORT  |Image$$RAM$$ZI$$Base|
                IMPORT  |Image$$RAM$$ZI$$Length|

;                LDR     SP, =__initial_sp
; stack
                LDR     SP, =|Image$$STACK$$ZI$$Limit|

                ;cache disable
                DSB
                ISB
                ; cache enable
                BL      PreCacheInit

                ;RAM space init
                LDR r0, = |Load$$RAM$$Base|
                LDR r1, = |Image$$RAM$$Base|
                CMP r0, r1
                BEQ do_zi_init
                MOV r2, r1
                LDR r4, = |Image$$RAM$$Length|
                ADD r2, r2, r4
                BL copy
do_zi_init
                LDR r1, = |Image$$RAM$$ZI$$Base|
                MOV r2, r1
                LDR r4, = |Image$$RAM$$ZI$$Length|
                ADD r2, r2, r4
                MOV r3, #0
                BL zi_init

                ;stack space zero init
                MOVS    R0, #0
                LDR     R1, =|Image$$STACK$$ZI$$Base|
                LDR     R2, =|Image$$STACK$$ZI$$Limit|

FillZero
                STR     R0, [R1], #4
                CMP     R1, R2
                BCC     FillZero

                LDR     R0, =SystemInit
                BLX     R0

                LDR     R0, =main
                BX      R0
                ENDP

copy
                CMP r1, r2
                LDRCC r3, [r0], #4
                STRCC r3, [r1], #4
                BCC copy
                BX LR
zi_init
                CMP r1, r2
                STRCC r3, [r1], #4
                BCC zi_init
                BX LR

PreCacheInit    PROC
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

                LDR     R0, =Cache_Op_Base
                LDR     R1, =Cache_Invalidate_All
                STR     R1, [R0, #0]

                BX      LR
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMIISR     PROC
                EXPORT  NMIISR                [WEAK]
                B       .
                ENDP
FaultISR\
                PROC
                EXPORT  FaultISR          [WEAK]
                B       .
                ENDP
ExpDefHandler\
                PROC
                EXPORT  ExpDefHandler          [WEAK]
                B       .
                ENDP

                ALIGN
;Jump_Cmd
Jump_Cmd
                ORR     r0, #0x01
                BX      r0
;*******************************************************************************
; User Stack and Heap initialization
;*******************************************************************************
                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
                 
__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF
                NOP
                END

;************************ (C) COPYRIGHT MEDIATEK *****END OF FILE*****
