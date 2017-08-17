/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
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

#include "stdio.h"
#include "stdlib.h"

/* Cortex-M MCU related headers */
#if defined(__GNUC__)
#include "cmsis_nvic.h"
#endif
#include "mt7687.h"

#include "flash_map.h"
#ifdef MTK_FOTA_DUAL_IMAGE_ENABLE
#include "flash_map_dual.h"
#include "fota_dual_image.h"
#endif

#include "top.h"
#include "hal_gpt.h"

#ifdef MTK_BOOTLOADER_RESTORE_DEFAULT_ENABLE
#include "hal_gpio.h"
#include "hal_flash.h"
#endif

#include "hw_uart.h"
#include "xmodem.h"

#ifndef MOD_CFG_LOADER_DISABLE_FOTA
#include "fota_config.h"
#endif

#include "bsp_gpio_ept_config.h"

#ifdef MTK_MPERF_ENABLE
#include "mperf.h"
#endif

#include "spi_flash.h"

#define CM4_SYSRAM_END            0x20040000

#define CACHE_CTRL_BASE     ((volatile uint32_t *)0x01530000)
#define CACHE_OP_BASE       ((volatile uint32_t *)0x01530004)
#define CACHE_CHANNEL_EN    ((volatile uint32_t *)0x0153002C)
#define CACHE_ENTRY_START   ((volatile uint32_t *)0x01540000)
#define CACHE_ENTRY_END     ((volatile uint32_t *)0x01540040)

#define CACHE_DISABLE       (0x00000000)
#define CACHE_ENABLE        (0x0000030D)
#define CACHE_START_ADDR    (0x10000100)
#define CACHE_END_ADDR      (0x11000000)
#define CACHE_INVALIDATE_ALL (0x03)

#ifdef MTK_MPERF_ENABLE
static mperf_profile_t        MPERF_RANDOM_SECT g_mperf_profile;
static mperf_profile_sample_t MPERF_RANDOM_SECT g_mperf_samples[MPERF_SAMPLE_ARRAY_SIZE];
#define MTK_MPERF_BL_WRAPPER(statement) statement
#else
#define MTK_MPERF_BL_WRAPPER(statement) do { } while (0)
#endif


int  main(void);
void ResetISR(void);
void NMIISR(void);
void FaultISR(void);
void ExpDefHandler(void);

#if !defined(__GNUC__)
extern void Jump_Cmd(int);
#endif

#if defined(__GNUC__)
/*
 * Symbols defined in linker script.
 */
extern unsigned long _text_start;
extern unsigned long _text_end;
extern unsigned long _text_lma_start;
extern unsigned long _data_start;
extern unsigned long _data_end;
extern unsigned long _data_lma_start;
extern unsigned long _bss_start;
extern unsigned long _bss_end;
extern unsigned long _bss_lma_start;

static unsigned long KernelStack[2048];

/*
 * ARM CM4 Exception Vectors
 */
__attribute__ ((section(".except_vectors"))) unsigned long __isr_vector[32] =
{
    ((unsigned long)(KernelStack)) + sizeof(KernelStack),                 // Master stack pointer(MSP)
    (unsigned long) &_text_lma_start + sizeof(__isr_vector) + 1 , // 1, Reset, in XIP, +1 is for thumb mode
    (unsigned long) NMIISR,                                               // 2, NMI
    (unsigned long) FaultISR,                                             // 3, Hard fault
    (unsigned long) ExpDefHandler,                                        // 4, MPU fault
    (unsigned long) ExpDefHandler,                                        // 5, Bus fault
    (unsigned long) ExpDefHandler,                                        // 6, Usage fault
    (unsigned long) 0,                                                    // 7, Reserved
    (unsigned long) 0,                                                    // 8, Reserved
    (unsigned long) 0,                                                    // 9, Reserved
    (unsigned long) 0,                                                    // 10, Reserved
    (unsigned long) ExpDefHandler,                                        // 11, SVCall
    (unsigned long) ExpDefHandler,                                        // 12, Debug monitor
    (unsigned long) 0,                                                    // 13, Reserved
    (unsigned long) ExpDefHandler,                                        // 14, PendSV
    (unsigned long) 0,                                                    // 15, Systick
    (unsigned long) ExpDefHandler,                                        // 16, irq0, UART1
    (unsigned long) ExpDefHandler,                                        // 17, irq1,
    /* ... */
};

