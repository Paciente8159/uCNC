/*
	Name: boardmap_devkit_s3.h
	Description: Contains all MCU and PIN definitions for Core ESP32 C3 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 10-03-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_DEVKIT_S3_H
#define BOARDMAP_DEVKIT_S3_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_ESP32S3
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "CORE ESP32 S3"
#endif

#ifndef F_CPU
#define F_CPU 240000000UL
#endif

// // Setup step pins
// #define STEP3_BIT 18 // assigns STEP3 pin
// #define STEP2_BIT 17 // assigns STEP2 pin
// #define STEP1_BIT 16 // assigns STEP1 pin
// #define STEP0_BIT 15 // assigns STEP0 pin

// // // Setup dir pins
// #define DIR3_BIT 9 // assigns DIR2 pin
// #define DIR2_BIT 10 // assigns DIR2 pin
// #define DIR1_BIT 11 // assigns DIR1 pin
// #define DIR0_BIT 12 // assigns DIR0 pin

// // Setup control input pins
// #define ESTOP_BIT 2
// #define ESTOP_ISR
// #define ESTOP_PULLUP
// #define FHOLD_BIT 4
// #define FHOLD_ISR
// #define FHOLD_PULLUP
// #define CS_RES_BIT 36
// #define CS_RES_ISR
// #define CS_RES_PULLUP

// // Setup limit pins
#define LIMIT_Z_BIT 6 // assigns LIMIT_Z pin
#define LIMIT_Z_ISR		 // assigns LIMIT_Z ISR
#define LIMIT_Y_BIT 5	 // assigns LIMIT_Y pin
#define LIMIT_Y_ISR		 // assigns LIMIT_Y ISR
#define LIMIT_X_BIT 4 // assigns LIMIT_X pin
#define LIMIT_X_ISR		 // assigns LIMIT_X ISR

// // Setup probe pin
#define PROBE_BIT 7
#define PROBE_ISR
// #define DOUT31_BIT 38

// Setup com pins
#define TX_BIT 43
#define RX_BIT 44
#define RX_PULLUP

#define USB_DP_BIT 20
#define USB_DM_BIT 19
	// only uncomment this if other port other then 0 is used
	// #define UART_PORT 0

// 	// Setup PWM
// #define PWM0_BIT 1 // assigns PWM0 pin
// #define PWM0_TIMER 0
// #define PWM0_CHANNEL 0

// Setup generic IO Pins
// spindle dir
// #define DOUT0_BIT 2

// // coolant
// #define DOUT2_BIT 34

// // Stepper enable pin. For Grbl on Uno board a single pin is used
// #define STEP0_EN_BIT 8

	// Setup the Step Timer used has the heartbeat for µCNC
	// Timer 1 is used by default
	// #define ITP_TIMER 1

	// RTC Timer on ESP32 is granteed by a FreeRTOS

// #define ONESHOT_TIMER 2

// #define ANALOG0_BIT 2
// #define ANALOG0_CHANNEL 2
// #define ANALOG0_ADC 2

// #define SERVO0_BIT 33
// #define I2C_CLK_BIT 22
// #define I2C_DATA_BIT 21

#define IC74HC595_CUSTOM_SHIFT_IO // Enables custom MCU data shift transmission. In ESP32 that is via I2S
#define IC74HC595_I2S_WS 26
#define IC74HC595_I2S_CLK 25
#define IC74HC595_I2S_DATA 27
// #define IC74HC595_I2S_PORT 0
// uses 3 x 74HS595 but for I2S use this value has to be set to 4 (I2S sends data as 32-bit (4bytes))
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
