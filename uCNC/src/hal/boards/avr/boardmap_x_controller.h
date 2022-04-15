/*
	Name: boardmap_uno.h
	Description: Contains all MCU and PIN definitions for Inventables X-Controller to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 04/02/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_X_CONTROLLER_H
#define BOARDMAP_X_CONTROLLER_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "X CONTROLLER"
#endif

#include "boardmap_uno.h"

// free FHOLD and CS RES pins
#ifdef FHOLD_PORT
#undef FHOLD_PORT
#endif
#ifdef FHOLD_BIT
#undef FHOLD_BIT
#endif
#ifdef CS_RES_PORT
#undef CS_RES_PORT
#endif
#ifdef CS_RES_BIT
#undef CS_RES_BIT
#endif

#define INVERT_EMERGENCY_STOP

#ifdef __cplusplus
}
#endif

#endif
