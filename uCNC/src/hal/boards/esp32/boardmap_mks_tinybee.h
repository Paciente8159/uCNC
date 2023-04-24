/*
	Name: boardmap_mks_tinybee.h
	Description: Contains all MCU and PIN definitions for Arduino WeMos D1 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 11/10/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_MKS_TINYBEE_H
#define BOARDMAP_MKS_TINYBEE_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "MKS Tinybee"
#endif

// Setup limit pins
#define LIMIT_Z_BIT 22	// assigns LIMIT_Z pin
// #define LIMIT_Z_ISR		// assigns LIMIT_Z ISR
#define LIMIT_Y_BIT 32	// assigns LIMIT_Y pin
// #define LIMIT_Y_ISR		// assigns LIMIT_Y ISR
#define LIMIT_X_BIT 33	// assigns LIMIT_X pin
// #define LIMIT_X_ISR  	// assigns LIMIT_X ISR

// Setup com pins
#define RX_BIT 3
#define TX_BIT 1
#define RX_PULLUP
// only uncomment this if other port other then 0 is used
// #define COM_PORT 0

// configure the 74HC595 modules
#define DOUT4_BIT 27
#define DOUT5_BIT 25
#define DOUT6_BIT 26
// uses 3 x 74HS595
#define IC74HC595_COUNT 3
// #define IC74HC595_DELAY_CYCLES 0

//Use I2S to shift data in ESP32
#define IC74HC595_CUSTOM_SHIFT_IO //Enables custom MCU data shift transmission. In ESP32 that is via I2S
#define IC74HC595_I2S_WS 26
#define IC74HC595_I2S_CLK 25
#define IC74HC595_I2S_DATA 27
// #define IC74HC595_I2S_PORT 0

#define STEP0_EN_IO_OFFSET 0
#define STEP0_IO_OFFSET 1
#define DIR0_IO_OFFSET 2
#define STEP1_EN_IO_OFFSET 3
#define STEP1_IO_OFFSET 4
#define DIR1_IO_OFFSET 5
#define STEP2_EN_IO_OFFSET 6
#define STEP2_IO_OFFSET 7
#define DIR2_IO_OFFSET 8
#define STEP3_EN_IO_OFFSET 9
#define STEP3_IO_OFFSET 10
#define DIR3_IO_OFFSET 11
#define STEP4_EN_IO_OFFSET 12
#define STEP4_IO_OFFSET 13
#define DIR4_IO_OFFSET 14
#define PWM0_IO_OFFSET 16
#define PWM1_IO_OFFSET 17
#define PWM2_IO_OFFSET 18
#define PWM3_IO_OFFSET 19
#define PWM4_IO_OFFSET 20
#define DOUT0_IO_OFFSET 21
#define DOUT2_IO_OFFSET 22
	// Setup the Step Timer used has the heartbeat for µCNC
	// Timer 1 is used by default
	//#define ITP_TIMER 1

	// RTC Timer on ESP32 is granteed by a FreeRTOS

#define ONESHOT_TIMER 2

#define SPI_CLK_BIT 18
#define SPI_SDO_BIT 23
#define SPI_SDI_BIT 19
#define SPI_CS_BIT 5

//sd card detect
#define DIN19_BIT 34

// include the IO expander
#include "../../../modules/ic74hc595.h"

#ifdef __cplusplus
}
#endif

#endif
