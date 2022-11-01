/*
		Name: KINEMATIC_LINEAR_DELTA.c
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
#include <stdint.h>
#include <math.h>

#define COS120 -0.5f
#define SIN120 0.8660254037844386468f
#define SQRT3 1.7320508075688772935f
#define SIN30 0.5f
#define TAN30 0.5773502691896257645f
#define TAN60 1.7320508075688772935f

static float delta_half_f_tg30;
static float delta_base_effector_radius_diff;
static float steps_to_angle[3];

void kinematics_init(void)
{
	delta_half_f_tg30 = -0.5 * 0.57735 * g_settings.delta_effector_radius;
	delta_base_effector_radius_diff = g_settings.delta_base_radius - g_settings.delta_effector_radius;
	steps_to_angle[0] = 1.0f / g_settings.step_per_mm[0];
	steps_to_angle[1] = 1.0f / g_settings.step_per_mm[1];
	steps_to_angle[2] = 1.0f / g_settings.step_per_mm[2];
}

// inverse kinematics
// helper functions, calculates angle theta1 (for YZ-pane)
int8_t delta_calcAngleYZ(float x0, float y0, float z0, float *theta)
{
	float y1 = delta_half_f_tg30;							// f/2 * tg 30
	y0 -= 0.5 * 0.57735 * g_settings.delta_effector_radius; // shift center to edge
	// z = a + b*y
	float a = (x0 * x0 + y0 * y0 + z0 * z0 + g_settings.delta_bicep_length * g_settings.delta_bicep_length - g_settings.delta_forearm_radius * g_settings.delta_forearm_radius - y1 * y1) / (2 * z0);
	float b = (y1 - y0) / z0;
	// discriminant
	float d = -(a + b * y1) * (a + b * y1) + g_settings.delta_bicep_length * (b * b * g_settings.delta_bicep_length + g_settings.delta_bicep_length);
	if (d < 0)
		return -1;									  // non-existing point
	float yj = (y1 - a * b - sqrtf(d)) / (b * b + 1); // choosing outer point
	float zj = a + b * yj;
	*theta = 180.0f * atanf(-zj / (y1 - yj)) / M_PI + ((yj > y1) ? 180.0f : 0.0f);
	return 0;
}

void kinematics_apply_inverse(float *axis, int32_t *steps)
{
	float theta1, theta2, theta3;
#if AXIS_COUNT > 3
	for (uint8_t i = 3; i < AXIS_COUNT; i++)
	{
		steps[i] = (int32_t)lroundf(g_settings.step_mm[i] * axis[i]);
	}
#endif

	if (!delta_calcAngleYZ(axis[AXIS_X], axis[AXIS_Y], axis[AXIS_Z], &theta1))
	{
		if (!delta_calcAngleYZ(axis[AXIS_X] * COS120 + axis[AXIS_Y] * SIN120, axis[AXIS_Y] * COS120 - axis[AXIS_X] * SIN120, axis[AXIS_Z], &theta2))
		{
			if (!delta_calcAngleYZ(axis[AXIS_X] * COS120 - axis[AXIS_Y] * SIN120, axis[AXIS_Y] * COS120 - axis[AXIS_X] * SIN120, axis[AXIS_Z], &theta3))
			{
				// converts angle to steps
				steps[0] = g_settings.step_per_mm[0] * theta1;
				steps[1] = g_settings.step_per_mm[1] * theta2;
				steps[2] = g_settings.step_per_mm[2] * theta3;
				return;
			}
		}
	}
}

void kinematics_apply_forward(int32_t *steps, float *axis)
{
	float t = fast_flt_div2(delta_base_effector_radius_diff * TAN30);

	float theta1 = steps[0] * steps_to_angle[0] * DEG_RAD_MULT;
	float theta2 = steps[1] * steps_to_angle[1] * DEG_RAD_MULT;
	float theta3 = steps[2] * steps_to_angle[2] * DEG_RAD_MULT;

	float y1 = -(t + g_settings.delta_bicep_length * cos(theta1));
	float z1 = -g_settings.delta_bicep_length * sin(theta1);

	float y2 = (t + g_settings.delta_bicep_length * cos(theta2)) * SIN30;
	float x2 = y2 * TAN60;
	float z2 = -g_settings.delta_bicep_length * sin(theta2);

	float y3 = (t + g_settings.delta_bicep_length * cos(theta3)) * SIN30;
	float x3 = -y3 * TAN60;
	float z3 = -g_settings.delta_bicep_length * sin(theta3);

	float dnm = (y2 - y1) * x3 - (y3 - y1) * x2;

	float w1 = y1 * y1 + z1 * z1;
	float w2 = x2 * x2 + y2 * y2 + z2 * z2;
	float w3 = x3 * x3 + y3 * y3 + z3 * z3;

	// x = (a1*z + b1)/dnm
	float a1 = (z2 - z1) * (y3 - y1) - (z3 - z1) * (y2 - y1);
	float b1 = -fast_flt_div2((w2 - w1) * (y3 - y1) - (w3 - w1) * (y2 - y1));

	// y = (a2*z + b2)/dnm;
	float a2 = -(z2 - z1) * x3 + (z3 - z1) * x2;
	float b2 = fast_flt_div2((w2 - w1) * x3 - (w3 - w1) * x2);

	// a*z^2 + b*z + c = 0
	float a = a1 * a1 + a2 * a2 + dnm * dnm;
	float b = fast_flt_mul2(a1 * b1 + a2 * (b2 - y1 * dnm) - z1 * dnm * dnm);
	float c = (b2 - y1 * dnm) * (b2 - y1 * dnm) + b1 * b1 + dnm * dnm * (z1 * z1 - g_settings.delta_forearm_radius * g_settings.delta_forearm_radius);

	// discriminant
	float d = b * b - (float)fast_flt_mul4(a * c);
	if (d < 0)
	{
		axis[AXIS_X] = NAN;
		axis[AXIS_Y] = NAN;
		axis[AXIS_Z] = NAN;
		return;
	}

	float z = -(float)fast_flt_div2(b + sqrt(d)) / a;
	axis[AXIS_X] = (a1 * z + b1) / dnm;
	axis[AXIS_Y] = (a2 * z + b2) / dnm;
	axis[AXIS_Z] = z;

#if AXIS_COUNT > 3
	for (uint8_t i = 3; i < AXIS_COUNT; i++)
	{
		axis[i] = (((float)steps[i]) / g_settings.step_per_mm[i]);
	}
#endif
}

uint8_t kinematics_home(void)
{
	if (mc_home_axis(AXIS_Z, LIMITS_DELTA_MASK))
	{
		return KINEMATIC_HOMING_ERROR_Z;
	}

#ifndef DISABLE_A_HOMING
#if (defined(AXIS_A) && !(LIMIT_A < 0))
	if (mc_home_axis(AXIS_A, LIMIT_A_MASK))
	{
		return (KINEMATIC_HOMING_ERROR_X | KINEMATIC_HOMING_ERROR_Y | KINEMATIC_HOMING_ERROR_Z);
	}
#endif
#endif

#ifndef DISABLE_B_HOMING
#if (defined(AXIS_B) && !(LIMIT_B < 0))
	if (mc_home_axis(AXIS_B, LIMIT_B_MASK))
	{
		return KINEMATIC_HOMING_ERROR_B;
	}
#endif
#endif

#ifndef DISABLE_C_HOMING
#if (defined(AXIS_C) && !(LIMIT_C < 0))
	if (mc_home_axis(AXIS_C, LIMIT_C_MASK))
	{
		return KINEMATIC_HOMING_ERROR_C;
	}
#endif
#endif

	// unlocks the machine to go to offset
	cnc_unlock(true);
	cnc_set_exec_state(EXEC_HOMING);
	float target[AXIS_COUNT];
	motion_data_t block_data = {0};
	mc_get_position(target);

	// pull of only on the Z axis
	target[AXIS_Z] += ((g_settings.homing_dir_invert_mask & (1 << AXIS_Z)) ? -g_settings.homing_offset : g_settings.homing_offset);

	block_data.feed = g_settings.homing_fast_feed_rate;
	block_data.spindle = 0;
	block_data.dwell = 0;
	// starts offset and waits to finnish
	mc_line(target, &block_data);
	itp_sync();

	cnc_clear_exec_state(EXEC_HOMING);

	memset(target, 0, sizeof(target));
#ifndef SET_ORIGIN_AT_HOME_POS
	if (g_settings.homing_dir_invert_mask & (1 << AXIS_Z))
	{
		target[AXIS_Z] = g_settings.max_distance[AXIS_Z];
	}
#endif

	// reset position
	itp_reset_rt_position(target);

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
	/*if (!g_settings.soft_limits_enabled || cnc_get_exec_state(EXEC_HOMING))
	{
		return true;
	}

	if (axis[AXIS_X] < -delta_base_max_travel || axis[AXIS_X] > delta_base_max_travel)
	{
		return false;
	}

	if (axis[AXIS_Y] < -delta_base_max_travel || axis[AXIS_Y] > delta_base_max_travel)
	{
		return false;
	}*/

#ifdef SET_ORIGIN_AT_HOME_POS
	if (axis[AXIS_Z] < -g_settings.max_distance[AXIS_Z] || axis[AXIS_Z] > 0)
	{
		return false;
	}
#else
	if (axis[AXIS_Z] > g_settings.max_distance[AXIS_Z] || axis[AXIS_Z] < 0)
	{
		return false;
	}
#endif

	return true;
}

#endif
