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

#ifndef MCUMAP_UNO_GRBL_H
#define MCUMAP_UNO_GRBL_H

/*
	MCU specific definitions and replacements
*/
#include <avr/pgmspace.h>
#include <avr/cpufunc.h>

//defines the frequency of the mcu
#define F_CPU 16000000UL
//defines the maximum and minimum step rates
#define F_STEP_MAX 30000
#define F_STEP_MIN 4
//defines special mcu to access flash strings and arrays
#define __rom__ PROGMEM
#define __romstr__ PSTR
#define rom_strptr pgm_read_byte
#define rom_strcpy strcpy_P
#define rom_strncpy strncpy_P
#define rom_memcpy memcpy_P
#define rom_read_byte pgm_read_byte

//used by the parser
//this method is faster then normal multiplication (for 32 bit for 16 and 8 bits is slightly lower)
//if not defined utils.h will use standard math operation
#define fast_mult10(X) ((((X)<<2) + (X))<<1)

//defines the port with the interrupt on change feature for this AVR
#define PCMASK0_INREG PINB
#define PCMASK1_INREG PINJ
#define PCMASK2_INREG PINK

/*
	Setup the IO pins 
	The definition of the pins must match the PORT/REGISTER bit offset and not the IDE pin number
	with the formula:
	
	Ex:
	If DIR0 is bit 1 (<BIT_OFFSET>) of PORTC DIR0 will be 1

	Also the step, dir, limits, etc.. ports must be defined because mcu.h uses this to perform direct port IO operations
	Output ports have the suffix _OUTREG
	Input ports have the suffix _INREG

	For exemple:
	STEP pins are outputs. So STEP_OUTREG must be defined to the corresponding mcu IO port.
	
*/

//SAME AS GRBL for test purposes
//Setup step pins
#define STEP2 2
#define STEP1 1
#define STEP0 0
#define STEPS_OUTREG PORTC
#define STEPS_DIRREG DDRC

//Setup dir pins
#define DIR2 2
#define DIR1 1
#define DIR0 0
#define DIRS_OUTREG PORTL
#define DIRS_DIRREG DDRL

//Setup limit pins
#define LIMIT_Z 6
#define LIMIT_Y 5
#define LIMIT_X 4
#define LIMITS_INREG PINB
#define LIMITS_DIRREG DDRB
#define LIMITS_PULLUPREG PORTB
#define LIMITS_ISR_ID 0

//Active limits switch weak pull-ups
#define LIMIT_X_PULLUP
#define LIMIT_Y_PULLUP
#define LIMIT_Z_PULLUP

//Setup probe pin
#define PROBE 4
#define PROBE_INREG PINJ
#define PROBE_DIRREG DDRJ
#define PROBE_PULLUPREG PORTJ
#define PROBE_ISR_ID 1

//Setup control input pins
#define ESTOP 2
#define FHOLD 3
//#define CS_RES 2
#define CONTROLS_INREG PINJ
#define CONTROLS_DIRREG DDRJ
#define CONTROLS_PULLUPREG PORTJ
#define CONTROLS_ISR_ID 1

//Active controls switch weak pull-ups
//#define ESTOP_PULLUP
//#define FHOLD_PULLUP

//Setup com pins
#define RX 0
#define TX 1
#define COM_DIRREG DDRE
#define COM_INREG UDR0
#define COM_OUTREG UDR0
#define COM_ID 0 //if only one or default comment this

//Setup PWM
#define PWM0 6
#define PWM0_DIRREG DDRH
#define PWM0_TIMER_ID 2 //assigns PWM0 timer
#define PWM0_OCR_ID B //assigns PWM0 pwm output source

//Setup generic IO Pins
//Functions are set in config.h file
/*#define DOUT0 6
#define DOUT1 7
#define DOUTS_R0_OUTREG PORTE
#define DOUTS_R0_DIRREG DDRE
#define DOUT8 3
#define DOUTS_R1_OUTREG PORTE
#define DOUTS_R1_DIRREG DDRE*/

#define DOUT16 7 //assigns DOUT16 to PIN7
#define DOUT17 6 //assigns DOUT17 to PIN6
#define DOUT18 5 //assigns DOUT18 to PIN5
#define DOUTS_R2_OUTREG PORTA //assings R2_OUTREG (DOUT16-23) to PORT A
#define DOUTS_R2_DIRREG DDRA

#define STEP0_EN_OUTPIN 16 //assigns STEP0_EN to DOUT16
#define STEP1_EN_OUTPIN 17 //liassignsnks STEP0_EN to DOUT17
#define STEP2_EN_OUTPIN 18 //assigns STEP0_EN to DOUT18

//Setup the Timer used has the heartbeat for µCNC
#define STEP_TIMER_ID 1

#endif
