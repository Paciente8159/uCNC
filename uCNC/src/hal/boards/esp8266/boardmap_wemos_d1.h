/*
		Name: boardmap_wemos_d1.h
		Description: Contains all MCU and PIN definitions for Arduino WeMos D1 to run µCNC.

		Copyright: Copyright (c) João Martins
		Author: João Martins
		Date: 17/06/2022

		µCNC is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version. Please see <http://www.gnu.org/licenses/>

		µCNC is distributed WITHOUT ANY WARRANTY;
		Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
		See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_WEMOS_D1_H
#define BOARDMAP_WEMOS_D1_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_ESP8266
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "WEMOS D1"
#endif

#undef ENABLE_WIFI
#define ENABLE_WIFI

// SAME AS GRBL for test purposes
// Setup step pins
#define STEP2_BIT 4	 // assigns STEP2 pin
#define STEP2_PORT D // assigns STEP2 port
#define STEP1_BIT 5	 // assigns STEP1 pin
#define STEP1_PORT D // assigns STEP1 port
#define STEP0_BIT 16 // assigns STEP0 pin
#define STEP0_PORT D // assigns STEP0 port

// Setup dir pins
#define DIR2_BIT 13 // assigns DIR2 pin
#define DIR2_PORT D // assigns DIR2 port
#define DIR1_BIT 12 // assigns DIR1 pin
#define DIR1_PORT D // assigns DIR1 port
#define DIR0_BIT 14 // assigns DIR0 pin
#define DIR0_PORT D // assigns DIR0 port

// Setup control input pins
// #define ESTOP_BIT 0
// #define ESTOP_PORT A
// #define ESTOP_ISR

// Setup com pins
#define RX_BIT 3
#define TX_BIT 1
#define RX_PORT D
#define TX_PORT D
#define RX_PULLUP
	// only uncomment this if other port other then 0 is used
	// #define UART_PORT 0

	// Setup PWM
// #define PWM0_BIT 2	// assigns PWM0 pin
// #define PWM0_PORT D // assigns PWM0 pin

// Setup generic IO Pins
// spindle dir
#define DOUT0_BIT 15
#define DOUT0_PORT D

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 0
#define STEP0_EN_PORT D

// SPI
// #define SPI_CLK_BIT 14
// #define SPI_CLK_PORT D
// #define SPI_SDO_BIT 13
// #define SPI_SDO_PORT D
// #define SPI_SDI_BIT 12
// #define SPI_SDI_PORT D

#define DOUT31_BIT 2
#define DOUT31_PORT D

#ifdef __cplusplus
}
#endif

#endif
