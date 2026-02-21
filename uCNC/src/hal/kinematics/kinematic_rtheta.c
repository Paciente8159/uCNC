/*
		Name: KINEMATIC_RTHETA.c
		Description: Implements all kinematics math equations to translate the motion of a r-theta machine.
				Also implements the homing motion for this type of machine.

		Copyright: Copyright (c) Devansh Garg
		Author: Devansh Garg
		Date: 04/03/2025

		µCNC is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version. Please see <http://www.gnu.org/licenses/>

		µCNC is distributed WITHOUT ANY WARRANTY;
		Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
		See the	GNU General Public License for more details.
*/

#include "../../cnc.h"

#if (KINEMATIC == KINEMATICS_RTHETA)

#include <stdint.h>
#include <math.h>

#define FULL_TURN_INV 0.0027777777777777778f
#define DOUBLE_PI_INV (0.5f * M_PI_INV)

static float arm_angle_fact;
static float arm_length_fact;
static float theta_reduction_ratio;
static float theta_reduction_ratio_inv;
static float arm;

void kinematics_init(void)
{
	// reset home offset
	arm_angle_fact = 2.0f * M_PI / g_settings.step_per_mm[0];
	arm_length_fact = 1 / g_settings.step_per_mm[1];
	theta_reduction_ratio = g_settings.rtheta_theta_reduction_ratio;
	theta_reduction_ratio_inv = 1 / g_settings.rtheta_theta_reduction_ratio;
	arm = g_settings.rtheta_arm_length;
	mc_sync_position();
}

void kinematics_apply_inverse(float *axis, int32_t *steps)
{
	float angle = atan2f(axis[AXIS_Y], axis[AXIS_X]);
	float distance = sqrt(axis[AXIS_X] * axis[AXIS_X] + axis[AXIS_Y] * axis[AXIS_Y]);

	steps[0] = (int32_t)roundf(theta_reduction_ratio * angle * DOUBLE_PI_INV * g_settings.step_per_mm[0]);
	steps[1] = (int32_t)roundf(distance * g_settings.step_per_mm[1]);

#if AXIS_COUNT > 2
	for (uint8_t i = 2; i < AXIS_COUNT; i++)
	{
		steps[i] = (int32_t)lroundf(g_settings.step_per_mm[i] * axis[i]);
	}
#endif
}

void kinematics_apply_forward(int32_t *steps, float *axis)
{
	// calcs X and Y based on theta angle and arm length

	float angle = steps[AXIS_X] * arm_angle_fact * theta_reduction_ratio_inv;
	float distance = steps[AXIS_Y] * arm_length_fact;
	axis[AXIS_X] = distance * cos(angle);
	axis[AXIS_Y] = distance * sin(angle);

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
	uint8_t error = STATUS_OK;

#ifndef DISABLE_ALL_LIMITS
#if AXIS_Z_HOMING_MASK != 0
	error = mc_home_axis(AXIS_Z_HOMING_MASK, LINACT2_LIMIT_MASK);
	if (error != STATUS_OK)
	{
		return error;
	}
#endif

#if AXIS_X_HOMING_MASK != 0
	error = mc_home_axis(AXIS_X_HOMING_MASK, LINACT0_LIMIT_MASK);
	if (error != STATUS_OK)
	{
		return error;
	}
#endif

#if AXIS_Y_HOMING_MASK != 0
	error = mc_home_axis(AXIS_Y_HOMING_MASK, LINACT1_LIMIT_MASK);
	if (error != STATUS_OK)
	{
		return error;
	}
#endif

#if AXIS_A_HOMING_MASK != 0
	error = mc_home_axis(AXIS_A_HOMING_MASK, LINACT3_LIMIT_MASK);
	if (error != STATUS_OK)
	{
		return error;
	}
#endif

#if AXIS_B_HOMING_MASK != 0
	error = mc_home_axis(AXIS_B_HOMING_MASK, LINACT4_LIMIT_MASK);
	if (error != STATUS_OK)
	{
		return error;
	}
#endif

#if AXIS_C_HOMING_MASK != 0
	error = mc_home_axis(AXIS_C_HOMING_MASK, LINACT5_LIMIT_MASK);
	if (error != STATUS_OK)
	{
		return error;
	}
#endif

	cnc_unlock(true);

	int32_t steps_homing[STEPPER_COUNT] = {0};
	steps_homing[0] = g_settings.rtheta_theta_homing_angle * g_settings.step_per_mm[0] * FULL_TURN_INV;
	steps_homing[1] = g_settings.rthera_arm_homing_distance * g_settings.step_per_mm[1] * FULL_TURN_INV;
	kinematics_apply_forward(steps_homing, target);
	itp_reset_rt_position(target);
	mc_sync_position();

	motion_data_t block_data = {0};
	mc_get_position(target);

	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		target[i] += ((g_settings.homing_dir_invert_mask & (1 << i)) ? -g_settings.homing_offset : g_settings.homing_offset);
	}

	block_data.feed = g_settings.homing_fast_feed_rate;
	block_data.spindle = 0;
	block_data.dwell = 0;
	// starts offset and waits to finnish
	error = mc_line(target, &block_data);
	itp_sync();
#endif

	return error;
}

void kinematics_apply_transform(float *axis)
{
}

void kinematics_apply_reverse_transform(float *axis)
{
}

bool kinematics_check_boundaries(float *axis)
{
	if (!g_settings.soft_limits_enabled || cnc_get_exec_state(EXEC_HOMING))
	{
		return true;
	}

	float distance_to_center_sqr = axis[AXIS_X] * axis[AXIS_X] + axis[AXIS_Y] * axis[AXIS_Y];

	if (distance_to_center_sqr > arm * arm)
	{
		return false;
	}

	for (uint8_t i = AXIS_COUNT; i != 2;)
	{
		i--;
		if (g_settings.max_distance[i]) // ignore any undefined axis
		{
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
	}

	return true;
}

#endif
