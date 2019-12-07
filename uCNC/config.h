#ifndef CONFIG_H
#define CONFIG_H

#include "mcus.h"
#include "machines.h"

#define VERSION_NUMBER_HIGH 0
#define VERSION_NUMBER_LOW 1
#define REVISION_NUMBER 'A'

/*
	Choose the mcu type
	Mcu virtual: simulates a mcu on a PC
	Mcu atmega328p: atmega328p mcu
*/
/*#define MCU MCU_ATMEGA328P
#define F_CPU 16000000UL
#define __DEBUG__
#define __SIMUL__*/
/*
	Machine kynematics
	Defines the machine kynematics (cartesian, corexy, delta, custom, ...)
*/
#define MACHINE_KINEMATICS MACHINE_CARTESIAN_XYZ

//defines the number of supported coordinate systems
#define COORD_SYS_COUNT 6

#define N_ARC_CORRECTION 12

#endif
