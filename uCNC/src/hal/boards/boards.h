/*
	Name: boards.h
	Description: Defines the available board types.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/02/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDS_H
#define BOARDS_H

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
#define BOARD_BLUEPILL 10
#define BOARD_BLACKPILL 11
#define BOARD_MZERO 20
#define BOARD_ZERO 21
#define BOARD_RE_ARM 30
#define BOARD_MKS_BASE13 31
#define BOARD_WEMOS_D1 40
#define BOARD_WEMOS_D1_R32 50
#define BOARD_VIRTUAL 99

// special purpose board
#define BOARD_RAMPS14_MIRROR 200

#ifdef __cplusplus
}
#endif

#endif
