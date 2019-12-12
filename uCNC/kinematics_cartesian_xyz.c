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

void kinematics_home()
{
	float axis[3];
	tc_set_limit_mask(LIMITS_MASK);
	
	//starts by homing Z axis
	axis[AXIS_X] = 0;
	axis[AXIS_Y] = 0;
	axis[AXIS_Z] = g_settings.max_distance[AXIS_Z] * 1.5f;
	//checks homing dir
	if(CHECKBIT(g_settings.homing_dir_invert_mask,AXIS_Z))
	{
		axis[AXIS_Z] = -axis[AXIS_Z];
	}
	
	planner_add_line((float*)&axis, g_settings.homing_fast_feed_rate);
	while(CHECKFLAG(g_cnc_state.exec_state, EXEC_RUN)
	{
		cnc_doevents();
	}
	
	
	planner_reset_position();
	planner_add_line((float*)&axis, g_settings.homing_slow_feed_rate);
}

#endif
