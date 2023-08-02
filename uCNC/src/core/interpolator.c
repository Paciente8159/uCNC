/*
	Name: interpolator.c
	Description: Implementation of a linear acceleration interpolator for µCNC.
		The linear acceleration interpolator generates step profiles with constant acceleration.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 13/10/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <float.h>

#ifndef INTERPOLATOR_BUFFER_SIZE
#define INTERPOLATOR_BUFFER_SIZE 5 // number of windows in the buffer
#endif

// sets the sample frequency for the Riemann sum integral
#ifndef INTERPOLATOR_FREQ
#define INTERPOLATOR_FREQ 100
#endif

#define INTERPOLATOR_DELTA_T (1.0f / INTERPOLATOR_FREQ)
// determines the size of the maximum riemann sample that can be performed taking in acount the maximum allowable step rate
#define INTERPOLATOR_DELTA_CONST_T (MIN((1.0f / INTERPOLATOR_BUFFER_SIZE), ((float)(0xFFFF >> DSS_MAX_OVERSAMPLING) / (float)F_STEP_MAX)))
#define INTERPOLATOR_FREQ_CONST (1.0f / INTERPOLATOR_DELTA_CONST_T)

// circular buffers
// creates new type PULSE_BLOCK_BUFFER
static itp_block_t itp_blk_data[INTERPOLATOR_BUFFER_SIZE];
static uint8_t itp_blk_data_write;

static itp_segment_t itp_sgm_data[INTERPOLATOR_BUFFER_SIZE];
static volatile uint8_t itp_sgm_data_write;
static volatile uint8_t itp_sgm_data_read;
// static buffer_t itp_sgm_buffer;

static planner_block_t *itp_cur_plan_block;

// keeps track of the machine realtime position
static int32_t itp_rt_step_pos[STEPPER_COUNT];
// flag to force the interpolator to recalc entry and exit limit position of acceleration/deacceleration curves
static bool itp_needs_update;
#if DSS_MAX_OVERSAMPLING > 0
// stores the previous dss setting used by the interpolator
static uint8_t prev_dss;
#endif
static int16_t prev_spindle;
// pointer to the segment being executed
static itp_segment_t *itp_rt_sgm;
#if (defined(ENABLE_DUAL_DRIVE_AXIS) || defined(IS_DELTA_KINEMATICS))
static volatile uint8_t itp_step_lock;
#endif

#ifdef ENABLE_RT_SYNC_MOTIONS
// deprecated with new hooks
// volatile int32_t itp_sync_step_counter;

void itp_update_feed(float feed)
{
	planner_block_t *p = planner_get_block();
	p->feed_sqr = feed * feed;
	itp_needs_update = true;
	uint16_t ticks, presc;
	mcu_freq_to_clocks(feed, &ticks, &presc);
	for (uint8_t i = 0; i < INTERPOLATOR_BUFFER_SIZE; i++)
	{
		itp_sgm_data[i].timer_counter = ticks;
		itp_sgm_data[i].timer_prescaller = presc;
		// mark for update
		itp_sgm_data[i].flags |= ITP_UPDATE_ISR;
	}
}

bool itp_sync_ready(void)
{
	__ATOMIC__
	{
		if (itp_rt_sgm)
		{
			return ((itp_rt_sgm->flags & (ITP_SYNC | ITP_CONST)) == (ITP_SYNC | ITP_CONST));
		}
	}

	return false;
}

CREATE_HOOK(itp_rt_pre_stepbits);
CREATE_HOOK(itp_rt_stepbits);
#endif

static void itp_sgm_buffer_read(void);
static void itp_sgm_buffer_write(void);
FORCEINLINE static bool itp_sgm_is_full(void);
FORCEINLINE static bool itp_sgm_is_empty(void);
/*FORCEINLINE*/ static void itp_sgm_clear(void);
FORCEINLINE static void itp_blk_buffer_write(void);
static void itp_blk_clear(void);
// FORCEINLINE static void itp_nomotion(uint8_t type, uint16_t delay);

/*
	Interpolator segment buffer functions
*/
static void itp_sgm_buffer_read(void)
{
	uint8_t read;

	read = itp_sgm_data_read;
	if (read == itp_sgm_data_write)
	{
		return;
	}

	if (++read == INTERPOLATOR_BUFFER_SIZE)
	{
		read = 0;
	}

	itp_sgm_data_read = read;
}

static void itp_sgm_buffer_write(void)
{
	uint8_t write;

	write = itp_sgm_data_write;

	if (++write == INTERPOLATOR_BUFFER_SIZE)
	{
		write = 0;
	}

	if (itp_sgm_data_read != write)
	{
		itp_sgm_data_write = write;
	}
}

static bool itp_sgm_is_full(void)
{
	uint8_t write, read;

	write = itp_sgm_data_write;
	read = itp_sgm_data_read;

	if (++write == INTERPOLATOR_BUFFER_SIZE)
	{
		write = 0;
	}
	return (write == read);
}

static bool itp_sgm_is_empty(void)
{
	return (itp_sgm_data_read == itp_sgm_data_write);
}

static void itp_sgm_clear(void)
{
	itp_sgm_data_write = 0;
	itp_sgm_data_read = 0;
	// resets the sgm pointer and stored dss
	itp_rt_sgm = NULL;
#if DSS_MAX_OVERSAMPLING > 0
	prev_dss = 0;
#endif
	prev_spindle = 0;
	memset(itp_sgm_data, 0, sizeof(itp_sgm_data));
}

static void itp_blk_buffer_write(void)
{
	// curcular always. No need to control override
	if (++itp_blk_data_write == INTERPOLATOR_BUFFER_SIZE)
	{
		itp_blk_data_write = 0;
	}
}

static void itp_blk_clear(void)
{
	itp_blk_data_write = 0;
	memset(itp_blk_data, 0, sizeof(itp_blk_data));
}

