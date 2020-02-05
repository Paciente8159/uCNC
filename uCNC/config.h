/*
	Name: config.h
	Description: Compile time configurations for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 19/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef CONFIG_H
#define CONFIG_H

//include lists of available option
#include "mcus.h"
#include "machines.h"

/*
	Choose the mcu/board
	MCU_VIRTUAL: simulates a mcu on a PC
	MCU_UNO_GRBL: atmega328p mcu (Arduino Uno)
*/
#define MCU MCU_UNO_GRBL

#include "mcudefs.h"
/*
	Machine kynematics
	Defines the machine kynematics (cartesian, corexy, delta, custom, ...)
*/
#define MACHINE_KINEMATICS MACHINE_CARTESIAN_XYZ

/*
	Serial COM
	Defines the serial COM baud rate
	Uses 1 start bit + 8 bit + 1 stop bit (no parity)
*/
#define BAUD 115200

/*
	Defines the number of supported coordinate systems supported by µCNC
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
#define USE_SPINDLE
#ifdef USE_SPINDLE
//set PWM channel to use (valid values 0 - 3)
#define SPINDLE_PWM PWM0
//sets the spindle dir pin mask
#define SPINDLE_DIR DOUT0
/*
	Number of seconds of delay before motions restart after releasing from a hold or after setting a new spindle speed
	This is used by spindle to ensure spindle gets up to speed in motions
*/
#define DELAY_ON_RESUME 4
#define DELAY_ON_SPINDLE_SPEED_CHANGE 1
//#define LASER_MODE
#endif

#define USE_TOOL_CHANGER

/*
	Define a stepper enable pin
*/
//#define STEPPER_ENABLE DOUT1_MASK

/*
	Define a coolant flood and mist pin
*/
#define USE_COOLANT
#ifdef USE_COOLANT
#define COOLANT_FLOOD DOUT1
#define COOLANT_MIST DOUT2
#endif

/*
	Special definitions used to debug code
*/
//#define __DEBUG__
//#define __PERFSTATS__

/*
	Feed overrides increments and percentage ranges
*/
#define FEED_OVR_MAX 200
#define FEED_OVR_MIN 10
#define FEED_OVR_COARSE 10
#define FEED_OVR_FINE 1

/*
	Rapid feed overrides percentages
*/
#define RAPID_FEED_OVR1 50
#define RAPID_FEED_OVR2 25

/*
	Spindle speed overrides increments percentages and ranges
*/
#define SPINDLE_OVR_MAX 200
#define SPINDLE_OVR_MIN 10
#define SPINDLE_OVR_COARSE 10
#define SPINDLE_OVR_FINE 1

/*
	G-code options
*/
//#define GCODE_PROCESS_LINE_NUMBERS
//#define PROCESS_COMMENTS
//#define GCODE_ACCEPT_WORD_E
//set factor for countinuos mode
//value must be set between 0.0 and 1.0 If set to 1.0 is the same as exact path mode (G61)
#define G64_MAX_ANGLE_FACTOR 0.2f

/*
	Report specific options
*/
#define STATUS_WCO_REPORT_MIN_FREQUENCY 30
#define STATUS_OVR_REPORT_MIN_FREQUENCY STATUS_WCO_REPORT_MIN_FREQUENCY - 1

/*
	Compilation specific options
*/
//#define FORCE_GLOBALS_TO_0 //ensure all variables are set to 0 at start up
//#define CRC_WITHOUT_LOOKUP_TABLE //saves a little program memory bytes but much more slow CRC check
//#define NO_FAST_SQRT //disable the using of Quake III style fast sqrt. Feed rate display will be more precise.

#endif
