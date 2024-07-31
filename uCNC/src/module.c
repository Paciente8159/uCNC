/*
	Name: module.c
	Description: Module extensions for µCNC.
	All entry points for extending µCNC core functionalities are declared in this module

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 21-02-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "cnc.h"
#include "modules/digimstep.h"
#include "modules/digipot.h"
#include "modules/encoder.h"
#include "modules/pid.h"
#include "modules/ic74hc595.h"
#include "modules/modbus.h"
#include "modules/softi2c.h"
#include "modules/softspi.h"
#include "modules/softuart.h"
#include "modules/system_languages.h"
#include "modules/system_menu.h"
#include "modules/file_system.h"

uint8_t g_module_lockguard;
/**
 *
 * this is the place to declare all parser extension registration calls
 * Example: LOAD_MODULE(m42);
 *
 **/
static FORCEINLINE void load_modules(void)
{
// PLACE YOUR MODULES HERE
#ifdef LOAD_MODULES_OVERRIDE
	LOAD_MODULES_OVERRIDE();
#endif
	LOAD_MODULE(graphic_display);
}

void mod_init(void)
{
	g_module_lockguard = 0;
#ifdef ENABLE_DIGITAL_MSTEP
	LOAD_MODULE(digimstep);
#endif

#ifdef STEPPER_CURR_DIGIPOT
	LOAD_MODULE(digipot);
#endif

#if ENCODERS > 0
	LOAD_MODULE(encoder);
#endif

#ifdef ENABLE_LASER_PPI
	LOAD_MODULE(laser_ppi);
#endif

#ifdef ENABLE_PLASMA_THC
	LOAD_MODULE(plasma_thc);
#endif

#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
	// file system commands
	LOAD_MODULE(file_system);
#endif

	load_modules();
}