/*
	Interpolator functions
*/
// declares functions called by the stepper ISR
void itp_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
	// resets buffers
	memset(itp_rt_step_pos, 0, sizeof(itp_rt_step_pos));
	memset(itp_blk_data, 0, sizeof(itp_blk_data));
	memset(itp_sgm_data, 0, sizeof(itp_sgm_data));
	itp_rt_sgm = NULL;
#if (defined(ENABLE_DUAL_DRIVE_AXIS) || defined(IS_DELTA_KINEMATICS))
	itp_step_lock = 0;
#endif
#endif
	itp_cur_plan_block = NULL;
	itp_needs_update = false;
	// initialize circular buffers
	itp_blk_clear();
	itp_sgm_clear();
}

#if S_CURVE_ACCELERATION_LEVEL != 0
// evals the point in a s-curve function
// receives a value between 0 and 1
// outputs a value along a curve according to the scale
static float s_curve_function(float pt)
{
#if S_CURVE_ACCELERATION_LEVEL == 5
	return 0.5 * (tanh(6 * pt - 3) + 1);
#elif S_CURVE_ACCELERATION_LEVEL == 4
	// from this https://forum.duet3d.com/topic/4802/6th-order-jerk-controlled-motion-planning/95
	float pt_sqr = fast_flt_pow2(pt);
	float k = 3.0f * (pt_sqr - 2.5f * pt) + 5.0f;
	k = fast_flt_mul2(k) * pt_sqr * pt;
	return k;
#elif S_CURVE_ACCELERATION_LEVEL == 3
	// from this https://en.wikipedia.org/wiki/Sigmoid_function
	pt -= 0.5f;
	// optimized fast inverse aproximation
	float k = (0.15f + ABS(pt));
	int32_t *i = (int32_t *)&k;
	*i = 0x7EEF1AA0 - *i;
	k = (0.65f * pt * k + 0.5f);
	return CLAMP(0, k, 1);
#elif S_CURVE_ACCELERATION_LEVEL == 2
	// from this https://en.wikipedia.org/wiki/Sigmoid_function
	pt -= 0.5f;
	// optimized fast inverse aproximation
	float k = (0.25f + ABS(pt));
	int32_t *i = (int32_t *)&k;
	*i = 0x7EEF1AA0 - *i;
	k = (0.75f * pt * k + 0.5f);
	return CLAMP(0, k, 1);
#elif S_CURVE_ACCELERATION_LEVEL == 1
	// from this https://en.wikipedia.org/wiki/Sigmoid_function
	pt -= 0.5f;
	// optimized fast inverse aproximation
	float k = (0.5f + ABS(pt));
	int32_t *i = (int32_t *)&k;
	*i = 0x7EEF1AA0 - *i;
	k = (pt * k + 0.5f);
	return CLAMP(0, k, 1);
#elif S_CURVE_ACCELERATION_LEVEL == -1
	float k, pt_sqr;
	int32_t *i;
	switch (g_settings.s_curve_profile)
	{
	case 1:
		// from this https://en.wikipedia.org/wiki/Sigmoid_function
		pt -= 0.5f;
		// optimized fast inverse aproximation
		k = (0.5f + ABS(pt));
		i = (int32_t *)&k;
		*i = 0x7EEF1AA0 - *i;
		k = (pt * k + 0.5f);
		return CLAMP(0, k, 1);
	case 2:
		// from this https://en.wikipedia.org/wiki/Sigmoid_function
		pt -= 0.5f;
		// optimized fast inverse aproximation
		k = (0.25f + ABS(pt));
		i = (int32_t *)&k;
		*i = 0x7EEF1AA0 - *i;
		k = (0.75f * pt * k + 0.5f);
		return CLAMP(0, k, 1);
	case 3:
		// from this https://en.wikipedia.org/wiki/Sigmoid_function
		pt -= 0.5f;
		// optimized fast inverse aproximation
		k = (0.15f + ABS(pt));
		i = (int32_t *)&k;
		*i = 0x7EEF1AA0 - *i;
		k = (0.65f * pt * k + 0.5f);
		return CLAMP(0, k, 1);
	case 4:
		// from this https://forum.duet3d.com/topic/4802/6th-order-jerk-controlled-motion-planning/95
		pt_sqr = fast_flt_pow2(pt);
		k = 3.0f * (pt_sqr - 2.5f * pt) + 5.0f;
		k = fast_flt_mul2(k) * pt_sqr * pt;
		return k;
	case 5:
		return 0.5 * (tanh(6 * pt - 3) + 1);
	default:
		// defaults to linear
		return pt;
	}
#endif
}
#endif

