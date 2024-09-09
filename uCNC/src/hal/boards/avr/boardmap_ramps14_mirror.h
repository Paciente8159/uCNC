/*
	Name: boardmap_ramps14_mirror.h
	Description: Contains all MCU and PIN definitions for Arduino UNO to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 11/05/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_RAMPS14_MIRROR_H
#define BOARDMAP_RAMPS14_MIRROR_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_AVR
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "RAMPS 1.4 MIRROR"
#endif

#define AVR6

#include "mega_ide_pins.h"

#define PCINT0_PORT B
#define PCINT1_PORT J
#define PCINT2_PORT K

	// // Setup step pins
	// #define STEP0_BIT 0	 // assigns STEP0 pin
	// #define STEP0_PORT F // assigns STEP0 port
	// #define STEP1_BIT 6	 // assigns STEP1 pin
	// #define STEP1_PORT F // assigns STEP1 port
	// #define STEP2_BIT 3	 // assigns STEP2 pin
	// #define STEP2_PORT L // assigns STEP2 port
	// #define STEP3_BIT 4	 // assigns STEP3 pin
	// #define STEP3_PORT A // assigns STEP3 port
	// #define STEP4_BIT 1	 // assigns STEP4 pin
	// #define STEP4_PORT C // assigns STEP4 port

	// // Setup dir pins
	// #define DIR0_BIT 1	// assigns DIR0 pin
	// #define DIR0_PORT F // assigns DIR0 port
	// #define DIR1_BIT 7	// assigns DIR1 pin
	// #define DIR1_PORT F // assigns DIR1 port
	// #define DIR2_BIT 1	// assigns DIR2 pin
	// #define DIR2_PORT L // assigns DIR2 port
	// #define DIR3_BIT 6	// assigns DIR2 pin
	// #define DIR3_PORT A // assigns DIR2 port
	// #define DIR4_BIT 3	// assigns DIR2 pin
	// #define DIR4_PORT C // assigns DIR2 port

	// Setup limit pins

#define DIN0_BIT 5	// assigns LIMIT_X pin
#define DIN0_PORT E // assigns LIMIT_X port
#define DIN0_ISR -6 // assigns LIMIT_X ISR
#define DIN8_BIT 4	// assigns LIMIT_X2 pin
#define DIN8_PORT E // assigns LIMIT_X2 port
// #define LIMIT_X2_ISR -5 // assigns LIMIT_X2 ISR
#define DIN1_BIT 1	// assigns LIMIT_Y pin
#define DIN1_PORT J // assigns LIMIT_Y port
#define DIN1_ISR 1	// assigns LIMIT_Y ISR
#define DIN9_BIT 0	// assigns LIMIT_Y2 pin
#define DIN9_PORT J // assigns LIMIT_Y2 port
// #define LIMIT_Y2_ISR 1	// assigns LIMIT_Y2 ISR
#define DIN2_BIT 3	// assigns LIMIT_Z pin
#define DIN2_PORT D // assigns LIMIT_Z port
#define DIN2_ISR -4 // assigns LIMIT_Z ISR

// // Setup probe pin
#define DIN10_BIT 2
#define DIN10_PORT D
	// #define PROBE_ISR -3

#define DIN0_PULLUP
#define DIN1_PULLUP
#define DIN2_PULLUP
#define DIN8_PULLUP
#define DIN9_PULLUP
#define DIN10_PULLUP

// Setup com pins
#define RX_BIT 0
#define TX_BIT 1
#define RX_PORT E
#define TX_PORT E
#define RX_PULLUP
// only uncomment this if other port other then 0 is used
#define UART_PORT 0

	// // Setup PWM
	// #define PWM0_BIT 5	// assigns PWM0 pin
	// #define PWM0_PORT H // assigns PWM0 pin
	// #define PWM0_CHANNEL C
	// #define PWM0_TIMER 4

	// // Setup generic IO Pins
	// // Functionalities are set in config.h file

	// // blink led
	// #define DOUT31_BIT 7
	// #define DOUT31_PORT B

	// // Stepper enable pin. For Grbl on Uno board a single pin is used
	// #define STEP0_EN_BIT 7
	// #define STEP0_EN_PORT D
	// #define STEP1_EN_BIT 2
	// #define STEP1_EN_PORT F
	// #define STEP2_EN_BIT 0
	// #define STEP2_EN_PORT K
	// #define STEP3_EN_BIT 2
	// #define STEP3_EN_PORT A
	// #define STEP4_EN_BIT 7
	// #define STEP4_EN_PORT C

	// 	// Setup the Step Timer used has the heartbeat for µCNC
	// 	// Timer 1 is used by default
	// 	//#define ITP_TIMER 1
	// 	// Setup the RTC Timer used by µCNC to provide an (mostly) accurate time base for all time dependent functions
	// 	// Timer 0 is set by default
	// 	//#define RTC_TIMER 0

	// // TMC0 UART
	// #define DOUT23_BIT 1
	// #define DOUT23_PORT G
	// #define DIN23_BIT 1
	// #define DIN23_PORT K
	// #define DIN23_PULLUP

	// // TMC1 UART
	// #define DOUT24_BIT 5
	// #define DOUT24_PORT F
	// #define DIN24_BIT 2
	// #define DIN24_PORT K
	// #define DIN24_PULLUP

	// // TMC2 UART
	// #define DOUT25_BIT 7
	// #define DOUT25_PORT L
	// #define DIN25_BIT 3
	// #define DIN25_PORT K
	// #define DIN25_PULLUP

	// // TMC3 UART
	// #define DOUT26_BIT 5
	// #define DOUT26_PORT L
	// #define DIN26_BIT 4
	// #define DIN26_PORT K
	// #define DIN26_PULLUP

	// 	// // TMC4 UART
	// 	// #define DOUT27_BIT 3
	// 	// #define DOUT27_PORT F
	// 	// #define DIN27_BIT 4
	// 	// #define DIN27_PORT F
	// 	// #define DIN27_PULLUP

	// #define SERVO0_BIT 5
	// #define SERVO0_PORT G
	// #define SERVO1_BIT 3
	// #define SERVO1_PORT E
	// #define SERVO2_BIT 3
	// #define SERVO2_PORT H
	// #define SERVO3_BIT 5
	// #define SERVO3_PORT B

	// 	// SERVO3 pin supports ISR and can be used as an encoder/counter
	// 	//  #define DIN0_BIT 5
	// 	//  #define DIN0_PORT B
	// 	//  #define DIN0_ISR 0

#ifdef __cplusplus
}
#endif

#endif
