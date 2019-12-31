/*
	Name: kinematics_cartesian_xyz.c
	Copyright: 
	Author: Jo�o Martins
	Date: 24/09/19 11:27
	Description: Implements the kinematics interface module for a XYZ cartesian machine
*/

#include "config.h"

#if(MACHINE_KINEMATICS==MACHINE_CARTESIAN_XYZ)
#include <stdio.h>
#include <math.h>
#include "mcu.h"
#include "settings.h"
#include "kinematics.h"
#include "machinedefs.h"
#include "motion_control.h"
#include "planner.h"
#include "trigger_control.h"
#include "cnc.h"

float step_mm_inv[3];

void kinematics_init()
{
	step_mm_inv[0] = 1.0f / g_settings.step_per_mm[0];
	step_mm_inv[1] = 1.0f / g_settings.step_per_mm[1];
	step_mm_inv[2] = 1.0f / g_settings.step_per_mm[2];
}

void kinematics_apply_inverse(float* axis, uint32_t* steps)
{
	steps[0] = (uint32_t)lroundf(g_settings.step_per_mm[0] * axis[AXIS_X]);
	steps[1] = (uint32_t)lroundf(g_settings.step_per_mm[1] * axis[AXIS_Y]);
	steps[2] = (uint32_t)lroundf(g_settings.step_per_mm[2] * axis[AXIS_Z]);
}

void kinematics_apply_forward(uint32_t* steps, float* axis)
{
	if(cnc_is_homed())
	{
		axis[AXIS_X] = step_mm_inv[0] * steps[0];
		axis[AXIS_Y] = step_mm_inv[1] * steps[1];
		axis[AXIS_Z] = step_mm_inv[2] * steps[2];
	}
	else
	{
		axis[AXIS_X] = step_mm_inv[0] * (int32_t)steps[0];
		axis[AXIS_Y] = step_mm_inv[1] * (int32_t)steps[1];
		axis[AXIS_Z] = step_mm_inv[2] * (int32_t)steps[2];
	}
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
