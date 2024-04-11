/*
		Name: boardmap_rp_pico_w.h
		Description: Contains all MCU and PIN definitions for Raspberry Pi Pico W to run µCNC.

		Copyright: Copyright (c) João Martins
		Author: João Martins
		Date: 16/01/2023

		µCNC is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version. Please see <http://www.gnu.org/licenses/>

		µCNC is distributed WITHOUT ANY WARRANTY;
		Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
		See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_RPI_PICO_W_H
#define BOARDMAP_RPI_PICO_W_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "RPi Pico W"
#endif

#define BOARD_HAS_CUSTOM_SYSTEM_COMMANDS

#include "boardmap_rpi_pico.h"

// led pin is controlled by the external WiFi controller
#undef DIO31_BIT

#ifdef __cplusplus
}
#endif

#endif
