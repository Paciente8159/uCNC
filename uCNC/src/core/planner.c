/**
	Name: planner.c
	Description: Chain planner for linear motions and acceleration/deacceleration profiles.
		It uses a similar algorithm to Grbl.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 24/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>

typedef struct
{
	uint8_t feed_override;
	uint8_t rapid_feed_override;
#if TOOL_COUNT > 0
	int16_t spindle_speed;
	uint8_t spindle_speed_override;
	planner_flags_t state_flags;
#endif
} planner_state_t;

static planner_block_t planner_data[PLANNER_BUFFER_SIZE];
static uint8_t planner_data_write;
static uint8_t planner_data_read;
static uint8_t planner_data_blocks;
static planner_state_t planner_state;
static uint8_t planner_ovr_counter;

FORCEINLINE static void planner_add_block(void);
FORCEINLINE static uint8_t planner_buffer_next(uint8_t index);
FORCEINLINE static uint8_t planner_buffer_prev(uint8_t index);
FORCEINLINE static void planner_recalculate(void);
FORCEINLINE static void planner_buffer_clear(void);

/*
	Adds a new line to the trajectory planner
	The planner is responsible for calculating the entry and exit speeds of the transitions
	The trajectory planner does the following actions:
		1. Calculates the direction change of the new movement
		2. Adjusts maximum entry feed according to the angle of the junction point
		3. Recalculates all chained segments

	For profiling the motion 4 feeds are calculated
		1. The target feed
		2. The rapid motion feed given the direction (maximum allowed feed with overrides)
		3. The entry feed (initialy set to 0)
		4. The maximum entry feed given the juntion angle between planner blocks
*/
void planner_add_line(motion_data_t *block_data)
{
#ifdef ENABLE_LINACT_PLANNER
	static float last_dir_vect[STEPPER_COUNT];
#endif

	// clear the planner block
	uint8_t index = planner_data_write;
	float cos_theta = block_data->cos_theta;
	memset(&planner_data[index], 0, sizeof(planner_block_t));
	planner_data[index].dirbits = block_data->dirbits;
	planner_data[index].main_stepper = block_data->main_stepper;
	planner_data[index].planner_flags.reg &= ~STATE_COPY_FLAG_MASK;
	planner_data[index].planner_flags.reg |= (block_data->motion_flags.reg & STATE_COPY_FLAG_MASK); // copies the motion flags relative to coolant spindle running and feed_override

#if (defined(KINEMATICS_MOTION_BY_SEGMENTS) || defined(BRESENHAM_16BIT))
	planner_data[index].feed_conversion = block_data->feed_conversion;
#endif

#if TOOL_COUNT > 0
	planner_data[index].spindle = block_data->spindle;
#endif
#ifdef GCODE_PROCESS_LINE_NUMBERS
	planner_data[index].line = block_data->line;

#endif

#ifdef ENABLE_BACKLASH_COMPENSATION
	if (CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_BACKLASH_COMPENSATION))
	{
		planner_data[index].planner_flags.bit.backlash_comp = true;
	}
#endif

	memcpy(planner_data[index].steps, block_data->steps, sizeof(planner_data[index].steps));

	// calculates the normalized vector with the amount of motion in any linear actuator
	// also calculates the maximum feedrate and acceleration for each linear actuator
#ifdef ENABLE_LINACT_PLANNER
	float inv_total_steps = 1.0f / (float)(block_data->full_steps);
#endif

#ifdef ENABLE_LINACT_PLANNER
	float dir_vect[STEPPER_COUNT];
	memset(dir_vect, 0, sizeof(dir_vect));

	for (uint8_t i = STEPPER_COUNT; i != 0;)
	{
		i--;
		if (planner_data[index].steps[i] != 0)
		{
			dir_vect[i] = inv_total_steps * (float)planner_data[index].steps[i];

			if (!planner_buffer_is_empty())
			{
				cos_theta += last_dir_vect[i] * dir_vect[i];
#ifdef ENABLE_LINACT_COLD_START
				if (last_dir_vect[i] == 0) // checks if actuator is starting from a full stop
				{
					// then forces a full start and stop motion
					cos_theta = -100;
				}
#endif
			}

			last_dir_vect[i] = dir_vect[i];
		}
		else
		{
			last_dir_vect[i] = 0;
		}
	}

