/*
	OCP TTF Board

	This file should be part of a folder named 'ttf' at <root>/.platformio/framework-arduino-samd/variants
	<root> for the developer on a Mac was /Users/<username>
	It was adopted from the Atmel Xplained Pro file/folder named w25_xplained_pro in the same folder.
	February 2023
	Richard Lewis/ASTL Enterprises LLC/rlewis@astlenterprises.com rick@fmspcb.com
*/

#include "variant.h"

const PinDescription g_APinDescription[] = {

/*
 +------------+------------------+--------+-----------------+--------+-----------------------+---------+---------+--------+--------+----------+----------+
 | Pin number | TTF Pin          |  PIN   | Notes           | Peri.A |     Peripheral B      | Perip.C | Perip.D | Peri.E | Peri.F | Periph.G | Periph.H |
 |            |                  |        |                 |   EIC  | ADC |  AC | PTC | DAC | SERCOMx | SERCOMx |  TCCx  |  TCCx  |    COM   | AC/GLCK  |
 |            |                  |        |                 |(EXTINT)|(AIN)|(AIN)|     |     | (x/PAD) | (x/PAD) | (x/WO) | (x/WO) |          |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 | 00         | OCP_SCAN_LD_N    |  PA22  |                 |  *06   |     |     | X10 |     |   3/00  |   5/00  |* TC4/0 | TCC0/4 |          | GCLK_IO6 |
 | 01         | OCP_MAIN_PWR_EN  |  PA23  |                 |  *07   |     |     | X11 |     |   3/01  |   5/01  |* TC4/1 | TCC0/5 | USB/SOF  | GCLK_IO7 |
 | 02         | OCP_SCAN_DATA_IN |  PA10  |                 |   10   | *18 |     | X02 |     |   0/02  |   2/02  |*TCC1/0 | TCC0/2 | I2S/SCK0 | GCLK_IO4 |
 | 03         | OCP_SCAN_CLK     |  PA11  |                 |   11   | *19 |     | X03 |     |   0/03  |   2/03  |*TCC1/1 | TCC0/3 | I2S/FS0  | GCLK_IO5 |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 | 04         | OCP_PRSNTB1_N    |  PB10  |                 |  *10   |     |     |     |     |         |   4/02  |* TC5/0 | TCC0/4 | I2S/MCK1 | GCLK_IO4 |
 | 05         | P1_LINKA_N       |  PB11  |                 |  *11   |     |     |     |     |         |   4/03  |* TC5/1 | TCC0/5 | I2S/SCK1 | GCLK_IO5 |
 | 06         | UART_TX_UNUSED   |  PA20  |                 |  *04   |     |     | X08 |     |   5/02  |   3/02  |        |*TCC0/6 | I2S/SCK0 | GCLK_IO4 |
 | 07         | SCAN_VER_0       |  PA21  |                 |  *05   |     |     | X09 |     |   5/03  |   3/03  |        |*TCC0/7 | I2S/FS0  | GCLK_IO5 |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 */
  { PORTA, 22, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, PWM4_CH0,   TC4_CH0,      EXTERNAL_INT_6    },
  { PORTA, 23, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, PWM4_CH1,   TC4_CH1,      EXTERNAL_INT_7    },
  { PORTA, 10, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, PWM1_CH0,   TCC1_CH0,     EXTERNAL_INT_NONE },
  { PORTA, 11, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
  
  { PORTB, 10, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, PWM5_CH0,   TC5_CH0,      EXTERNAL_INT_10   },
  { PORTB, 11, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, PWM5_CH1,   TC5_CH1,      EXTERNAL_INT_11   },
  { PORTA, 20, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER_ALT), No_ADC_Channel, PWM0_CH6,   TCC0_CH6,     EXTERNAL_INT_4    },
  { PORTA, 21, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER_ALT), No_ADC_Channel, PWM0_CH7,   TCC0_CH7,     EXTERNAL_INT_5    },