void itp_run(void)
{
	// conversion vars
	static uint32_t accel_until = 0;
	static uint32_t deaccel_from = 0;
	static float junction_speed = 0;
	static float feed_convert = 0;
	static float partial_distance = 0;
	static float t_acc_integrator = 0;
	static float t_deac_integrator = 0;
#if S_CURVE_ACCELERATION_LEVEL != 0
	static float acc_step = 0;
	static float acc_step_acum = 0;
	static float acc_scale = 0;
	static float acc_init_speed = 0;

	static float deac_step = 0;
	static float deac_step_acum = 0;
	static float deac_scale = 0;

#endif

	itp_segment_t *sgm = NULL;
	bool start_is_synched = false;

	// creates segments and fills the buffer
	while (!itp_sgm_is_full())
	{
		if (cnc_get_exec_state(EXEC_ALARM))
		{
			// on any active alarm exits
			return;
		}

		// no planner blocks has beed processed or last planner block was fully processed
		if (itp_cur_plan_block == NULL)
		{
			// planner is empty or interpolator block buffer full. Nothing to be done
			// itp block will never be full if itp segment is not full
			if (planner_buffer_is_empty() /* || itp_blk_is_full()*/)
			{
				break;
			}
			// get the first block in the planner
			itp_cur_plan_block = planner_get_block();
			// clear the data block
			memset(&itp_blk_data[itp_blk_data_write], 0, sizeof(itp_block_t));
#ifdef GCODE_PROCESS_LINE_NUMBERS
			itp_blk_data[itp_blk_data_write].line = itp_cur_plan_block->line;
#endif

// overwrites previous values
#ifdef ENABLE_BACKLASH_COMPENSATION
			itp_blk_data[itp_blk_data_write].backlash_comp = itp_cur_plan_block->planner_flags.bit.backlash_comp;
#endif
			itp_blk_data[itp_blk_data_write].dirbits = itp_cur_plan_block->dirbits;
#ifdef ENABLE_DUAL_DRIVE_AXIS
#ifdef DUAL_DRIVE0_AXIS
			itp_blk_data[itp_blk_data_write].dirbits |= CHECKFLAG(itp_blk_data[itp_blk_data_write].dirbits, STEP_DUAL0) ? STEP_DUAL0_MASK : 0;
#endif
#ifdef DUAL_DRIVE1_AXIS
			itp_blk_data[itp_blk_data_write].dirbits |= CHECKFLAG(itp_blk_data[itp_blk_data_write].dirbits, STEP_DUAL1) ? STEP_DUAL1_MASK : 0;
#endif
#ifdef DUAL_DRIVE2_AXIS
			itp_blk_data[itp_blk_data_write].dirbits |= CHECKFLAG(itp_blk_data[itp_blk_data_write].dirbits, STEP_DUAL2) ? STEP_DUAL2_MASK : 0;
#endif
#ifdef DUAL_DRIVE3_AXIS
			itp_blk_data[itp_blk_data_write].dirbits |= CHECKFLAG(itp_blk_data[itp_blk_data_write].dirbits, STEP_DUAL3) ? STEP_DUAL3_MASK : 0;
#endif
#endif
			step_t total_steps = itp_cur_plan_block->steps[itp_cur_plan_block->main_stepper];
			itp_blk_data[itp_blk_data_write].total_steps = total_steps << 1;

			feed_convert = itp_cur_plan_block->feed_conversion;

#ifdef STEP_ISR_SKIP_IDLE
			itp_blk_data[itp_blk_data_write].idle_axis = 0;
#endif
#ifdef STEP_ISR_SKIP_MAIN
			itp_blk_data[itp_blk_data_write].main_stepper = itp_cur_plan_block->main_stepper;
#endif
			for (uint8_t i = 0; i < STEPPER_COUNT; i++)
			{
				itp_blk_data[itp_blk_data_write].errors[i] = total_steps;
				itp_blk_data[itp_blk_data_write].steps[i] = itp_cur_plan_block->steps[i] << 1;
#ifdef STEP_ISR_SKIP_IDLE
				if (!itp_cur_plan_block->steps[i])
				{
					itp_blk_data[itp_blk_data_write].idle_axis |= (1 << i);
				}
#endif
			}

			// flags block for recalculation of speeds
			itp_needs_update = true;

			// checks for synched motion
			if (itp_cur_plan_block->planner_flags.bit.synched)
			{
				start_is_synched = true;
			}
		}

		uint32_t remaining_steps = itp_cur_plan_block->steps[itp_cur_plan_block->main_stepper];

		sgm = &itp_sgm_data[itp_sgm_data_write];

		// clear the data segment
		memset(sgm, 0, sizeof(itp_segment_t));
		sgm->block = &itp_blk_data[itp_blk_data_write];

		float current_speed = fast_flt_sqrt(itp_cur_plan_block->entry_feed_sqr);

		// if an hold is active forces to deaccelerate
		if (cnc_get_exec_state(EXEC_HOLD))
		{
			// forces deacceleration by overriding the profile juntion points
			accel_until = remaining_steps;
			deaccel_from = remaining_steps;
			itp_needs_update = true;
		}
		else if (itp_needs_update) // forces recalculation of acceleration and deacceleration profiles
		{
			itp_needs_update = false;
			float exit_speed_sqr = planner_get_block_exit_speed_sqr();
			float junction_speed_sqr = planner_get_block_top_speed(exit_speed_sqr);

			junction_speed = fast_flt_sqrt(junction_speed_sqr);
			float accel_inv = fast_flt_inv(itp_cur_plan_block->acceleration);

			accel_until = remaining_steps;
			deaccel_from = 0;
			if (junction_speed_sqr != itp_cur_plan_block->entry_feed_sqr)
			{
				float accel_dist = ABS(junction_speed_sqr - itp_cur_plan_block->entry_feed_sqr) * accel_inv;
				accel_dist = fast_flt_div2(accel_dist);
				accel_until -= floorf(accel_dist);
				float t = ABS(junction_speed - current_speed);
#if S_CURVE_ACCELERATION_LEVEL != 0
				acc_scale = t;
				acc_step_acum = 0;
				acc_init_speed = current_speed;
#endif
				t *= accel_inv;
				// slice up time in an integral number of periods (half with positive jerk and half with negative)
				float slices_inv = fast_flt_inv(ceilf(INTERPOLATOR_FREQ * t));
				t_acc_integrator = t * slices_inv;
#if S_CURVE_ACCELERATION_LEVEL != 0
				acc_step = slices_inv;
#endif
				if ((junction_speed_sqr < itp_cur_plan_block->entry_feed_sqr))
				{
					t_acc_integrator = -t_acc_integrator;
				}
			}

			// if entry speed already a junction speed updates it.
			if (accel_until == remaining_steps)
			{
				itp_cur_plan_block->entry_feed_sqr = junction_speed_sqr;
			}

			if (junction_speed_sqr > exit_speed_sqr)
			{
				float deaccel_dist = (junction_speed_sqr - exit_speed_sqr) * accel_inv;
				deaccel_dist = fast_flt_div2(deaccel_dist);
				deaccel_from = floorf(deaccel_dist);
				// same as before t can be calculated using the normal ramp equation
				float t = ABS(junction_speed - fast_flt_sqrt(exit_speed_sqr));
#if S_CURVE_ACCELERATION_LEVEL != 0
				deac_scale = t;
				deac_step_acum = 0;
#endif
				t *= accel_inv;
				// slice up time in an integral number of periods (half with positive jerk and half with negative)
				float slices_inv = fast_flt_inv(ceilf(INTERPOLATOR_FREQ * t));
				t_deac_integrator = t * slices_inv;

#if S_CURVE_ACCELERATION_LEVEL != 0
				deac_step = slices_inv;
#endif
			}
		}

		float speed_change;
		float profile_steps_limit;
		float integrator;
		// acceleration profile
		if (remaining_steps > accel_until)
		{
			/*
				computes the traveled distance within a fixed amount of time
				this time is the reverse integrator frequency (INTERPOLATOR_DELTA_T)
				for constant acceleration or deceleration the traveled distance will be equal
				to the same distance traveled at a constant speed given that
				constant_speed = 0.5 * (final_speed - initial_speed) + initial_speed
				where
				(final_speed - initial_speed) = acceleration * INTERPOLATOR_DELTA_T;
			*/
			integrator = t_acc_integrator;
#if S_CURVE_ACCELERATION_LEVEL != 0
			float acum = acc_step_acum;
			acum += acc_step;
			acc_step_acum = MIN(acum, 0.999f);
			float new_speed = acc_scale * s_curve_function(acum) + acc_init_speed;
			new_speed = (t_acc_integrator >= 0) ? (new_speed + acc_init_speed) : (acc_init_speed - new_speed);
			speed_change = new_speed - current_speed;
#else
			speed_change = integrator * itp_cur_plan_block->acceleration;
#endif

			profile_steps_limit = accel_until;
			sgm->flags = ITP_UPDATE_ISR | ITP_ACCEL;
		}
		else if (remaining_steps > deaccel_from)
		{
			// constant speed segment
			speed_change = 0;
			profile_steps_limit = deaccel_from;
			integrator = INTERPOLATOR_DELTA_T;
			sgm->flags = (remaining_steps == accel_until) ? (ITP_UPDATE_ISR | ITP_CONST) : ITP_CONST;
		}
		else
		{
			integrator = t_deac_integrator;
#if S_CURVE_ACCELERATION_LEVEL != 0
			float acum = deac_step_acum;
			acum += deac_step;
			deac_step_acum = MIN(acum, 0.999f);
			float new_speed = junction_speed - deac_scale * s_curve_function(acum);
			speed_change = new_speed - current_speed;
#else
			speed_change = -(integrator * itp_cur_plan_block->acceleration);
#endif
			profile_steps_limit = 0;
			sgm->flags = ITP_UPDATE_ISR | ITP_DEACCEL;
		}

		// update speed at the end of segment
		if (speed_change)
		{
			itp_cur_plan_block->entry_feed_sqr = MAX(0, fast_flt_pow2((current_speed + speed_change)));
		}

		/*
			common calculations for all three profiles (accel, constant and deaccel)
		*/
		current_speed += fast_flt_div2(speed_change);

		if (current_speed <= 0)
		{
			if (cnc_get_exec_state(EXEC_HOLD))
			{
				return;
			}

			// speed can't be negative
			current_speed = 0;
		}

		partial_distance += current_speed * integrator;

		// computes how many steps it will perform at this speed and frame window
		uint16_t segm_steps = (uint16_t)floorf(partial_distance);
		partial_distance -= segm_steps;

		// if computed steps exceed the remaining steps for the motion shortens the distance
		if (segm_steps > (remaining_steps - profile_steps_limit))
		{
			segm_steps = (uint16_t)(remaining_steps - profile_steps_limit);
		}

// The DSS (Dynamic Step Spread) algorithm reduces stepper vibration by spreading step distribution at lower speads.
// This is done by oversampling the Bresenham line algorithm by multiple factors of 2.
// This way stepping actions fire in different moments in order to reduce vibration caused by the stepper internal mechanics.
// This works in a similar way to Grbl's AMASS but has a modified implementation to minimize the processing penalty on the ISR and also take less static memory.
// DSS never loads the step generating ISR with a frequency above half of the absolute maximum frequency
#if (DSS_MAX_OVERSAMPLING != 0)
		float dss_speed = current_speed;
		uint8_t dss = 0;
		while (dss_speed < DSS_CUTOFF_FREQ && dss < DSS_MAX_OVERSAMPLING && segm_steps)
		{
			dss_speed = fast_flt_mul2(dss_speed);
			dss++;
		}

		if (dss != prev_dss)
		{
			sgm->flags = ITP_UPDATE_ISR;
		}
		sgm->next_dss = dss - prev_dss;
		prev_dss = dss;

		// completes the segment information (step speed, steps) and updates the block
		sgm->remaining_steps = segm_steps << dss;
		dss_speed = MIN(dss_speed, g_settings.max_step_rate);
		mcu_freq_to_clocks(dss_speed, &(sgm->timer_counter), &(sgm->timer_prescaller));
#else
		sgm->remaining_steps = segm_steps;
		current_speed = MIN(current_speed, g_settings.max_step_rate);
		mcu_freq_to_clocks(current_speed, &(sgm->timer_counter), &(sgm->timer_prescaller));
#endif

		sgm->feed = current_speed * feed_convert;
#if TOOL_COUNT > 0
		// calculates dynamic laser power
		if (g_settings.laser_mode == LASER_PWM_MODE)
		{
			float top_speed_inv = fast_flt_invsqrt(itp_cur_plan_block->feed_sqr);
			int16_t newspindle = planner_get_spindle_speed(MIN(1, current_speed * top_speed_inv));

			if ((prev_spindle != newspindle))
			{
				prev_spindle = newspindle;
				sgm->flags |= ITP_UPDATE_TOOL;
			}

			sgm->spindle = newspindle;
		}
#ifdef ENABLE_LASER_PPI
		else if (g_settings.laser_mode & (LASER_PPI_VARPOWER_MODE | LASER_PPI_MODE))
		{
			int16_t newspindle;
			if (g_settings.laser_mode & LASER_PPI_VARPOWER_MODE)
			{
				float new_s = (float)ABS(planner_get_spindle_speed(1));
				new_s /= (float)g_settings.spindle_max_rpm;
				if (g_settings.laser_mode & LASER_PPI_MODE)
				{
					float blend = g_settings.laser_ppi_mixmode_uswidth;
					new_s = (new_s * blend) + (1.0f - blend);
				}

				newspindle = (int16_t)((float)g_settings.laser_ppi_uswidth * new_s);
				sgm->spindle = newspindle;
			}
			else
			{
				newspindle = g_settings.laser_ppi_uswidth;
				sgm->spindle = newspindle;
			}

			if ((prev_spindle != (int16_t)newspindle) && newspindle)
			{
				prev_spindle = (int16_t)newspindle;
				sgm->flags |= ITP_UPDATE_TOOL;
			}
		}
#endif
#endif
		remaining_steps -= segm_steps;

		if (remaining_steps == accel_until) // resets float additions error
		{
			itp_cur_plan_block->entry_feed_sqr = fast_flt_pow2(junction_speed);
		}

		itp_cur_plan_block->steps[itp_cur_plan_block->main_stepper] = remaining_steps;

		// checks for synched motion
		if (itp_cur_plan_block->planner_flags.bit.synched)
		{
			sgm->flags |= ITP_SYNC;
		}

		if (remaining_steps == 0)
		{
			itp_blk_buffer_write();
			itp_cur_plan_block = NULL;
			planner_discard_block(); // discards planner block
#if (DSS_MAX_OVERSAMPLING != 0)
			prev_dss = 0;
#endif
			// accel_profile = 0; //no updates necessary to planner
			// break;
		}

		// finally write the segment
		itp_sgm_buffer_write();
	}
#if TOOL_COUNT > 0
	// updated the coolant pins
	tool_set_coolant(planner_get_coolant());
#endif

	// starts the step isr if is stopped and there are segments to execute
	itp_start(start_is_synched);
}