static void jump_to(unsigned long pc_addr, unsigned long sp_addr)
{
    __asm(

        "    isb                        \n\t"
        "    dsb                        \n\t"

        /* update MSP */
        "    mov   sp, %0               \n\t"

        /* jump to Reset ISR and never return */
        "    mov   r0, %1               \n\t"
        "    bx    r0                   \n\t"
        :: "r" (sp_addr), "r" (pc_addr) :
    );
}

/*
 * Using specific section(.reset_isr) to mark this routine in ld-script to arrange its location
 * (to behind the vector table)
 */
__attribute__ ((section(".reset_isr"))) void ResetISR(void)
{
    unsigned long current_pc;
    /*
     *  The local variables in ResetISR() are cleared to zero.
     */
    unsigned long *src = NULL;
    unsigned long *dst = NULL;
    unsigned long *end = NULL;

    *CACHE_CTRL_BASE    = CACHE_DISABLE;
    *CACHE_ENTRY_START  = CACHE_START_ADDR;
    *CACHE_ENTRY_END    = CACHE_END_ADDR;
    *CACHE_CHANNEL_EN   = 1;
    *CACHE_CTRL_BASE    = CACHE_ENABLE;
    *CACHE_OP_BASE      = CACHE_INVALIDATE_ALL;

    /*
     * Loader code: Copy both text and data sections from flash to SYSRAM.
     */
    /*
     * This routine(ResetISR()) is running on Flash(XIP), although its VMA is resident in SYSRAM.
     *
     * The reason that it can run safely is we assume GCC generates PIC code for following code.
     *
     * For example, the following "ldr" instruction are "PC related" based addressing, after GCC compiles the
     * C statement.

            unsigned long text_sect_len = (unsigned long)&_text_end - (unsigned long)&_text_start;

            2000040e:       4a22            ldr     r2, [pc, #136]  ; (20000498 <done+0xa>)
            20000410:       4b22            ldr     r3, [pc, #136]  ; (2000049c <done+0xe>)
            20000412:       1ad3            subs    r3, r2, r3
            20000414:       607b            str     r3, [r7, #4]

     * Another way to run it safely on XIP is using pure assembly instead of C code.
     *
         */

    unsigned long text_sect_len = (unsigned long)&_text_end - (unsigned long)&_text_start;
    unsigned long data_sect_len = (unsigned long)&_data_end - (unsigned long)&_data_start;

    //*((volatile unsigned int *)(0x83050030)) = 0x3;

    /* Get current program counter to judge whether if we are resident in SYSRAM or others */
    __asm volatile(
        /* update MSP */
        "       bl    getpc            \n\t"
        "getpc:                        \n\t"
        "       mov    %0, r14         \n\t"
                : "=r" (current_pc) ::
        );
    if(current_pc > (unsigned long)&_text_start && current_pc < ((unsigned long)&_text_start+text_sect_len)){
        /* do nothing, we may be resident in SYSRAM(VMA) already. */
    }else{
        /* resident in flash, do self-relocation */
        src = (unsigned long *)&_text_lma_start;
        dst = (unsigned long *)&_text_start;
        end = (unsigned long *)((unsigned char *)(&_text_lma_start) + text_sect_len);

        for(;src < end; src++, dst++)
            *dst = *src;

        src = (unsigned long *)&_data_lma_start;
        dst = (unsigned long *)&_data_start;
        end = (unsigned long *)((unsigned char *)(&_data_lma_start) + data_sect_len);

        for(; src < end; src++, dst++)
            *dst = *src;
    }

    /*
     * Assume the relocation is performed.
         * Trigger CM4 CPU invalidate internal buffer/cache
     */
    __asm volatile( "dsb" );
    __asm volatile( "isb" );

    /* BSS/ZI section */
    /*
     * Can't write in C to clear BSS section because the local stack variables of
     * this function are also resident in BSS section(kernel stack) actually, and the clear
     * action would clear these local variables to zero unexpectedly, and lead
     * to infinite loop.
     *
     * So using cpu registers operation to complete it.
     */
    __asm volatile(
        "    ldr    r0, =_bss_start \n\t"
        "    ldr    r1, =_bss_end   \n\t"
        "loop:                      \n\t"
        "    cmp    r0, r1          \n\t"
        "    bcc    clear           \n\t"
        "    b      done            \n\t"
        "clear:                     \n\t"
        "    movs   r2, #0          \n\t"
        "    str    r2, [r0]        \n\t"
        "    adds   r0, #4          \n\t"
        "    b      loop            \n\t"
        "done:                      \n\t");

    /*
     * Branch to main() which is on SYSRAM. but we can't just call main() directly.
     *
     * That is because compiler will generate PIC code (mentioned as above comment) for main() call statment,
     * and it will jump to main() in flash(XIP).
     *
     * So we force a long jump by assembly here.
     */
        __asm volatile(
        "       ldr   r0, =main \n\t"
        "       bx    r0        \n\t");

}
#endif

