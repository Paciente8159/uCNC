/*
	Name: boardmap_mzero.h
	Description: Contains all PIN definitions for Arduino M0 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 13-12-2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_MZERO_H
#define BOARDMAP_MZERO_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "Arduino M0"
#endif

#ifndef SAMD21
#define SAMD21
#endif

#ifndef F_CPU
#define F_CPU 48000000UL
#endif

#ifndef PLANNER_BUFFER_SIZE
#define PLANNER_BUFFER_SIZE 30
#endif
#ifndef RX_BUFFER_CAPACITY
#define RX_BUFFER_CAPACITY 256
#endif

// Setup step pins
#define STEP0_BIT 8	 // assigns STEP0 pin
#define STEP0_PORT A // assigns STEP0 port
#define STEP1_BIT 9	 // assigns STEP1 pin
#define STEP1_PORT A // assigns STEP1 port
#define STEP2_BIT 14 // assigns STEP2 pin
#define STEP2_PORT A // assigns STEP2 port

// Setup dir pins
#define DIR0_BIT 15 // assigns DIR0 pin
#define DIR0_PORT A // assigns DIR0 port
#define DIR1_BIT 20 // assigns DIR1 pin
#define DIR1_PORT A // assigns DIR1 port
#define DIR2_BIT 21 // assigns DIR2 pin
#define DIR2_PORT A // assigns DIR2 port

// Setup limit pins
#define LIMIT_X_BIT 7  // assigns LIMIT_X pin
#define LIMIT_X_PORT A // assigns LIMIT_X port
#define LIMIT_Y_BIT 18 // assigns LIMIT_Y pin
#define LIMIT_Y_PORT A // assigns LIMIT_Y port
#define LIMIT_Z_BIT 19 // assigns LIMIT_Z pin
#define LIMIT_Z_PORT A // assigns LIMIT_Z port

// Enable limits switch interrupt
#define LIMIT_X_ISR
#define LIMIT_Y_ISR
#define LIMIT_Z_ISR

// Setup control input pins
#define ESTOP_BIT 2
#define ESTOP_PORT A
#define FHOLD_BIT 8
#define FHOLD_PORT B
#define CS_RES_BIT 9
#define CS_RES_PORT B

// Setup probe pin
#define PROBE_BIT 2
#define PROBE_PORT B

// Enable controls switch interrupt
// REMOVED - LIMIT_Y AND ESTOP SHARE THE SAME EXTIE(2)
//  #define ESTOP_ISR
//  #define FHOLD_ISR
//  #define CS_RES_ISR

// On the STM32 always use sync TX UART (async doesn't work well)
#if (INTERFACE == INTERFACE_USART)
#define COM_PORT 1
#define TX_BIT 10
#define TX_PORT A
#define TX_MUX D
#define TX_PAD 2
#define RX_BIT 11
#define RX_PORT A
#define RX_MUX D
#define RX_PAD 3
// set COM number. By default COM0 if not set. Arduino IDE already uses SERCOM0 and SERCOM1
#define COM_NUMBER 2
#elif (INTERFACE == INTERFACE_USB)
#define USB_DM_BIT 24
#define USB_DM_PORT A
#define USB_DM_MUX G
#define USB_DP_BIT 25
#define USB_DP_PORT A
#define USB_DP_MUX G
#endif

// Setup PWM
#define PWM0_BIT 16	   // assigns PWM0 pin
#define PWM0_PORT A	   // assigns PWM0 port
#define PWM0_CHANNEL 0 // assigns PWM0 channel
#define PWM0_TIMER 2   // assigns PWM0 timer
#define PWM0_MUX E	   // assigns PWM0 mux

// Setup generic IO Pins
// Functionalities are set in cnc_hal_config.h file

// spindle dir
#define DOUT0_BIT 17
#define DOUT0_PORT A

// analog input
//  #define ANALOG0_BIT 5
//  #define ANALOG0_PORT A
//  #define ANALOG0_CHANNEL 5

// // servo output
// #define SERVO0_BIT 5
// #define SERVO0_PORT A

// coolant
#define DOUT1_BIT 4
#define DOUT1_PORT A

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 6
#define STEP0_EN_PORT A

	// Setup the Step Timer used has the heartbeat for µCNC
	//#define ITP_TIMER 5

#ifdef __cplusplus
}
#endif

#endif
