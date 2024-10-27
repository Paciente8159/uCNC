/*
	Name: boardmap_uno.h
	Description: Contains all MCU and PIN definitions for Arduino UNO similar to Grbl 1.1+ to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 04/02/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_UNO_H
#define BOARDMAP_UNO_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_AVR
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "Arduino UNO"
#endif

#define PCINT0_PORT B
#define PCINT1_PORT C
#define PCINT2_PORT D

// SAME AS GRBL for test purposes
// Setup step pins
#define STEP2_BIT 4	 // assigns STEP2 pin
#define STEP2_PORT D // assigns STEP2 port
#define STEP1_BIT 3	 // assigns STEP1 pin
#define STEP1_PORT D // assigns STEP1 port
#define STEP0_BIT 2	 // assigns STEP0 pin
#define STEP0_PORT D // assigns STEP0 port
// #define STEP6_BIT 4 //assigns STEP6 pin (will mirror DUAL_AXIS0)
// #define STEP6_PORT C //assigns STEP6 port (will mirror DUAL_AXIS0)

// Setup dir pins
#define DIR2_BIT 7	// assigns DIR2 pin
#define DIR2_PORT D // assigns DIR2 port
#define DIR1_BIT 6	// assigns DIR1 pin
#define DIR1_PORT D // assigns DIR1 port
#define DIR0_BIT 5	// assigns DIR0 pin
#define DIR0_PORT D // assigns DIR0 port

// Setup limit pins
#define LIMIT_Z_BIT 4	 // assigns LIMIT_Z pin
#define LIMIT_Z_PORT B // assigns LIMIT_Z port
#define LIMIT_Z_ISR 0	 // assigns LIMIT_Z ISR
											 // #define LIMIT_Y2_BIT 4 //Z and second Y limit share the pin
											 // #define LIMIT_Y2_PORT B //Z and second Y limit share the pin
											 // #define LIMIT_Y2_ISR 0 //Z and second Y limit share the pin

#define LIMIT_Y_BIT 2	 // assigns LIMIT_Y pin
#define LIMIT_Y_PORT B // assigns LIMIT_Y port
#define LIMIT_Y_ISR 0	 // assigns LIMIT_Y ISR

#define LIMIT_X_BIT 1	 // assigns LIMIT_X pin
#define LIMIT_X_PORT B // assigns LIMIT_X port
#define LIMIT_X_ISR 0	 // assigns LIMIT_X ISR

// Setup probe pin
#define PROBE_BIT 5
#define PROBE_PORT C
#define PROBE_ISR 1

// Setup control input pins
#define ESTOP_BIT 0
#define FHOLD_BIT 1
#define CS_RES_BIT 2
#define ESTOP_PORT C
#define FHOLD_PORT C
#define CS_RES_PORT C
#define ESTOP_ISR 1
#define FHOLD_ISR 1
#define CS_RES_ISR 1

// Setup com pins
#define RX_BIT 0
#define TX_BIT 1
#define RX_PORT D
#define TX_PORT D
#define RX_PULLUP
	// only uncomment this if other port other then 0 is used
	// #define UART_PORT 0

	// Setup PWM
#define PWM0_BIT 3	// assigns PWM0 pin
#define PWM0_PORT B // assigns PWM0 pin
#define PWM0_CHANNEL A
#define PWM0_TIMER 2

// Setup generic IO Pins
// Functionalities are set in cnc_hal_config.h file

// spindle dir
#define DOUT0_BIT 5
#define DOUT0_PORT B

// coolant
#define DOUT2_BIT 3
#define DOUT2_PORT C

	// // spindle speed sensor
	//  #define ANALOG0_BIT 4
	//  #define ANALOG0_PORT C
	//  #define ANALOG0_CHANNEL 4

// 	// servo type signal pin
// #define SERVO0_BIT 4
// #define SERVO0_PORT C

// // encoders
// #define DIN0_BIT 0
// #define DIN0_PORT C
// #define DIN0_ISR 1
// #define DIN8_BIT 1
// #define DIN8_PORT C

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 0
#define STEP0_EN_PORT B

	// Setup the Step Timer used has the heartbeat for µCNC
	// Timer 1 is used by default
	// #define ITP_TIMER 1

	// Setup the RTC Timer used by µCNC to provide an (mostly) accurate time base for all time dependent functions
	// Timer 0 is set by default
	// #define RTC_TIMER 0

#define ONESHOT_TIMER 2

/**
 *
 * These are some paramaters needed to reduce code size for the UNO board
 *
 * **/
// reduces RAM usage a bit to prevent hardware resets
#ifndef PLANNER_BUFFER_SIZE
#define PLANNER_BUFFER_SIZE 14
#endif
// reduces ITP code size by avoiding some optimizations
#undef STEP_ISR_SKIP_MAIN
#undef STEP_ISR_SKIP_IDLE

#ifndef USE_MACRO_BUFFER
#define USE_MACRO_BUFFER
#endif

#ifndef DISABLE_SETTINGS_MODULES
#define DISABLE_SETTINGS_MODULES
#endif

#ifndef DISABLE_MULTISTREAM_SERIAL
#define DISABLE_MULTISTREAM_SERIAL
#endif

#ifndef DISABLE_RTC_CODE
#define DISABLE_RTC_CODE
#endif

#ifdef EMULATE_GRBL_STARTUP
#undef EMULATE_GRBL_STARTUP
#define EMULATE_GRBL_STARTUP 2
#endif

#ifndef PRINT_FTM_MINIMAL
#define PRINT_FTM_MINIMAL
#endif

#define DISABLE_COORDINATES_SYSTEM_RAM

#ifdef __cplusplus
}
#endif

#endif