void NMIISR(void)
{
    while(1);
}

void FaultISR(void)
{
    while(1);
}

void MPUFaultFhandler(unsigned long add)
{
}

void ExpDefHandler(void)
{
    while(1);
}


void SerialFlashISR(void)
{
    // should not happened
    while(1);
}

#ifndef MOD_CFG_LOADER_DISABLE_FOTA

#ifndef MTK_FOTA_DUAL_IMAGE_ENABLE
#define UPGRADE_BLOCK_SIZE  (4096)

static void process_fota(void)
{
    static uint8_t buffer[ UPGRADE_BLOCK_SIZE ];

    fota_init(&fota_flash_default_config);

    if (fota_is_empty(FOTA_PARITION_TMP) == FOTA_STATUS_IS_FULL) {
        hw_uart_puts("fota: upgrade\r\n");
    } else {
        hw_uart_puts("fota: TMP is empty, skip upgrade\r\n");
        return;
    }

    /* prepare source */
    fota_seek(FOTA_PARITION_TMP, 0);

    /* prepare target */
    fota_seek(FOTA_PARITION_CM4, 0);

    hw_uart_puts("fota: start copy\r\n");
    /* start FOTA */
    if (fota_copy(FOTA_PARITION_CM4,
                  FOTA_PARITION_TMP,
                  &buffer[0],
                  UPGRADE_BLOCK_SIZE) == FOTA_STATUS_OK) {
        hw_uart_puts("fota: upgrade done\r\n");
        return;
    }
    hw_uart_puts("fota: upgrade failed\r\n");
    return;
}
#endif /* MTK_FOTA_DUAL_IMAGE_ENABLE */
#endif /* ifndef MOD_CFG_LOADER_DISABLE_FOTA */

#ifdef MTK_BOOTLOADER_RESTORE_DEFAULT_ENABLE
static int sys_restore_default_request(void)
{
#define RTC_INT_GPIO_PIN    HAL_GPIO_0       /* HDK board: WS3367, MT7687 Main Board-V22 */
#define RTC_INT_GPIO_PMUX   HAL_GPIO_0_GPIO0 /* HDK board: WS3367, MT7687 Main Board-V22 */

    hal_gpio_status_t   r_gpio;
    hal_gpio_data_t     data;

    r_gpio = hal_gpio_init(RTC_INT_GPIO_PIN);
    r_gpio = hal_gpio_set_direction(RTC_INT_GPIO_PIN, HAL_GPIO_DIRECTION_INPUT);
    r_gpio = hal_gpio_pull_down(RTC_INT_GPIO_PIN);
    r_gpio = hal_gpio_deinit(RTC_INT_GPIO_PIN);

    r_gpio = hal_gpio_get_input(RTC_INT_GPIO_PIN, &data);

    return (data != 0);
}

