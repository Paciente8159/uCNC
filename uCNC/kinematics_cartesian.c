/*
	Name: kinematics_cartesian_xyz.c
	Description: Implements all kinematics math equations to translate the motion of a cartesian XYZ machine.
		Also implements the homing motion for this type of machine.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 26/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "config.h"

#if(MACHINE_KINEMATICS==MACHINE_CARTESIAN)
#include <stdio.h>
#include <math.h>
#include "mcu.h"
#include "settings.h"
#include "kinematics.h"
#include "io_control.h"
#include "motion_control.h"
#include "grbl_interface.h"

void kinematics_apply_inverse(float* axis, uint32_t* steps)
{
	#ifdef AXIS_X
    steps[AXIS_X] = (uint32_t)lroundf(g_settings.step_per_mm[AXIS_X] * axis[AXIS_X]);
    #endif
    #ifdef AXIS_Y
    steps[AXIS_Y] = (uint32_t)lroundf(g_settings.step_per_mm[AXIS_Y] * axis[AXIS_Y]);
    #endif
    #ifdef AXIS_Z
    steps[AXIS_Z] = (uint32_t)lroundf(g_settings.step_per_mm[AXIS_Z] * axis[AXIS_Z]);
    #endif
    #ifdef AXIS_A
    steps[AXIS_A] = (uint32_t)lroundf(g_settings.step_per_mm[AXIS_A] * axis[AXIS_A]);
    #endif
    #ifdef AXIS_B
    steps[AXIS_B] = (uint32_t)lroundf(g_settings.step_per_mm[AXIS_B] * axis[AXIS_B]);
    #endif
    #ifdef AXIS_C
    steps[AXIS_C] = (uint32_t)lroundf(g_settings.step_per_mm[AXIS_C] * axis[AXIS_C]);
    #endif
}

void kinematics_apply_forward(uint32_t* steps, float* axis)
{
	#ifdef AXIS_X
    axis[AXIS_X] = (float)(((int32_t)steps[AXIS_X]) / g_settings.step_per_mm[AXIS_X]);
    #endif
    #ifdef AXIS_Y
    axis[AXIS_Y] = (float)(((int32_t)steps[AXIS_Y]) / g_settings.step_per_mm[AXIS_Y]);
    #endif
    #ifdef AXIS_Z
    axis[AXIS_Z] = (float)(((int32_t)steps[AXIS_Z]) / g_settings.step_per_mm[AXIS_Z]);
    #endif
    #ifdef AXIS_A
    axis[AXIS_A] = (float)(((int32_t)steps[AXIS_A]) / g_settings.step_per_mm[AXIS_A]);
    #endif
    #ifdef AXIS_B
    axis[AXIS_B] = (float)(((int32_t)steps[AXIS_B]) / g_settings.step_per_mm[AXIS_B]);
    #endif
    #ifdef AXIS_C
    axis[AXIS_C] = (float)(((int32_t)steps[AXIS_C]) / g_settings.step_per_mm[AXIS_C]);
    #endif
}

uint8_t kinematics_home()
{
    uint8_t result = 0;
    
    #ifdef AXIS_Z
    result = mc_home_axis(AXIS_Z, LIMIT_Z_MASK);
    if(result != 0)
    {
        return result;
    }
	#endif
	#ifdef AXIS_X
    result = mc_home_axis(AXIS_X, LIMIT_X_MASK);
    if(result != 0)
    {
        return result;
    }
	#endif
	#ifdef AXIS_Y
    result = mc_home_axis(AXIS_Y, LIMIT_Y_MASK);
    if(result != 0)
    {
        return result;
    }
	#endif
	
	#ifdef AXIS_A
    result = mc_home_axis(AXIS_A, LIMIT_A_MASK);
    if(result != 0)
    {
        return result;
    }
	#endif
	#ifdef AXIS_B
    result = mc_home_axis(AXIS_B, LIMIT_B_MASK);
    if(result != 0)
    {
        return result;
    }
	#endif
	#ifdef AXIS_C
    result = mc_home_axis(AXIS_C, LIMIT_C_MASK);
    if(result != 0)
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
