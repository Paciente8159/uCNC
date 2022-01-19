/*
	Name: kinematic_delta.c
	Description: Implements all kinematics math equations to translate the motion of a delta machine.
		Also implements the homing motion for this type of machine.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 19/01/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"

#if (KINEMATIC == KINEMATIC_DELTA)
#include <stdio.h>
#include <math.h>

#define STEPPER0_FACTX (cos(STEPPER0_ANGLE * M_PI / 180.0f));
#define STEPPER0_FACTY (sin(STEPPER0_ANGLE * M_PI / 180.0f));
#define STEPPER1_FACTX (cos(STEPPER1_ANGLE * M_PI / 180.0f));
#define STEPPER1_FACTY (sin(STEPPER1_ANGLE * M_PI / 180.0f));
#define STEPPER2_FACTX (cos(STEPPER2_ANGLE * M_PI / 180.0f));
#define STEPPER2_FACTY (sin(STEPPER2_ANGLE * M_PI / 180.0f));

static float delta_arm_sqr;
static float delta_base_height;
static float delta_inv_x[3];
static float delta_inv_y[3];

void kinematics_init()
{
        float delta_triang_base = (g_settings.delta_armbase_radius - g_settings.delta_efector_radius);
        delta_arm_sqr = g_settings.delta_arm_length * g_settings.delta_arm_length;
        delta_inv_x[0] = delta_triang_base * STEPPER0_FACTX;
        delta_inv_x[1] = delta_triang_base * STEPPER1_FACTX;
        delta_inv_x[2] = delta_triang_base * STEPPER2_FACTX;
        delta_inv_y[0] = delta_triang_base * STEPPER0_FACTY;
        delta_inv_y[1] = delta_triang_base * STEPPER1_FACTY;
        delta_inv_y[2] = delta_triang_base * STEPPER2_FACTY;
        delta_base_height = sqrtf(delta_arm_sqr - delta_inv_x[0] * delta_inv_x[0] - delta_inv_y[0] * delta_inv_y[0]) + g_settings.delta_efector_height;
}

void kinematics_apply_inverse(float *axis, int32_t *steps)
{
        float x = delta_inv_x[0] - axis[AXIS_X];
        float y = delta_inv_y[0] - axis[AXIS_Y];
        float z = delta_base_height + axis[AXIS_Z];
        float steps_mm = sqrt(delta_arm_sqr - (x * x) - (y * y)) - z;
        steps[0] = (int32_t)lroundf(g_settings.step_per_mm[0] * steps_mm);
        x = delta_inv_x[1] - axis[AXIS_X];
        y = delta_inv_y[1] - axis[AXIS_Y];
        steps_mm = sqrt(delta_arm_sqr - (x * x) - (y * y)) - z;
        steps[1] = (int32_t)lroundf(g_settings.step_per_mm[1] * steps_mm);
        x = delta_inv_x[2] - axis[AXIS_X];
        y = delta_inv_y[2] - axis[AXIS_Y];
        steps_mm = sqrt(delta_arm_sqr - (x * x) - (y * y)) - z;
        steps[2] = (int32_t)lroundf(g_settings.step_per_mm[2] * steps_mm);
#ifdef AXIS_A
        steps[3] = (int32_t)lroundf(g_settings.step_per_mm[3] * axis[AXIS_A]);
#endif
#ifdef AXIS_B
        steps[4] = (int32_t)lroundf(g_settings.step_per_mm[4] * axis[AXIS_B]);
#endif
#ifdef AXIS_C
        steps[5] = (int32_t)lroundf(g_settings.step_per_mm[5] * axis[AXIS_C]);
#endif
}

void kinematics_apply_forward(int32_t *steps, float *axis)
{
        axis[AXIS_X] = (((float)steps[0]) / g_settings.step_per_mm[0]);

        axis[AXIS_Y] = (((float)steps[1]) / g_settings.step_per_mm[1]);

        axis[AXIS_Z] = (((float)steps[2]) / g_settings.step_per_mm[2]);

#ifdef AXIS_A
        axis[AXIS_A] = (((float)steps[3]) / g_settings.step_per_mm[3]);
#endif
#ifdef AXIS_B
        axis[AXIS_B] = (((float)steps[4]) / g_settings.step_per_mm[4]);
#endif
#ifdef AXIS_C
        axis[AXIS_C] = (((float)steps[5]) / g_settings.step_per_mm[5]);
#endif
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

void kinematics_apply_transform(float *axis)
{
}

void kinematics_apply_reverse_transform(float *axis)
{
}

#endif