static int sys_restore_default_do(fota_flash_t *t)
{
    int                 addr;
    int                 i;

    hal_flash_init();

    for (i = 0; i < t->table_entries; i++) {
        if (t->table[i].id != FOTA_PARITION_NV) {
            continue;
        }

        addr = t->table[i].address;

        while ((addr + 4096) <= (t->table[i].address + t->table[i].length)) {
            hal_flash_erase(addr, HAL_FLASH_BLOCK_4K);
            addr += 4096;
        }
    }

    hal_flash_deinit();
}
#else
#define sys_restore_default_request()   (0)
#define sys_restore_default_do(__p__)
#endif /* MTK_BOOTLOADER_RESTORE_DEFAULT_ENABLE */


#ifdef MTK_BOOTLOADER_MENU_ENABLE
#define FOTU_BLOCK_SIZE (256)

 typedef struct fotu_job_s
{
    uint32_t partition;
    uint32_t size;
    uint32_t pos;
} fotu_job_t;


static int callbacks;
static int rx_bytes;

static void fotu_rx_callback(void *ptr, uint8_t *buffer, int len)
{
    fotu_job_t  *job = ptr;

    if (!ptr)
        return;

    if (len <= 0) {
        return;
    }

    job->pos += len;

    if (job->pos > job->size) {
        return;
    }

    callbacks += 1;
    rx_bytes  += len;
    fota_write(job->partition, buffer, len);
}

static void fotu_do(uint32_t partition, uint32_t len)
{
    uint8_t     buffer[ FOTU_BLOCK_SIZE ];
    fotu_job_t  fotu_job = { .partition = partition, .size = len, .pos = 0 };

    /* prepare target */
    fota_init(&fota_flash_default_config);
    fota_seek(partition, 0);

    xmodem_block_rx(&fotu_job, buffer, len, fotu_rx_callback);

    //hw_uart_printf("callback %d\n", callbacks);
    //hw_uart_printf("rx_bytes %d\n", rx_bytes);

    rx_bytes = callbacks = 0;

    return;
}

#define WAIT_USER_INPUT_SEC	(60)    /* wait 1 secs for user input */

static void user_select(void)
{
    int loop_count;
    int ch;

    while (1) {
        /* show menu and get user's input */
        while (hw_uart_getc() != -1)
            ;

        hw_uart_printf("\r\n");
        hw_uart_printf("1. Burn flash to 0x%x(BootLoader)\r\n", LOADER_BASE+0x10000000);
        hw_uart_printf("2. Burn flash to 0x%x(XIP)\r\n", CM4_CODE_BASE+0x10000000);
        hw_uart_printf("3. Burn flash to 0x%x(FW)\r\n", N9_RAMCODE_BASE+0x10000000);
        hw_uart_printf("c. Boot from flash 0x%x(XIP)\r\n", CM4_CODE_BASE+0x10000000);

        loop_count = WAIT_USER_INPUT_SEC * 10;

        /* wait for user input */
        while (loop_count > 0 && (ch = hw_uart_getc()) == -1) {
            hal_gpt_delay_ms(100);
            loop_count--;
            if (!(loop_count % 10)) {
                hw_uart_printf("\r  %d   ", loop_count/10);
            }
            ch = 'c';
        }

        hw_uart_printf("\r\n");
        hw_uart_printf("Your choose %c\r\n", ch);

        /* dispatch user's input */
        switch (ch) {
        case '1':
            fotu_do(FOTA_PARITION_LOADER, LOADER_LENGTH);
            break;

        case '2':
            fotu_do(FOTA_PARITION_CM4, CM4_CODE_LENGTH);
            break;

        case '3':
            fotu_do(FOTA_PARITION_NCP, N9_RAMCODE_LENGTH);
            break;

        case 'C':
        case 'c':
            return;
            break;

        default:
            break;
        }
    }
}
#else
#define user_select()
#endif /* ifndef MTK_BOOTLOADER_MENU_ENABLE */


