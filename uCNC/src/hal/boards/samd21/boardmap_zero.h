/*
	Name: boardmap_zero.h
	Description: Contains all PIN definitions for Arduino Zero to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 13-12-2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_ZERO_H
#define BOARDMAP_ZERO_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "Arduino Zero"
#endif

#include "boardmap_mzero.h"

// Swap pins
#undef STEP0_BIT
#undef STEP2_BIT
#define STEP0_BIT 14 // assigns STEP0 pin
#define STEP2_BIT 8	 // assigns STEP2 pin


#ifdef __cplusplus
}
#endif

#endif
