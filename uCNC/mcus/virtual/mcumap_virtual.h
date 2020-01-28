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
#define F_CPU 1000
#define F_STEP_MAX 500
#define F_STEP_MIN 1
#define __rom__
#define __romstr__
#define __romarr__ const char
#define rom_strptr *
#define rom_strcpy strcpy
#define rom_strncpy strncpy
#define rom_memcpy memcpy
#define rom_read_byte

//commento to use console only
#define COMPORT "\\\\.\\COM3"

#ifndef COMPORT
#define USECONSOLE
#define COMPORT ""
#endif

//defines a pointer to an unknow stucture that is defined in the mcu_virtual
typedef struct virtual_map_t
{
	uint8_t steps;
	uint8_t dirs;
	uint8_t controls;
	uint8_t limits;
	uint8_t probe;
	uint32_t outputs;
	unsigned char uart;
}VIRTUAL_MAP;
typedef VIRTUAL_MAP* virtports_t;
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
#define SAFETY_DOOR 1
#define FHOLD 2
#define CS_RES 3
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
#define DOUTS_R0_OUTREG virtualports->outputs
#define DOUT8 0
#define DOUTS_R1_OUTREG virtualports->outputs

#define COM_INREG virtualports->uart
#define COM_OUTREG virtualports->uart

#endif
