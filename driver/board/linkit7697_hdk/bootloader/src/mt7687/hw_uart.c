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

#include <stdlib.h>

#include <hal_uart.h>
#include <hal_gpio.h>
#include <hal_lp.h>
#include <stdarg.h>
#include <stdint.h>

#if defined(BL_DEBUG)

//#define __ADV_DBG_PRINT__

#define BL_SLIM_UDIV_R(N, D, R) (((R)=(N)%(D)), ((N)/(D)))
#define BL_SLIM_UDIV(N, D) ((N)/(D))
#define BL_SLIM_UMOD(N, D) ((N)%(D))
#define BL_MAXCHARS     512
#define BL_MAXFRACT     10000
#define BL_NUM_FRACT     4

static char buf[BL_MAXCHARS] = {0} ;


/**********************************************************
Description : Convert an integer as string.
Input       : Buffer pointer, value to convert.
Output      : Non
***********************************************************/

static void bl_itoa(char **buf, uint32_t i, uint32_t base)
{
    char *s;
#define LEN   20
    uint32_t rem;
    static char rev[LEN + 1];

    rev[LEN] = 0;
    if (i == 0) {
        (*buf)[0] = '0';
        ++(*buf);
        return;
    }
    s = &rev[LEN];
    while (i) {
        i = BL_SLIM_UDIV_R(i, base, rem);
        if (rem < 10) {
            *--s = rem + '0';
        } else if (base == 16) {
            *--s = "abcdef"[rem - 10];
        }
    }
    while (*s) {
        (*buf)[0] = *s++;
        ++(*buf);
    }
}


/**********************************************************
Description : Convert floating point to string.
Input       : Buffer pointer, value to convert.
Output      : Non
***********************************************************/

static void bl_itof(char **buf, int32_t i)
{
    char *s;
#define LEN   20
    int32_t rem, j;
    static char rev[LEN + 1];

    rev[LEN] = 0;
    s = &rev[LEN];
    for (j = 0 ; j < BL_NUM_FRACT ; j++) {
        i = BL_SLIM_UDIV_R(i, 10, rem);
        *--s = rem + '0';
    }
    while (*s) {
        (*buf)[0] = *s++;
        ++(*buf);
    }
}


