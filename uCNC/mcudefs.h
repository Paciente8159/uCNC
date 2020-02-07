/*
	Name: mcudefs.h
	Description: Defines the available machine types.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUSDEFS_H
#define MCUSDEFS_H

#include "mcus.h"

/*
	MCU port map
*/
#if(MCU == MCU_GRBL)
#include "mcus\avr\mcumap_grbl.h"
#include "mcus\avr\mcumap_avr.h"
#endif

#if(MCU == MCU_RAMBO14)
#include "mcus\avr\mcumap_rambo14.h"
#include "mcus\avr\mcumap_avr.h"
#endif

#if(MCU == MCU_RAMPS14)
#include "mcus\avr\mcumap_ramps14.h"
#include "mcus\avr\mcumap_avr.h"
#endif

#if(MCU == MCU_VIRTUAL)
#include "mcus\virtual\mcumap_virtual.h"
#endif

#ifndef MCU
#error Undefined mcu
#endif

#endif
