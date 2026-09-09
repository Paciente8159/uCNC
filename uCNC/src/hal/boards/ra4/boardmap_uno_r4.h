/*
	Name: boardmap_uno_r4.h
	Description: Contains all MCU and PIN definitions for Arduino UNO R4 (RA4M1) to run uCNC.

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins
	Date: 09/09/2026

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef BOARDMAP_UNO_R4_H
#define BOARDMAP_UNO_R4_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_RA4M1
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "Arduino UNO R4"
#endif

#ifndef F_CPU
#define F_CPU SystemCoreClock
#endif

// Pin mapping for Arduino UNO R4 Minima (RA4M1)
// D0  = P100, D1  = P101, D2  = P104, D3  = P103
// D4  = P105, D5  = P002, D6  = P106, D7  = P003
// D8  = P004, D9  = P206, D10 = P102, D11 = P107
// D12 = P113, D13 = P112
// A0  = P000, A1  = P001, A2  = P014, A3  = P015
// A4  = P012, A5  = P013

// Setup step pins (reusing UNO shield layout)
#define STEP0_BIT 4  // D2 (P104)
#define STEP0_PORT 1 // PORT1
#define STEP1_BIT 3  // D3 (P103)
#define STEP1_PORT 1 // PORT1
#define STEP2_BIT 5  // D4 (P105)
#define STEP2_PORT 1 // PORT1

// Setup dir pins
#define DIR0_BIT 2  // D5 (P002)
#define DIR0_PORT 0 // PORT0
#define DIR1_BIT 6  // D6 (P106)
#define DIR1_PORT 1 // PORT1
#define DIR2_BIT 3  // D7 (P003)
#define DIR2_PORT 0 // PORT0

// Setup limit pins
#define LIMIT_X_BIT 6  // D9 (P206)
#define LIMIT_X_PORT 2 // PORT2
#define LIMIT_X_ISR

#define LIMIT_Y_BIT 2  // D10 (P102)
#define LIMIT_Y_PORT 1 // PORT1
#define LIMIT_Y_ISR

#define LIMIT_Z_BIT 7  // D11 (P107)
#define LIMIT_Z_PORT 1 // PORT1
#define LIMIT_Z_ISR

// Setup probe pin
#define PROBE_BIT 0  // A0 (P000)
#define PROBE_PORT 0 // PORT0

// Setup control input pins
#define ESTOP_BIT  13 // D12 (P113)
#define ESTOP_PORT 1  // PORT1
#define ESTOP_ISR

#define FHOLD_BIT 4   // D8 (P004)
#define FHOLD_PORT 0  // PORT0
#define FHOLD_ISR

#define CS_RES_BIT 1  // A1 (P001)
#define CS_RES_PORT 0 // PORT0
#define CS_RES_ISR

// Setup COM (UART) pins - SCI1 on P100(RX)/P101(TX)
#define TX_BIT 1
#define TX_PORT 1
#define RX_BIT 0
#define RX_PORT 1
#define RX_PULLUP
#define COM_UART 1

// Setup PWM (spindle)
#define PWM0_BIT 3     // D3 (P103) - uses GPT compare output
#define PWM0_PORT 1
#define PWM0_TIMER 4   // GPT164
#define PWM0_CHANNEL A

// Setup generic IO Pins
// Spindle direction
#define DOUT0_BIT 14 // A2 (P014)
#define DOUT0_PORT 0 // PORT0

// Coolant
#define DOUT2_BIT 15 // A3 (P015)
#define DOUT2_PORT 0 // PORT0

// Stepper enable pin
#define STEP0_EN_BIT 12 // D13 (P112) - also LED_BUILTIN
#define STEP0_EN_PORT 1 // PORT1

// Timer allocation
#define ITP_TIMER 0     // GPT320 (32-bit)
#define RTC_TIMER 0     // SysTick
#define ONESHOT_TIMER 3 // GPT163 (16-bit)

/**
 * Memory and performance settings for UNO R4
 */
#ifndef PLANNER_BUFFER_SIZE
#define PLANNER_BUFFER_SIZE 20
#endif

#ifdef __cplusplus
}
#endif

#endif