static void process_menu(void)
{
    unsigned long sp_addr = 0;
    unsigned long pc_addr = 0;

    (void)sp_addr;
    (void)pc_addr;

    if (sys_restore_default_request()) {
        hw_uart_printf("restore default\r\n");
        sys_restore_default_do(&fota_flash_default_config);
    }

#ifndef MOD_CFG_LOADER_DISABLE_FOTA
    MTK_MPERF_BL_WRAPPER(mperf_profile_tag(&g_mperf_profile, "fb"));
  #ifndef MTK_FOTA_DUAL_IMAGE_ENABLE
    process_fota();
  #endif  /*MOD_CFG_LOADER_DISABLE_FOTA*/
    MTK_MPERF_BL_WRAPPER(mperf_profile_tag(&g_mperf_profile, "fd"));
#endif

    user_select();

    sp_addr = CM4_SYSRAM_END;
    MTK_MPERF_BL_WRAPPER(mperf_profile_tag(&g_mperf_profile, "bx"));

#ifndef MTK_FOTA_DUAL_IMAGE_ENABLE
    hw_uart_printf("jump to (%x) \r\n", (CM4_CODE_BASE + 0x10000000) );

#if defined(__GNUC__)
    pc_addr = (CM4_CODE_BASE + 0x10000000) | 0x1; /* switch to ARM state */
    jump_to(pc_addr, sp_addr);
//#elif defined(__CC_ARM)
#else
    Jump_Cmd((CM4_CODE_BASE + 0x10000000));
#endif

#else /* MTK_FOTA_DUAL_IMAGE_ENABLE */
    fota_image_type_t fit = FOTA_DUAL_IMAGE_A;

    fota_dual_image_init();
    fota_query_active_image(&fit);

    if (FOTA_DUAL_IMAGE_A == fit) {
        pc_addr = CM4_CODE_BASE_A          ;
    } else {
        pc_addr = CM4_CODE_BASE_B          ;
    }
    hw_uart_printf("jump to (%x) \r\n", (pc_addr + 0x10000000) );
#if defined(__GNUC__)
    pc_addr = (pc_addr + 0x10000000) | 0x1; /* switch to ARM state */
    jump_to(pc_addr, sp_addr);
//#elif defined(__CC_ARM)
#else
    Jump_Cmd((pc_addr + 0x10000000));
#endif

#endif /* MTK_FOTA_DUAL_IMAGE_ENABLE */

    return;
}


int main(void)
{
#ifdef MTK_MPERF_ENABLE
    g_mperf_profile.magic       = MPERF_PROFILE_MAGIC_NO;
    g_mperf_profile.index       = 0;
    g_mperf_profile.array_size  = MPERF_SAMPLE_ARRAY_SIZE;
    g_mperf_profile.samples     = &g_mperf_samples[0];
#endif

    MTK_MPERF_BL_WRAPPER(mperf_profile_enable(&g_mperf_profile));

    MTK_MPERF_BL_WRAPPER(mperf_profile_tag(&g_mperf_profile, "00"));

#if defined(__GNUC__)
    /*
     * Setup NVIC vector tables to "__isr_vector" which is in SYSRAM.
     */

    __isr_vector[1] = (unsigned long)ResetISR;

    NVIC_SetupVectorTable((unsigned long)__isr_vector);
#endif
    /*
     * Capture Exceptions.
     */

    SCB->SHCSR = SCB_SHCSR_USGFAULTENA_Msk |
                 SCB_SHCSR_BUSFAULTENA_Msk |
                 SCB_SHCSR_MEMFAULTENA_Msk;
    /*
     * Enable UART.
     */

    top_xtal_init();

    if( 0 == (HAL_REG_32(CM4_WIC_PEND_STA0_ADDR) & BIT(31)) ){
        /* (HAL_REG_32(CM4_WIC_PEND_STA0_ADDR) & BIT(31)) is hal_lp_get_wic_status() */

        /* initialize all pins by EPT */
        bsp_ept_gpio_setting_init();
    }

    hw_uart_init();

    MTK_MPERF_BL_WRAPPER(mperf_profile_tag(&g_mperf_profile, "ua"));

    hw_uart_puts("loader init \r\n");

    /* sti */
    __enable_irq();

    MTK_MPERF_BL_WRAPPER(mperf_profile_tag(&g_mperf_profile, "mu"));

    flash_switch_mode(SPIQ);
    /* user_target_addr */
    process_menu();

    return 0;
}


