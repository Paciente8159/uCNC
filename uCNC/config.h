/*
	Name: config.h
	Description: Compile time configurations for uCNC.
	
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 19/09/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef CONFIG_H
#define CONFIG_H

//include lists of available option
#include "mcus.h"
#include "machines.h"

/*
	Choose the mcu type
	MCU_VIRTUAL: simulates a mcu on a PC
	MCU_ATMEGA328P: atmega328p mcu (Arduino Uno)
*/
//#define MCU MCU_ATMEGA328P

/*
	Machine kynematics
	Defines the machine kynematics (cartesian, corexy, delta, custom, ...)
*/
#define MACHINE_KINEMATICS MACHINE_CARTESIAN_XYZ

//defines the uCNC generic mapping
#include "mcumap.h"

/*
	Serial COM
	Defines the serial COM baud rate
	Uses 1 start bit + 8 bit + 1 stop bit (no parity)
*/
#define BAUD 115200

/*
	Defines the number of supported coordinate systems supported by uCNC
	Can be any value between 1 and 9
*/

#define COORD_SYS_COUNT 6

/*
	Number of segments of an arc computed with aprox. of sin/cos math operation before performing a full calculation
*/
#define N_ARC_CORRECTION 12

/*
	Echo recieved commands
	Uncomment to enable. Only necessary to debug communication problems
*/
//#define ECHO_CMD

/*
	Spindle configurations.
	Uncomment to enable
*/
//#define USE_SPINDLE
#ifdef USE_SPINDLE
	//set PWM channel to use (valid values 0 - 3)
	#define SPINDLE_PWM_CHANNEL 0
	//sets the spindle dir pin mask
	#define SPINDLE_DIR DOUT0_MASK
#endif

/*
	Define a stepper enable pin
*/
//#define STEPPER_ENABLE DOUT1_MASK

/*
	Define a coolant enable pin
*/
//#define COOLANT_ENABLE_PIN DOUT2

/*
	Special definitions used to debug code
*/
//#define __DEBUG__
//#define __PERFSTATS__

#endif
