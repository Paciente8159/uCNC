/*
	Name: boarddefs.h
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

#ifndef BOARDDEFS_H
#define BOARDDEFS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define __BOARDMAP_FILE__(B) #B
#define BOARDMAP_FILE(B) __BOARDMAP_FILE__(B)

#include "../mcus/mcus.h"

#ifndef BOARDMAP
#include "boards_helper.h"

#ifndef BOARDMAP
#error "Undefined boardmap. You need to define your boardmap file"
#endif
#endif

#include BOARDMAP

// reinclude the override file
#include "../../../boardmap_overrides.h"
#include "pin_mapping_helper.h"
#include "../mcus/mcudefs.h" //configures the MCU for the selected board

#ifdef __cplusplus
}
#endif

#endif