#endif

	// converts to steps per second (st/s)
	float feed = block_data->feed;
	float rapid_feed = block_data->max_feed;
	planner_data[index].acceleration = block_data->max_accel;

	if (feed > rapid_feed)
	{
		feed = rapid_feed;
	}

	planner_data[index].feed_sqr = fast_flt_pow2(feed);
	planner_data[index].rapid_feed_sqr = fast_flt_pow2(rapid_feed);

	// consider initial angle factor of 1 (90 degree angle corner or more)
	float angle_factor = 1.0f;
	uint8_t prev = 0;

	if (!planner_buffer_is_empty())
	{
		prev = planner_buffer_prev(index); // BUFFER_PTR(planner_buffer, prev_index);
#ifdef ENABLE_LINACT_COLD_START
		if ((planner_data[prev].dirbits ^ planner_data[index].dirbits))
		{
			cos_theta = 0;
		}
#endif
	}
	else
	{
		cos_theta = 0;
	}

	cos_theta = CLAMP(0, cos_theta, 1.0f);

	// if more than one move stored cals juntion speeds and recalculates speed profiles
	if (cos_theta != 0 && !CHECKFLAG(block_data->motion_mode, PLANNER_MOTION_EXACT_STOP | MOTIONCONTROL_MODE_BACKLASH_COMPENSATION))
	{
		if (cos_theta != 1.0f)
		{
			// calculates the junction angle with previous
			if (cos_theta > 0)
			{
				// uses the half angle identity conversion to convert from cos(theta) to tan(theta/2) where:
				//	tan(theta/2) = sqrt((1-cos(theta)/(1+cos(theta))
				// to simplify the calculations it multiplies by sqrt((1+cos(theta)/(1+cos(theta))
				// transforming the equation to sqrt((1^2-cos(theta)^2))/(1+cos(theta))
				// this way the output will be between 0<tan(theta/2)<inf
				// but if theta is 0<theta<90 the tan(theta/2) will be 0<tan(theta/2)<1
				// all angles greater than 1 that can be excluded
				angle_factor = 1.0f / (1.0f + cos_theta);
				cos_theta = (1.0f - fast_flt_pow2(cos_theta));
				angle_factor *= fast_flt_sqrt(cos_theta);
			}

			// sets the maximum allowed speed at junction (if angle doesn't force a full stop)
			float factor = ((!CHECKFLAG(block_data->motion_mode, PLANNER_MOTION_CONTINUOUS)) ? 0 : g_settings.g64_angle_factor);
			angle_factor = CLAMP(0, angle_factor - factor, 1);

			if (angle_factor < 1.0f)
			{
				float junc_feed_sqr = (1 - angle_factor);
				junc_feed_sqr = fast_flt_pow2(junc_feed_sqr);
				junc_feed_sqr *= planner_data[prev].feed_sqr;
				// the maximum feed is the minimal feed between the previous feed given the angle and the current feed
				planner_data[index].entry_max_feed_sqr = MIN(planner_data[index].feed_sqr, junc_feed_sqr);
			}
		}
		else
		{
			planner_data[index].entry_max_feed_sqr = MIN(planner_data[index].feed_sqr, planner_data[prev].feed_sqr);
		}

		// forces reaclculation with the new block
		planner_recalculate();
	}

	// advances the buffer
	planner_add_block();
}

/*
	Planner buffer functions
*/

static void planner_add_block(void)
{
	uint8_t index = planner_data_write;
	uint8_t blocks = planner_data_blocks;
#if TOOL_COUNT > 0
	// planner is empty update tools with current planner values
	if (!blocks)
	{
		planner_state.spindle_speed = planner_data[index].spindle;
		planner_state.state_flags.reg = planner_data[index].planner_flags.reg;
	}
#endif

	if (++index == PLANNER_BUFFER_SIZE)
	{
		index = 0;
	}

	planner_data_write = index;
	blocks++;
	planner_data_blocks = blocks;
}

