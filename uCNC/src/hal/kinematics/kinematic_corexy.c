/*
	Name: kinematic_corexy.c
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
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../cnc.h"

#if (KINEMATIC == KINEMATIC_COREXY)
#include "../../interface/settings.h"
#include "../../core/motion_control.h"
#include <stdio.h>
#include <math.h>

    void kinematics_apply_inverse(float *axis, int32_t *steps)
    {
        steps[0] = (int32_t)lroundf(g_settings.step_mm[0] * (axis[AXIS_X] + axis[AXIS_Y]));
        steps[1] = (int32_t)lroundf(g_settings.step_mm[1] * (axis[AXIS_X] - axis[AXIS_Y]));
        steps[2] = (int32_t)lroundf(g_settings.step_mm[2] * axis[AXIS_Z]);
    }

    void kinematics_apply_forward(int32_t *steps, float *axis)
    {
        axis[AXIS_X] = (float)(step_mm_inv[0] * 0.5f * (float)(steps[0] + steps[1]));
        axis[AXIS_Y] = (float)(step_mm_inv[1] * 0.5f * (float)(steps[0] - steps[1]));
        axis[AXIS_Z] = (float)(step_mm_inv[2] * steps[2]);
    }

    uint8_t kinematics_home(void)
    {
        uint8_t result = 0;

#ifdef AXIS_Z
        result = mc_home_axis(AXIS_Z, LIMIT_Z_MASK);
        if (result != 0)
        {
            return result;
        }
#endif
#ifdef AXIS_X
        result = mc_home_axis(AXIS_X, LIMIT_X_MASK);
        if (result != 0)
        {
            return result;
        }
#endif
#ifdef AXIS_Y
        result = mc_home_axis(AXIS_Y, LIMIT_Y_MASK);
        if (result != 0)
        {
            return result;
        }
#endif

#ifdef AXIS_A
        result = mc_home_axis(AXIS_A, LIMIT_A_MASK);
        if (result != 0)
        {
            return result;
        }
#endif
#ifdef AXIS_B
        result = mc_home_axis(AXIS_B, LIMIT_B_MASK);
        if (result != 0)
        {
            return result;
        }
#endif
#ifdef AXIS_C
        result = mc_home_axis(AXIS_C, LIMIT_C_MASK);
        if (result != 0)
        {
            return result;
        }
#endif

        return STATUS_OK;
    }

    void kinematics_lock_step(uint8_t limits_mask)
    {
        // do nothing
    }

    void kinematics_apply_transform(float *axis)
    {
        /*
	Define your custom transform
    */
    }

    void kinematics_apply_reverse_transform(float *axis)
    {
        /*
	Define your custom transform inverse operation
    */
    }

#endif

#ifdef __cplusplus
}
#endif
