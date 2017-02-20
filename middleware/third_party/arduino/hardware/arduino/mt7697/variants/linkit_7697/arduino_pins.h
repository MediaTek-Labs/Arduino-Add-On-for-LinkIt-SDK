#ifndef __ARDUINO_PINS_H__
#define __ARDUINO_PINS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pin_mux.h"

#if 1
/*
	Left-hand side:
	P0: GPIO2  		UART0_RX
	P1: GPIO3 		UART0_TX
	P2: GPIO0
	P3: GPIO39
	P4: GPIO34
	P5: GPIO33

	Righ-hand side:
	P6:  GPIO37
	P7:  GPIO36
	P8:  GPIO27		I2C1_CLK
	P9:  GPIO28		I2C1_DATA
	P10: GPIO32		SPI_CS0
	P11: GPIO29		SPI_MOSI
	P12: GPIO30		SPI_MISO
	P13: GPIO31		SPI_SCK
	P14: GPIO57		ADC_IN0
	P15: GPIO58		ADC_IN1
	P16: GPIO59		ADC_IN2
	P17: GPIO60		ADC_IN3
*/

typedef enum arduino_pin {
	/*
	 * NOTE: The order of [0...13] should not be changed!!
	 */
	RXB	    = 0,        /*  0: UART0_RX: GPIO2 */
	TXB	    = 1,        /*  1: UART0_TX: GPIO3 */
	GPIO2   = 0,        /*  0: UART0_RX */
	GPIO3   = 1,        /*  1: UART0_TX */
	GPIO0   = 2,        /*  2 */
	GPIO39  = 3,        /*  3 */
	GPIO34  = 4,        /*  4 */
	GPIO33  = 5,        /*  5 */
	GPIO37  = 6,        /*  6 */
	GPIO36  = 7,        /*  7 */
    TXA     = 6,        /*  6: UART1_TX: GPIO37 */
    RXA     = 7,        /*  7: UART1_RX: GPIO36 */

	GPIO27  = 8,        /*  8 */
	GPIO28  = 9,        /*  9 */
	SCL     = 8,        /* 8: SCL */
	SDA     = 9,        /* 9: SDA */
	GPIO32  = 10,       /* 10: SS */
	SS      = 10,
	GPIO29  = 11,       /* 11: MOSI */
	MOSI    = 11,
	GPIO30  = 12,       /* 12: MISO */
	MISO    = 12,
	GPIO31  = 13,       /* 13: SCK */
	SCK     = 13,

	GPIO57  = 14,       /* 14: GPIO57 */
	GPIO58  = 15,       /* 15: GPIO58 */
	GPIO59  = 16,       /* 16: GPIO59 */
	GPIO60  = 17,       /* 17: GPIO60 */
	A0      = 14,       /* 14: ADC_IN0: GPIO57 */
	A1      = 15,       /* 15: ADC_IN1: GPIO58 */
	A2      = 16,       /* 16: ADC_IN2: GPIO59 */
	A3      = 17,       /* 17: ADC_IN3: GPIO60 */
	MAX_ARDUINO_PIN,	/* end of pin definition */
    INVALID_PIN_NO
} arduino_pin_t;

#else
/*
 *   GPIO2: GPIO4     SPI_D0_EXT   | 20
 *   GPIO5: GPIO5     SPI_D1_EXT   | 21
 *   GPIO7: GPIO7     SPI_CS_EXT   | 22
 *  GPIO24: GPIO24    SPI_D2_EXT   | 23
 *  GPIO25: GPIO25    SPI_D3_EXT   | 24
 *  GPIO26: GPIO26    SPI_CLK_EXT  | 25
 *     RXA: GPIO2     CM4_UART1_RX | 26
 *     TXA: GPIO3     CM4_UART1_TX | 27
 *
 *       0: GPIO36    RXB UART2    |  0        14 |    SCL: GPIO27    I2C0 SCL
 *       1: GPIO37    TXB UART2    |  1        15 |    SDA: GPIO28    I2C0 SDA
 *       2: GPIO0                  |  2        16 |     A3: GPIO57    ADC_IN0
 *       3: GPIO1                  |  3        17 |     A2: GPIO58    ADC_IN1
 *       4: GPIO6                  |  4        18 |     A1: GPIO59    ADC_IN2
 *       5: GPIO33                 |  5        19 |     A0: GPIO60    ADC_IN3
 *       6: GPIO34                 |  6           |
 *       7: GPIO35                 |  7           |
 *                                 |  8           |
 *       8: GPIO38                 |  9           |
 *       9: GPIO39                 | 10           |
 *      10: GPIO32    SS           | 11           |
 *      11: GPIO29    MOSI         | 12           |
 *      12: GPIO30    MISO         | 13           |
 *      13: GPIO31    SCK          |              |
 *     GND: GND                    |              |
 *       X: NC                     |              |
 *     SDA: GPIO28    I2C0 SDA     | 15           |
 *     SCL: GPIO27    ICC0 SCL     | 14           |
 */

typedef enum arduino_pin {
	/*
	 * NOTE: The order of [0...13] should not be changed!!
	 */
	RXB	= 0,				/*  0: UART2_RX: GPIO36 */
	TXB	= 1,				/*  1: UART2_TX: GPIO37 */
	GPIO36	= 0,				/*  0: UART2_RX */
	GPIO37,					/*  1: UART2_TX */
	GPIO0,					/*  2 */
	GPIO1,					/*  3 */
	GPIO6,					/*  4 */
	GPIO33,					/*  5 */
	GPIO34,					/*  6 */
	GPIO35,					/*  7 */

	GPIO38,					/*  8 */
	GPIO39,					/*  9 */
	GPIO32,					/* 10: SS */
	GPIO29,					/* 11: MOSI */
	GPIO30,					/* 12: MISO */
	GPIO31,					/* 13: SCK */

	SCL,					/* 14: I2C0 SCL: GPIO27 */
	SDA,					/* 15: I2c0 SDA: GPIO28 */
	A3,					/* 16: ADC_IN0: GPIO57 */
	A2,					/* 17: ADC_IN1: GPIO58 */
	A1,					/* 18: ADC_IN2: GPIO59 */
	A0,					/* 19: ADC_IN3: GPIO60 */

	GPIO4,					/* 20 */
	GPIO5,					/* 21 */
	GPIO7,					/* 22 */
	GPIO24,					/* 23 */
	GPIO25,					/* 24 */
	GPIO26,					/* 25 */
	RXA,					/* 26: UART1_RX: GPIO2 */
	TXA,					/* 27: UART1_TX: GPIO3 */
	MAX_PIN_NO,
	INVALID_PIN_NO
} arduino_pin_t;
#endif

extern pin_desc_t *get_arduino_pin_desc(arduino_pin_t pin_no);

#ifdef __cplusplus
}
#endif

#endif
