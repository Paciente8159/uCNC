/*
	Name: mcumap_virtual.h
	Description: Contains all MCU and PIN definitions for a PC to run uCNC.
	
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

#ifndef MCUMAP_VIRTUAL_H
#define MCUMAP_VIRTUAL_H

#include <stdint.h>
#include "util\timer.h"
#define F_CPU 1000
#define F_STEP_MAX 500
#define F_STEP_MIN 1
#define __rom__
#define __romstr__
#define rom_strptr *
#define rom_strcpy strcpy
#define rom_strncpy strncpy
#define rom_memcpy memcpy
#define rom_read_byte

typedef struct virtual_map_t virtual_map_t;

typedef virtual_map_t* virtports_t;
extern virtports_t virtualports;

//joints step/dir pins
#define STEP0 0
#define STEP1 1
#define STEP2 2
#define STEPS_OUTREG virtualports->steps

#define DIR0 0
#define DIR1 1
#define DIR2 2
#define DIRS_OUTREG virtualports->dirs

//critical inputs
#define ESTOP 0
#define FHOLD 1
#define CS_RES 2
#define CONTROLS_INREG virtualports->controls

#define LIMIT_X 0
#define LIMIT_Y 1
#define LIMIT_Z 2
#define LIMITS_INREG virtualports->limits

#define PROBE 3
#define PROBE_INREG virtualports->limits

#define PWM0 0
#define DOUT0 0
#define DOUT1 1
#define DOUT2 2

#endif
