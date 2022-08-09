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

#ifndef BOARDMAP_WEMOS_D1_R32_H
#define BOARDMAP_WEMOS_D1_R32_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "WEMOS D1 R32"
#endif

// SAME AS GRBL for test purposes
// Setup step pins
#define STEP2_BIT 17  // assigns STEP2 pin
#define STEP2_PORT D // assigns STEP2 port
#define STEP1_BIT 25  // assigns STEP1 pin
#define STEP1_PORT D // assigns STEP1 port
#define STEP0_BIT 26 // assigns STEP0 pin
#define STEP0_PORT D // assigns STEP0 port

// Setup dir pins
#define DIR2_BIT 14 // assigns DIR2 pin
#define DIR2_PORT D // assigns DIR2 port
#define DIR1_BIT 27 // assigns DIR1 pin
#define DIR1_PORT D // assigns DIR1 port
#define DIR0_BIT 16 // assigns DIR0 pin
#define DIR0_PORT D // assigns DIR0 port

// Setup control input pins
// #define ESTOP_BIT 2
// #define ESTOP_PORT D
// #define ESTOP_ISR
// #define ESTOP_PULLUP
// #define FHOLD_BIT 4
// #define FHOLD_PORT D
// #define FHOLD_ISR
// #define FHOLD_PULLUP
// #define CS_RES_BIT 35
// #define CS_RES_PORT D
// #define CS_RES_ISR
// #define CS_RES_PULLUP

// Setup limit pins
// #define LIMIT_Z_BIT 19	// assigns LIMIT_Z pin
// #define LIMIT_Z_PORT D	// assigns LIMIT_Z port
// #define LIMIT_Z_ISR		// assigns LIMIT_Z ISR
// #define LIMIT_Y_BIT 5	// assigns LIMIT_Y pin
// #define LIMIT_Y_PORT D	// assigns LIMIT_Y port
// #define LIMIT_Y_ISR		// assigns LIMIT_Y ISR
// #define LIMIT_X_BIT 13	// assigns LIMIT_X pin
// #define LIMIT_X_PORT D	// assigns LIMIT_X port
// #define LIMIT_X_ISR  	// assigns LIMIT_X ISR

// Setup probe pin
// #define PROBE_BIT 39
// #define PROBE_PORT D
// #define PROBE_ISR

// Setup com pins
#if (INTERFACE == INTERFACE_UART)
#define RX_BIT 3
#define TX_BIT 1
#define RX_PORT D
#define TX_PORT D
// only uncomment this if other port other then 0 is used
// #define COM_PORT 0
#endif

    // Setup PWM
#define PWM0_BIT 23  // assigns PWM0 pin
#define PWM0_PORT D // assigns PWM0 pin

// Setup generic IO Pins
// spindle dir
#define DOUT0_BIT 18
#define DOUT0_PORT D

// coolant
#define DOUT0_BIT 18
#define DOUT0_PORT D

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 12
#define STEP0_EN_PORT D

	// Setup the Step Timer used has the heartbeat for µCNC
	// Timer 1 is used by default
	//#define ITP_TIMER 1

	// Setup the RTC Timer used by µCNC to provide an (mostly) accurate time base for all time dependent functions
	// Timer 0 is set by default
	//#define RTC_TIMER 0

#ifdef __cplusplus
}
#endif

#endif
