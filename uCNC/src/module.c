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
#include "modules/tmcdriver.h"
#include "modules/digimstep.h"
#include "modules/digipot.h"
#include "modules/encoder.h"
#include "modules/pid.h"

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
}

void mod_init(void)
{
#ifdef ENABLE_DIGITAL_MSTEP
	LOAD_MODULE(digimstep);
#endif

#ifdef STEPPER_CURR_DIGIPOT
	LOAD_MODULE(digipot);
#endif

#if PID_CONTROLLERS > 0
	LOAD_MODULE(pid);
#endif

#if ENCODERS > 0
	LOAD_MODULE(encoder);
#endif

#ifdef ENABLE_TMC_DRIVERS
	LOAD_MODULE(tmcdriver);
#endif

	load_modules();
}
