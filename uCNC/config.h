#ifndef CONFIG_H
#define CONFIG_H

#include "boards.h"
#include "machines.h"

/*
	Choose the board type
	Board virtual: simulates a board on a PC
	Board uno: Arduino uno board 
*/
//#define BOARD BOARD_VIRTUAL
//#define BOARD BOARD_UNO

/*
	Machine kynematics
	Defines the machine kynematics (cartesian, corexy, delta, custom, ...)
*/
#define MACHINE_KINEMATICS MACHINE_CARTESIAN_XYZ

#define STEPPER_COUNT 3
#define COORD_SYS_COUNT 6


#define DEBUGMODE
//setup IO masks
#define STEPDIR_INVERT_MASK 0x0000
#define OUTPUT_INVERT_MASK 0x0000

#define CRITICAL_INVERT_MASK 0x00
#define INPUT_INVERT_MASK 0x0000

//#define USE_PULLUPS
#ifdef USE_PULLUPS
	#define CRITICAL_PULLUP_MASK 0x00
	#define INPUT_PULLUP_MASK 0x0000
#endif

#define COM_BUFFER_SIZE 10

#define TOTAL_STEPPERS 5
#define MIN_PULSE_WIDTH_US 5

#endif