void itp_update(void)
{
	// flags executing block for update
	itp_needs_update = true;
}

void itp_stop(void)
{
	uint8_t state = cnc_get_exec_state(EXEC_ALLACTIVE);
	// any stop command while running triggers an HALT alarm
	if (state & EXEC_RUN)
	{
		cnc_set_exec_state(EXEC_UNHOMED);
	}

	// end of JOG
	if (CHECKFLAG(state, (EXEC_JOG | EXEC_HOLD)) == EXEC_JOG)
	{
		if (itp_is_empty() && planner_buffer_is_empty())
		{
			cnc_clear_exec_state(EXEC_JOG);
		}
	}

	io_set_steps(g_settings.step_invert_mask);
#if TOOL_COUNT > 0
	if (g_settings.laser_mode)
	{
		tool_set_speed(0);
	}
#endif

	mcu_stop_itp_isr();
	cnc_clear_exec_state(EXEC_RUN);
}

void itp_stop_tools(void)
{
	tool_stop();
}

void itp_clear(void)
{
	itp_cur_plan_block = NULL;
	itp_blk_clear();
	itp_sgm_clear();
}

void itp_get_rt_position(int32_t *position)
{
	memcpy(position, itp_rt_step_pos, sizeof(itp_rt_step_pos));

#if STEPPERS_ENCODERS_MASK != 0
#if (defined(STEP0_ENCODER) && AXIS_TO_STEPPERS > 0)
	itp_rt_step_pos[0] = encoder_get_position(STEP0_ENCODER);
#endif
#if (defined(STEP1_ENCODER) && AXIS_TO_STEPPERS > 1)
	itp_rt_step_pos[1] = encoder_get_position(STEP1_ENCODER);
#endif
#if (defined(STEP2_ENCODER) && AXIS_TO_STEPPERS > 2)
	itp_rt_step_pos[2] = encoder_get_position(STEP2_ENCODER);
#endif
#if (defined(STEP3_ENCODER) && AXIS_TO_STEPPERS > 3)
	itp_rt_step_pos[3] = encoder_get_position(STEP3_ENCODER);
#endif
#if (defined(STEP4_ENCODER) && AXIS_TO_STEPPERS > 4)
	itp_rt_step_pos[4] = encoder_get_position(STEP4_ENCODER);
#endif
#if (defined(STEP5_ENCODER) && AXIS_TO_STEPPERS > 5)
	itp_rt_step_pos[5] = encoder_get_position(STEP5_ENCODER);
#endif
#endif
}

