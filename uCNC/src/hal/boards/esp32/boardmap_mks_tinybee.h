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

#ifdef INTERFACE
#undef INTERFACE
#endif
#define INTERFACE INTERFACE_UART

// Setup limit pins
#define LIMIT_Z_BIT 22	// assigns LIMIT_Z pin
// #define LIMIT_Z_ISR		// assigns LIMIT_Z ISR
#define LIMIT_Y_BIT 32	// assigns LIMIT_Y pin
// #define LIMIT_Y_ISR		// assigns LIMIT_Y ISR
#define LIMIT_X_BIT 33	// assigns LIMIT_X pin
// #define LIMIT_X_ISR  	// assigns LIMIT_X ISR

// Setup com pins
#if (INTERFACE == INTERFACE_UART)
#define RX_BIT 3
#define TX_BIT 1
// only uncomment this if other port other then 0 is used
// #define COM_PORT 0
#endif

// configure the 74HC595 modules
#define DOUT4_BIT 27
#define DOUT5_BIT 25
#define DOUT6_BIT 26
// uses 3 x 74HS595
#define IC74HC595_COUNT 3
#define IC74HC595_DELAY_CYCLES 0

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

	// Setup the RTC Timer used by µCNC to provide an (mostly) accurate time base for all time dependent functions
	// Timer 0 is set by default
	//#define RTC_TIMER 0

#define ONESHOT_TIMER 2

#define SPI_CLK_BIT 18
#define SPI_SDO_BIT 23
#define SPI_SDI_BIT 19
#define SPI_CS_BIT 5

// include the IO expander
#include "../../../modules/ic74hc595.h"

#ifdef __cplusplus
}
#endif

#endif
