/*
	Name: boardmap_rambo14.h
	Description: Contains all MCU and PIN definitions for Arduino UNO to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 05/02/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_RAMBO14_H
#define BOARDMAP_RAMBO14_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_AVR
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "RAMBO 1.4"
#endif

#define AVR6

#define PCINT0_PORT B
#define PCINT1_PORT J
#define PCINT2_PORT K

// SAME AS GRBL for test purposes
// Setup step pins
#define STEP4_BIT 4	 // assigns STEP4 pin
#define STEP4_PORT C // assigns STEP4 port
#define STEP3_BIT 3	 // assigns STEP3 pin
#define STEP3_PORT C // assigns STEP3 port
#define STEP2_BIT 2	 // assigns STEP2 pin
#define STEP2_PORT C // assigns STEP2 port
#define STEP1_BIT 1	 // assigns STEP1 pin
#define STEP1_PORT C // assigns STEP1 port
#define STEP0_BIT 0	 // assigns STEP0 pin
#define STEP0_PORT C // assigns STEP0 port

// Setup dir pins
#define DIR4_BIT 7	// assigns DIR4 pin
#define DIR4_PORT L // assigns DIR4 port
#define DIR3_BIT 6	// assigns DIR3 pin
#define DIR3_PORT L // assigns DIR3 port
#define DIR2_BIT 2	// assigns DIR2 pin
#define DIR2_PORT L // assigns DIR2 port
#define DIR1_BIT 1	// assigns DIR1 pin
#define DIR1_PORT L // assigns DIR1 port
#define DIR0_BIT 0	// assigns DIR0 pin
#define DIR0_PORT L // assigns DIR0 port

	// Setup limit pins

#define LIMIT_X_BIT 6		// assigns LIMIT_X pin
#define LIMIT_X_PORT B	// assigns LIMIT_X port
#define LIMIT_X_ISR 0		// assigns LIMIT_X ISR
#define LIMIT_X2_BIT 2	// assigns LIMIT_X pin
#define LIMIT_X2_PORT A // assigns LIMIT_X port
#define LIMIT_Y_BIT 5		// assigns LIMIT_Y pin
#define LIMIT_Y_PORT B	// assigns LIMIT_Y port
#define LIMIT_Y_ISR 0		// assigns LIMIT_Y ISR
#define LIMIT_Y2_BIT 1	// assigns LIMIT_Y pin
#define LIMIT_Y2_PORT A // assigns LIMIT_Y port
#define LIMIT_Z_BIT 4		// assigns LIMIT_Z pin
#define LIMIT_Z_PORT B	// assigns LIMIT_Z port
#define LIMIT_Z_ISR 0		// assigns LIMIT_Z ISR

// Setup probe pin
#define PROBE_BIT 7
#define PROBE_PORT C

// Setup com pins
#define RX_BIT 0
#define TX_BIT 1
#define RX_PORT E
#define TX_PORT E
#define RX_PULLUP
// only uncomment this if other port other then 0 is used
#define UART_PORT 0

// Setup PWM
#define PWM0_BIT 6	// assigns PWM0 pin
#define PWM0_PORT H // assigns PWM0 pin
#define PWM0_CHANNEL B
#define PWM0_TIMER 2

// Setup generic IO Pins
// Functionalities are set in config.h file
// #define DOUT0_BIT 5
// #define DOUT0_PORT B
// #define DOUT1_BIT 0
// define DOUT1_PORT B
// #define DOUT2_BIT 3
// #define DOUT2_PORT C

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 7
#define STEP0_EN_PORT A
#define STEP1_EN_BIT 6
#define STEP1_EN_PORT A
#define STEP2_EN_BIT 5
#define STEP2_EN_PORT A
#define STEP3_EN_BIT 4
#define STEP3_EN_PORT A
#define STEP4_EN_BIT 3
#define STEP4_EN_PORT A

	// Setup the Step Timer used has the heartbeat for µCNC
	// Timer 1 is used by default
	// #define ITP_TIMER 1
	// Setup the RTC Timer used by µCNC to provide an (mostly) accurate time base for all time dependent functions
	// Timer 0 is set by default
	// #define RTC_TIMER 0

// blink led
#define DOUT31_BIT 7
#define DOUT31_PORT B

// STEP0 MICROSTEP
#ifndef STEPPER0_HAS_MSTEP
#define STEPPER0_HAS_MSTEP
#endif
#define DOUT20_BIT 1
#define DOUT20_PORT G
#define DOUT12_BIT 0
#define DOUT12_PORT G

// STEP1 MICROSTEP
#ifndef STEPPER1_HAS_MSTEP
#define STEPPER1_HAS_MSTEP
#endif
#define DOUT21_BIT 7
#define DOUT21_PORT K
#define DOUT13_BIT 2
#define DOUT13_PORT G

// STEP2 MICROSTEP
#ifndef STEPPER2_HAS_MSTEP
#define STEPPER2_HAS_MSTEP
#endif
#define DOUT22_BIT 6
#define DOUT22_PORT K
#define DOUT14_BIT 5
#define DOUT14_PORT K

// STEP3 MICROSTEP
#ifndef STEPPER3_HAS_MSTEP
#define STEPPER3_HAS_MSTEP
#endif
#define DOUT23_BIT 3
#define DOUT23_PORT K
#define DOUT15_BIT 4
#define DOUT15_PORT K

// STEP4 MICROSTEP
#ifndef STEPPER4_HAS_MSTEP
#define STEPPER4_HAS_MSTEP
#endif
#define DOUT24_BIT 1
#define DOUT24_PORT K
#define DOUT16_BIT 2
#define DOUT16_PORT K

// stepper current digital potenciometer
#ifndef STEPPER_CURR_DIGIPOT
#define STEPPER_CURR_DIGIPOT
#endif
#define DOUT11_BIT 7
#define DOUT11_PORT D
#define DOUT29_BIT 2
#define DOUT29_PORT B
#define DOUT30_BIT 1
#define DOUT30_PORT B
#define DIN29_BIT 3
#define DIN29_PORT B

#define ONESHOT_TIMER 2

#define ENABLE_PARSER_MODULES

#ifdef __cplusplus
}
#endif

#endif
