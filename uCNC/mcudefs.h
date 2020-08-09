/*
	Name: mcudefs.h
	Description: Defines the available machine types.

	Copyright: Copyright (c) Jo�o Martins
	Author: Jo�o Martins
	Date: 01/11/2019

	�CNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	�CNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUSDEFS_H
#define MCUSDEFS_H

#include "mcus.h"

/*
	MCU port map
*/
#if(MCU != 0)
#else
#error Invalid mcu configuration
#endif

#if(MCU == MCU_AVR)
#include "mcus/avr/mcumap_avr.h"
#endif

#if(MCU == MCU_STM32F10X)
#include "mcus/stm32f10x/mcumap_stm32f10x.h"
#endif

#if(MCU == MCU_VIRTUAL)
#include "mcus/virtual/mcumap_virtual.h"
#endif

#ifndef MCU
#error Undefined mcu
#endif

#endif