void planner_discard_block(void)
{
	uint8_t blocks = planner_data_blocks;
	if (!blocks)
	{
		return;
	}

	uint8_t index = planner_data_read;

	if (++index == PLANNER_BUFFER_SIZE)
	{
		index = 0;
	}

	blocks--;
#if TOOL_COUNT > 0
	if (blocks)
	{
		planner_state.spindle_speed = planner_data[index].spindle;
		planner_state.state_flags.reg = planner_data[index].planner_flags.reg;
	}
#endif

	planner_data_blocks = blocks;
	planner_data_read = index;
}

static uint8_t planner_buffer_next(uint8_t index)
{
	if (++index == PLANNER_BUFFER_SIZE)
	{
		index = 0;
	}

	return index;
}

static uint8_t planner_buffer_prev(uint8_t index)
{
	if (index == 0)
	{
		index = PLANNER_BUFFER_SIZE;
	}

	return --index;
}

bool planner_buffer_is_empty(void)
{
	return (!planner_data_blocks);
}

bool planner_buffer_is_full(void)
{
	return (planner_data_blocks == PLANNER_BUFFER_SIZE);
}

static void planner_buffer_clear(void)
{
	planner_data_write = 0;
	planner_data_read = 0;
	planner_data_blocks = 0;
#ifdef FORCE_GLOBALS_TO_0
	memset(planner_data, 0, sizeof(planner_data));
#endif
}

void planner_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
#if TOOL_COUNT > 0
	planner_state.planner_spindle = 0;
	planner_state.coolant = 0;
#endif
#endif
	planner_buffer_clear();
	planner_feed_ovr_reset();
	planner_rapid_feed_ovr_reset();
#if TOOL_COUNT > 0
	planner_spindle_ovr_reset();
	planner_coolant_ovr_reset();
#endif
}

void planner_clear(void)
{
	// clears all motions stored in the buffer
	planner_buffer_clear();
#if TOOL_COUNT > 0
	planner_state.spindle_speed = 0;
	planner_state.state_flags.reg = 0;
#endif
}

planner_block_t *planner_get_block(void)
{
	return &planner_data[planner_data_read];
}

float planner_get_block_exit_speed_sqr(void)
{
	// only one block in the buffer (exit speed is 0)
	if (planner_data_blocks < 2)
		return 0;

	// exit speed = next block entry speed
	uint8_t next = planner_buffer_next(planner_data_read);
	float exit_speed_sqr = planner_data[next].entry_feed_sqr;
	float rapid_feed_sqr = planner_data[next].rapid_feed_sqr;

	if (planner_data[next].planner_flags.bit.feed_override)
	{
		if (planner_state.feed_override != 100)
		{
			exit_speed_sqr *= fast_flt_pow2((float)planner_state.feed_override);
			exit_speed_sqr *= 0.0001f;
		}

		// if rapid overrides are active the feed must not exceed the rapid motion feed
		if (planner_state.rapid_feed_override != 100)
		{
			rapid_feed_sqr *= fast_flt_pow2((float)planner_state.rapid_feed_override);
			rapid_feed_sqr *= 0.0001f;
		}
	}

	return MIN(exit_speed_sqr, rapid_feed_sqr);
}

