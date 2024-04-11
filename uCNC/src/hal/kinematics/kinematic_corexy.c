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

#include "../../cnc.h"

#if (KINEMATIC == KINEMATIC_COREXY)
#include <stdio.h>
#include <math.h>

void kinematics_init(void)
{
}

void kinematics_apply_inverse(float *axis, int32_t *steps)
{
	steps[0] = (int32_t)lroundf(g_settings.step_per_mm[0] * (axis[AXIS_X] + axis[AXIS_Y]));
	steps[1] = (int32_t)lroundf(g_settings.step_per_mm[1] * (axis[AXIS_X] - axis[AXIS_Y]));

#if AXIS_COUNT > 2
	for (uint8_t i = 2; i < AXIS_COUNT; i++)
	{
		steps[i] = (int32_t)lroundf(g_settings.step_per_mm[i] * axis[i]);
	}
#endif
}

void kinematics_apply_forward(int32_t *steps, float *axis)
{
	axis[AXIS_X] = (float)(0.5f * (float)(steps[0] + steps[1]) / g_settings.step_per_mm[0]);
	axis[AXIS_Y] = (float)(0.5f * (float)(steps[0] - steps[1]) / g_settings.step_per_mm[1]);

#if AXIS_COUNT > 2
	for (uint8_t i = 2; i < AXIS_COUNT; i++)
	{
		axis[i] = (((float)steps[i]) / g_settings.step_per_mm[i]);
	}
#endif
}

uint8_t kinematics_home(void)
{
	float target[AXIS_COUNT];

#ifndef DISABLE_ALL_LIMITS
#if AXIS_Z_HOMING_MASK != 0
	if (mc_home_axis(AXIS_Z_HOMING_MASK, LINACT2_LIMIT_MASK))
	{
		return KINEMATIC_HOMING_ERROR_Z;
	}
#endif

#ifndef ENABLE_XY_SIMULTANEOUS_HOMING

#if AXIS_X_HOMING_MASK != 0
	if (mc_home_axis(AXIS_X_HOMING_MASK, LINACT0_LIMIT_MASK))
	{
		return KINEMATIC_HOMING_ERROR_X;
	}
#endif

#if AXIS_Y_HOMING_MASK != 0
	if (mc_home_axis(AXIS_Y_HOMING_MASK, LINACT1_LIMIT_MASK))
	{
		return KINEMATIC_HOMING_ERROR_Y;
	}
#endif

#else

#if AXIS_X_HOMING_MASK != 0 && AXIS_Y_HOMING_MASK != 0
	if (mc_home_axis(AXIS_X_HOMING_MASK | AXIS_Y_HOMING_MASK, LINACT0_LIMIT_MASK | LINACT1_LIMIT_MASK))
	{
		return KINEMATIC_HOMING_ERROR_XY;
	}
#elif AXIS_X_HOMING_MASK != 0
	if (mc_home_axis(AXIS_X_HOMING_MASK, LINACT0_LIMIT_MASK))
	{
		return KINEMATIC_HOMING_ERROR_X;
	}
#elif AXIS_Y_HOMING_MASK != 0
	if (mc_home_axis(AXIS_Y_HOMING_MASK, LINACT1_LIMIT_MASK))
	{
		return KINEMATIC_HOMING_ERROR_Y;
	}
#endif

#endif

#if AXIS_A_HOMING_MASK != 0
	if (mc_home_axis(AXIS_A_HOMING_MASK, LINACT3_LIMIT_MASK))
	{
		return KINEMATIC_HOMING_ERROR_A;
	}
#endif

#if AXIS_B_HOMING_MASK != 0
	if (mc_home_axis(AXIS_B_HOMING_MASK, LINACT4_LIMIT_MASK))
	{
		return KINEMATIC_HOMING_ERROR_B;
	}
#endif

#if AXIS_C_HOMING_MASK != 0
	if (mc_home_axis(AXIS_C_HOMING_MASK, LINACT5_LIMIT_MASK))
	{
		return KINEMATIC_HOMING_ERROR_C;
	}
#endif

	cnc_unlock(true);
	motion_data_t block_data = {0};
	mc_get_position(target);

	for (uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
		target[i] += ((g_settings.homing_dir_invert_mask & (1 << i)) ? -g_settings.homing_offset : g_settings.homing_offset);
	}

	block_data.feed = g_settings.homing_fast_feed_rate;
	block_data.spindle = 0;
	block_data.dwell = 0;
	// starts offset and waits to finnish
	mc_line(target, &block_data);
	itp_sync();
#endif

#ifdef SET_ORIGIN_AT_HOME_POS
	memset(target, 0, sizeof(target));
#else
	for (uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
		target[i] = (!(g_settings.homing_dir_invert_mask & (1 << i)) ? 0 : g_settings.max_distance[i]);
	}
#endif

	// reset position
	itp_reset_rt_position(target);

	return STATUS_OK;
}

bool kinematics_check_boundaries(float *axis)
{
	if (!g_settings.soft_limits_enabled || cnc_get_exec_state(EXEC_HOMING))
	{
		return true;
	}

	for (uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
#ifdef SET_ORIGIN_AT_HOME_POS
		float value = !(g_settings.homing_dir_invert_mask & (1 << i)) ? axis[i] : -axis[i];
#else
		float value = axis[i];
#endif
		if (value > g_settings.max_distance[i] || value < 0)
		{
			return false;
		}
	}

	return true;
}

#endif
