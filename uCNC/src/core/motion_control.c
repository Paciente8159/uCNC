/*
	Name: motion_control.c
	Description: Contains the building blocks for performing motions/actions in µCNC
	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 19/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include <math.h>
#include <string.h>
#include <float.h>

// line motion can be break in 3 types
// - continuous
// - segmented and uniform (segments don't change direction)
// - segmented and non uniform (segments can/might change direction)

// segmented motions
#if (defined(KINEMATICS_MOTION_BY_SEGMENTS) || defined(BRESENHAM_16BIT) || defined(ENABLE_G39_H_MAPPING))
#define MOTION_SEGMENTED
#endif

// non uniform segmented motions
#if (defined(KINEMATICS_MOTION_BY_SEGMENTS) || defined(ENABLE_G39_H_MAPPING))
#define MOTION_NON_UNIFORM
#endif

// default segment length for non linear kinematics
#ifdef KINEMATICS_MOTION_BY_SEGMENTS
#ifndef KINEMATICS_MOTION_SEGMENT_SIZE
#define KINEMATICS_MOTION_SEGMENT_SIZE 1.0f
#endif
#endif

#define KINEMATICS_MOTION_SEGMENT_INV_SIZE (1.0f / KINEMATICS_MOTION_SEGMENT_SIZE)

#ifndef AXIS_DIR_VECTORS
#ifdef ABC_INDEP_FEED_CALC
#define AXIS_DIR_VECTORS MIN(AXIS_COUNT, 3)
#else
#define AXIS_DIR_VECTORS AXIS_COUNT
#endif
#endif

static bool mc_flush_pending;
static bool mc_checkmode;
static int32_t mc_last_step_pos[STEPPER_COUNT];
static float mc_last_target[AXIS_COUNT];
// static float mc_prev_target_dir[AXIS_COUNT];
#ifdef ENABLE_BACKLASH_COMPENSATION
static uint8_t mc_last_dirbits;
#endif

#ifdef ENABLE_G39_H_MAPPING

#ifndef H_MAPPING_EEPROM_STORE_ENABLED
static float hmap_x;
static float hmap_y;
static float hmap_x_offset;
static float hmap_y_offset;
static float hmap_offsets[H_MAPING_ARRAY_SIZE];
#else
#define hmap_x g_settings.hmap_x
#define hmap_y g_settings.hmap_y
#define hmap_x_offset g_settings.hmap_x_offset
#define hmap_y_offset g_settings.hmap_y_offset
#define hmap_offsets g_settings.hmap_offsets
#endif

// the maximum subsegment length factor
#define H_MAPING_SEGMENT_INV_SIZE (MAX(((float)H_MAPING_GRID_FACTOR / hmap_x_offset), ((float)H_MAPING_GRID_FACTOR / hmap_y_offset)))

FORCEINLINE static float mc_apply_hmap(float *target);
#endif

#ifdef ENABLE_MOTION_CONTROL_MODULES
// event_mc_line_segment_handler
WEAK_EVENT_HANDLER(mc_line_segment)
{
	DEFAULT_EVENT_HANDLER(mc_line_segment);
}

// event_mc_home_axis_start
WEAK_EVENT_HANDLER(mc_home_axis_start)
{
	DEFAULT_EVENT_HANDLER(mc_home_axis_start);
}

// event_mc_home_axis_finish
WEAK_EVENT_HANDLER(mc_home_axis_finish)
{
	DEFAULT_EVENT_HANDLER(mc_home_axis_finish);
}
#endif

void mc_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
	memset(mc_last_step_pos, 0, sizeof(mc_last_step_pos));
	memset(mc_last_target, 0, sizeof(mc_last_target));
#ifdef ENABLE_BACKLASH_COMPENSATION
	mc_last_dirbits = 0;
#endif
#endif
#ifdef ENABLE_G39_H_MAPPING
#ifndef H_MAPPING_EEPROM_STORE_ENABLED
	memset(hmap_offsets, 0, sizeof(hmap_offsets));
#endif
#endif
	mc_checkmode = false;
	mc_sync_position();
}

bool mc_get_checkmode(void)
{
	return mc_checkmode;
}

bool mc_toogle_checkmode(void)
{
	mc_checkmode = !mc_checkmode;
	return mc_checkmode;
}

static void FORCEINLINE mc_restore_step_mode(uint8_t *mode)
{
	itp_set_step_mode(*mode);
}

static uint8_t mc_line_segment(int32_t *step_new_pos, motion_data_t *block_data)
{
// resets accumulator vars of the block
#ifdef ENABLE_LINACT_PLANNER
	block_data->full_steps = 0;
#endif
#ifdef MOTION_NON_UNIFORM
	block_data->dirbits = 0;
#endif
	uint32_t max_steps = 0;

	for (uint8_t i = STEPPER_COUNT; i != 0;)
	{
		i--;
		int32_t s = step_new_pos[i] - mc_last_step_pos[i];
		uint32_t steps = (uint32_t)ABS(s);
		block_data->steps[i] = (step_t)steps;

#ifdef MOTION_NON_UNIFORM
		// with the delta dir bits need to be rechecked
		if (s < 0)
		{
			block_data->dirbits |= (1 << i);
		}
#endif

#ifdef ENABLE_LINACT_PLANNER
		block_data->full_steps += steps;
#endif

		if (max_steps < steps)
		{
			max_steps = steps;
#ifdef MOTION_NON_UNIFORM
			// with the delta main stepper need to be rechecked
			block_data->main_stepper = i;
#endif
		}
	}

	// no significant motion will take place. don't send any thing to the planner
	if (!max_steps)
	{
		return STATUS_OK;
	}

	if (!mc_checkmode) // check mode (gcode simulation) doesn't send code to planner
	{
#ifdef ENABLE_BACKLASH_COMPENSATION
		// checks if any of the linear actuators there is a shift in direction
		uint8_t inverted_steps = mc_last_dirbits ^ block_data->dirbits;
		if (inverted_steps)
		{
			motion_data_t backlash_block_data = {0};
			memcpy(&backlash_block_data, block_data, sizeof(motion_data_t));
			memset(backlash_block_data.steps, 0, sizeof(backlash_block_data.steps));
			// resets accumulator vars
#ifdef ENABLE_LINACT_PLANNER
			backlash_block_data.full_steps = 0;
#endif
			backlash_block_data.feed = backlash_block_data.max_feed; // max feedrate possible (same as rapid move)

			SETFLAG(backlash_block_data.motion_mode, MOTIONCONTROL_MODE_BACKLASH_COMPENSATION);
			backlash_block_data.motion_flags.bit.backlash_comp = 1;
			max_steps = 0;
			for (uint8_t i = STEPPER_COUNT; i != 0;)
			{
				i--;
				if (inverted_steps & (1 << i))
				{
					backlash_block_data.steps[i] = g_settings.backlash_steps[i];
#ifdef ENABLE_LINACT_PLANNER
					backlash_block_data.full_steps += backlash_block_data.steps[i];
#endif
					if (max_steps < backlash_block_data.steps[i])
					{
						max_steps = backlash_block_data.steps[i];
						backlash_block_data.main_stepper = i;
					}
				}
			}

			while (planner_buffer_is_full())
			{
				if (!cnc_dotasks())
				{
					return STATUS_CRITICAL_FAIL;
				}
			}

			planner_add_line(&backlash_block_data);
			// dwell should only execute on the first request
			block_data->dwell = 0;
			mc_last_dirbits = block_data->dirbits;
		}
#endif

		bool mc_flushed = false;
		while (planner_buffer_is_full() && !mc_flushed)
		{
			if (!cnc_dotasks())
			{
				return STATUS_CRITICAL_FAIL;
			}
			mc_flushed = mc_flush_pending;
		}

		mc_flush_pending = false;

		if (mc_flushed)
		{
			return STATUS_JOG_CANCELED;
		}

#ifdef ENABLE_MOTION_CONTROL_MODULES
		// event_mc_line_segment_handler
		EVENT_INVOKE(mc_line_segment, block_data);
#endif

#ifdef ENABLE_STEPPERS_DISABLE_TIMEOUT
		io_enable_steppers(g_settings.step_enable_invert); // re-enable steppers for motion
#endif

		planner_add_line(block_data);
		// dwell should only execute on the first request
		block_data->dwell = 0;
	}

	// stores current step target position
	memcpy(mc_last_step_pos, step_new_pos, sizeof(mc_last_step_pos));

	return STATUS_OK;
}

// all motions should go through mc_line before entering the final planner pipeline
// after this stage the motion follows a pipeline that performs the following steps
// 1. decouples the target point from the remaining pipeline
// 2. applies all kinematic transformations to the target
// 3. converts the target in actuator position
// 4. calculates motion change from the previous line
// 5. if line is to big to be executed correctly by the interpolator, breaks it in to smaller line segments
uint8_t mc_line(float *target, motion_data_t *block_data)
{
	float prev_target[AXIS_COUNT];
#ifndef ENABLE_LINACT_PLANNER
	static float last_dir_vect[AXIS_DIR_VECTORS];
#endif
	float dir_vect[AXIS_DIR_VECTORS];
	block_data->dirbits = 0; // reset dirbits (this prevents odd behaviour generated by long arcs)
	block_data->main_stepper = 255; // reset the main stepper

#ifdef ENABLE_G39_H_MAPPING
	// modify the gcode with Hmap
	float target_hmap_offset = (CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_APPLY_HMAP)) ? (mc_apply_hmap(target)) : 0;
	target[AXIS_TOOL] += target_hmap_offset;
#endif

	// check travel limits (soft limits)
	if (!kinematics_check_boundaries(target))
	{
#ifdef ENABLE_G39_H_MAPPING
		// unmodify target
		target[AXIS_TOOL] -= target_hmap_offset;
#endif

#if !defined(MODIFY_SOFT_LIMIT_TO_ERROR) || defined(ALLOW_SOFT_LIMIT_JOG_MOTION_CLAMPING)
		if (cnc_get_exec_state(EXEC_JOG))
#endif
		{
#ifndef ALLOW_SOFT_LIMIT_JOG_MOTION_CLAMPING
#ifndef IGNORE_JOG_TARGET_SOFT_LIMIT_ERROR
			itp_sync();
			cnc_set_exec_state(EXEC_HOLD);
#endif
			return STATUS_TRAVEL_EXCEEDED;
#endif
		}
#if !defined(MODIFY_SOFT_LIMIT_TO_ERROR) || defined(ALLOW_SOFT_LIMIT_JOG_MOTION_CLAMPING)
		else
#endif
		{
#ifndef MODIFY_SOFT_LIMIT_TO_ERROR
			cnc_alarm(EXEC_ALARM_SOFT_LIMIT);
			return STATUS_OK;
#else
			return STATUS_TRAVEL_EXCEEDED;
#endif
		}
	}

#ifdef ENABLE_EMBROIDERY
	if ((g_settings.tool_mode & EMBROIDERY_MODE) && !block_data->spindle)
	{
		if (itp_sync() != STATUS_OK)
		{
			return STATUS_CRITICAL_FAIL;
		}
		tool_set_speed(0);
		while (tool_get_speed() && cnc_dotasks())
			;
	}
#endif

	uint8_t error = STATUS_OK;

	// gets the previous machine position (transformed to calculate the direction vector and traveled distance)
	memcpy(prev_target, mc_last_target, sizeof(mc_last_target));

#ifdef ENABLE_G39_H_MAPPING
	// modify the gcode with Hmap
	float h_offset = (CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_APPLY_HMAP)) ? (mc_apply_hmap(prev_target)) : 0;
	prev_target[AXIS_TOOL] += h_offset;
#endif

	// calculates the traveled distance
	float line_dist = 0;
	float motion_segment[AXIS_COUNT];
	for (uint8_t i = 0; i < AXIS_DIR_VECTORS; i++)
	{
		motion_segment[i] = target[i] - prev_target[i];
		dir_vect[i] = motion_segment[i];
		line_dist += fast_flt_pow2(dir_vect[i]);
	}

// remove the hmap postion of the motion
#ifdef ENABLE_G39_H_MAPPING
	motion_segment[AXIS_TOOL] += (h_offset - target_hmap_offset);
#endif

#ifndef ABC_INDEP_FEED_CALC
	// no motion. bail out.
	if (line_dist == 0)
	{
#ifdef ENABLE_G39_H_MAPPING
		// unmodify target
		target[AXIS_TOOL] -= target_hmap_offset;
#endif
		return STATUS_OK;
	}
#endif

	int32_t step_new_pos[STEPPER_COUNT];

	// converts transformed target to stepper position
	kinematics_coordinates_to_steps(target, step_new_pos);
	// calculates the amount of steps performed for this motion

	uint32_t max_steps = 0;
	for (uint8_t i = AXIS_TO_STEPPERS; i != 0;)
	{
		i--;
		int32_t steps = step_new_pos[i] - mc_last_step_pos[i];
#ifndef MOTION_NON_UNIFORM
		if (steps < 0)
		{
			block_data->dirbits |= (1 << i);
		}
#endif

		steps = ABS(steps);
		if (max_steps < (uint32_t)steps)
		{
			max_steps = steps;
#ifndef MOTION_NON_UNIFORM
			block_data->main_stepper = i;
#endif
		}
	}

	// no significant motion will take place. don't send any thing to the planner
	if (!max_steps)
	{
#ifdef ENABLE_G39_H_MAPPING
		// unmodify target
		target[AXIS_TOOL] -= target_hmap_offset;
#endif
		return STATUS_OK;
	}

	// feed values
	float max_feed = FLT_MAX;
	float max_accel = FLT_MAX;
	float feed = block_data->feed;
	float inv_dist = 0;

#ifdef ABC_INDEP_FEED_CALC
	if (line_dist != 0)
#endif
	{

		// angle between motion lines
		block_data->cos_theta = 0;
		// calculates the linear distance traveled
		line_dist = fast_flt_sqrt(line_dist);
		inv_dist = fast_flt_inv(line_dist);

		// calculates max junction speed factor in (axis driven). Else the cos_theta is calculated in the planner (linear actuator driven)
		for (uint8_t i = 0; i < AXIS_DIR_VECTORS; i++)
		{
			// calculates the normalized vector
			float normal_vect = dir_vect[i] * inv_dist;
#ifndef ENABLE_LINACT_PLANNER
			block_data->cos_theta += normal_vect * last_dir_vect[i];
			last_dir_vect[i] = normal_vect;
#endif
			dir_vect[i] = normal_vect;
			normal_vect = ABS(normal_vect);
			// denormalize max feed rate for each axis
			float denorm_param = fast_flt_div(g_settings.max_feed_rate[i], normal_vect);
			max_feed = MIN(max_feed, denorm_param);
			max_feed = MIN(max_feed, F_STEP_MAX);
			denorm_param = fast_flt_div(g_settings.acceleration[i], normal_vect);
			max_accel = MIN(max_accel, denorm_param);
		}
		max_feed *= inv_dist;
		max_accel *= inv_dist;
	}

#if defined(ABC_INDEP_FEED_CALC) && (AXIS_COUNT > 3)
	uint8_t slowest = 0;
	for (uint8_t i = 3; i < AXIS_COUNT; i++)
	{
		if (step_new_pos[i] != mc_last_step_pos[i])
		{
			if (g_settings.max_feed_rate[i] < max_feed)
			{
				slowest = i;
			}
			max_accel = MIN(max_accel, g_settings.acceleration[i]);
		}
	}

	if (slowest)
	{
		max_feed = MIN(max_feed, g_settings.max_feed_rate[slowest]);
		if (!line_dist)
		{
			line_dist = target[slowest] - prev_target[slowest];
			inv_dist = fast_flt_inv(line_dist);
		}
	}
#endif

#ifdef ENABLE_LASER_PPI
	g_settings.acceleration[STEPPER_COUNT - 1] = FLT_MAX;
	float ppi_max_feedrate = FLT_MAX;
	float ppi_step_rate = g_settings.step_per_mm[STEPPER_COUNT - 1];
	if (g_settings.tool_mode & (PPI_MODE | PPI_VARPOWER_MODE))
	{
		if (!ppi_step_rate)
		{
			ppi_step_rate = g_settings.laser_ppi * MM_INCH_MULT;
		}
	}
	else
	{
		ppi_step_rate = 0;
	}
	g_settings.step_per_mm[STEPPER_COUNT - 1] = ppi_step_rate;
	ppi_max_feedrate = (60000000.0f / (g_settings.laser_ppi_uswidth * ppi_step_rate));
	mc_last_step_pos[STEPPER_COUNT - 1] = 0;
	float laser_pulses_per_mm = 0;
	if (block_data->motion_flags.bit.spindle_running && block_data->spindle)
	{
		laser_pulses_per_mm = ppi_step_rate;
		// modify PPI settings according o the S value
		if (g_settings.tool_mode & PPI_MODE)
		{
			float laser_ppi_scale = fast_flt_div((float)block_data->spindle, (float)g_settings.spindle_max_rpm);
			if (g_settings.tool_mode & PPI_VARPOWER_MODE)
			{
				float blend = g_settings.laser_ppi_mixmode_ppi;
				laser_ppi_scale = (laser_ppi_scale * blend) + (1.0f - blend);
			}

			laser_pulses_per_mm *= laser_ppi_scale;
		}

		laser_pulses_per_mm *= line_dist;
		// adjust max feed rate to ppi settings
		max_feed = MIN(max_feed, ppi_max_feedrate * inv_dist);
	}

	step_new_pos[STEPPER_COUNT - 1] = laser_pulses_per_mm;
	if (step_new_pos[STEPPER_COUNT - 1] > max_steps)
	{
		max_steps = MAX(max_steps, step_new_pos[STEPPER_COUNT - 1]);
		block_data->main_stepper = STEPPER_COUNT - 1;
	}
#endif

	// calculated amount ot time @ the given feed rate
	float step_feed = (!CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_INVERSEFEED) ? (block_data->feed * inv_dist) : block_data->feed);
	float feed_convert_to_steps_per_sec = (float)max_steps;
	// convert accel already in steps/s
	// use max accel if accel is not already set by previous calculations (for example synched motions)
	block_data->max_accel = (!block_data->max_accel) ? (feed_convert_to_steps_per_sec * max_accel) : (block_data->max_accel * inv_dist * feed_convert_to_steps_per_sec);
	// convert feed from steps/min to steps/s
	feed_convert_to_steps_per_sec *= MIN_SEC_MULT;
	step_feed *= feed_convert_to_steps_per_sec;
	max_feed *= feed_convert_to_steps_per_sec;
	block_data->feed = MIN(max_feed, step_feed);
	block_data->max_feed = max_feed;

	block_data->feed_conversion = fast_flt_div(line_dist, feed_convert_to_steps_per_sec);

#ifdef MOTION_SEGMENTED
	// this contains a motion. Any tool update will be done here
	uint32_t line_segments = 1;
#ifdef ENABLE_G39_H_MAPPING
	if (CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_APPLY_HMAP))
	{
		line_segments = MAX((uint32_t)ceilf(line_dist * H_MAPING_SEGMENT_INV_SIZE), line_segments);
	}
#endif
#ifdef KINEMATICS_MOTION_BY_SEGMENTS
	line_segments = MAX((uint32_t)ceilf(line_dist * KINEMATICS_MOTION_SEGMENT_INV_SIZE), line_segments);
#endif
#ifdef BRESENHAM_16BIT
	// checks the amount of steps that this motion translates to
	// if the amount of steps is higher than the limit for the 16bit bresenham algorithm
	// splits the line into smaller segments
	if (max_steps > MAX_STEPS_PER_LINE)
	{
		line_segments = MAX((max_steps >> MAX_STEPS_PER_LINE_BITS) + 1, line_segments);
	}
#endif

	if (line_segments > 1)
	{
		float m_inv = 1.0f / (float)line_segments;
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			motion_segment[i] *= m_inv;
		}

#ifdef ENABLE_G39_H_MAPPING
		// unmodify target
		prev_target[AXIS_TOOL] -= h_offset;
#endif
	}

	bool is_subsegment = false;
	while (--line_segments)
	{
		is_subsegment = true;
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			prev_target[i] += motion_segment[i];
		}

#ifdef ENABLE_G39_H_MAPPING
		h_offset = (CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_APPLY_HMAP)) ? (mc_apply_hmap(prev_target)) : 0;
		prev_target[AXIS_TOOL] += h_offset;
#endif
		kinematics_coordinates_to_steps(prev_target, step_new_pos);
		error = mc_line_segment(step_new_pos, block_data);
#ifdef ENABLE_G39_H_MAPPING
		// unmodify target
		prev_target[AXIS_TOOL] -= h_offset;
#endif
		if (error)
		{
			memcpy(target, prev_target, sizeof(prev_target));
			block_data->feed = feed;
			return error;
		}
		// after the first segment all following segments are inline
		block_data->cos_theta = 1;
	}

	if (is_subsegment)
	{
		kinematics_coordinates_to_steps(target, step_new_pos);
	}
#endif
	error = mc_line_segment(step_new_pos, block_data);

#ifdef ENABLE_G39_H_MAPPING
	// unmodify target
	target[AXIS_TOOL] -= target_hmap_offset;
#endif

	// stores the new position for the next motion
	memcpy(mc_last_target, target, sizeof(mc_last_target));
	// restores feed and clears max acceleration to enable recalculation on next motion
	block_data->feed = feed;
	block_data->max_accel = 0;
	return error;
}

#ifndef DISABLE_ARC_SUPPORT
// applies an algorithm similar to grbl with slight changes
uint8_t mc_arc(float *target, float center_offset_a, float center_offset_b, float radius, uint8_t axis_0, uint8_t axis_1, bool isclockwise, motion_data_t *block_data)
{
	float mc_position[AXIS_COUNT];

	// copy motion control last position
	mc_get_position(mc_position);

	float ptcenter_a = mc_position[axis_0] + center_offset_a;
	float ptcenter_b = mc_position[axis_1] + center_offset_b;

	float pt0_a = -center_offset_a; // Radius vector from center to current location
	float pt0_b = -center_offset_b;
	float pt1_a = target[axis_0] - ptcenter_a; // Radius vector from center to current location
	float pt1_b = target[axis_1] - ptcenter_b;

	// dot product between vect_a and vect_b
	float dotprod = pt0_a * pt1_a + pt0_b * pt1_b;
	// determinant
	float det = pt0_a * pt1_b - pt0_b * pt1_a;
	float arc_angle = atan2(det, dotprod);

	if (isclockwise)
	{
		if (arc_angle >= 0)
		{
			arc_angle -= 2 * M_PI;
		}
	}
	else
	{
		if (arc_angle <= 0)
		{
			arc_angle += 2 * M_PI;
		}
	}

	// uses as temporary vars
	float radiusangle = radius * arc_angle;
	radiusangle = fast_flt_div2(radiusangle);
	float diameter = fast_flt_mul2(radius);
	uint16_t segment_count = floor(fabs(radiusangle) / sqrt(g_settings.arc_tolerance * (diameter - g_settings.arc_tolerance)));
	float arc_per_sgm = (segment_count != 0) ? arc_angle / segment_count : arc_angle;

	// for all other axis finds the linear motion distance
	float increment[AXIS_COUNT];

	for (uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
		increment[i] = (target[i] - mc_position[i]) / segment_count;
	}

	increment[axis_0] = 0;
	increment[axis_1] = 0;

	if (CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_INVERSEFEED))
	{
		// split the required time to complete the motion with the number of segments
		block_data->feed *= segment_count;
	}

	// calculates an aproximation to sine and cosine of the angle segment
	// improves the error for the cosine by calculating an extra term of the taylor series at the expence of an extra multiplication and addition
	// applies arc correction has grbl does
	float arc_per_sgm_sqr = arc_per_sgm * arc_per_sgm;
	float cos_per_sgm = 1 - M_COS_TAYLOR_1 * arc_per_sgm_sqr;
	float sin_per_sgm = arc_per_sgm * cos_per_sgm;
	cos_per_sgm = arc_per_sgm_sqr * (cos_per_sgm + 1);
	cos_per_sgm = 1 - fast_flt_div4(cos_per_sgm);

	uint8_t count = 0;

	for (uint16_t i = 1; i < segment_count; i++)
	{
		if (count < N_ARC_CORRECTION)
		{
			// Apply incremental vector rotation matrix.
			float new_pt = pt0_a * sin_per_sgm + pt0_b * cos_per_sgm;
			pt0_a = pt0_a * cos_per_sgm - pt0_b * sin_per_sgm;
			pt0_b = new_pt;
			count++;
		}
		else
		{
			// Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
			// Compute exact location by applying transformation matrix from initial radius vector(=-offset).
			float angle = i * arc_per_sgm;
			float precise_cos = cos(angle);
			// calculates sine using sine and cosine relation equation
			//	sin(x)^2 + cos(x)^2 = 1
			//
			// this is executes in about 50% the time of a sin function
			// https://www.nongnu.org/avr-libc/user-manual/benchmarks.html
			float precise_sin = sqrt(1 - precise_cos * precise_cos);
			if (angle >= 0)
			{
				precise_sin = (ABS(angle) <= M_PI) ? precise_sin : -precise_sin;
			}
			else
			{
				precise_sin = (ABS(angle) <= M_PI) ? -precise_sin : precise_sin;
			}

			pt0_a = -center_offset_a * precise_cos + center_offset_b * precise_sin;
			pt0_b = -center_offset_a * precise_sin - center_offset_b * precise_cos;
			count = 0;
		}

		// Update arc_target location
		mc_position[axis_0] = ptcenter_a + pt0_a;
		mc_position[axis_1] = ptcenter_b + pt0_b;
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			if (i != axis_0 && i != axis_1)
			{
				mc_position[i] += increment[i];
			}
		}

		uint8_t error = mc_line(mc_position, block_data);
		if (error)
		{
			return error;
		}
	}
	// Ensure last segment arrives at target location.
	return mc_line(target, block_data);
}
#endif

uint8_t mc_dwell(motion_data_t *block_data)
{
	if (!mc_checkmode) // check mode (gcode simulation) doesn't send code to planner
	{
		mc_update_tools(block_data);
		cnc_dwell_ms(block_data->dwell);
	}

	return STATUS_OK;
}

uint8_t mc_pause(void)
{
	if (!mc_checkmode) // check mode (gcode simulation) doesn't send code to planner
	{
		if (itp_sync() != STATUS_OK)
		{
			return STATUS_CRITICAL_FAIL;
		}
		cnc_set_exec_state(EXEC_HOLD);
	}
	return STATUS_OK;
}

uint8_t mc_update_tools(motion_data_t *block_data)
{
#if (TOOL_COUNT > 0)
	if (!mc_checkmode) // check mode (gcode simulation) doesn't send code to planner
	{
		if (itp_sync() != STATUS_OK)
		{
			return STATUS_CRITICAL_FAIL;
		}
		// synchronizes the tools
		if (block_data)
		{
			planner_sync_tools(block_data);
		}
		itp_sync_spindle();
	}
#endif
	return STATUS_OK;
}

#ifdef ENABLE_MOTION_CONTROL_MODULES
void mc_home_axis_finalize(homing_status_t *status)
{
	EVENT_INVOKE(mc_home_axis_finish, status);
}
#endif

bool mc_home_motion(uint8_t axis_mask, bool is_origin_search, motion_data_t *block_data)
{
	float target[AXIS_COUNT];

	// Sync motion control with real time positon
	mc_sync_position();
	mc_get_position(target);

	// Set movement distance for each axis
	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		uint8_t imask = (1 << i);
		if (imask & axis_mask)
		{
			// Invert the distance if configuration says so
			if (g_settings.homing_dir_invert_mask & axis_mask)
			{
				target[i] -= (is_origin_search) ? (g_settings.max_distance[i] * -1.5f) : g_settings.homing_offset * 5.0f;
			}
			else
			{
				target[i] += (is_origin_search) ? (g_settings.max_distance[i] * -1.5f) : g_settings.homing_offset * 5.0f;
			}
		}
	}

	if (cnc_unlock(true) != UNLOCK_OK)
	{
		return false;
	}
	mc_line(target, block_data);

	if (itp_sync() != STATUS_OK)
	{
		// Motion failed
		return false;
	}

	// Flush buffers and stop motion
	itp_stop();
	itp_clear();
	planner_clear();

	// Motion completed successfully
	return true;
}

uint8_t mc_home_axis(uint8_t axis_mask, uint8_t axis_limit)
{
	motion_data_t block_data = {0};
	uint8_t limits_flags;
	uint8_t restore_step_mode __attribute__((__cleanup__(mc_restore_step_mode))) = itp_set_step_mode(ITP_STEP_MODE_REALTIME);

#ifdef ENABLE_MOTION_CONTROL_MODULES
	homing_status_t homing_status __attribute__((__cleanup__(mc_home_axis_finalize))) = {axis_mask, axis_limit, STATUS_OK};
#endif

	// #ifdef ENABLE_G39_H_MAPPING
	// 	// resets height map
	// 	memset(hmap_offsets, 0, sizeof(hmap_offsets));
	// #endif

#ifdef ENABLE_MOTION_CONTROL_MODULES
	EVENT_INVOKE(mc_home_axis_start, &homing_status);
	homing_status.status = STATUS_CRITICAL_FAIL;
#endif

	if (!g_settings.hard_limits_enabled)
	{
#ifdef ALLOW_SOFTWARE_HOMING
		return STATUS_OK;
#else
		return STATUS_HARDLIMITS_DISABLED;
#endif
	}

// locks limits to accept axis limit mask only or else throw error
#ifdef ENABLE_MULTI_STEP_HOMING
	io_lock_limits(axis_limit);
#endif
	io_invert_limits(0);
	cnc_unlock(true);

	if (cnc_get_exec_state(EXEC_HOLD | EXEC_ALARM) || CHECKFLAG(io_get_limits(), LIMITS_MASK))
	{
		cnc_alarm(EXEC_ALARM_HOMING_FAIL_LIMIT_ACTIVE);
		return STATUS_CRITICAL_FAIL;
	}

	// initializes planner block data
	block_data.feed = g_settings.homing_fast_feed_rate;
	block_data.motion_mode = MOTIONCONTROL_MODE_FEED;

#ifdef ENABLE_LONG_HOMING_CYCLE
	uint8_t homing_passes = 2;
	while (homing_passes--)
	{
#endif
		if (!mc_home_motion(axis_mask, true, &block_data))
		{
			return STATUS_CRITICAL_FAIL;
		}

		cnc_delay_ms(g_settings.debounce_ms); // adds a delay before reading io pin (debounce)
		limits_flags = io_get_limits();

		// the wrong switch was activated bails
		if (!CHECKFLAG(limits_flags, axis_limit))
		{
			cnc_set_exec_state(EXEC_UNHOMED);
			cnc_alarm(EXEC_ALARM_HOMING_FAIL_APPROACH);
			return STATUS_CRITICAL_FAIL;
		}

		// temporary inverts the limit mask to trigger ISR on switch release
		io_invert_limits(axis_limit);

#ifndef ENABLE_LONG_HOMING_CYCLE
		// modify the speed to slow search speed
		block_data.feed = g_settings.homing_slow_feed_rate;
#endif

		if (!mc_home_motion(axis_mask, false, &block_data))
		{
			return STATUS_CRITICAL_FAIL;
		}

		cnc_delay_ms(g_settings.debounce_ms); // adds a delay before reading io pin (debounce)
		// resets limit mask
		io_invert_limits(0);
		limits_flags = io_get_limits();
		if (CHECKFLAG(limits_flags, axis_limit))
		{
			cnc_set_exec_state(EXEC_UNHOMED);
			cnc_alarm(EXEC_ALARM_HOMING_FAIL_PULLOFF);
			return STATUS_CRITICAL_FAIL;
		}

#ifdef ENABLE_LONG_HOMING_CYCLE
		// do the second pass at slow speed
		block_data.feed = g_settings.homing_slow_feed_rate;
	}
#endif

#ifdef ENABLE_MOTION_CONTROL_MODULES
	// if cleanup is called at any other exit point then homing has failed
	homing_status.status = STATUS_OK;
#endif
	return STATUS_OK;
}

#ifndef DISABLE_PROBING_SUPPORT
uint8_t mc_probe(float *target, uint8_t flags, motion_data_t *block_data)
{
#if ASSERT_PIN(PROBE)
	uint8_t restore_step_mode __attribute__((__cleanup__(mc_restore_step_mode))) = itp_set_step_mode(ITP_STEP_MODE_REALTIME);
#ifdef ENABLE_G39_H_MAPPING
	// disable hmap for probing motion
	block_data->motion_mode &= ~MOTIONCONTROL_MODE_APPLY_HMAP;
#endif
	if (itp_sync() != STATUS_OK)
	{
		return STATUS_CRITICAL_FAIL;
	}

	bool probe_ok = io_get_probe();
	probe_ok = (flags & MOTIONCONTROL_PROBE_INVERT) ? probe_ok : !probe_ok;

	if (!probe_ok)
	{
		if (!(flags & MOTIONCONTROL_PROBE_NOALARM_ONFAIL))
		{
			cnc_alarm(EXEC_ALARM_PROBE_FAIL_INITIAL);
		}
		return STATUS_OK;
	}

	// enable the probe
	io_enable_probe();
	mc_line(target, block_data);

	if (block_data->main_stepper == 255)
	{
		return STATUS_GCODE_INVALID_TARGET; // target is the current position (invalid)
	}

	// similar to itp_sync
	do
	{
		if (!cnc_dotasks())
		{
			break;
		}
		if (io_get_probe() ^ (flags & 0x01))
		{
#ifndef ENABLE_RT_PROBE_CHECKING
			mcu_probe_changed_cb();
#endif
			break;
		}
	} while (!itp_is_empty() || !planner_buffer_is_empty());

	// wait for a stop
	while (cnc_dotasks() && cnc_get_exec_state(EXEC_RUN))
		;
	// disables the probe
	io_disable_probe();
	itp_clear();
	// clears the buffer but conserves the tool data
	while (!planner_buffer_is_empty())
	{
		planner_discard_block();
	}
	// clears hold
	cnc_clear_exec_state(EXEC_HOLD);

	// sync the position of the motion control
	mc_sync_position();

	cnc_delay_ms(g_settings.debounce_ms); // adds a delay before reading io pin (debounce)
	probe_ok = io_get_probe();
	probe_ok = (flags & MOTIONCONTROL_PROBE_INVERT) ? !probe_ok : probe_ok;
	if (!probe_ok)
	{
		if (!(flags & MOTIONCONTROL_PROBE_NOALARM_ONFAIL))
		{
			cnc_alarm(EXEC_ALARM_PROBE_FAIL_CONTACT);
		}
		return STATUS_PROBE_UNSUCCESS;
	}
#endif

	return STATUS_PROBE_SUCCESS;
}
#endif

void mc_get_position(float *target)
{
	memcpy(target, mc_last_target, sizeof(mc_last_target));
}

void mc_sync_position(void)
{
	itp_get_rt_position(mc_last_step_pos);
	kinematics_steps_to_coordinates(mc_last_step_pos, mc_last_target);
	parser_sync_position();
}

uint8_t mc_incremental_jog(float *target_offset, motion_data_t *block_data)
{
	float new_target[AXIS_COUNT];
	uint8_t state = cnc_get_exec_state(EXEC_ALLACTIVE);

	if ((state & ~EXEC_JOG) || cnc_has_alarm()) // if any other than idle or jog discards the command
	{
		return STATUS_IDLE_ERROR;
	}

	cnc_set_exec_state(EXEC_JOG);

	// gets the previous machine position (transformed to calculate the direction vector and traveled distance)
	memcpy(new_target, mc_last_target, sizeof(mc_last_target));
	for (uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
		new_target[i] += target_offset[i];
	}

#if TOOL_COUNT > 0
	block_data->motion_flags.reg = g_planner_state.state_flags.reg;
	block_data->spindle = g_planner_state.spindle_speed;
#endif

	uint8_t error = mc_line(new_target, block_data);

	if (error == STATUS_OK)
	{
		parser_sync_position();
	}

	return error;
}

void mc_flush_pending_motion(void)
{
	mc_flush_pending = true;
}

#ifdef ENABLE_G39_H_MAPPING

void mc_print_hmap(void)
{
	proto_info("HMAP start corner: %f;%f", hmap_x, hmap_y);
	proto_info("HMAP end corner: %f;%f", hmap_x + hmap_x_offset, hmap_y + hmap_y_offset);
	proto_info("HMAP control points: %hd", H_MAPING_ARRAY_SIZE);

	// print map
	for (uint8_t j = 0; j < H_MAPING_GRID_FACTOR; j++)
	{
		for (uint8_t i = 0; i < H_MAPING_GRID_FACTOR; i++)
		{
			uint8_t map = i + (H_MAPING_GRID_FACTOR * j);
			float new_h = hmap_offsets[map];
			proto_info("HMAP: %hd; %hd; %f", i, j, new_h);
		}
	}
}

static float mc_apply_hmap(float *target)
{
	float x_weight = (target[AXIS_X] - hmap_x) / hmap_x_offset;
	float y_weight = (target[AXIS_Y] - hmap_y) / hmap_y_offset;
	uint8_t height_row, height_col;

	// outside of the region don't apply hmap
	if (x_weight < 0 || x_weight > 1 || y_weight < 0 || y_weight > 1)
	{
		return 0;
	}

	// checks the partition
	x_weight *= (H_MAPING_GRID_FACTOR - 1);
	y_weight *= (H_MAPING_GRID_FACTOR - 1);

	// prevent exact offset error
	height_row = (uint8_t)MAX(0, (x_weight - 0.000001f));
	height_col = (uint8_t)MAX(0, (y_weight - 0.000001f));

	x_weight -= height_row;
	y_weight -= height_col;

	float a0, a1, a2, a3;

	a0 = hmap_offsets[H_MAPING_GRID_FACTOR * height_col + height_row];
	a1 = hmap_offsets[H_MAPING_GRID_FACTOR * height_col + height_row + 1];
	a2 = hmap_offsets[H_MAPING_GRID_FACTOR * height_col + height_row + H_MAPING_GRID_FACTOR];
	a3 = hmap_offsets[H_MAPING_GRID_FACTOR * height_col + height_row + H_MAPING_GRID_FACTOR + 1] + a0 - a1 - a2;
	a1 -= a0;
	a2 -= a0;

	return (a0 + a1 * x_weight + a2 * y_weight + a3 * x_weight * y_weight);
}

uint8_t mc_build_hmap(float *target, float *offset, float retract_h, motion_data_t *block_data)
{
	// // generate dummy map
	// // store coordinates
	// hmap_x = target[AXIS_X];
	// hmap_y = target[AXIS_Y];
	// hmap_x_offset = offset[0];
	// hmap_y_offset = offset[1];

	// for (uint8_t j = 0; j < H_MAPING_GRID_FACTOR; j++)
	// {
	// 	for (uint8_t i = 0; i < H_MAPING_GRID_FACTOR; i++)
	// 	{
	// 		uint8_t map = i + (H_MAPING_GRID_FACTOR * j);
	// 		float new_h = (2.0f * rand() / RAND_MAX) - 1.0f;
	// 		hmap_offsets[map] = new_h;
	// 	}
	// }

	// float h_offset_base2 = hmap_offsets[0];
	// // make offsets relative to point 0,0
	// for (uint8_t j = 0; j < H_MAPING_GRID_FACTOR; j++)
	// {
	// 	for (uint8_t i = 0; i < H_MAPING_GRID_FACTOR; i++)
	// 	{
	// 		uint8_t map = i + (H_MAPING_GRID_FACTOR * j);
	// 		float new_h = hmap_offsets[map] - h_offset_base2;
	// 		hmap_offsets[map] = new_h;
	// 	}
	// }
	// // print map
	// mc_print_hmap();

	// return STATUS_OK;

	uint8_t error;
	float start_x = target[AXIS_X];
	float start_y = target[AXIS_Y];
	float offset_x = offset[0] / (H_MAPING_GRID_FACTOR - 1);
	float offset_y = offset[1] / (H_MAPING_GRID_FACTOR - 1);
	float position[AXIS_COUNT];
	float feed = block_data->feed;
	float new_hmap_offsets[H_MAPING_ARRAY_SIZE];

	// clear the previous map
	memset(new_hmap_offsets, 0, sizeof(new_hmap_offsets));

	mc_get_position(position);

	float minretract_h = position[AXIS_TOOL] + retract_h;
	float maxretract_h = minretract_h;

	for (uint8_t j = 0; j < H_MAPING_GRID_FACTOR; j++)
	{
		target[AXIS_X] = start_x;
		for (uint8_t i = 0; i < H_MAPING_GRID_FACTOR; i++)
		{
			block_data->feed = FLT_MAX;
			// retract (higher if needed)
			maxretract_h = MAX(maxretract_h, position[AXIS_TOOL] + retract_h);
			position[AXIS_TOOL] = MAX(minretract_h, position[AXIS_TOOL] + retract_h);
			error = mc_line(position, block_data);
			if (error != STATUS_OK)
			{
				return error;
			}

			// transverse motion to position
			position[AXIS_X] = target[AXIS_X];
			position[AXIS_Y] = target[AXIS_Y];
			error = mc_line(position, block_data);
			if (error != STATUS_OK)
			{
				return error;
			}

			// probe
			block_data->feed = feed;
			memcpy(position, target, sizeof(position));
			if (mc_probe(position, 0, block_data) != STATUS_PROBE_SUCCESS)
			{
				return STATUS_CRITICAL_FAIL;
			}

			// store position
			int32_t probe_position[STEPPER_COUNT];
			parser_get_probe(probe_position);
			kinematics_steps_to_coordinates(probe_position, position);
			new_hmap_offsets[i + H_MAPING_GRID_FACTOR * j] = position[AXIS_TOOL];
			proto_probe_result(1);

			// update to new target
			target[AXIS_X] += offset_x;
		}

		target[AXIS_Y] += offset_y;
	}

	block_data->feed = FLT_MAX;
	// fast retract if needed
	// retract (higher if needed)
	block_data->feed = FLT_MAX;
	position[AXIS_TOOL] = maxretract_h;
	error = mc_line(position, block_data);
	if (error != STATUS_OK)
	{
		return error;
	}

	// transverse to 1st point
	position[AXIS_X] = start_x;
	position[AXIS_Y] = start_y;
	block_data->feed = FLT_MAX;
	error = mc_line(position, block_data);
	if (error != STATUS_OK)
	{
		return error;
	}

	// move to 1st point at feed speed
	block_data->feed = feed;
	position[AXIS_TOOL] = new_hmap_offsets[0];
	error = mc_line(position, block_data);
	if (error != STATUS_OK)
	{
		return error;
	}

	float h_offset_base = new_hmap_offsets[0];
	// make offsets relative to point 0,0
	for (uint8_t j = 0; j < H_MAPING_GRID_FACTOR; j++)
	{
		for (uint8_t i = 0; i < H_MAPING_GRID_FACTOR; i++)
		{
			uint8_t map = i + (H_MAPING_GRID_FACTOR * j);
			float new_h = new_hmap_offsets[map] - h_offset_base;
			new_hmap_offsets[map] = new_h;
		}
	}

	// store coordinates
	hmap_x = start_x;
	hmap_y = start_y;
	hmap_x_offset = offset[0];
	hmap_y_offset = offset[1];

	// copy the new map tp the hmap array
	memcpy(hmap_offsets, new_hmap_offsets, sizeof(new_hmap_offsets));
#ifdef H_MAPPING_EEPROM_STORE_ENABLED
	// store the new map
	settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
#endif

	// print map
	mc_print_hmap();

	if (itp_sync() != STATUS_OK)
	{
		return STATUS_CRITICAL_FAIL;
	}

	// sync position of all systems
	mc_sync_position();

	return STATUS_OK;
}

void mc_clear_hmap(void)
{
	memset(hmap_offsets, 0, sizeof(hmap_offsets));
}
#endif

#ifdef ENABLE_MOTION_CONTROL_PLANNER_HIJACKING
static int32_t mc_last_step_pos_copy[STEPPER_COUNT];
static float mc_last_target_copy[AXIS_COUNT];
// stores the motion controller reference positions
void mc_store(void)
{
	memcpy(mc_last_step_pos_copy, mc_last_step_pos, sizeof(mc_last_step_pos));
	memcpy(mc_last_target_copy, mc_last_target, sizeof(mc_last_target));
}
// restores the motion controller reference positions
void mc_restore(void)
{
	memcpy(mc_last_step_pos, mc_last_step_pos_copy, sizeof(mc_last_step_pos));
	memcpy(mc_last_target, mc_last_target_copy, sizeof(mc_last_target));
}
#endif
