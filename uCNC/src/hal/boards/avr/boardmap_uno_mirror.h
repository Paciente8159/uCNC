/*
	Name: boardmap_uno_mirror.h
	Description: Contains all MCU and PIN definitions for Arduino UNO to run µCNC in mirror mode.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 04-03-2026

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_UNO_MIRROR_H
#define BOARDMAP_UNO_MIRROR_H

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

#ifndef F_CPU
#define F_CPU 16000000UL
#endif


// SAME AS GRBL for test purposes
// Setup step pins
#define DIN2_BIT 4	 // assigns STEP2 pin
#define DIN2_PORT D // assigns STEP2 port
#define DIN1_BIT 3	 // assigns STEP1 pin
#define DIN1_PORT D // assigns STEP1 port
#define DIN0_BIT 2	 // assigns STEP0 pin
#define DIN0_PORT D // assigns STEP0 port
#define DIN2_ISR 20
#define DIN1_ISR 19
#define DIN0_ISR 18
#define DIN2_PULLUP
#define DIN1_PULLUP
#define DIN0_PULLUP

// Setup dir pins
#define DIN10_BIT 7	// assigns DIR2 pin
#define DIN10_PORT D // assigns DIR2 port
#define DIN9_BIT 6	// assigns DIR1 pin
#define DIN9_PORT D // assigns DIR1 port
#define DIN8_BIT 5	// assigns DIR0 pin
#define DIN8_PORT D // assigns DIR0 port
#define DIN10_PULLUP
#define DIN9_PULLUP
#define DIN8_PULLUP

// Setup com pins
#define RX_BIT 0
#define TX_BIT 1
#define RX_PORT D
#define TX_PORT D
#define RX_PULLUP

// led
#define DOUT31_BIT 5
#define DOUT31_PORT B

/**
 *
 * These are some paramaters needed to reduce code size for the UNO board
 *
 * **/
// reduces RAM usage a bit to prevent hardware resets
#ifndef PLANNER_BUFFER_SIZE
#define PLANNER_BUFFER_SIZE 14
#endif

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
#define EMULATE_GRBL_STARTUP 3
#endif

#ifndef PRINT_FTM_MINIMAL
#define PRINT_FTM_MINIMAL
#endif

#define DISABLE_COORDINATES_SYSTEM_RAM

#define ENCODERS 3
#define ENC0_PULSE DIN0
#define ENC1_PULSE DIN1
#define ENC2_PULSE DIN2
#define ENC0_DIR DIN8
#define ENC1_DIR DIN9
#define ENC2_DIR DIN10
#define STEP0_ENCODER ENC0
#define STEP1_ENCODER ENC1
#define STEP2_ENCODER ENC2

#ifdef __cplusplus
}
#endif

#endif
