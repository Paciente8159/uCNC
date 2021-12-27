/*
	Name: spindle.c
	Description: Defines a spindle tool using PWM0-speed and DOUT0-dir for µCNC.
                 Defines a coolant output using DOUT1 and DOUT2.

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

#include "../../../cnc.h"
#include "../../../core/io_control.h"
#include "../tool_helper.h"

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

    void spindle1_set_speed(uint8_t value, bool invert)
    {
        //easy macro to execute the same code as below
        //SET_SPINDLE(SPINDLE_PWM, SPINDLE_DIR, value, invert);

        //speed optimized version (in AVR it's 24 instruction cycles)
        #if SPINDLE_DIR >= 0
        if(SPINDLE_DIR>0) {
                if (!invert)
                {
                    mcu_clear_output(SPINDLE_DIR);
                }
                else
                {
                    mcu_set_output(SPINDLE_DIR);
                }
        }
        #endif

        #if SPINDLE_PWM >= 0
        if(SPINDLE_PWM>0) {
                mcu_set_pwm(SPINDLE_PWM, value);
        }
        #endif
    }

    void spindle1_set_coolant(uint8_t value)
    {
        //easy macro
        SET_COOLANT(COOLANT_FLOOD, COOLANT_MIST, value);
    }

    uint8_t spindle1_get_speed(void) {
        return mcu_get_pwm(SPINDLE_PWM);
    }

    const tool_t __rom__ spindle1 = {
        .startup_code = NULL,
        .shutdown_code = NULL,
        .set_speed = &spindle1_set_speed,
        .set_coolant = &spindle1_set_coolant,
        .get_speed = &spindle1_get_speed,
        .pid_controller = NULL};

#ifdef __cplusplus
}
#endif
