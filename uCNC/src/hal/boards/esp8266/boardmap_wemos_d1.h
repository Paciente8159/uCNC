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
// #define STEP2_BIT 4	 // assigns STEP2 pin
// #define STEP2_PORT D // assigns STEP2 port
// #define STEP1_BIT 5	 // assigns STEP1 pin
// #define STEP1_PORT D // assigns STEP1 port
// #define STEP0_BIT 16 // assigns STEP0 pin
// #define STEP0_PORT D // assigns STEP0 port

// // // Setup dir pins
// #define DIR2_BIT 13 // assigns DIR2 pin
// #define DIR2_PORT D // assigns DIR2 port
// #define DIR1_BIT 12 // assigns DIR1 pin
// #define DIR1_PORT D // assigns DIR1 port
// #define DIR0_BIT 14 // assigns DIR0 pin
// #define DIR0_PORT D // assigns DIR0 port

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
// #define DOUT0_BIT 15
// #define DOUT0_PORT D

// Stepper enable pin. For Grbl on Uno board a single pin is used
// #define STEP0_EN_BIT 0
// #define STEP0_EN_PORT D

#define LIMIT_X_BIT 4	 // assigns STEP2 pin
#define LIMIT_X_PORT D // assigns STEP2 port
#define LIMIT_Y_BIT 5	 // assigns STEP1 pin
#define LIMIT_Y_PORT D // assigns STEP1 port
#define LIMIT_Z_BIT 16 // assigns STEP0 pin
#define LIMIT_Z_PORT D // assigns STEP0 port

// SPI
#define SPI_CLK_BIT 14
#define SPI_CLK_PORT D
#define SPI_SDO_BIT 13
#define SPI_SDO_PORT D
#define SPI_SDI_BIT 12
#define SPI_SDI_PORT D
#define SPI_CS_BIT 15
#define SPI_CS_PORT D

#define IC74HC595_CUSTOM_SHIFT_IO // Enables custom MCU data shift transmission. In ESP32 that is via I2S
#define IC74HC595_COUNT 4

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
#define DOUT0_IO_OFFSET 22
#define DOUT2_IO_OFFSET 23

#ifdef __cplusplus
}
#endif

#endif