int32_t itp_get_rt_position_index(int8_t index)
{
	__ATOMIC__
	{
		return itp_rt_step_pos[index];
	}

	return 0;
}

void itp_reset_rt_position(float *origin)
{
	if (!g_settings.homing_enabled)
	{
		memset(origin, 0, (sizeof(float) * AXIS_COUNT));
	}

	// sync origin and steppers position
	kinematics_apply_inverse(origin, itp_rt_step_pos);

#if STEPPERS_ENCODERS_MASK != 0
	encoders_itp_reset_rt_position(origin);
#endif
}

float itp_get_rt_feed(void)
{
	float feed = 0;
	if (!cnc_get_exec_state(EXEC_RUN))
	{
		return feed;
	}

	if (!itp_sgm_is_empty())
	{
		feed = itp_sgm_data[itp_sgm_data_read].feed;
	}

	return feed;
}

bool itp_is_empty(void)
{
	return (itp_sgm_is_empty() && (itp_rt_sgm == NULL));
}

// flushes all motions from all systems (planner or interpolator)
// used to make a sync motion
uint8_t itp_sync(void)
{
	while (!itp_is_empty() || !planner_buffer_is_empty())
	{
		if (!cnc_dotasks())
		{
			if (cnc_get_exec_state(EXEC_HOMING_HIT) == EXEC_HOMING_HIT)
			{
				break;
			}
			return STATUS_CRITICAL_FAIL;
		}
	}

	return STATUS_OK;
}

