/*
	Name: mcumap_grbl.h
	Description: Contains all MCU and PIN definitions for Arduino UNO to run µCNC.

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

#ifndef BOARDMAP_BLACKPILL_H
#define BOARDMAP_BLACKPILL_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "STM32 Blackpill"
#endif

// Setup step pins
#define DIN0_BIT 0	 // assigns STEP0 pin
#define DIN0_PORT A // assigns STEP0 port
#define DIN1_BIT 1	 // assigns STEP1 pin
#define DIN1_PORT A // assigns STEP1 port
#define DIN2_BIT 2	 // assigns STEP2 pin
#define DIN2_PORT A // assigns STEP2 port
#define DIN3_BIT 3	 // assigns STEP3 pin
#define DIN3_PORT A // assigns STEP3 port
#define DIN0_PULLUP
#define DIN0_ISR
#define DIN1_PULLUP
#define DIN1_ISR
#define DIN2_PULLUP
#define DIN2_ISR
#define DIN3_PULLUP
#define DIN3_ISR

// Setup dir pins
#define DIN8_BIT 4	// assigns DIR0 pin
#define DIN8_PORT A // assigns DIR0 port
#define DIN9_BIT 5	// assigns DIR1 pin
#define DIN9_PORT A // assigns DIR1 port
#define DIN10_BIT 6	// assigns DIR2 pin
#define DIN10_PORT A // assigns DIR2 port
#define DIN11_BIT 7	// assigns DIR3 pin
#define DIN11_PORT A // assigns DIR3 port
#define DIN8_PULLUP
#define DIN9_PULLUP
#define DIN10_PULLUP
#define DIN11_PULLUP

#define UART_PORT 1
#define TX_BIT 9
#define TX_PORT A
#define RX_BIT 10
#define RX_PORT A
#define RX_PULLUP

#ifdef __cplusplus
}
#endif

#endif
