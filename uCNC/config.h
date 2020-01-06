/*
	Name: config.h
	Description: Compile time configurations for uCNC
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

#include "mcus.h"
#include "machines.h"

/*
	Choose the mcu type
	MCU_VIRTUAL: simulates a mcu on a PC
	MCU_ATMEGA328P: atmega328p mcu (Arduino Uno)
*/

/*#define MCU MCU_ATMEGA328P
#define __DEBUG__
#define __SIMUL__*/
#define BAUD 115200

/*
	Machine kynematics
	Defines the machine kynematics (cartesian, corexy, delta, custom, ...)
*/
#define MACHINE_KINEMATICS MACHINE_CARTESIAN_XYZ

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

#endif