// sync spindle in a stopped motion
void itp_sync_spindle(void)
{
#if TOOL_COUNT > 0
	tool_set_speed(planner_get_spindle_speed(0));
#endif
}

#if (defined(ENABLE_DUAL_DRIVE_AXIS) || defined(IS_DELTA_KINEMATICS))
void itp_lock_stepper(uint8_t lockmask)
{
	itp_step_lock = lockmask;
}
#endif

#ifdef GCODE_PROCESS_LINE_NUMBERS
uint32_t itp_get_rt_line_number(void)
{
	return ((itp_sgm_data[itp_sgm_data_read].block != NULL) ? itp_sgm_data[itp_sgm_data_read].block->line : 0);
}
#endif

// always fires after pulse
MCU_CALLBACK void mcu_step_reset_cb(void)
{
	// always resets all stepper pins
	io_set_steps(g_settings.step_invert_mask);
}

MCU_CALLBACK void mcu_step_cb(void)
{
	static uint8_t stepbits = 0;
	static bool itp_busy = false;

#ifdef RT_STEP_PREVENT_CONDITION
	if (RT_STEP_PREVENT_CONDITION)
	{
		return;
	}
#endif

	if (itp_busy) // prevents reentrancy
	{
		return;
	}

	if (itp_rt_sgm != NULL)
	{
		if (itp_rt_sgm->flags & ITP_UPDATE)
		{
			if (itp_rt_sgm->flags & ITP_UPDATE_ISR)
			{
				mcu_change_itp_isr(itp_rt_sgm->timer_counter, itp_rt_sgm->timer_prescaller);
			}

#if TOOL_COUNT > 0
			if (itp_rt_sgm->flags & ITP_UPDATE_TOOL)
			{
				tool_set_speed(itp_rt_sgm->spindle);
			}
#endif
			itp_rt_sgm->flags &= ~(ITP_UPDATE);
		}

		// no step remaining discards current segment
		if (!itp_rt_sgm->remaining_steps)
		{
			itp_rt_sgm->block = NULL;
			itp_rt_sgm = NULL;
			itp_sgm_buffer_read();
		}
	}

	uint8_t new_stepbits = stepbits;
	io_toggle_steps(new_stepbits);
	// sets step bits
#ifdef ENABLE_RT_SYNC_MOTIONS
	HOOK_INVOKE(itp_rt_stepbits, new_stepbits, itp_rt_sgm->flags);
#endif

	// if buffer empty loads one
	if (itp_rt_sgm == NULL)
	{
		// if buffer is not empty
		if (!itp_sgm_is_empty())
		{
			// loads a new segment
			itp_rt_sgm = &itp_sgm_data[itp_sgm_data_read];
			cnc_set_exec_state(EXEC_RUN);
			if (itp_rt_sgm->block != NULL)
			{
#if (DSS_MAX_OVERSAMPLING != 0)
				if (itp_rt_sgm->next_dss != 0)
				{
#ifdef STEP_ISR_SKIP_MAIN
					itp_rt_sgm->block->main_stepper = 255; // disables direct step increment to force step calculation
#endif
					uint8_t dss;
					if (itp_rt_sgm->next_dss > 0)
					{
						dss = itp_rt_sgm->next_dss;
						itp_rt_sgm->block->total_steps <<= dss;
#if (STEPPER_COUNT > 0)
						itp_rt_sgm->block->errors[0] <<= dss;
#endif
#if (STEPPER_COUNT > 1)
						itp_rt_sgm->block->errors[1] <<= dss;
#endif
#if (STEPPER_COUNT > 2)
						itp_rt_sgm->block->errors[2] <<= dss;
#endif
#if (STEPPER_COUNT > 3)
						itp_rt_sgm->block->errors[3] <<= dss;
#endif
#if (STEPPER_COUNT > 4)
						itp_rt_sgm->block->errors[4] <<= dss;
#endif
#if (STEPPER_COUNT > 5)
						itp_rt_sgm->block->errors[5] <<= dss;
#endif
					}
					else
					{
						dss = -itp_rt_sgm->next_dss;
						itp_rt_sgm->block->total_steps >>= dss;
#if (STEPPER_COUNT > 0)
						itp_rt_sgm->block->errors[0] >>= dss;
#endif
#if (STEPPER_COUNT > 1)
						itp_rt_sgm->block->errors[1] >>= dss;
#endif
#if (STEPPER_COUNT > 2)
						itp_rt_sgm->block->errors[2] >>= dss;
#endif
#if (STEPPER_COUNT > 3)
						itp_rt_sgm->block->errors[3] >>= dss;
#endif
#if (STEPPER_COUNT > 4)
						itp_rt_sgm->block->errors[4] >>= dss;
#endif
#if (STEPPER_COUNT > 5)
						itp_rt_sgm->block->errors[5] >>= dss;
#endif
					}
				}
#endif
				// set dir pins for current
				io_set_dirs(itp_rt_sgm->block->dirbits);
			}
		}
		else
		{
			cnc_clear_exec_state(EXEC_RUN); // this naturally clears the RUN flag. Any other ISR stop does not clear the flag.
			itp_stop();						// the buffer is empty. The ISR can stop
			return;
		}
	}

/*
Must put this on G33 module
#ifdef ENABLE_RT_SYNC_MOTIONS
	if (new_stepbits && (itp_rt_sgm->flags & ITP_SYNC))
	{
		itp_sync_step_counter++;
	}
#endif
*/

	new_stepbits = 0;
	itp_busy = true;
	mcu_enable_global_isr();

	// steps remaining starts calc next step bits
	if (itp_rt_sgm->remaining_steps)
	{
		if (itp_rt_sgm->block != NULL)
		{
// prepares the next step bits mask
#if (STEPPER_COUNT > 0)
#ifdef STEP_ISR_SKIP_MAIN
			if (itp_rt_sgm->block->main_stepper == 0)
			{
				new_stepbits |= STEP0_ITP_MASK;
			}
			else
			{
#endif
#ifdef STEP_ISR_SKIP_IDLE
				if (!(itp_rt_sgm->block->idle_axis & STEP0_MASK))
				{
#endif
					itp_rt_sgm->block->errors[0] += itp_rt_sgm->block->steps[0];
					if (itp_rt_sgm->block->errors[0] > itp_rt_sgm->block->total_steps)
					{
						itp_rt_sgm->block->errors[0] -= itp_rt_sgm->block->total_steps;
						new_stepbits |= STEP0_ITP_MASK;
					}
#ifdef STEP_ISR_SKIP_IDLE
				}
#endif
#ifdef STEP_ISR_SKIP_MAIN
			}
#endif
#endif
#if (STEPPER_COUNT > 1)
#ifdef STEP_ISR_SKIP_MAIN
			if (itp_rt_sgm->block->main_stepper == 1)
			{
				new_stepbits |= STEP1_ITP_MASK;
			}
			else
			{
#endif
#ifdef STEP_ISR_SKIP_IDLE
				if (!(itp_rt_sgm->block->idle_axis & STEP1_MASK))
				{
#endif
					itp_rt_sgm->block->errors[1] += itp_rt_sgm->block->steps[1];
					if (itp_rt_sgm->block->errors[1] > itp_rt_sgm->block->total_steps)
					{
						itp_rt_sgm->block->errors[1] -= itp_rt_sgm->block->total_steps;
						new_stepbits |= STEP1_ITP_MASK;
					}
#ifdef STEP_ISR_SKIP_IDLE
				}
#endif
#ifdef STEP_ISR_SKIP_MAIN
			}
#endif
#endif
#if (STEPPER_COUNT > 2)
#ifdef STEP_ISR_SKIP_MAIN
			if (itp_rt_sgm->block->main_stepper == 2)
			{
				new_stepbits |= STEP2_ITP_MASK;
			}
			else
			{
#endif
#ifdef STEP_ISR_SKIP_IDLE
				if (!(itp_rt_sgm->block->idle_axis & STEP2_MASK))
				{
#endif
					itp_rt_sgm->block->errors[2] += itp_rt_sgm->block->steps[2];
					if (itp_rt_sgm->block->errors[2] > itp_rt_sgm->block->total_steps)
					{
						itp_rt_sgm->block->errors[2] -= itp_rt_sgm->block->total_steps;
						new_stepbits |= STEP2_ITP_MASK;
					}
#ifdef STEP_ISR_SKIP_IDLE
				}
#endif
#ifdef STEP_ISR_SKIP_MAIN
			}
#endif
#endif
#if (STEPPER_COUNT > 3)
#ifdef STEP_ISR_SKIP_MAIN
			if (itp_rt_sgm->block->main_stepper == 3)
			{
				new_stepbits |= STEP3_ITP_MASK;
			}
			else
			{
#endif
#ifdef STEP_ISR_SKIP_IDLE
				if (!(itp_rt_sgm->block->idle_axis & STEP3_MASK))
				{
#endif
					itp_rt_sgm->block->errors[3] += itp_rt_sgm->block->steps[3];
					if (itp_rt_sgm->block->errors[3] > itp_rt_sgm->block->total_steps)
					{
						itp_rt_sgm->block->errors[3] -= itp_rt_sgm->block->total_steps;
						new_stepbits |= STEP3_ITP_MASK;
					}
#ifdef STEP_ISR_SKIP_IDLE
				}
#endif
#ifdef STEP_ISR_SKIP_MAIN
			}
#endif
#endif
#if (STEPPER_COUNT > 4)
#ifdef STEP_ISR_SKIP_MAIN
			if (itp_rt_sgm->block->main_stepper == 4)
			{
				new_stepbits |= STEP4_ITP_MASK;
			}
			else
			{
#endif
#ifdef STEP_ISR_SKIP_IDLE
				if (!(itp_rt_sgm->block->idle_axis & STEP4_MASK))
				{
#endif
					itp_rt_sgm->block->errors[4] += itp_rt_sgm->block->steps[4];
					if (itp_rt_sgm->block->errors[4] > itp_rt_sgm->block->total_steps)
					{
						itp_rt_sgm->block->errors[4] -= itp_rt_sgm->block->total_steps;
						new_stepbits |= STEP4_ITP_MASK;
					}
#ifdef STEP_ISR_SKIP_IDLE
				}
#endif
#ifdef STEP_ISR_SKIP_MAIN
			}
#endif
#endif
#if (STEPPER_COUNT > 5)
#ifdef STEP_ISR_SKIP_MAIN
			if (itp_rt_sgm->block->main_stepper == 5)
			{
				new_stepbits |= STEP5_ITP_MASK;
			}
			else
			{
#endif
#ifdef STEP_ISR_SKIP_IDLE
				if (!(itp_rt_sgm->block->idle_axis & STEP5_MASK))
				{
#endif
					itp_rt_sgm->block->errors[5] += itp_rt_sgm->block->steps[5];
					if (itp_rt_sgm->block->errors[5] > itp_rt_sgm->block->total_steps)
					{
						itp_rt_sgm->block->errors[5] -= itp_rt_sgm->block->total_steps;
						new_stepbits |= STEP5_ITP_MASK;
					}
#ifdef STEP_ISR_SKIP_IDLE
				}
#endif
#ifdef STEP_ISR_SKIP_MAIN
			}
#endif
#endif

			uint8_t dirs = itp_rt_sgm->block->dirbits;
#ifdef ENABLE_RT_SYNC_MOTIONS
			static uint8_t last_dirs = 0;
			if (new_stepbits)
			{
				HOOK_INVOKE(itp_rt_pre_stepbits, &new_stepbits, &dirs);
				if (dirs != last_dirs)
				{
					last_dirs = dirs;
					io_set_dirs(dirs);
				}
			}
#endif

// updates the stepper coordinates
#if (STEPPER_COUNT > 0)
			if (new_stepbits & STEP0_ITP_MASK)
			{
#ifdef ENABLE_BACKLASH_COMPENSATION
				if (!itp_rt_sgm->block->backlash_comp)
				{
#endif
					if (dirs & DIR0_MASK)
					{
						itp_rt_step_pos[0]--;
					}
					else
					{
						itp_rt_step_pos[0]++;
					}
#ifdef ENABLE_BACKLASH_COMPENSATION
				}
#endif
			}
#endif
#if (STEPPER_COUNT > 1)
			if (new_stepbits & STEP1_ITP_MASK)
			{
#ifdef ENABLE_BACKLASH_COMPENSATION
				if (!itp_rt_sgm->block->backlash_comp)
				{
#endif
					if (dirs & DIR1_MASK)
					{
						itp_rt_step_pos[1]--;
					}
					else
					{
						itp_rt_step_pos[1]++;
					}
#ifdef ENABLE_BACKLASH_COMPENSATION
				}
#endif
			}
#endif
#if (STEPPER_COUNT > 2)
			if (new_stepbits & STEP2_ITP_MASK)
			{
#ifdef ENABLE_BACKLASH_COMPENSATION
				if (!itp_rt_sgm->block->backlash_comp)
				{
#endif
					if (dirs & DIR2_MASK)
					{
						itp_rt_step_pos[2]--;
					}
					else
					{
						itp_rt_step_pos[2]++;
					}
#ifdef ENABLE_BACKLASH_COMPENSATION
				}
#endif
			}
#endif
#if (STEPPER_COUNT > 3)
			if (new_stepbits & STEP3_ITP_MASK)
			{
#ifdef ENABLE_BACKLASH_COMPENSATION
				if (!itp_rt_sgm->block->backlash_comp)
				{
#endif
					if (dirs & DIR3_MASK)
					{
						itp_rt_step_pos[3]--;
					}
					else
					{
						itp_rt_step_pos[3]++;
					}
#ifdef ENABLE_BACKLASH_COMPENSATION
				}
#endif
			}
#endif

#if (STEPPER_COUNT > 4)
			if (new_stepbits & STEP4_ITP_MASK)
			{
#ifdef ENABLE_BACKLASH_COMPENSATION
				if (!itp_rt_sgm->block->backlash_comp)
				{
#endif
					if (dirs & DIR4_MASK)
					{
						itp_rt_step_pos[4]--;
					}
					else
					{
						itp_rt_step_pos[4]++;
					}
#ifdef ENABLE_BACKLASH_COMPENSATION
				}
#endif
			}
#endif

#if (STEPPER_COUNT > 5)
			if (new_stepbits & STEP5_ITP_MASK)
			{
#ifdef ENABLE_BACKLASH_COMPENSATION
				if (!itp_rt_sgm->block->backlash_comp)
				{
#endif
					if (dirs & DIR5_MASK)
					{
						itp_rt_step_pos[5]--;
					}
					else
					{
						itp_rt_step_pos[5]++;
					}
#ifdef ENABLE_BACKLASH_COMPENSATION
				}
#endif
			}
#endif
		}

		// no step remaining discards current segment
		--itp_rt_sgm->remaining_steps;
	}

	mcu_disable_global_isr(); // lock isr before clearin busy flag
	itp_busy = false;

#if (defined(ENABLE_DUAL_DRIVE_AXIS) || defined(IS_DELTA_KINEMATICS))
	stepbits = (new_stepbits & ~itp_step_lock);
#else
	stepbits = new_stepbits;
#endif
}

