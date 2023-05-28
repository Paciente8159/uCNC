/*
	Name: boardmap_ramps14.h
	Description: Contains all MCU and PIN definitions for Arduino UNO to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 05/02/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_RAMPS14_H
#define BOARDMAP_RAMPS14_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "RAMPS 1.4"
#endif

#define AVR6

#include "mega_ide_pins.h"

#define PCINT0_PORT B
#define PCINT1_PORT J
#define PCINT2_PORT K

// Setup step pins
#define STEP0_BIT 0	 // assigns STEP0 pin
#define STEP0_PORT F // assigns STEP0 port
#define STEP1_BIT 6	 // assigns STEP1 pin
#define STEP1_PORT F // assigns STEP1 port
#define STEP2_BIT 3	 // assigns STEP2 pin
#define STEP2_PORT L // assigns STEP2 port
#define STEP3_BIT 4	 // assigns STEP3 pin
#define STEP3_PORT A // assigns STEP3 port
#define STEP4_BIT 1	 // assigns STEP4 pin
#define STEP4_PORT C // assigns STEP4 port

// Setup dir pins
#define DIR0_BIT 1	// assigns DIR0 pin
#define DIR0_PORT F // assigns DIR0 port
#define DIR1_BIT 7	// assigns DIR1 pin
#define DIR1_PORT F // assigns DIR1 port
#define DIR2_BIT 1	// assigns DIR2 pin
#define DIR2_PORT L // assigns DIR2 port
#define DIR3_BIT 6	// assigns DIR2 pin
#define DIR3_PORT A // assigns DIR2 port
#define DIR4_BIT 3	// assigns DIR2 pin
#define DIR4_PORT C // assigns DIR2 port

	// Setup limit pins

// #define LIMIT_X_BIT 5	// assigns LIMIT_X pin
// #define LIMIT_X_PORT E	// assigns LIMIT_X port
// #define LIMIT_X_ISR -6	// assigns LIMIT_X ISR
#define LIMIT_X2_BIT 4	// assigns LIMIT_X2 pin
#define LIMIT_X2_PORT E // assigns LIMIT_X2 port
#define LIMIT_X2_ISR -5 // assigns LIMIT_X2 ISR
// #define LIMIT_Y_BIT 1	// assigns LIMIT_Y pin
// #define LIMIT_Y_PORT J	// assigns LIMIT_Y port
// #define LIMIT_Y_ISR 1	// assigns LIMIT_Y ISR
#define LIMIT_Y2_BIT 0	// assigns LIMIT_Y2 pin
#define LIMIT_Y2_PORT J // assigns LIMIT_Y2 port
#define LIMIT_Y2_ISR 1	// assigns LIMIT_Y2 ISR
#define LIMIT_Z_BIT 3	// assigns LIMIT_Z pin
#define LIMIT_Z_PORT D	// assigns LIMIT_Z port
#define LIMIT_Z_ISR -4	// assigns LIMIT_Z ISR

// Setup probe pin
#define PROBE_BIT 2
#define PROBE_PORT D
#define PROBE_ISR -3

// Setup com pins
#define RX_BIT 0
#define TX_BIT 1
#define RX_PORT E
#define TX_PORT E
#define RX_PULLUP
// only uncomment this if other port other then 0 is used
#define UART_PORT 0

// Setup PWM
#define PWM0_BIT 5	// assigns PWM0 pin
#define PWM0_PORT H // assigns PWM0 pin
#define PWM0_CHANNEL C
#define PWM0_TIMER 4

#define PWM1_BIT 6	// assigns PWM0 pin
#define PWM1_PORT H // assigns PWM0 pin
#define PWM1_CHANNEL B
#define PWM1_TIMER 2

#define PWM2_BIT 4	// assigns PWM0 pin
#define PWM2_PORT B // assigns PWM0 pin
#define PWM2_CHANNEL A
#define PWM2_TIMER 2

// Setup generic IO Pins
// Functionalities are set in config.h file

// blink led
#define DOUT31_BIT 7
#define DOUT31_PORT B

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 7
#define STEP0_EN_PORT D
#define STEP1_EN_BIT 2
#define STEP1_EN_PORT F
#define STEP2_EN_BIT 0
#define STEP2_EN_PORT K
#define STEP3_EN_BIT 2
#define STEP3_EN_PORT A
#define STEP4_EN_BIT 7
#define STEP4_EN_PORT C

	// Setup the Step Timer used has the heartbeat for µCNC
	// Timer 1 is used by default
	//#define ITP_TIMER 1
	// Setup the RTC Timer used by µCNC to provide an (mostly) accurate time base for all time dependent functions
	// Timer 0 is set by default
	//#define RTC_TIMER 0

// TMC0 UART
#define DOUT20_BIT 1
#define DOUT20_PORT G
#define DIN20_BIT 1
#define DIN20_PORT K
#define DIN20_PULLUP

// TMC1 UART
#define DOUT21_BIT 5
#define DOUT21_PORT F
#define DIN21_BIT 2
#define DIN21_PORT K
#define DIN21_PULLUP

// TMC2 UART
#define DOUT22_BIT 7
#define DOUT22_PORT L
#define DIN22_BIT 3
#define DIN22_PORT K
#define DIN22_PULLUP

// TMC3 UART
#define DOUT23_BIT 5
#define DOUT23_PORT L
#define DIN23_BIT 4
#define DIN23_PORT K
#define DIN23_PULLUP

	// // TMC4 UART
	// #define DOUT24_BIT 3
	// #define DOUT24_PORT F
	// #define DIN24_BIT 4
	// #define DIN24_PORT F
	// #define DIN24_PULLUP

#define SERVO3_BIT 5
#define SERVO3_PORT G
#define SERVO2_BIT 3
#define SERVO2_PORT E
#define SERVO1_BIT 3
#define SERVO1_PORT H
#define SERVO0_BIT 5
#define SERVO0_PORT B

	// SERVO3 pin supports ISR and can be used as an encoder/counter
	//  #define DIN0_BIT 5
	//  #define DIN0_PORT B
	//  #define DIN0_ISR 0

// hardware I2C
#define I2C_CLK_BIT 0
#define I2C_CLK_PORT D
#define I2C_DATA_BIT 1
#define I2C_DATA_PORT D
// software I2C
// #define DIN30_BIT 0
// #define DIN30_PORT D
// #define DIN30_PULLUP
// #define DIN31_BIT 1
// #define DIN31_PORT D
// #define DIN31_PULLUP

// hardware SPI
#define SPI_SDO_BIT 2
#define SPI_SDO_PORT B
#define SPI_SDI_BIT 3
#define SPI_SDI_PORT B
#define SPI_CLK_BIT 1
#define SPI_CLK_PORT B
#define SPI_CS_BIT 0
#define SPI_CS_PORT B
#define SPI_FREQ 100000UL
//software SPI
// #define DOUT29_BIT 2
// #define DOUT29_PORT B
// #define DIN29_BIT 3
// #define DIN29_PORT B
// #define DOUT30_BIT 1
// #define DOUT30_PORT B
//sd card detect
#define DIN19_BIT 0
#define DIN19_PORT L
#define DIN19_PULLUP

//pins for smart adapter
//clk
#define DOUT4_BIT 1
#define DOUT4_PORT A
//data
#define DOUT5_BIT 0
#define DOUT5_PORT H
//cs
#define DOUT6_BIT 1
#define DOUT6_PORT H
//beep
#define DOUT7_BIT 0
#define DOUT7_PORT C
//enc btn
#define DIN16_BIT 2
#define DIN16_PORT C
#define DIN16_PULLUP
//enc 1
#define DIN17_BIT 4
#define DIN17_PORT C
#define DIN17_PULLUP
//enc 2
#define DIN18_BIT 6
#define DIN18_PORT C
#define DIN18_PULLUP

#define ONESHOT_TIMER 4

#ifdef __cplusplus
}
#endif

#endif