/**********************************************************
Description : debug print used internally
Input       : Buffer pointer, value to convert.
Output      : None
***********************************************************/
void bl_print_internal(char *fmt, va_list ap)
{
    int32_t    ival;
    char    *p, *sval;
    char    *bp, cval;

    int64_t   dval;
    int32_t    fract;
#ifdef __ADV_DBG_PRINT__
    uint32_t    uival, uival1, uival2;
    char    *bp_old;
    int32_t    i, j;
#endif /* __ADV_DBG_PRINT__ */
    bp = buf;
    *bp = 0;

    for (p = fmt; *p; p++) {
        if (*p != '%') {
            *bp++ = *p;
            continue;
        }
        switch (*++p) {
            case 'd':
                ival = va_arg(ap, int32_t);
                if (ival < 0) {
                    *bp++ = '-';
                    ival = -ival;
                }
                bl_itoa (&bp, ival, 10);
                break;

            case 'o':
                ival = va_arg(ap, int32_t);
                if (ival < 0) {
                    *bp++ = '-';
                    ival = -ival;
                }
                *bp++ = '0';
                bl_itoa (&bp, ival, 8);
                break;

            case 'x':
                ival = va_arg(ap, int32_t);
                if (ival < 0) {
                    *bp++ = '-';
                    ival = -ival;
                }
                *bp++ = '0';
                *bp++ = 'x';
                bl_itoa (&bp, ival, 16);
                break;
#ifdef __ADV_DBG_PRINT__
            case 'u':
                uival= va_arg(ap, unsigned int);
                *bp++= '0';
                *bp++= 'x';
                bp_old = bp;
                uival1 = uival >> 16;
                uival2 = uival & 0x0000ffff;
                bl_itoa(&bp, uival1, 16);
                i = (unsigned int)bp - (unsigned int)bp_old;
                if (i < 4) {
                    for (j = 3; j > (3 - i); j--) {
                        bp_old[j] = bp_old[j - (3 - i) - 1];
                    }
                    for (j = 0; j <= (3 - i); j++)
                        bp_old[j] = '0';
                }
                bp = bp_old + 4;
                bp_old = bp;
                bl_itoa(&bp, uival2, 16);
                i = (unsigned int)bp - (unsigned int)bp_old;
                if (i < 4) {
                    for (j = 3; j > (3 - i); j--) {
                        bp_old[j] = bp_old[j - (3 - i) - 1];
                    }
                    for (j = 0; j <= (3 - i); j++)
                        bp_old[j] = '0';
                }
                bp = bp_old + 4;
                break;
#endif /* __ADV_DBG_PRINT__ */
            case 'p':
                ival = va_arg(ap, int32_t);
                *bp++ = '0';
                *bp++ = 'x';
                bl_itoa (&bp, ival, 16);
                break;

            case 'c':
                cval = va_arg(ap, int32_t);
                *bp++ = cval;
                break;

            case 'f':
                dval = va_arg(ap, int64_t);
                if (dval < 0) {
                    *bp++ = '-';
                    dval = -dval;
                }
                if (dval >= 1.0) {
                    bl_itoa (&bp, (int32_t)dval, 10);
                } else {
                    *bp++ = '0';
                }
                *bp++ = '.';
                fract = (int32_t)((dval - (int64_t)(int32_t)dval) * (int64_t)(BL_MAXFRACT));
                bl_itof(&bp, fract);
                break;

            case 's':
                for (sval = va_arg(ap, char *) ; *sval ; sval++ ) {
                    *bp++ = *sval;
                }
                break;
        }
    }

    *bp = 0;
    for (bp = buf; *bp; bp++) {
        hal_uart_put_char(HAL_UART_0,*bp);
    }
}


void hw_uart_init(void)
{
    hal_uart_config_t config = {
                                 .baudrate    = HAL_UART_BAUDRATE_115200,
                                 .word_length = HAL_UART_WORD_LENGTH_8,
                                 .stop_bit    = HAL_UART_STOP_BIT_1,
                                 .parity      = HAL_UART_PARITY_NONE
                               };

    hal_uart_init(HAL_UART_0, &config);

#if 0
    hal_uart_put_char(HAL_UART_0, 'a');
    hal_uart_put_char(HAL_UART_0, 'b');
    hal_uart_put_char(HAL_UART_0, 'c');
    hal_uart_put_char(HAL_UART_0, '\n');
    hal_uart_put_char(HAL_UART_0, '\r');
#endif

}


void hw_uart_puts(const char *str)
{
    if (str == NULL) {
        return;
    }

    while (*str) {
        hal_uart_put_char(HAL_UART_0, *str);

        if (*str == '\n') {
            hal_uart_put_char(HAL_UART_0, '\r');
        }

        str++;
    }
}

void hw_uart_putc(char c)
{
    hal_uart_put_char(HAL_UART_0, c);
}

void hw_uart_printf(char *str, ...)
{
    if (str == NULL) {
        return;
    }

    va_list ap;
    va_start (ap, str);
    bl_print_internal(str, ap);
    va_end (ap);
}

/*
 * @fixme there is no exported non-blocking HAL UART get char API.
 */
extern int getc(void);

int hw_uart_getc(void)
{
    return getc();
}

#else

void hw_uart_init(void)
{

}


void hw_uart_puts(const char *str)
{
}

void hw_uart_putc(char c)
{
}

void hw_uart_printf(char *str, ...)
{

}

int hw_uart_getc(void)
{
    return -1;
}


#endif /* BL_DEBUG */
