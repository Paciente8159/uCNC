/*
		Name: KINEMATIC_LINEAR_SCARA.c
		Description: Implements all kinematics math equations to translate the motion of a scara machine.
				Also implements the homing motion for this type of machine.

		Copyright: Copyright (c) João Martins
		Author: João Martins
		Date: 28/072023

		µCNC is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version. Please see <http://www.gnu.org/licenses/>

		µCNC is distributed WITHOUT ANY WARRANTY;
		Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
		See the	GNU General Public License for more details.
*/

#include "../../cnc.h"

#if (KINEMATIC == KINEMATIC_SCARA)
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define FULL_TURN_INV 0.0027777777777777778f
#define DOUBLE_PI_INV (0.5f * M_PI_INV)


static float scara_arm_angle_fact[2];
static float scara_min_distance_to_center_sqr;
static float scara_max_distance_to_center_sqr;

void kinematics_init(void)
{
	// reset home offset
	scara_arm_angle_fact[0] = 2.0f * M_PI / g_settings.step_per_mm[0];
	scara_arm_angle_fact[1] = 2.0f * M_PI / g_settings.step_per_mm[1];
	scara_max_distance_to_center_sqr = g_settings.scara_arm_length + g_settings.scara_forearm_length;
	scara_max_distance_to_center_sqr *= scara_max_distance_to_center_sqr;
	scara_min_distance_to_center_sqr = g_settings.scara_arm_length - g_settings.scara_forearm_length;
	scara_min_distance_to_center_sqr *= scara_min_distance_to_center_sqr;
	mc_sync_position();
}

void kinematics_apply_inverse(float *axis, int32_t *steps)
{
	float arm = g_settings.scara_arm_length;
	float forearm = g_settings.scara_forearm_length;

	float distance = (axis[AXIS_X] * axis[AXIS_X] + axis[AXIS_Y] * axis[AXIS_Y] - arm * arm - forearm * forearm) / (2.0f * arm * forearm);
	float angle2 = acosf(distance);

	steps[1] = (int32_t)roundf(angle2 * DOUBLE_PI_INV * g_settings.step_per_mm[1]);
	float angle1 = atan2f(axis[AXIS_Y], axis[AXIS_X]) - atan2f(forearm * sin(angle2), (arm + forearm * distance));
	steps[0] = (int32_t)roundf(angle1 * DOUBLE_PI_INV * g_settings.step_per_mm[0]);

#if AXIS_COUNT > 2
	for (uint8_t i = 2; i < AXIS_COUNT; i++)
	{
		steps[i] = (int32_t)lroundf(g_settings.step_per_mm[i] * axis[i]);
	}
#endif
}

void kinematics_apply_forward(int32_t *steps, float *axis)
{
	// calcs X and Y based on arms lengths and angles

	// calculate joints positions
	float joint1 = steps[0] * scara_arm_angle_fact[0];
	float joint2 = steps[1] * scara_arm_angle_fact[1];

	joint2 += joint1;

	float arm = g_settings.scara_arm_length;
	float forearm = g_settings.scara_forearm_length;

	axis[AXIS_X] = arm * cos(joint1) + forearm * cos(joint2);
	axis[AXIS_Y] = arm * sin(joint1) + forearm * sin(joint2);

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

#if AXIS_A_HOMING_MASK != 0
	if (mc_home_axis(AXIS_A_HOMING_MASK, LINACT3_LIMIT_MASK))
	{
		return (KINEMATIC_HOMING_ERROR_X | KINEMATIC_HOMING_ERROR_Y | KINEMATIC_HOMING_ERROR_Z);
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
	// flags homing clear by the unlock
	int32_t steps_homing[STEPPER_COUNT] = {0};
	steps_homing[0] = g_settings.scara_arm_homing_angle * g_settings.step_per_mm[0] * FULL_TURN_INV;
	steps_homing[1] = g_settings.scara_forearm_homing_angle * g_settings.step_per_mm[1] * FULL_TURN_INV;
	kinematics_apply_forward(steps_homing, target);
	itp_reset_rt_position(target);
	mc_sync_position();

	cnc_set_exec_state(EXEC_HOMING);
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
	mc_line(target, &block_data);
	itp_sync();
#endif
	// unlocks the machine to go to offset
	cnc_clear_exec_state(EXEC_HOMING);
	
	return STATUS_OK;
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

	if (distance_to_center_sqr < scara_min_distance_to_center_sqr || distance_to_center_sqr > scara_max_distance_to_center_sqr)
	{
		return false;
	}

	for (uint8_t i = AXIS_COUNT; i != 2;)
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
