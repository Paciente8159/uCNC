/*
		Name: KINEMATIC_LINEAR_DELTA.c
		Description: Implements all kinematics math equations to translate the motion of a delta machine.
				Also implements the homing motion for this type of machine.

		Copyright: Copyright (c) João Martins
		Author: João Martins
		Date: 03/11/2022

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
#define HALF_TAN30 0.2886751345948128823f
#define TAN60 1.7320508075688772935f
#define FULL_TURN_INV 0.0027777777777777778f

static float delta_base_half_f_tg30;
static float delta_effector_half_f_tg30;
static float delta_base_effector_half_f_tg30;
static float steps_per_angle[3];
static float delta_cuboid_xy;
static float delta_cuboid_z_min;
static float delta_cuboid_z_max;
static float delta_cuboid_z_home;

static void delta_calc_bounds(void)
{
	float maxx = -g_settings.delta_effector_radius - g_settings.delta_base_radius - g_settings.delta_forearm_length - g_settings.delta_bicep_length;
	float maxy = maxx;
	float maxz = maxx;
	float minx = -maxx;
	float miny = -maxx;
	float minz = -maxx;
	float s = MAX(g_settings.step_per_mm[0], MAX(g_settings.step_per_mm[1], g_settings.step_per_mm[2]));
	float axis[AXIS_COUNT];
	int32_t steps[AXIS_COUNT];
	int32_t r[8][3];
	float btf = g_settings.max_distance[AXIS_Z];

	memset(axis, 0, AXIS_COUNT * sizeof(float));
	memset(steps, 0, AXIS_COUNT * sizeof(uint32_t));

	kinematics_apply_forward(steps, axis);
	delta_cuboid_z_home = axis[AXIS_Z];

	// find extents
	for (int32_t z = 0; z < s; ++z)
	{
		steps[0] = z;
		steps[1] = z;
		steps[2] = z;
		kinematics_apply_forward(steps, axis);
		if (axis[0] != NAN)
		{
			if (minz > axis[2])
				minz = axis[2];
			if (maxz < axis[2])
				maxz = axis[2];
		}
	}
	if (minz < -g_settings.max_distance[AXIS_Z])
		minz = -btf;
	if (maxz < -btf)
		maxz = -btf;

	float middlez = (maxz + minz) * 0.5;
	//  $('#output').append("<p>("+maxz+","+minz+","+middlez+")</p>");
	float original_dist = (maxz - middlez);
	float dist = original_dist * 0.5;
	float sum = 0;
	float mint1 = g_settings.step_per_mm[0];
	float maxt1 = -g_settings.step_per_mm[0];
	float mint2 = g_settings.step_per_mm[1];
	float maxt2 = -g_settings.step_per_mm[1];
	float mint3 = g_settings.step_per_mm[2];
	float maxt3 = -g_settings.step_per_mm[2];

	do
	{
		sum += dist;
		axis[AXIS_X] = sum;
		axis[AXIS_Y] = sum;
		axis[AXIS_Z] = middlez + sum;
		kinematics_apply_inverse(axis, &r[0]);
		axis[AXIS_X] = sum;
		axis[AXIS_Y] = -sum;
		axis[AXIS_Z] = middlez + sum;
		kinematics_apply_inverse(axis, &r[1]);
		axis[AXIS_X] = -sum;
		axis[AXIS_Y] = -sum;
		axis[AXIS_Z] = middlez + sum;
		kinematics_apply_inverse(axis, &r[2]);
		axis[AXIS_X] = -sum;
		axis[AXIS_Y] = sum;
		axis[AXIS_Z] = middlez + sum;
		kinematics_apply_inverse(axis, &r[3]);
		axis[AXIS_X] = sum;
		axis[AXIS_Y] = sum;
		axis[AXIS_Z] = middlez - sum;
		kinematics_apply_inverse(axis, &r[4]);
		axis[AXIS_X] = sum;
		axis[AXIS_Y] = -sum;
		axis[AXIS_Z] = middlez - sum;
		kinematics_apply_inverse(axis, &r[5]);
		axis[AXIS_X] = -sum;
		axis[AXIS_Y] = -sum;
		axis[AXIS_Z] = middlez - sum;
		kinematics_apply_inverse(axis, &r[6]);
		axis[AXIS_X] = -sum;
		axis[AXIS_Y] = sum;
		axis[AXIS_Z] = middlez - sum;
		kinematics_apply_inverse(axis, &r[7]);
		if (r[0][0] == INT32_MAX || r[1][0] == INT32_MAX || r[2][0] == INT32_MAX || r[3][0] == INT32_MAX ||
			r[4][0] == INT32_MAX || r[5][0] == INT32_MAX || r[6][0] == INT32_MAX || r[7][0] == INT32_MAX)
		{
			sum -= dist;
			dist *= 0.5;
		}
		else
		{
			for (uint8_t i = 0; i < 8; ++i)
			{
				if (mint1 > r[i][0])
					mint1 = r[i][0];
				if (maxt1 < r[i][0])
					maxt1 = r[i][0];
				if (mint2 > r[i][1])
					mint2 = r[i][1];
				if (maxt2 < r[i][1])
					maxt2 = r[i][1];
				if (mint3 > r[i][2])
					mint3 = r[i][2];
				if (maxt3 < r[i][2])
					maxt3 = r[i][2];
			}
		}
	} while (original_dist > sum && dist > 0.1);

	delta_cuboid_xy = sum;
	delta_cuboid_z_min = minz;
	delta_cuboid_z_max = maxz;
}

void kinematics_init(void)
{
	delta_base_half_f_tg30 = HALF_TAN30 * g_settings.delta_base_radius;
	delta_effector_half_f_tg30 = HALF_TAN30 * g_settings.delta_effector_radius;
	delta_base_effector_half_f_tg30 = HALF_TAN30 * (g_settings.delta_base_radius - g_settings.delta_effector_radius);
	steps_per_angle[0] = g_settings.step_per_mm[0] * FULL_TURN_INV;
	steps_per_angle[1] = g_settings.step_per_mm[1] * FULL_TURN_INV;
	steps_per_angle[2] = g_settings.step_per_mm[2] * FULL_TURN_INV;
	delta_calc_bounds();
}

// inverse kinematics
// helper functions, calculates angle theta1 (for YZ-pane)
int8_t delta_calcAngleYZ(float x0, float y0, float z0, float *theta)
{
	float re = g_settings.delta_forearm_length;
	float rf = g_settings.delta_bicep_length;
	float y1 = -delta_base_half_f_tg30; // f/2 * tg 30
	y0 -= delta_effector_half_f_tg30;	// shift center to edge
	// z = a + b*y
	float a = fast_flt_div2((x0 * x0 + y0 * y0 + z0 * z0 + rf * rf - re * re - y1 * y1)) / z0;
	float b = (y1 - y0) / z0;
	// discriminant
	float d = -(a + b * y1) * (a + b * y1) + rf * (b * b * rf + rf);
	if (d < 0)
	{
		*theta = NAN;
		return -1;
	}
	// non-existing point
	float yj = (y1 - a * b - sqrt(d)) / (b * b + 1); // choosing outer point
	float zj = a + b * yj;
	// original code
	// *theta = 180.0f * atan(-zj / (y1 - yj)) * M_PI_INV + ((yj > y1) ? 180.0f : 0);
	*theta = 180.0f * atan2(-zj, (y1 - yj)) * M_PI_INV;
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
			if (!delta_calcAngleYZ(axis[AXIS_X] * COS120 - axis[AXIS_Y] * SIN120, axis[AXIS_Y] * COS120 + axis[AXIS_X] * SIN120, axis[AXIS_Z], &theta3))
			{
				// converts angle to steps
				steps[0] = steps_per_angle[0] * theta1;
				steps[1] = steps_per_angle[1] * theta2;
				steps[2] = steps_per_angle[2] * theta3;
				return;
			}
		}
	}

	steps[0] = INT32_MAX;
	steps[1] = INT32_MAX;
	steps[2] = INT32_MAX;
}

void kinematics_apply_forward(int32_t *steps, float *axis)
{
	float rf = g_settings.delta_bicep_length;
	float re = g_settings.delta_forearm_length;

	float t = delta_base_effector_half_f_tg30;
	// float dtr = pi / (float)180.0;

	float theta1 = (float)steps[0] * DEG_RAD_MULT / steps_per_angle[0];
	float theta2 = (float)steps[1] * DEG_RAD_MULT / steps_per_angle[1];
	float theta3 = (float)steps[2] * DEG_RAD_MULT / steps_per_angle[2];

	float y1 = -(t + rf * cos(theta1));
	float z1 = -rf * sin(theta1);

	float y2 = (t + rf * cos(theta2)) * SIN30;
	float x2 = y2 * TAN60;
	float z2 = -rf * sin(theta2);

	float y3 = (t + rf * cos(theta3)) * SIN30;
	float x3 = -y3 * TAN60;
	float z3 = -rf * sin(theta3);

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
	float b = fast_flt_mul2((a1 * b1 + a2 * (b2 - y1 * dnm) - z1 * dnm * dnm));
	float c = (b2 - y1 * dnm) * (b2 - y1 * dnm) + b1 * b1 + dnm * dnm * (z1 * z1 - re * re);

	// discriminant
	float d = b * b - fast_flt_mul4((a * c));
	if (d < 0)
	{
		// return NaN
		axis[AXIS_X] = NAN;
		axis[AXIS_Y] = NAN;
		axis[AXIS_Z] = NAN;
		return; // non-existing point
	}

	float z0 = -fast_flt_div2((b + sqrt(d))) / a;
	axis[AXIS_X] = (a1 * z0 + b1) / dnm;
	axis[AXIS_Y] = (a2 * z0 + b2) / dnm;
	axis[AXIS_Z] = z0;

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

	int32_t angle_steps[AXIS_COUNT];
	memset(angle_steps, 0, sizeof(angle_steps));
	float angle = g_settings.delta_bicep_homing_angle;
	angle_steps[0] = roundf(angle * steps_per_angle[0]);
	angle_steps[1] = roundf(angle * steps_per_angle[1]);
	angle_steps[2] = roundf(angle * steps_per_angle[2]);
	kinematics_apply_forward(angle_steps, target);

	// sync systems (interpolator, motion control and parser - the latest is synched ny motion control)
	itp_reset_rt_position(target);
	mc_sync_position();

	// pull of only on the Z axis
	target[AXIS_Z] += ((g_settings.homing_dir_invert_mask & (1 << AXIS_Z)) ? -g_settings.homing_offset : g_settings.homing_offset);

	block_data.feed = g_settings.homing_fast_feed_rate;
	block_data.spindle = 0;
	block_data.dwell = 0;
	// starts offset and waits to finnish
	mc_line(target, &block_data);
	itp_sync();

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

	float xy_limit = delta_cuboid_xy;

	if (axis[AXIS_X] < -xy_limit || axis[AXIS_X] > xy_limit)
	{
		return false;
	}

	if (axis[AXIS_Y] < -xy_limit || axis[AXIS_Y] > xy_limit)
	{
		return false;
	}

	if (axis[AXIS_Z] < delta_cuboid_z_min || axis[AXIS_Z] > delta_cuboid_z_max)
	{
		return false;
	}

	return true;
}

#endif
