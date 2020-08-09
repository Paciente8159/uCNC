/*
	Name: mcumap_virtual.h
	Description: Contains all MCU and PIN definitions for a PC to run µCNC.
	
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
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
#define rom_read_byte *

//commento to use console only
//#define COMPORT "\\\\.\\COM3"

#ifndef COMPORT
#define USECONSOLE
#define COMPORT ""
#endif

//defines a pointer to an unknow stucture that is defined in the mcu_virtual
typedef struct virtual_map_t
{
	uint32_t outputs;
	uint32_t inputs;
	unsigned char uart;
}VIRTUAL_MAP;
typedef VIRTUAL_MAP* virtports_t;
extern virtports_t virtualports;

//joints step/dir pins
#define STEP0 0
#define STEP1 1
#define STEP2 2
#define STEP6 11
#define DIR0 3
#define DIR1 4
#define DIR2 5
#define PWM0 6
#define DOUT0 7
#define DOUT1 8
#define DOUT2 9
#define DOUT3 10
#define STEPS_EN DOUT3
#define OUTREG virtualports->outputs

#define mcu_get_output(X) (OUTREG & (1<<(X)))
#define mcu_set_output(X) (OUTREG |= (1<<(X)))
#define mcu_clear_output(X) (OUTREG &= ~(1<<(X)))
#define mcu_toggle_output(X) (OUTREG ^= (1<<(X)))

//critical inputs
#define ESTOP 0
#define SAFETY_DOOR 1
#define FHOLD 2
#define CS_RES 3
#define LIMIT_X 4
#define LIMIT_Y 5
#define LIMIT_Z 6
#define LIMIT_Y2 6
#define PROBE 7

#define INREG virtualports->inputs
#define mcu_get_input(X) (INREG & (1<<(X)))

#define COM_INREG virtualports->uart
#define COM_OUTREG virtualports->uart

#endif