//     void itp_nomotion(uint8_t type, uint16_t delay)
//     {
//         while (itp_sgm_is_full())
//         {
//             if (!cnc_dotasks())
//             {
//                 return;
//             }
//         }

//         itp_sgm_data[itp_sgm_data_write].block = NULL;
//         //clicks every 100ms (10Hz)
//         if (delay)
//         {
//             mcu_freq_to_clocks(10, &(itp_sgm_data[itp_sgm_data_write].timer_counter), &(itp_sgm_data[itp_sgm_data_write].timer_prescaller));
//         }
//         else
//         {
//             mcu_freq_to_clocks(g_settings.max_step_rate, &(itp_sgm_data[itp_sgm_data_write].timer_counter), &(itp_sgm_data[itp_sgm_data_write].timer_prescaller));
//         }
//         itp_sgm_data[itp_sgm_data_write].remaining_steps = MAX(delay, 0);
//         itp_sgm_data[itp_sgm_data_write].feed = 0;
//         itp_sgm_data[itp_sgm_data_write].flags = type;
// #if TOOL_COUNT > 0
//         if (g_settings.laser_mode)
//         {
//             itp_sgm_data[itp_sgm_data_write].spindle = 0;
//             itp_sgm_data[itp_sgm_data_write].spindle_inv = false;
//         }
//         else
//         {
//             planner_get_spindle_speed(1, &(itp_sgm_data[itp_sgm_data_write].spindle), &(itp_sgm_data[itp_sgm_data_write].spindle_inv));
//         }
// #endif
//         itp_sgm_buffer_write();
//     }

void itp_start(bool is_synched)
{
	// starts the step isr if is stopped and there are segments to execute
	if (!cnc_get_exec_state(EXEC_RUN | EXEC_HOLD | EXEC_ALARM) && !itp_sgm_is_empty()) // exec state is not hold or alarm and not already running
	{
		// check if the start is controlled by synched motion before start
		if (!is_synched)
		{
			__ATOMIC__
			{
				cnc_set_exec_state(EXEC_RUN); // flags that it started running
				mcu_start_itp_isr(itp_sgm_data[itp_sgm_data_read].timer_counter, itp_sgm_data[itp_sgm_data_read].timer_prescaller);
			}
		}
	}
}
