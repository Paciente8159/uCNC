/*
	Name: kinematics_corexy.c
	Description: Implements all kinematics math equations to translate the motion of a coreXY machine.
		Also implements the homing motion for this type of machine.
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 11/11/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
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

float step_mm_inv[3];

void kinematics_init()
{
	step_mm_inv[0] = 1.0f / (float)g_settings.step_mm[0];
	step_mm_inv[1] = 1.0f / (float)g_settings.step_mm[1];
	step_mm_inv[2] = 1.0f / (float)g_settings.step_mm[2];
}

void kinematics_apply_inverse(float* axis, uint32_t* steps)
{
	steps[0] = g_settings.step_mm[0] * (axis[AXIS_X] + axis[AXIS_Y]);
	steps[1] = g_settings.step_mm[1] * (axis[AXIS_X] - axis[AXIS_Y]);
	steps[2] = g_settings.step_mm[2] * axis[AXIS_Z];
}

void kinematics_apply_forward(uint32_t* steps, float* axis)
{
	axis[AXIS_X] = step_mm_inv[0] * 0.5f * (steps[0] + steps[1]);
	axis[AXIS_Y] = step_mm_inv[1] * 0.5f * (steps[0] - steps[1]);
	axis[AXIS_Z] = step_mm_inv[2] * steps[2];
}

void kinematics_home()
{
	uint8_t result = 0;
	result = mc_home_axis(AXIS_Z, LIMIT_Z_MASK);
	if(result != 0)
	{
		//disables homing and reenables alarm messages
		cnc_clear_exec_state(EXEC_HOMING);
		cnc_alarm(result);
		return;
	}
	
	result = mc_home_axis(AXIS_X, LIMIT_X_MASK);
	if(result != 0)
	{
		//disables homing and reenables alarm messages
		cnc_clear_exec_state(EXEC_HOMING);
		cnc_alarm(result);
		return;
	}
	
	result = mc_home_axis(AXIS_Y, LIMIT_Y_MASK);
	if(result != 0)
	{
		//disables homing and reenables alarm messages
		cnc_clear_exec_state(EXEC_HOMING);
		cnc_alarm(result);
		return;
	}
	
	//move to offset position
	cnc_offset_home();
	
	//if all ok resets coordinates and flags homed state
	cnc_reset_position();
}

#endif
