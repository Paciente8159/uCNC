/*
	Name: boardmap_core_c3.h
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

#ifndef BOARDMAP_CORE_C3_H
#define BOARDMAP_CORE_C3_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_ESP32C3
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "CORE ESP32 C3"
#endif

// SAME AS GRBL for test purposes
// Setup step pins
#define STEP2_BIT 2 // assigns STEP2 pin
#define STEP1_BIT 1 // assigns STEP1 pin
#define STEP0_BIT 0 // assigns STEP0 pin

// Setup dir pins
#define DIR2_BIT 5 // assigns DIR2 pin
#define DIR1_BIT 4 // assigns DIR1 pin
#define DIR0_BIT 3 // assigns DIR0 pin

// Setup control input pins
// #define ESTOP_BIT 6
// #define ESTOP_ISR
// #define ESTOP_PULLUP
// #define FHOLD_BIT 7
// #define FHOLD_ISR
// #define FHOLD_PULLUP
// #define CS_RES_BIT 8
// #define CS_RES_ISR
// #define CS_RES_PULLUP

// Setup limit pins
// #define LIMIT_Z_BIT 9 // assigns LIMIT_Z pin
// #define LIMIT_Z_ISR		 // assigns LIMIT_Z ISR
// #define LIMIT_Y_BIT 10	 // assigns LIMIT_Y pin
// #define LIMIT_Y_ISR		 // assigns LIMIT_Y ISR
// #define LIMIT_X_BIT 11 // assigns LIMIT_X pin
// #define LIMIT_X_ISR		 // assigns LIMIT_X ISR

// Setup probe pin
#define PROBE_BIT 10
#define PROBE_ISR

// Setup com pins
#define RX_BIT 20
#define TX_BIT 21
#define RX_PULLUP
	// only uncomment this if other port other then 0 is used
	// #define UART_PORT 0

#define USB_DP_BIT 18
#define USB_DM_BIT 19

	// Setup PWM
#define PWM0_BIT 9 // assigns PWM0 pin
#define PWM0_TIMER 0
#define PWM0_CHANNEL 0

// Setup generic IO Pins
// spindle dir
#define DOUT0_BIT 8

// coolant
// #define DOUT2_BIT 15

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 3

	// Setup the Step Timer used has the heartbeat for µCNC
	// Timer 1 is used by default
	// #define ITP_TIMER 1

	// RTC Timer on ESP32 is granteed by a FreeRTOS

#define ONESHOT_TIMER 2

// #define ANALOG0_BIT 2
// #define ANALOG0_CHANNEL 2
// #define ANALOG0_ADC 2

// #define SERVO0_BIT 33
#define I2C_CLK_BIT 22
#define I2C_DATA_BIT 21

#ifdef __cplusplus
}
#endif

#endif
