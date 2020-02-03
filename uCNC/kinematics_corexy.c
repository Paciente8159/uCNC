/*
	Name: kinematics_corexy.c
	Description: Implements all kinematics math equations to translate the motion of a coreXY machine.
		Also implements the homing motion for this type of machine.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 11/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/
#include "config.h"

#if(MACHINE_KINEMATICS==MACHINE_COREXY)
#include <stdio.h>
#include "mcu.h"
#include "settings.h"
#include "kinematics.h"
#include "machinedefs.h"

void kinematics_apply_inverse(float* axis, uint32_t* steps)
{
    steps[0] = (uint32_t)lroundf(g_settings.step_mm[0] * (axis[AXIS_X] + axis[AXIS_Y]));
    steps[1] = (uint32_t)lroundf(g_settings.step_mm[1] * (axis[AXIS_X] - axis[AXIS_Y]));
    steps[2] = (uint32_t)lroundf(g_settings.step_mm[2] * axis[AXIS_Z]);
}

void kinematics_apply_forward(uint32_t* steps, float* axis)
{
    axis[AXIS_X] = (float)(step_mm_inv[0] * 0.5f * (steps[0] + steps[1]));
    axis[AXIS_Y] = (float)(step_mm_inv[1] * 0.5f * (steps[0] - steps[1]));
    axis[AXIS_Z] = (float)(step_mm_inv[2] * steps[2]);
}

void kinematics_home()
{
    uint8_t result = 0;
    result = mc_home_axis(AXIS_Z, LIMIT_Z_MASK);
    if(result != 0)
    {
        return result;
    }

    result = mc_home_axis(AXIS_X, LIMIT_X_MASK);
    if(result != 0)
    {
        return result;
    }

    result = mc_home_axis(AXIS_Y, LIMIT_Y_MASK);
    if(result != 0)
    {
        return result;
    }

    return STATUS_OK;
}

void kinematics_apply_transform(float* axis)
{
    /*
	Define your custom transform
    */
}

void kinematics_apply_reverse_transform(float* axis)
{
    /*
	Define your custom transform inverse operation
    */
}

#endif
