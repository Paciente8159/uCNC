/*
    Name: lase1.c
    Description: Defines a laser tool using PWM0 for µCNC.
                 Defines a coolant output using DOUT1 (can be used for air assist).

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 17/12/2021

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#include <stdbool.h>
#include <math.h>

/**
 * This configures a simple spindle control with a pwm assigned to PWM0 and dir invert assigned to DOUT0
 * This spindle also has a coolant pin assigned to DOUT1
 *
 * */

// give names to the pins (easier to identify)
#define LASER_PWM PWM0
#define COOLANT_FLOOD DOUT1

static bool previous_laser_mode;

void laser_startup_code(void)
{
    // force laser mode
    previous_laser_mode = g_settings.laser_mode;
    g_settings.laser_mode = 1;
}

void laser_shutdown_code(void)
{
    // restore laser mode
    g_settings.laser_mode = previous_laser_mode;
}

void laser_set_speed(uint8_t value, bool invert)
{
// easy macro to execute the same code as below
// SET_LASER(LASER_PWM, value, invert);

// speed optimized version (in AVR it's 24 instruction cycles)
#if !(LASER_PWM < 0)
    mcu_set_pwm(LASER_PWM, value);
#endif
}

void laser_set_coolant(uint8_t value)
{
    // easy macro
    SET_COOLANT(COOLANT_FLOOD, -1, value);
}

uint16_t laser_get_speed(void)
{
#if !(LASER_PWM < 0)
    float laser = (float)mcu_get_pwm(LASER_PWM) * g_settings.spindle_max_rpm * UINT8_MAX_INV;
    return (uint16_t)roundf(laser);
#else
    return 0;
#endif
}

const tool_t __rom__ laser = {
    .startup_code = &laser_startup_code,
    .shutdown_code = &laser_shutdown_code,
    .set_speed = &laser_set_speed,
    .set_coolant = NULL,
#if PID_CONTROLLERS > 0
    .pid_update = NULL,
    .pid_error = NULL,
#endif
    .get_speed = &laser_get_speed};