float planner_get_block_top_speed(float exit_speed_sqr)
{
	/*
	Computed the junction speed

	At full acceleration and deacceleration we have the following equations
		v_max_entry^2 = v_entry^2 + 2 * d_start * acceleration
		v_max_exit^2 = v_exit^2 + 2 * d_deaccel * acceleration

	In this case v_max_entry^2 = v_max_exit^2 at the point where

	d_deaccel = d_total - d_start;

	this translates to the equation

	v_max^2 = (v_exit^2 + 2 * acceleration * distance + v_entry)/2
	*/
	// calculates the difference between the entry speed and the exit speed
	uint8_t index = planner_data_read;
	float speed_delta = exit_speed_sqr - planner_data[index].entry_feed_sqr;
	// caclculates the speed increase/decrease for the given distance
	float junction_speed_sqr = planner_data[index].acceleration * (float)(planner_data[index].steps[planner_data[index].main_stepper]);
	junction_speed_sqr = fast_flt_mul2(junction_speed_sqr);
	// if there is enough space to accelerate computes the junction speed
	if (junction_speed_sqr >= speed_delta)
	{
		junction_speed_sqr += exit_speed_sqr + planner_data[index].entry_feed_sqr;
		junction_speed_sqr = fast_flt_div2(junction_speed_sqr);
	}
	else if (exit_speed_sqr > planner_data[index].entry_feed_sqr)
	{
		// will never reach the desired exit speed even accelerating all the way
		junction_speed_sqr += planner_data[index].entry_feed_sqr;
	}
	else
	{
		// will overshoot the desired exit speed even deaccelerating all the way
		junction_speed_sqr = planner_data[index].entry_feed_sqr;
	}

	float rapid_feed_sqr = planner_data[index].rapid_feed_sqr;
	float target_speed_sqr = planner_data[index].feed_sqr;
	if (planner_data[index].planner_flags.bit.feed_override)
	{
		if (planner_state.feed_override != 100)
		{
			target_speed_sqr *= fast_flt_pow2((float)planner_state.feed_override);
			target_speed_sqr *= 0.0001f;
		}

		// if rapid overrides are active the feed must not exceed the rapid motion feed
		if (planner_state.rapid_feed_override != 100)
		{
			rapid_feed_sqr *= fast_flt_pow2((float)planner_state.rapid_feed_override);
			rapid_feed_sqr *= 0.0001f;
		}
	}

	// can't ever exceed rapid move speed
	target_speed_sqr = MIN(target_speed_sqr, rapid_feed_sqr);
	return MIN(junction_speed_sqr, target_speed_sqr);
}

#if TOOL_COUNT > 0
int16_t planner_get_spindle_speed(float scale)
{
	if (planner_state.state_flags.bit.spindle_running)
	{
		float scaled_spindle = (float)planner_state.spindle_speed;
		bool neg = (planner_state.state_flags.bit.spindle_running == 2);

		if (g_settings.laser_mode && neg) // scales laser power only if invert is active (M4)
		{
			scaled_spindle *= scale; // scale calculated in laser mode (otherwise scale is always 1)
		}

		if (planner_data[planner_data_read].planner_flags.bit.feed_override && planner_state.spindle_speed_override != 100)
		{
			scaled_spindle = 0.01f * (float)planner_state.spindle_speed_override * scaled_spindle;
		}
		scaled_spindle = CLAMP(g_settings.spindle_min_rpm, scaled_spindle, g_settings.spindle_max_rpm);
		int16_t output = tool_range_speed(scaled_spindle);

		return (!neg) ? output : -output;
	}

	return 0;
}

float planner_get_previous_spindle_speed(void)
{
	return (float)planner_state.spindle_speed;
}

uint8_t planner_get_coolant(void)
{
	return (planner_state.state_flags.bit.coolant ^ planner_state.state_flags.bit.coolant_override);
}

uint8_t planner_get_previous_coolant(void)
{
	return planner_state.state_flags.bit.coolant;
}
#endif

static void planner_recalculate(void)
{
	uint8_t last = planner_data_write;
	uint8_t first = planner_data_read;
	uint8_t block = last;

	// starts in the last added block
	// calculates the maximum entry speed of the block so that it can do a full stop in the end
	if (planner_data_blocks < 2)
	{
		planner_data[block].entry_feed_sqr = 0;
		return;
	}
	// optimizes entry speeds given the current exit speed (backward pass)
	uint8_t next = block;
	float speedchange;

	while (!planner_data[block].planner_flags.bit.optimal && block != first)
	{
		if ((planner_data[block].entry_feed_sqr >= planner_data[block].entry_max_feed_sqr) || planner_data[block].planner_flags.bit.optimal)
		{
			// found optimal
			break;
		}
		speedchange = ((float)(planner_data[block].steps[planner_data[block].main_stepper] << 1)) * planner_data[block].acceleration;
		speedchange += (block != last) ? planner_data[next].entry_feed_sqr : 0;
		planner_data[block].entry_feed_sqr = MIN(planner_data[block].entry_max_feed_sqr, speedchange);

		next = block;
		block = planner_buffer_prev(block);
	}

	// optimizes exit speeds (forward pass)
	while (block != last)
	{
		// next block is moving at a faster speed
		if (planner_data[block].entry_feed_sqr < planner_data[next].entry_feed_sqr)
		{
			speedchange = ((float)(planner_data[block].steps[planner_data[block].main_stepper] << 1)) * planner_data[block].acceleration;
			// check if the next block entry speed can be achieved
			speedchange += planner_data[block].entry_feed_sqr;
			if (speedchange < planner_data[next].entry_feed_sqr)
			{
				// lowers next entry speed (aka exit speed) to the maximum reachable speed from current block
				// optimization achieved for this movement
				planner_data[next].entry_feed_sqr = speedchange;
				planner_data[next].planner_flags.bit.optimal = true;
			}
		}

		// if the executing block was updated then update the interpolator limits
		if (block == first)
		{
			itp_update();
		}

		block = next;
		next = planner_buffer_next(block);
	}
}

