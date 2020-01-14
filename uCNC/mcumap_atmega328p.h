/*
	Name: mcumap_atmega328p.h
	Description: Contains all MCU and PIN definitions for Arduino UNO to run uCNC.
	
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 01/11/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUMAP_ATMEGA328P_H
#define MCUMAP_ATMEGA328P_H

/*
	MCU specific definitions and replacements
*/
#include <avr/pgmspace.h>
#define F_CPU 16000000UL
#define F_STEP_MAX 30000
#define F_STEP_MIN 4
#define __rom__ PROGMEM
#define __romstr__ PSTR
#define rom_strptr pgm_read_byte_near
#define rom_strcpy strcpy_P
#define rom_strncpy strncpy_P
#define rom_memcpy memcpy_P
#define rom_read_byte pgm_read_byte
//used by the parser
//this method is faster then normal multiplication (for 32 bit for 16 and 8 bits is slightly lower)
#define fast_mult10(X) (((X<<2) + X)<<1)

/*
	Setup the IO pins 
	The definition of the pins must match the PORT/REGISTER bit offset and not the IDE pin number
	with the formula:
	
	Ex:
	If DIR0 is bit 1 (<BIT_OFFSET>) of PORTC DIR0 will be 1
*/

//Setup output pins
//SAME AS GRBL for test purposes
#define STEP0 0
#define DIR0 3
#define STEP1 1
#define DIR1 4
#define STEP2 2
#define DIR2 5
#define STEPS_OUTREG PORTC
#define STEPS_DIRREG DDRC
#define DIRS_OUTREG PORTC
#define DIRS_DIRREG DDRC

#define LIMIT_X 2
#define LIMIT_Y 3
#define LIMIT_Z 4
#define PROBE 5
#define LIMITS_INREG PIND
#define LIMITS_DIRREG DDRD
#define LIMITS_PULLUPREG PORTD
#define LIMITS_ISRREG PCMSK2
#define LIMITS_ISR_ID 2

#define LIMIT_X_PULLUP
#define LIMIT_Y_PULLUP
#define LIMIT_Z_PULLUP

//Setup control input pins
#define ESTOP 7
#define FHOLD 6
#define CONTROLS_INREG PIND
#define CONTROLS_DIRREG DDRD
#define CONTROLS_PULLUPREG PORTD
#define CONTROLS_ISRREG PCMSK2
#define CONTROLS_ISR_ID 2

#define ESTOP_PULLUP
#define FHOLD_PULLUP

//Setup com pins
#define RX 0
#define TX 1
#define COM_INREG PIND
#define COM_OUTREG PORTD
#define COM_DIRREG DDRD

#define PWM0 3
#define PWM0_DIRREG DDRB
#define PWM0_OCREG A
#define PWM0_TMRAREG TCCR2A
#define PWM0_TMRBREG TCCR2B
#define PWM0_CNTREG OCR2A
#define PWM0_PRESCMASK 0x04

#define DOUT0 0
#define DOUTS_R0_OUTREG PORTB
#define DOUTS_R0_DIRREG DDRB

#endif