/*
 +------------+------------------+--------+-----------------+--------+-----------------------+---------+---------+--------+--------+----------+----------+
 | Pin number | TTF Pin          |  PIN   | Notes           | Peri.A |     Peripheral B      | Perip.C | Perip.D | Peri.E | Peri.F | Periph.G | Periph.H |
 |            |                  |        |                 |   EIC  | ADC |  AC | PTC | DAC | SERCOMx | SERCOMx |  TCCx  |  TCCx  |    COM   | AC/GLCK  |
 |            |                  |        |                 |(EXTINT)|(AIN)|(AIN)|     |     | (x/PAD) | (x/PAD) | (x/WO) | (x/WO) |          |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 | 08         | OCP_SCAN_DATA_OUT|  PA08  |                 |  *00   |     |     | X04 |     |  *1/00  |   3/00  |*TCC2/0 | TCC0/6 |          | GCLK_IO2 |
 | 09         | OCP_AUX_PWR_EN   |  PA09  |                 |  *01   |     |     | X05 |     |  *1/01  |   3/01  | TCC2/1 | TCC0/7 |          | GCLK_IO3 |
 | 10         | UART_RX_UNUSED   |  PA19  |                 |   03   |     |     | X07 |     |  *1/03  |   3/03  |* TC3/1 | TCC0/3 | I2S/SD0  | AC/CMP1  |
 +------------+------------------+--------+-----------------+--------------------+-----+-----+---------+---------+--------+--------+----------+----------+
 |            |       Wire       |        |                 |        |     |     |     |     |         |         |        |        |          |          |
 | 11         | MCU_SDA          |  PA16  |                 |   NMI  | *16 |     | X00 |     |  *0/00  |   2/00  | TCC0/0 | TCC1/2 | I2S/SD1  |          |
 | 12         | MCU_SCL          |  PA17  |                 |   09   | *17 |     | X01 |     |  *0/01  |   2/01  | TCC0/1 | TCC1/3 | I2S/MCK0 |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 | 13         | P1_LED_ACT_N     |  PB23  | See             |   07   |     |     |     |     |         |  *5/03  |        |        |          | GCLK_IO1 |
 | 14         | OCP_PWRBRK_N     |  PB22  |                 |   06   |     |     |     |     |         |  *5/02  |        |        |          | GCLK_IO0 |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 */

  { PORTA,  8, PIO_DIGITAL,  (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, PWM2_CH0,   TCC2_CH0,     EXTERNAL_INT_0    },
  { PORTA,  9, PIO_DIGITAL,  (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_1    },
  { PORTA, 19, PIO_DIGITAL,  (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel, PWM3_CH1,   TC3_CH1,      EXTERNAL_INT_NONE },

  { PORTA, 16, PIO_SERCOM,  (PIN_ATTR_DIGITAL                                 ), No_ADC_Channel,  NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // SDA:  SERCOM1/PAD[0]
  { PORTA, 17, PIO_SERCOM,  (PIN_ATTR_DIGITAL                                 ), No_ADC_Channel,  NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // SCL:  SERCOM1/PAD[1]

  { PORTB, 23, PIO_DIGITAL, (PIN_ATTR_DIGITAL                                 ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // HRTBT LED
  { PORTB, 22, PIO_DIGITAL, (PIN_ATTR_DIGITAL                                 ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, 

/*
 +------------+------------------+--------+-----------------+--------+-----------------------+---------+---------+--------+--------+----------+----------+
 | Pin number | TTF Pin          |  PIN   | Notes           | Peri.A |     Peripheral B      | Perip.C | Perip.D | Peri.E | Peri.F | Periph.G | Periph.H |
 |            |                  | *=mod  |                 |   EIC  | ADC |  AC | PTC | DAC | SERCOMx | SERCOMx |  TCCx  |  TCCx  |    COM   | AC/GLCK  |
 |            |                  |        |                 |(EXTINT)|(AIN)|(AIN)|     |     | (x/PAD) | (x/PAD) | (x/WO) | (x/WO) |          |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 | 15         | NC_SPEED_A4      |  PA02  |                 |   02   | *00 |     | Y00 | OUT |         |         |        |        |          |          |
 | 16         | OCP_PRSNTB3_N    |  PB02  |                 |  *02   | *10 |     | Y08 |     |         |   5/00  |        |        |          |          |
 | 17         | FAN_ON_AUX       |  PB08  |                 |  *03   | *02 |     | Y09 |     |         |   5/01  |        |        |          |          |
 | 18         | P3_LINKA_N       |  PB09  |                 |   04   | *03 |  00 | Y02 |     |         |   0/00  |*TCC0/0 |        |          |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 | 19         | NC_SPEED_A7      |  PA05  |                 |   05   | *05 |  01 | Y03 |     |         |   0/01  |*TCC0/1 |        |          |          |
 | 20         | P3_LED_ACT_N     |  PA06  |                 |   06   | *06 |  02 | Y04 |     |         |   0/02  | TCC1/0 |        |          |          |
 | 21         | TP_LNKAC2        |  PA07  |                 |   07   | *07 |  03 | Y05 |     |         |   0/03  | TCC1/1 |        | I2S/SD0  |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 */
  { PORTA,  2, PIO_DIGITAL,  (PIN_ATTR_DIGITAL                                ), No_ADC_Channel,   NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
  { PORTB,  2, PIO_DIGITAL,  (PIN_ATTR_DIGITAL                                ), No_ADC_Channel,  NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_2    },
  { PORTB,  8, PIO_DIGITAL,  (PIN_ATTR_DIGITAL                                ), No_ADC_Channel,   NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
  { PORTB,  9, PIO_DIGITAL,  (PIN_ATTR_DIGITAL                                ), No_ADC_Channel,   NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
  
  { PORTA,  5, PIO_DIGITAL,  (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER    ), No_ADC_Channel,   PWM0_CH1,   TCC0_CH1,     EXTERNAL_INT_NONE },
  { PORTA,  6, PIO_DIGITAL,  (PIN_ATTR_DIGITAL                                ), No_ADC_Channel,   NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
  { PORTA,  7, PIO_DIGITAL,  (PIN_ATTR_DIGITAL                                ), No_ADC_Channel,   NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },

/*
 +------------+------------------+--------+-----------------+--------+-----------------------+---------+---------+--------+--------+----------+----------+
 | Pin number | TTF Pin          |  PIN   | Notes           | Peri.A |     Peripheral B      | Perip.C | Perip.D | Peri.E | Peri.F | Periph.G | Periph.H |
 |            |                  |        |                 |   EIC  | ADC |  AC | PTC | DAC | SERCOMx | SERCOMx |  TCCx  |  TCCx  |    COM   | AC/GLCK  |
 |            |                  |        |                 |(EXTINT)|(AIN)|(AIN)|     |     | (x/PAD) | (x/PAD) | (x/WO) | (x/WO) |          |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 |            |       USB        |        |                 |        |     |     |     |     |         |         |        |        |          |          |
 | 22         |                  |  PA24  | USB N           |   12   |     |     |     |     |   3/02  |   5/02  |  TC5/0 | TCC1/2 | USB/DM   |          |
 | 23         |                  |  PA25  | USB P           |   13   |     |     |     |     |   3/03  |   5/03  |  TC5/1 | TCC1/3 | USB/DP   |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 | 24         | OCP_PRSNTB0_N    |  PA18  |                 |   02   |     |     | X06 |     |   1/02  |   3/02  |  TC3/0 | TCC0/2 |          | AC/CMP0  |
 | 25         | NC_SPEED_A6      |  PA03  |                 |   03   |  01 |     | Y01 |     |         |         |        |        |          |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 */
  { PORTA, 24, PIO_COM,     (PIN_ATTR_NONE                                   ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // USB/DM
  { PORTA, 25, PIO_COM,     (PIN_ATTR_NONE                                   ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // USB/DP
  
  { PORTA, 18, PIO_DIGITAL, (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
  { PORTA,  3, PIO_DIGITAL, (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // DAC/VREFP

/*
 +------------+------------------+--------+-----------------+--------+-----------------------+---------+---------+--------+--------+----------+----------+
 | Pin number | TTF Pin          | PIN    | Notes           | Peri.A |     Peripheral B      | Perip.C | Perip.D | Peri.E | Peri.F | Periph.G | Periph.H |
 |            |                  |        |                 |   EIC  | ADC |  AC | PTC | DAC | SERCOMx | SERCOMx |  TCCx  |  TCCx  |    COM   | AC/GLCK  |
 |            |                  |        |                 |(EXTINT)|(AIN)|(AIN)|     |     | (x/PAD) | (x/PAD) | (x/WO) | (x/WO) |          |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 | 26         | NC_LINK_ACT_4    |  PA12  |                 |   12   |     |     |     |     |  *2/00  |   4/00  | TCC2/0 | TCC0/6 |          | AC/CMP0  |
 | 27         | NC_LINK_ACT_5    |  PA13  |                 |   13   |     |     |     |     |  *2/01  |   4/01  | TCC2/1 | TCC0/7 |          | AC/CMP1  |
 | 28         | OCP_PRSNTB2_N    |  PA14  |                 |   14   |     |     |     |     |   2/02  |   4/02  |  TC3/0 | TCC0/4 |          | GCLK_IO0 |
 | 29         | SCAN_VER_1       |  PA15  |                 |   15   |     |     |     |     |  *2/03  |   4/03  |  TC3/1 | TCC0/5 |          | GCLK_IO1 |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 | 30         | NC_LINK_ACT_6    |  PA27  |                 |   15   |     |     |     |     |         |         |        |        |          | GCLK_IO0 |
 | 31         | NCSI_RST_N       |  PA28  |                 |   08   |     |     |     |     |         |         |        |        |          | GCLK_IO0 |
 | 32         | NC_SPEED_A5      |  PA04  |                 |   08   |  02 |     | Y14 |     |         |   4/00  |  TC4/0 |        |          |          |
 | 33         | OCP_WAKE_N       |  PB03  |                 |  *09   |  03 |     | Y15 |     |         |   4/01  |  TC4/1 |        |          |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 | 34         | TEMP_WARN        |  PA00  |                 |   00   |     |     |     |     |         |   1/00  | TCC2/0 |        |          |          |
 | 35         | TEMP_CRIT        |  PA01  |                 |   01   |     |     |     |     |         |   1/01  | TCC2/1 |        |          |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 */

  { PORTA, 12, PIO_DIGITAL,     (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, 
  { PORTA, 13, PIO_DIGITAL,     (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, 
  { PORTA, 14, PIO_DIGITAL,    (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, 
  { PORTA, 15, PIO_DIGITAL,     (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, 

  { PORTA, 27, PIO_DIGITAL,    (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
  { PORTA, 28, PIO_DIGITAL,    (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
  { PORTA,  4, PIO_DIGITAL,    (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
  { PORTB,  3, PIO_DIGITAL,    (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_9    },

  { PORTA,  0, PIO_DIGITAL,    (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
  { PORTA,  1, PIO_DIGITAL,    (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
};

const void* g_apTCInstances[TCC_INST_NUM + TC_INST_NUM]={ TCC0, TCC1, TCC2, TC3, TC4, TC5 };

// Multi-serial objects instantiation
SERCOM sercom0(SERCOM0);
SERCOM sercom1(SERCOM1);
SERCOM sercom2(SERCOM2);
SERCOM sercom3(SERCOM3);
SERCOM sercom4(SERCOM4);
SERCOM sercom5(SERCOM5);

// Serial1
Uart Serial1(&sercom4, PIN_SERIAL1_RX, PIN_SERIAL1_TX, PAD_SERIAL1_RX, PAD_SERIAL1_TX);

void SERCOM4_Handler()
{
  Serial1.IrqHandler();
}

