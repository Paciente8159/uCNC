/*
	Name: tool0.c
	Description: Defines Tool0 for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 16/12/2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "cnc.h"
#include "interface/settings.h"
#include "core/io_control.h"

    /**
 * This configures a simple spindle control with a pwm assigned to PWM0 and dir invert assigned to DOUT0
 * This spindle also has a coolant pin assigned to DOUT1
 * 
 * */

//give names to the pins (easier to identify)
#define SPINDLE_PWM PWM0
#define SPINDLE_DIR DOUT0
#define COOLANT_FLOOD DOUT1
#define COOLANT_MIST DOUT2

    void tool0_set_spindle(uint8_t value, bool invert)
    {
#if SPINDLE_DIR >= 0
        if (!invert)
        {
            mcu_clear_output(SPINDLE_DIR);
        }
        else
        {
            mcu_set_output(SPINDLE_DIR);
        }
#endif

#if SPINDLE_PWM >= 0
        mcu_set_pwm(SPINDLE_PWM, value);
#endif
    }

    void tool0_set_coolant(uint8_t value)
    {
#if COOLANT_FLOOD >= 0
        if (value & COOLANT_MASK)
        {
            mcu_set_output(COOLANT_FLOOD);
        }
        else
        {
            mcu_clear_output(COOLANT_FLOOD);
        }
#endif
#if COOLANT_MIST >= 0
#if (COOLANT_FLOOD != COOLANT_MIST)
        if (value & MIST_MASK)
        {
            mcu_set_output(COOLANT_MIST);
        }
        else
        {
            mcu_clear_output(COOLANT_MIST);
        }
#endif
#endif
    }

    const tool_t __rom__ tool0 = {
        .startup_code = NULL,
        .shutdown_code = NULL,
        .set_spindle = &tool0_set_spindle,
        .set_coolant = &tool0_set_coolant,
        .get_spindle = NULL,
        .pid_controller = NULL};

#ifdef __cplusplus
}
#endif
