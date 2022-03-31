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
#include "modules/softspi.h"
#include "modules/softuart.h"

// this is the place to declare all parser extension registration calls
#ifdef ENABLE_PARSER_MODULES
// overridable handler
uint8_t __attribute__((weak)) mod_gcode_parse_hook(unsigned char word, uint8_t code, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
	EVENT_TYPE(gcode_parse_delegate) *ptr = gcode_parse_event;
	while ((error == STATUS_GCODE_UNSUPPORTED_COMMAND || error == STATUS_GCODE_UNUSED_WORDS) && (ptr != NULL))
	{
		if (ptr->fptr != NULL)
		{
			error = ptr->fptr(word, code, error, value, new_state, words, cmd);
		}

		ptr = ptr->next;
	}

	return error;
}

// overridable handler
uint8_t __attribute__((weak)) mod_gcode_exec_hook(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
	EVENT_TYPE(gcode_exec_delegate) *ptr = gcode_exec_event;
	uint8_t error = STATUS_GOCDE_EXTENDED_UNSUPPORTED;
	while ((cmd->group_extended != 0) && (ptr != NULL))
	{
		if (ptr->fptr != NULL)
		{
			error = ptr->fptr(new_state, words, cmd);
		}

		// checks if function catched the extended code
		if (error != STATUS_GOCDE_EXTENDED_UNSUPPORTED)
		{
			break;
		}

		ptr = ptr->next;
	}

	return error;
}
#endif

#ifdef ENABLE_INTERPOLATOR_MODULES

// declares the handler hook to be called inside the parser core
void __attribute__((weak)) mod_itp_reset_rt_position_hook(float *origin)
{
	EVENT_TYPE(itp_reset_rt_position_delegate) *ptr = itp_reset_rt_position_event;
	while (ptr != NULL)
	{
		if (ptr->fptr != NULL)
		{
			ptr->fptr(origin);
		}
		ptr = ptr->next;
	}
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
	DEFAULT_HANDLER(input_change);
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

static void softspi_ss_init()
{
	// initialize slave select pins
#ifdef STEPPER_CURR_DIGIPOT
	mcu_set_output(STEPPER_DIGIPOT_SS);
#endif

#ifdef STEPPER0_SPI_CS
	mcu_set_output(STEPPER0_SPI_CS);
#endif
#ifdef STEPPER1_SPI_CS
	mcu_set_output(STEPPER1_SPI_CS);
#endif
#ifdef STEPPER2_SPI_CS
	mcu_set_output(STEPPER2_SPI_CS);
#endif
#ifdef STEPPER3_SPI_CS
	mcu_set_output(STEPPER3_SPI_CS);
#endif
#ifdef STEPPER4_SPI_CS
	mcu_set_output(STEPPER4_SPI_CS);
#endif
#ifdef STEPPER5_SPI_CS
	mcu_set_output(STEPPER5_SPI_CS);
#endif
#ifdef STEPPER6_SPI_CS
	mcu_set_output(STEPPER6_SPI_CS);
#endif
#ifdef STEPPER7_SPI_CS
	mcu_set_output(STEPPER7_SPI_CS);
#endif
}

void mod_init(void)
{
	softspi_ss_init();

// MStep selectors via digital pins
#ifdef ENABLE_DIGITAL_MSTEP
	extern void digimstep_init(void);
	digimstep_init();
#ifdef ENABLE_PARSER_MODULES
	ADD_LISTENER(gcode_parse_delegate, m351_parse, gcode_parse_event);
	ADD_LISTENER(gcode_exec_delegate, m351_exec, gcode_exec_event);
#endif
#endif

#ifdef STEPPER_CURR_DIGIPOT
	extern void digipot_spi_init(void);
	digipot_spi_init();
#ifdef ENABLE_PARSER_MODULES
	ADD_LISTENER(gcode_parse_delegate, m907_parse, gcode_parse_event);
	ADD_LISTENER(gcode_exec_delegate, m907_exec, gcode_exec_event);
#endif
#endif

	// initializes PID module
#if PID_CONTROLLERS > 0
	extern void pid_init(void);
	pid_init(); // pid
#endif

// add listeners here
#ifdef ENABLE_PARSER_MODULES
	ADD_LISTENER(gcode_parse_delegate, m42_parse, gcode_parse_event);
	ADD_LISTENER(gcode_exec_delegate, m42_exec, gcode_exec_event);
#endif

#if ENCODERS > 0
	ADD_LISTENER(cnc_reset_delegate, encoders_reset_position, cnc_reset_event);
#endif

#ifdef ENABLE_TMC_DRIVERS
	extern void tmcdrivers_init(void);
	tmcdrivers_init();
	ADD_LISTENER(cnc_reset_delegate, tmcdrivers_init, cnc_reset_event);
#ifdef ENABLE_PARSER_MODULES
	ADD_LISTENER(gcode_parse_delegate, m350_parse, gcode_parse_event);
	ADD_LISTENER(gcode_exec_delegate, m350_exec, gcode_exec_event);
	ADD_LISTENER(gcode_parse_delegate, m906_parse, gcode_parse_event);
	ADD_LISTENER(gcode_exec_delegate, m906_exec, gcode_exec_event);
	ADD_LISTENER(gcode_parse_delegate, m920_parse, gcode_parse_event);
	ADD_LISTENER(gcode_exec_delegate, m920_exec, gcode_exec_event);
#endif
#endif
}
