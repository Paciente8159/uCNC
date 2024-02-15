/*
	Name: boardmap_bluepill_f0.h
	Description: Contains all MCU and PIN definitions for Bluepill F0 variant to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 15-02-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_BLUEPILL_F0_H
#define BOARDMAP_BLUEPILL_F0_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "STM32 Bluepill F0"
#endif

#include "boardmap_bluepill.h"

#undef ONESHOT_TIMER
#define ONESHOT_TIMER 17

#ifdef __cplusplus
}
#endif

#endif
