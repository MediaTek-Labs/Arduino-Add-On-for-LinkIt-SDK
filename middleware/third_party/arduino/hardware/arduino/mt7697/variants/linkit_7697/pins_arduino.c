#include "pins_arduino.h"

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
pin_desc_t *get_arduino_pin_desc(arduino_pin_t pin_no)
{
	static pin_desc_t arduino_pin_desc_tab[] = {
		[GPIO2]		= PIN_DESC_2,		/*  0: RXB */
		[GPIO3]		= PIN_DESC_3,		/*  1: TXB */
		[GPIO0]		= PIN_DESC_0, 		/*  2 */
		[GPIO39]	= PIN_DESC_39, 		/*  3 */
		[GPIO34]	= PIN_DESC_34, 		/*  4 */
		[GPIO33]	= PIN_DESC_33,		/*  5 */
		[GPIO37]	= PIN_DESC_37,		/*  6 */
		[GPIO36]	= PIN_DESC_36,		/*  7 */

		[GPIO27]	= PIN_DESC_27,		/*  8 */
		[GPIO28]	= PIN_DESC_28,		/*  9 */
		[GPIO32]	= PIN_DESC_32,		/* 10: SS   */
		[GPIO29]	= PIN_DESC_29,		/* 11: MOSI */
		[GPIO30]	= PIN_DESC_30,		/* 12: MISO */
		[GPIO31]	= PIN_DESC_31,		/* 13: SCK  */

		[A0]		= PIN_DESC_57,		/* 16: ADC_IN0  */
		[A1]		= PIN_DESC_58,		/* 17: ADC_IN1  */
		[A2]		= PIN_DESC_59,		/* 18: ADC_IN2  */
		[A3]		= PIN_DESC_60,		/* 19: ADC_IN3  */
	};

	if (pin_no >= sizeof(arduino_pin_desc_tab)/sizeof(pin_desc_t))
		return NULL;

	return &arduino_pin_desc_tab[pin_no];
}

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
pin_desc_t *get_arduino_pin_desc(arduino_pin_t pin_no)
{
	static pin_desc_t arduino_pin_desc_tab[] = {
		[GPIO36]	= PIN_DESC_36,		/*  0: RXB */
		[GPIO37]	= PIN_DESC_37,		/*  1: TXB */
		[GPIO0]		= PIN_DESC_0, 		/*  2 */
		[GPIO1]		= PIN_DESC_1, 		/*  3 */
		[GPIO6]		= PIN_DESC_6, 		/*  4 */
		[GPIO33]	= PIN_DESC_33,		/*  5 */
		[GPIO34]	= PIN_DESC_34,		/*  6 */
		[GPIO35]	= PIN_DESC_35,		/*  7 */

		[GPIO38]	= PIN_DESC_38,		/*  8 */
		[GPIO39]	= PIN_DESC_39,		/*  9 */
		[GPIO32]	= PIN_DESC_32,		/* 10: SS   */
		[GPIO29]	= PIN_DESC_29,		/* 11: MOSI */
		[GPIO30]	= PIN_DESC_30,		/* 12: MISO */
		[GPIO31]	= PIN_DESC_31,		/* 13: SCK  */

		[SCL]		= PIN_DESC_27,		/* 14: I2C0 SCL */
		[SDA]		= PIN_DESC_28,		/* 15: I2C0 SDA */
		[A3]		= PIN_DESC_57,		/* 16: ADC_IN0  */
		[A2]		= PIN_DESC_58,		/* 17: ADC_IN1  */
		[A1]		= PIN_DESC_59,		/* 18: ADC_IN2  */
		[A0]		= PIN_DESC_60,		/* 19: ADC_IN3  */

		[GPIO4]		= PIN_DESC_4,		/* 20: SPI_D0_EXT   */
		[GPIO5]		= PIN_DESC_5,		/* 21: SPI_D1_EXT   */
		[GPIO7]		= PIN_DESC_7,		/* 22: SPI_CS_EXT   */
		[GPIO24]	= PIN_DESC_24,		/* 23: SPI_D2_EXT   */
		[GPIO25]	= PIN_DESC_25,		/* 24: SPI_D3_EXT   */
		[GPIO26]	= PIN_DESC_26,		/* 25: SPI_CLK_EXT  */
		[RXA]		= PIN_DESC_2,		/* 26: CM4_UART1_RX */
		[TXA]		= PIN_DESC_3		/* 27: CM4_UART1_TX */
	};

	if (pin_no < 0)
		return NULL;

	if (pin_no >= sizeof(arduino_pin_desc_tab)/sizeof(pin_desc_t))
		return NULL;

	return &arduino_pin_desc_tab[pin_no];
}
#endif

