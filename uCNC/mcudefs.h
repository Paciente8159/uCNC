/*
	Name: mcudefs.h
	Description: Defines the available machine types.
	
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

#ifndef MCUSDEFS_H
#define MCUSDEFS_H

#include "mcus.h"

/*
	MCU port map
*/
#if(MCU == MCU_ATMEGA328P)
#include "mcus\avr\uno\mcumap_atmega328p.h"
#endif

#if(MCU == MCU_VIRTUAL)
#include "mcus\virtual\mcumap_virtual.h"
#endif

#ifndef MCU
#error Undefined mcu
#endif

#endif
