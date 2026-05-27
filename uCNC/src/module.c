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
#include "modules/shift_register.h"
#include "modules/modbus.h"
#include "modules/softi2c.h"
#include "modules/softspi.h"
#include "modules/softuart.h"
#include "modules/system_languages.h"
#include "modules/system_menu.h"

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
#ifndef DISABLE_G7_G8
	LOAD_MODULE(g7_g8);
#endif
#ifdef LOAD_MODULES_OVERRIDE
	LOAD_MODULES_OVERRIDE();
#elif defined(ENABLE_LVDS_RENDERER)
	LOAD_MODULE(lvds_renderer);
#endif
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
#if defined(ENC0_USE_HARDWARE_COUNTER) && (ENC0_USE_HARDWARE_COUNTER) && (ENC0_HW_COUNTER_TYPE == ENCODER_HW_PCNT)
	LOAD_MODULE(esp32_pcnt_encoder);
#elif defined(ENC0_USE_HARDWARE_COUNTER) && (ENC0_USE_HARDWARE_COUNTER) && (ENC0_HW_COUNTER_TYPE == ENCODER_HW_PIO)
	LOAD_MODULE(rp2350_pio_encoder);
#endif
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



#ifdef MODULE_DEBUG_ENABLED
bool mod_event_default_handler(mod_delegate_event_t **event, mod_delegate_event_t **last, void **args)
{
	mod_delegate_event_t *ptr = *last;
	if (!ptr)
	{
		ptr = *event;
	}
	while (ptr != NULL)
	{
		*last = ptr->next;
		if (ptr->fptr != NULL && !CHECKFLAG(ptr->fplock, (g_module_lockguard | LISTENER_RUNNING_LOCK)))
		{
			SETFLAG(ptr->fplock, LISTENER_RUNNING_LOCK);
			if (ptr->fptr(*args))
			{
				CLEARFLAG(ptr->fplock, LISTENER_RUNNING_LOCK);
				*last = *event; /*handled. restart.*/
				return EVENT_HANDLED;
			}
			CLEARFLAG(ptr->fplock, LISTENER_RUNNING_LOCK);
		}
		ptr = ptr->next;
	}
	return EVENT_CONTINUE;
}
#endif