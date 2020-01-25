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
#define USE_SPINDLE
#ifdef USE_SPINDLE
//set PWM channel to use (valid values 0 - 3)
#define SPINDLE_PWM_CHANNEL 0
//sets the spindle dir pin mask
#define SPINDLE_DIR_OUTPIN 0
#endif

/*
	Define a stepper enable pin
*/
//#define STEPPER_ENABLE DOUT1_MASK

/*
	Define a coolant flood and mist pin
*/
#define USE_COOLANT
#ifdef USE_COOLANT
#define COOLANT_FLOOD_OUTPIN 8
#define COOLANT_MIST_OUTPIN 9
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
#define GCODE_IGNORE_LINE_NUMBERS
//#define GCODE_ACCEPT_WORD_E

/*
	Report specific options
*/
#define STATUS_WCO_REPORT_MIN_FREQUENCY 30
#define STATUS_OVR_REPORT_MIN_FREQUENCY STATUS_WCO_REPORT_MIN_FREQUENCY - 1

/*
	Compilation specific options
*/
//#define FORCE_GLOBALS_TO_0
//#define CRC_WITHOUT_LOOKUP_TABLE //saves a little program memory bytes but much more slow CRC check

/*
	Number of seconds of delay before motions restart after releasing from a hold
*/

#define DELAY_ON_RESUME 4

#endif
