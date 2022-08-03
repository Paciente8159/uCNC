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

// this is the place to declare all parser extension registration calls
#ifdef ENABLE_PARSER_MODULES
// mod_gcode_parse_hook
WEAK_HOOK(gcode_parse)
{
	DEFAULT_HANDLER(gcode_parse);
}

// mod_gcode_exec_hook
WEAK_HOOK(gcode_exec)
{
	DEFAULT_HANDLER(gcode_exec);
}
#endif

#ifdef ENABLE_INTERPOLATOR_MODULES
// mod_itp_reset_rt_position_hook
WEAK_HOOK(itp_reset_rt_position)
{
	DEFAULT_HANDLER(itp_reset_rt_position);
}
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
// mod_cnc_reset_hook
WEAK_HOOK(cnc_reset)
{
	// for now only encoder module uses this hook and overrides it
	DEFAULT_HANDLER(cnc_reset);
}

// mod_rtc_tick_hook
WEAK_HOOK(rtc_tick)
{
	// for now only pid module uses this hook and overrides it
	DEFAULT_HANDLER(rtc_tick);
}

// mod_cnc_dotasks_hook
WEAK_HOOK(cnc_dotasks)
{
	// for now this is not used
	DEFAULT_HANDLER(cnc_dotasks);
}

// mod_cnc_stop_hook
WEAK_HOOK(cnc_stop)
{
	// for now only pid module uses this hook and overrides it
	DEFAULT_HANDLER(cnc_stop);
}

#endif

#ifdef ENABLE_SETTINGS_MODULES
// mod_settings_change_hook
WEAK_HOOK(settings_change)
{
	// for now only pid module uses this hook and overrides it
	DEFAULT_HANDLER(settings_change);
}
#endif

#ifdef ENABLE_PROTOCOL_MODULES
// mod_send_pins_states_hook
WEAK_HOOK(send_pins_states)
{
	// for now only encoder module uses this hook and overrides it
	// it actually overrides the mcu callback to be faster
	DEFAULT_HANDLER(send_pins_states);
}
#endif

#ifdef ENABLE_IO_MODULES
// mod_input_change_hook
WEAK_HOOK(input_change)
{
	// for now only encoder module uses this hook and overrides it
	// it actually overrides the mcu callback to be faster
	DEFAULT_HANDLER(input_change);
}
#endif

#ifdef ENABLE_MOTION_MODULES
// mod_probe_enable_hook
WEAK_HOOK(probe_enable)
{
	// for now this is not used
	DEFAULT_HANDLER(probe_enable);
}

// mod_probe_disable_hook
WEAK_HOOK(probe_disable)
{
	// for now this is not used
	DEFAULT_HANDLER(probe_disable);
}
#endif

static FORCEINLINE void load_modules(void);

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

void load_modules(void)
{
	// PLACE YOUR MODULES HERE
}