void planner_sync_tools(motion_data_t *block_data)
{
#if TOOL_COUNT > 0
	planner_state.spindle_speed = block_data->spindle;
	planner_state.state_flags.reg &= ~STATE_COPY_FLAG_MASK;
	planner_state.state_flags.reg |= (block_data->motion_flags.reg & STATE_COPY_FLAG_MASK);
#endif
}

// overrides
void planner_feed_ovr_inc(uint8_t value)
{
	uint8_t ovr_val = planner_state.feed_override;
	ovr_val += value;
	ovr_val = MAX(ovr_val, FEED_OVR_MIN);
	ovr_val = MIN(ovr_val, FEED_OVR_MAX);

	if (ovr_val != planner_state.feed_override)
	{
		planner_state.feed_override = ovr_val;
		planner_ovr_counter = 0;
		itp_update();
	}
}

void planner_rapid_feed_ovr(uint8_t value)
{
	if (planner_state.rapid_feed_override != value)
	{
		planner_state.rapid_feed_override = value;
		planner_ovr_counter = 0;
		itp_update();
	}
}

void planner_feed_ovr_reset(void)
{
	if (planner_state.feed_override != 100)
	{
		planner_state.feed_override = 100;
		planner_ovr_counter = 0;
		itp_update();
	}
}

void planner_rapid_feed_ovr_reset(void)
{
	if (planner_state.rapid_feed_override != 100)
	{
		planner_state.rapid_feed_override = 100;
		planner_ovr_counter = 0;
		itp_update();
	}
}

#if TOOL_COUNT > 0
void planner_spindle_ovr_inc(uint8_t value)
{
	uint8_t ovr_val = planner_state.spindle_speed_override;
	ovr_val += value;
	ovr_val = MAX(ovr_val, SPINDLE_OVR_MIN);
	ovr_val = MIN(ovr_val, SPINDLE_OVR_MAX);

	if (ovr_val != planner_state.spindle_speed_override)
	{
		planner_state.spindle_speed_override = ovr_val;
		planner_ovr_counter = 0;
	}
}

void planner_spindle_ovr_reset(void)
{
	planner_state.spindle_speed_override = 100;
	planner_ovr_counter = 0;
}

uint8_t planner_coolant_ovr_toggle(uint8_t value)
{
	planner_state.state_flags.bit.coolant_override ^= value;
	return planner_state.state_flags.bit.coolant_override;
}

void planner_coolant_ovr_reset(void)
{
	planner_state.state_flags.bit.coolant = 0;
	planner_state.state_flags.bit.coolant_override = 0;
}
#endif

bool planner_get_overflows(uint8_t *overflows)
{
	if (!planner_ovr_counter)
	{
		overflows[0] = planner_state.feed_override;
		overflows[1] = planner_state.rapid_feed_override;
#if TOOL_COUNT > 0
		overflows[2] = planner_state.spindle_speed_override;
#else
		overflows[2] = 0;
#endif
		planner_ovr_counter = STATUS_WCO_REPORT_MIN_FREQUENCY;
		return true;
	}

	planner_ovr_counter--;
	return false;
}

uint8_t planner_get_buffer_freeblocks()
{
	return PLANNER_BUFFER_SIZE - planner_data_blocks;
}
