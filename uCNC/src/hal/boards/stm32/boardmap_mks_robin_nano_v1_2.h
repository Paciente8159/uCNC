/*
	Name: boardmap_mks_robin_nano_v1_2.h
	Description: Contains all MCU and PIN definitions for board MKS Robin Nano V1.2 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 04/02/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_MKS_ROBIN_NANO_V12_H
#define BOARDMAP_MKS_ROBIN_NANO_V12_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "MKS Robin Nano 1.2"
#endif

#ifndef STM32F103xE
#define STM32F103xE
#endif

#ifndef STM32F10X_HD
#define STM32F10X_HD
#endif

#ifndef F_CPU
#define F_CPU 72000000UL
#endif

#ifndef PLANNER_BUFFER_SIZE
#define PLANNER_BUFFER_SIZE 30
#endif
#ifndef RX_BUFFER_CAPACITY
#define RX_BUFFER_CAPACITY 256
#endif

// Setup COM pins
#define COM_PORT 3

// Setup step pins
#define STEP0_BIT 3	 // assigns STEP0 pin
#define STEP0_PORT E // assigns STEP0 port
#define STEP1_BIT 0	 // assigns STEP1 pin
#define STEP1_PORT E // assigns STEP1 port
#define STEP2_BIT 5	 // assigns STEP2 pin
#define STEP2_PORT B // assigns STEP2 port
#define STEP3_BIT 6	 // assigns STEP3 pin
#define STEP3_PORT D // assigns STEP3 port
#define STEP4_BIT 6	 // assigns STEP3 pin
#define STEP4_PORT A // assigns STEP3 port

// Setup dir pins
#define DIR0_BIT 2	// assigns DIR0 pin
#define DIR0_PORT E // assigns DIR0 port
#define DIR1_BIT 9	// assigns DIR1 pin
#define DIR1_PORT B // assigns DIR1 port
#define DIR2_BIT 4	// assigns DIR2 pin
#define DIR2_PORT B // assigns DIR2 port
#define DIR3_BIT 3	// assigns DIR3 pin
#define DIR3_PORT D // assigns DIR3 port
#define DIR4_BIT 1	// assigns DIR3 pin
#define DIR4_PORT A // assigns DIR3 port

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 4
#define STEP0_EN_PORT E
#define STEP1_EN_BIT 1
#define STEP1_EN_PORT E
#define STEP2_EN_BIT 8
#define STEP2_EN_PORT B
#define STEP3_EN_BIT 3
#define STEP3_EN_PORT B
#define STEP4_EN_BIT 3
#define STEP4_EN_PORT A

// Setup limit pins
#define LIMIT_X_BIT 15	// assigns LIMIT_X pin
#define LIMIT_X_PORT A	// assigns LIMIT_X port
#define LIMIT_Y_BIT 12	// assigns LIMIT_Y pin
#define LIMIT_Y_PORT A	// assigns LIMIT_Y port
#define LIMIT_Z_BIT 4	// assigns LIMIT_Z+ pin
#define LIMIT_Z_PORT C	// assigns LIMIT_Z+ port
#define LIMIT_Z2_BIT 11 // assigns LIMIT_Z- pin
#define LIMIT_Z2_PORT A // assigns LIMIT_Z- port

// Enable limits switch interrupt
#define LIMIT_X_ISR
#define LIMIT_Y_ISR
#define LIMIT_Z_ISR
#define LIMIT_Z2_ISR

// Setup probe pin
#define PROBE_BIT 8
#define PROBE_PORT A
#define PROBE_ISR

// On the STM32 always use sync TX UART (async doesn't work well)
#ifdef COM_PORT
#define TX_BIT 10
#define TX_PORT B
#define RX_BIT 11
#define RX_PORT B
#endif

// Setup PWM
#define PWM0_BIT 0	// assigns PWM0 pin
#define PWM0_PORT A // assigns PWM0 pin
#define PWM0_CHANNEL 1
#define PWM0_TIMER 2
#define PWM1_BIT 0	// assigns PWM2 pin
#define PWM1_PORT B // assigns PWM2 pin
#define PWM1_CHANNEL 3
#define PWM1_TIMER 3
#define PWM2_BIT 1	// assigns PWM3 pin
#define PWM2_PORT B // assigns PWM3 pin
#define PWM2_CHANNEL 4
#define PWM2_TIMER 3

// spindle dir
// analog input
#define ANALOG0_BIT 0
#define ANALOG0_PORT C
#define ANALOG0_CHANNEL 10
#define ANALOG1_BIT 1
#define ANALOG1_PORT C
#define ANALOG1_CHANNEL 11
#define ANALOG2_BIT 2
#define ANALOG2_PORT C
#define ANALOG2_CHANNEL 12

// Setup the Step Timer used has the heartbeat for µCNC
#define ITP_TIMER 5

#ifdef __cplusplus
}
#endif

#endif
