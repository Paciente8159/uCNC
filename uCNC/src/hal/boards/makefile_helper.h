/*
	Name: makefile_helper.h
	Description: helper for makefiles.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-02-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MAKEFILE_HELPER_H
#define MAKEFILE_HELPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#define BOARD_UNO 1
#define BOARD_RAMBO14 2
#define BOARD_RAMPS14 3
#define BOARD_MKS_DLC 4
#define BOARD_X_CONTROLLER 5
#define BOARD_MKS_GEN_L_V1 6
#define BOARD_UNO_SHIELD_V3 7
#define BOARD_BLUEPILL 10
#define BOARD_BLACKPILL 11
#define BOARD_MKS_ROBIN_NANO_V1_2 12
#define BOARD_SKR_PRO_V1_2 13
#define BOARD_NUCLEO_F411RE_SHIELD_V3 14
#define BOARD_BLUEPILL_F0 15
#define BOARD_MZERO 20
#define BOARD_ZERO 21
#define BOARD_RE_ARM 30
#define BOARD_MKS_BASE13 31
#define BOARD_SKR_V14_TURBO 32
#define BOARD_WEMOS_D1 40
#define BOARD_WEMOS_D1_R32 50
#define BOARD_MKS_TINYBEE 51
#define BOARD_MKS_DLC32 52
#define BOARD_ESP32_SHIELD_V3 53
#define BOARD_RPI_PICO 60
#define BOARD_RPI_PICO_W 61
#define BOARD_VIRTUAL 99

// special purpose board
#define BOARD_RAMPS14_MIRROR 200
#define BOARD_CUSTOM -1

#ifdef BOARD_TYPE
#define AVR_BOARDMAPS "src/boards/avr/boardmap_"
#if (BOARD_TYPE == BOARD_UNO)
#define BOARD AVR_BOARDMAPS "uno.h"
#endif
#if (BOARD_TYPE == BOARD_RAMBO14)
#define BOARD AVR_BOARDMAPS "rambo14.h"
#endif
#if (BOARD_TYPE == BOARD_RAMPS14)
#define BOARD AVR_BOARDMAPS "ramps14.h"
#endif
#if (BOARD_TYPE == BOARD_MKS_DLC)
#define BOARD AVR_BOARDMAPS "mks_dlc.h"
#endif
#if (BOARD_TYPE == BOARD_X_CONTROLLER)
#define BOARD AVR_BOARDMAPS "x_controller.h"
#endif
#if (BOARD_TYPE == BOARD_MKS_GEN_L_V1)
#define BOARD AVR_BOARDMAPS "mks_gen_l_v1.h"
#endif
#if (BOARD_TYPE == BOARD_UNO_SHIELD_V3)
#define BOARD AVR_BOARDMAPS "uno_shield_v3.h"
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif