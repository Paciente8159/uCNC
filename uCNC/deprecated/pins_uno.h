/*
	Name: pins_uno.c - chain planner for linear motions and acceleration/deacceleration profiles
	Copyright: 2019 João Martins
	Author: João Martins
	Date: Oct/2019
	Description: uCNC is a free cnc controller software designed to be flexible and
	portable to several	microcontrollers/architectures.
	uCNC is a FREE SOFTWARE under the terms of the GPLv3 (see <http://www.gnu.org/licenses/>).
*/

#ifndef PINS_UNO_H
#define PINS_UNO_H

#include <avr/io.h>
//Regiter Mapping

//Maps the UNO internal 8-bit registers to the 32-bit machine register
//<PORT3>-<PORT2>-<PORT1>-<PORT0>

//PORT0
//#define PORTWR0 PORTB
//#define PORTRD0 PINB
//#define PORTDIR0 DDRB
//#define PORTISR0 PCMSK0

//port c as only outputs
//PORT1
#define PORTWR1 PORTC
//#define PORTRD1 PINC
#define PORTDIR1 DDRC
//#define PORTISR1 PCMSK1

//PORT2
#define PORTWR2 PORTD
#define PORTDIR2 DDRD
#define PORTRD2 PIND
#define PORTISR2 PCMSK2

//PORT3
//doesn't have

/*
	Setup the IO pins 
	The definition of the pins must match the PORT/REGISTER bit offset and not the IDE pin number
	with the formula:
	
	Ex:
	If DIR0 is bit 1 (<BIT_OFFSET>) of PORTC DIR0 will be 1
*/

//Setup output pins
#define STEP0 0
#define DIR0 3
#define STEP1 1
#define DIR1 4
#define STEP2 2
#define DIR2 5
#define STEPS_OUTREG PORTWR1
#define STEPS_DIRREG PORTDIR1
#define DIRS_OUTREG PORTWR1
#define DIRS_DIRREG PORTDIR1

#define LIMIT_X 5
#define LIMIT_Y 6
#define LIMIT_Z 7
#define LIMITS_INREG PORTRD2
#define LIMITS_DIRREG PORTDIR2
#define LIMITS_PULLUPREG PORTWR2
#define LIMITS_ISRREG PORTISR2

#define PROBE 4
#define PROBE_INREG PORTRD2
#define PROBE_DIRREG PORTDIR2
#define PROBE_PULLUPREG PORTWR2
#define PROBE_ISRREG PORTISR2

//Setup control input pins
#define ESTOP 2
#define FHOLD 3
#define CONTROLS_INREG PORTRD2
#define CONTROLS_DIRREG PORTDIR2
#define CONTROLS_PULLUPREG PORTWR2
#define CONTROLS_ISRREG PORTISR2

//Setup com pins
#define RX 0
#define TX 1
#define COM_INREG PORTRD2
#define COM_OUTREG PORTWR2
#define COM_DIRREG PORTDIR2

//not used
//#define COM_PULLUPREG PORTD
//#define COM_ISRREG PCMSK2

#endif
