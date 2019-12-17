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
#include "planner.h"
#include "trigger_control.h"
#include "cnc.h"
#include "grbl_interface.h"
#include "utils.h"

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
	if(g_cnc_state.is_homed)
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

uint8_t kinematic_home_axis(uint8_t axis, uint8_t axis_limit)
{
	float target[AXIS_COUNT];
	
	memset(&target, 0, sizeof(target));
	tc_home();
	
	//reset planner
	planner_reset_position();
	
	//unlock the cnc
	cnc_unlock();
	
	//if HOLD or ALARM are still active or the limit switch is not cleared fails to home
	if(g_cnc_state.exec_state >= EXEC_HOLD || (g_cnc_state.limits & axis_limit))
	{
		return EXEC_ALARM_HOMING_FAIL_LIMIT_ACTIVE;
	}
	
	target[axis] = g_settings.max_distance[axis] * 1.5f;
	//checks homing dir
	if(g_settings.homing_dir_invert_mask & axis)
	{
		target[axis] = -target[axis];
	}
	
	planner_add_line((float*)&target, g_settings.homing_fast_feed_rate);
	do{
		cnc_doevents();
	} while(g_cnc_state.exec_state & EXEC_RUN);
	
	//if limit was not triggered 
	if(!(g_cnc_state.limits & axis_limit))
	{
		return EXEC_ALARM_HOMING_FAIL_APPROACH;
	}
	
	cnc_unlock();
	//planner_reset_position();
	float next = -20;
	
	if(g_settings.homing_dir_invert_mask & axis)
	{
		next = -next;
	}
	
	target[axis] += next;
	
	planner_add_line((float*)&target, g_settings.homing_slow_feed_rate);

	do {
		cnc_doevents();
		if(!(g_cnc_state.limits & axis_limit))
		{
			g_cnc_state.rt_cmd = RT_CMD_FEED_HOLD;
		}
	} while(g_cnc_state.exec_state & EXEC_RUN);

	/*if((g_cnc_state.limits & axis_limit))
	{
		return EXEC_ALARM_HOMING_FAIL_APPROACH;
	}*/
	
	return 0;
}

void kinematics_home()
{
	uint8_t result = 0;
	result = kinematic_home_axis(AXIS_Z, LIMIT_Z_MASK);
	if(result != 0)
	{
		//disables homing and reenables alarm messages
		g_cnc_state.exec_state &= ~EXEC_HOMING;
		cnc_alarm(result);
		return;
	}
}

#endif
