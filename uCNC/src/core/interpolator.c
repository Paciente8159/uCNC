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

// Itp update flags
#define ITP_NOUPDATE 0
#define ITP_UPDATE_ISR 1
#define ITP_UPDATE_TOOL 2
#define ITP_UPDATE (ITP_UPDATE_ISR | ITP_UPDATE_TOOL)
#define ITP_SYNC_START 4

// contains data of the block being executed by the pulse routine
// this block has the necessary data to execute the Bresenham line algorithm
typedef struct itp_blk_
{
#ifdef STEP_ISR_SKIP_MAIN
	uint8_t main_stepper;
#endif
#ifdef STEP_ISR_SKIP_IDLE
	uint8_t idle_axis;
#endif
	uint8_t dirbits;
	step_t steps[STEPPER_COUNT];
	step_t total_steps;
	step_t errors[STEPPER_COUNT];
#ifdef GCODE_PROCESS_LINE_NUMBERS
	uint32_t line;
#endif
#ifdef ENABLE_BACKLASH_COMPENSATION
	bool backlash_comp;
#endif
} itp_block_t;

// contains data of the block segment being executed by the pulse and integrator routines
// the segment is a fragment of the motion defined in the block
// this also contains the acceleration/deacceleration info
typedef struct pulse_sgm_
{
	itp_block_t *block;
	uint16_t remaining_steps;
	uint16_t timer_counter;
	uint16_t timer_prescaller;
#if (DSS_MAX_OVERSAMPLING != 0)
	int8_t next_dss;
#endif
#if TOOL_COUNT > 0
	int16_t spindle;
#endif
	float feed;
	uint8_t flags;
} itp_segment_t;

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

#ifndef USE_LEGACY_STEP_INTERPOLATOR
void itp_run(void)
{
	// conversion vars
	static uint32_t accel_until = 0;
	static uint32_t deaccel_from = 0;
	static float top_speed = 0;
	static float exit_speed = 0;
	static float feed_convert = 0;
	static uint16_t accel_jumps = 0;
	static uint16_t deaccel_jumps = 0;
	float profile_steps_limit = 0;
	float partial_distance = 0;
	float avg_speed = 0;
#ifdef ENABLE_S_CURVE_ACCELERATION
	static float current_accel = 0;
	static float entry_speed = 0;
	static float jerk_accel = 0;
	static float jerk_deaccel = 0;
#endif

	itp_segment_t *sgm = NULL;

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
		}

		uint32_t remaining_steps = itp_cur_plan_block->steps[itp_cur_plan_block->main_stepper];

		sgm = &itp_sgm_data[itp_sgm_data_write];

		// clear the data segment
		memset(sgm, 0, sizeof(itp_segment_t));
		sgm->block = &itp_blk_data[itp_blk_data_write];

		// if an hold is active forces to deaccelerate
		if (cnc_get_exec_state(EXEC_HOLD))
		{
			// forces deacceleration by overriding the profile juntion points
			accel_until = remaining_steps;
			deaccel_from = remaining_steps;
			deaccel_jumps = 0xffff;
			itp_needs_update = true;
		}
		else if (itp_needs_update) // forces recalculation of acceleration and deacceleration profiles
		{
			itp_needs_update = false;
			float exit_speed_sqr = planner_get_block_exit_speed_sqr();
			float junction_speed_sqr = planner_get_block_top_speed(exit_speed_sqr);
			float accel_inv = 1.0f / itp_cur_plan_block->acceleration;
			top_speed = fast_flt_sqrt(junction_speed_sqr);
			exit_speed = fast_flt_sqrt(exit_speed_sqr);

			accel_until = remaining_steps;
			deaccel_from = 0;
#ifdef ENABLE_S_CURVE_ACCELERATION
			current_accel = 0;
#endif

			if (junction_speed_sqr != itp_cur_plan_block->entry_feed_sqr)
			{
				float d = ABS(junction_speed_sqr - itp_cur_plan_block->entry_feed_sqr) * accel_inv;
				d = fast_flt_div2(d);
				accel_until -= truncf(d);
#ifdef ENABLE_S_CURVE_ACCELERATION
				entry_speed = fast_flt_sqrt(itp_cur_plan_block->entry_feed_sqr);
#else
				float entry_speed = fast_flt_sqrt(itp_cur_plan_block->entry_feed_sqr);
#endif
				float t = ABS(top_speed - entry_speed);
				t *= accel_inv;
#ifdef ENABLE_S_CURVE_ACCELERATION
				jerk_accel = fast_flt_mul4(itp_cur_plan_block->acceleration / t);
#endif
				accel_jumps = (uint16_t)truncf(t * INTERPOLATOR_FREQ);
			}

			if (junction_speed_sqr > exit_speed_sqr)
			{
				float d = (junction_speed_sqr - exit_speed_sqr) * accel_inv;
				d = fast_flt_div2(d);
				deaccel_from = floorf(d);
				exit_speed = fast_flt_sqrt(exit_speed_sqr);
				float t = (top_speed - exit_speed);
				t *= accel_inv;
#ifdef ENABLE_S_CURVE_ACCELERATION
				jerk_deaccel = fast_flt_mul4(itp_cur_plan_block->acceleration / t);
#endif
				deaccel_jumps = (uint16_t)truncf(t * INTERPOLATOR_FREQ);
			}
		}

		float current_speed = fast_flt_sqrt(itp_cur_plan_block->entry_feed_sqr);
		float initial_speed = current_speed;
		sgm->flags = ITP_UPDATE_ISR;
		uint16_t segm_steps = 0;
		do
		{
			// acceleration profile
			if (remaining_steps > accel_until)
			{

				// computes the traveled distance within a fixed amount of time
				// this time is the reverse integrator frequency (t > INTERPOLATOR_DELTA_T)
				// for constant acceleration or deceleration the traveled distance will be equal
				// to the same distance traveled at a constant average speed given that
				// avg_speed = 0.5 * (final_speed + initial_speed)
				// where
				// final_speed = initial_speed + acceleration * t;
				// the travelled speed at interval t is aprox. given by
				// final_distance = initial_distance + avg_speed * t

				if ((accel_jumps > 1))
				{
#ifdef ENABLE_S_CURVE_ACCELERATION
					float prev_accel = current_accel;
					float accel_delta = (INTERPOLATOR_DELTA_T * jerk_accel);
					if ((entry_speed < top_speed) && (current_speed > fast_flt_div2(top_speed + entry_speed)))
					{
						accel_delta = -accel_delta;
					}

					if ((entry_speed > top_speed) && (current_speed < fast_flt_div2(top_speed + entry_speed)))
					{
						accel_delta = -accel_delta;
					}
					current_accel += accel_delta;
					// calcs the next average acceleration based on the jerk
					float avg_accel = fast_flt_div2(current_accel + prev_accel);
					// determines the acceleration profile (first half - convex, second half - concave)
					current_speed += (current_speed < top_speed) ? (INTERPOLATOR_DELTA_T * avg_accel) : (-INTERPOLATOR_DELTA_T * avg_accel);
#else
					current_speed += (current_speed < top_speed) ? (INTERPOLATOR_DELTA_T * itp_cur_plan_block->acceleration) : (-INTERPOLATOR_DELTA_T * itp_cur_plan_block->acceleration);
#endif
					avg_speed = fast_flt_div2(current_speed + initial_speed);
					partial_distance += avg_speed * INTERPOLATOR_DELTA_T;
					accel_jumps--;
				}
				else
				{
					// if the number of jumps required are less than 1 just jumps to the final distance and applies the same principle
					// in practice this translates to a Riemann sample with t < INTERPOLATOR_DELTA_T

					current_speed = top_speed;
					partial_distance = remaining_steps - accel_until;
				}

				profile_steps_limit = accel_until;
			}
			else if (remaining_steps > deaccel_from)
			{
				// constant speed segment
				if (remaining_steps != accel_until)
				{
					sgm->flags = ITP_NOUPDATE;
				}

				partial_distance += top_speed * INTERPOLATOR_DELTA_CONST_T;
				profile_steps_limit = deaccel_from;
				current_speed = top_speed;
				initial_speed = top_speed;
			}
			else
			{
				if ((deaccel_jumps > 1))
				{
#ifdef ENABLE_S_CURVE_ACCELERATION
					float prev_accel = current_accel;
					float accel_delta = (INTERPOLATOR_DELTA_T * jerk_deaccel);
					if ((current_speed < fast_flt_div2(top_speed + exit_speed)))
					{
						accel_delta = -accel_delta;
					}
					current_accel += accel_delta;
					// calcs the next average acceleration based on the jerk
					float avg_accel = fast_flt_div2(current_accel + prev_accel);
					// determines the acceleration profile (first half - convex, second half - concave)
					current_speed -= INTERPOLATOR_DELTA_T * avg_accel;
#else
					current_speed -= INTERPOLATOR_DELTA_T * itp_cur_plan_block->acceleration;
#endif
					// prevents negative or zero speeds
					float min_exit_speed = 2 * INTERPOLATOR_DELTA_T * itp_cur_plan_block->acceleration;
					bool flushsteps = false;
					if (current_speed < min_exit_speed)
					{
						current_speed = min_exit_speed * remaining_steps;
						flushsteps = true;
					}
					avg_speed = fast_flt_div2(current_speed + initial_speed);
					partial_distance += avg_speed * INTERPOLATOR_DELTA_T;
					deaccel_jumps--;
					// speed reached 0. just flush remaining steps
					if (flushsteps)
					{
						deaccel_jumps = 0;
						partial_distance = remaining_steps;
					}
				}
				else
				{
					// if the number of jumps required are less than 1 just jumps to the final distance and applies the same principle
					// in practice this translates to a Riemann sample with t < INTERPOLATOR_DELTA_T

					current_speed = exit_speed;
					partial_distance = remaining_steps;
				}

				profile_steps_limit = 0;
			}

			// computes how many steps it will perform at this speed and frame window
			segm_steps = (uint16_t)lroundf(partial_distance);
		} while (segm_steps == 0);

		avg_speed = fast_flt_div2(current_speed + initial_speed);
		//        float min_exit_speed = INTERPOLATOR_DELTA_T * itp_cur_plan_block->acceleration;
		//        if (current_speed > min_exit_speed)
		//        {
		//            avg_speed = fast_flt_div2(current_speed + initial_speed);
		//        }
		//        else
		//        {
		//            // prevents slow exits
		//            avg_speed = fast_flt_div2(min_exit_speed);
		//        }

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
		float dss_speed = avg_speed;
		uint8_t dss = 0;
		while (dss_speed < DSS_CUTOFF_FREQ && dss < DSS_MAX_OVERSAMPLING)
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
		avg_speed = MIN(avg_speed, g_settings.max_step_rate);
		mcu_freq_to_clocks(avg_speed, &(sgm->timer_counter), &(sgm->timer_prescaller));
#endif

		sgm->feed = avg_speed * feed_convert;
#if TOOL_COUNT > 0
		if (g_settings.laser_mode == LASER_PWM_MODE)
		{
			float top_speed_inv = fast_flt_invsqrt(itp_cur_plan_block->feed_sqr);
			int16_t newspindle = planner_get_spindle_speed(MIN(1, avg_speed * top_speed_inv));

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
			// fixes rounding errors
			current_speed = top_speed;
			profile_steps_limit = deaccel_from;
#ifdef ENABLE_S_CURVE_ACCELERATION
			current_accel = 0;
#endif
		}

		itp_cur_plan_block->entry_feed_sqr = fast_flt_pow2(current_speed);
		itp_cur_plan_block->steps[itp_cur_plan_block->main_stepper] = remaining_steps;

		// checks for synched motion
		if (itp_cur_plan_block->planner_flags.bit.synched)
		{
			sgm->flags |= ITP_SYNC_START;
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
	// check if the start is controlled by synched motion before start
	if (!(itp_sgm_data[itp_sgm_data_read].flags & ITP_SYNC_START))
	{
		cnc_set_exec_state(EXEC_RUN); // flags that it started running
		__ATOMIC__
		{
			mcu_start_itp_isr(itp_sgm_data[itp_sgm_data_read].timer_counter, itp_sgm_data[itp_sgm_data_read].timer_prescaller);
		}
	}
}
#else
void itp_run(void)
{
	// conversion vars
	static uint32_t accel_until = 0;
	static uint32_t deaccel_from = 0;
	static float junction_speed_sqr = 0;
	static float half_speed_change = 0;
	static bool initial_accel_negative = false;
	static float feed_convert = 0;
	static bool const_speed = false;

	itp_segment_t *sgm = NULL;

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
			// in every new block speed update is needed
			const_speed = false;

			half_speed_change = INTERPOLATOR_DELTA_T * itp_cur_plan_block->acceleration;
			half_speed_change = fast_flt_div2(half_speed_change);
		}

		uint32_t remaining_steps = itp_cur_plan_block->steps[itp_cur_plan_block->main_stepper];

		sgm = &itp_sgm_data[itp_sgm_data_write];

		// clear the data segment
		memset(sgm, 0, sizeof(itp_segment_t));
		sgm->block = &itp_blk_data[itp_blk_data_write];

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
			junction_speed_sqr = planner_get_block_top_speed(exit_speed_sqr);

			accel_until = remaining_steps;
			deaccel_from = 0;
			if (junction_speed_sqr != itp_cur_plan_block->entry_feed_sqr)
			{
				float accel_dist = ABS(junction_speed_sqr - itp_cur_plan_block->entry_feed_sqr) / itp_cur_plan_block->acceleration;
				accel_dist = fast_flt_div2(accel_dist);
				accel_until -= floorf(accel_dist);
				initial_accel_negative = (junction_speed_sqr < itp_cur_plan_block->entry_feed_sqr);
			}

			// if entry speed already a junction speed updates it.
			if (accel_until == remaining_steps)
			{
				itp_cur_plan_block->entry_feed_sqr = junction_speed_sqr;
			}

			if (junction_speed_sqr > exit_speed_sqr)
			{
				float deaccel_dist = (junction_speed_sqr - exit_speed_sqr) / itp_cur_plan_block->acceleration;
				deaccel_dist = fast_flt_div2(deaccel_dist);
				deaccel_from = floorf(deaccel_dist);
			}
		}

		float speed_change;
		float profile_steps_limit;
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
			speed_change = (!initial_accel_negative) ? half_speed_change : -half_speed_change;
			profile_steps_limit = accel_until;
			sgm->flags = ITP_UPDATE_ISR;
			const_speed = false;
		}
		else if (remaining_steps > deaccel_from)
		{
			// constant speed segment
			speed_change = 0;
			profile_steps_limit = deaccel_from;
			sgm->flags = (!const_speed) ? ITP_UPDATE_ISR : ITP_NOUPDATE;
			if (!const_speed)
			{
				const_speed = true;
			}
		}
		else
		{
			speed_change = -half_speed_change;
			profile_steps_limit = 0;
			sgm->flags = ITP_UPDATE_ISR;
			const_speed = false;
		}

		float current_speed = fast_flt_sqrt(itp_cur_plan_block->entry_feed_sqr);

		/*
			common calculations for all three profiles (accel, constant and deaccel)
		*/
		current_speed += speed_change;

		if (current_speed <= 0)
		{
			if (cnc_get_exec_state(EXEC_HOLD))
			{
				return;
			}

			// speed can't be negative
			current_speed = 0;
		}

		float partial_distance = current_speed * INTERPOLATOR_DELTA_T;

		if (partial_distance < 1)
		{
			partial_distance = 1;
		}

		// computes how many steps it will perform at this speed and frame window
		uint16_t segm_steps = (uint16_t)floorf(partial_distance);

		// if computed steps exceed the remaining steps for the motion shortens the distance
		if (segm_steps > (remaining_steps - profile_steps_limit))
		{
			segm_steps = (uint16_t)(remaining_steps - profile_steps_limit);
		}

		if (speed_change)
		{
			float new_speed_sqr = itp_cur_plan_block->acceleration * segm_steps;
			new_speed_sqr = fast_flt_mul2(new_speed_sqr);
			if (speed_change > 0)
			{
				// calculates the final speed at the end of this position
				new_speed_sqr += itp_cur_plan_block->entry_feed_sqr;
			}
			else
			{
				// calculates the final speed at the end of this position
				new_speed_sqr = itp_cur_plan_block->entry_feed_sqr - new_speed_sqr;
				new_speed_sqr = MAX(new_speed_sqr, 0); // avoids rounding errors since speed is always positive
			}
			current_speed = (fast_flt_sqrt(new_speed_sqr) + fast_flt_sqrt(itp_cur_plan_block->entry_feed_sqr));
			current_speed = fast_flt_div2(current_speed);
			itp_cur_plan_block->entry_feed_sqr = new_speed_sqr;
		}

// The DSS (Dynamic Step Spread) algorithm reduces stepper vibration by spreading step distribution at lower speads.
// This is done by oversampling the Bresenham line algorithm by multiple factors of 2.
// This way stepping actions fire in different moments in order to reduce vibration caused by the stepper internal mechanics.
// This works in a similar way to Grbl's AMASS but has a modified implementation to minimize the processing penalty on the ISR and also take less static memory.
// DSS never loads the step generating ISR with a frequency above half of the absolute maximum frequency
#if (DSS_MAX_OVERSAMPLING != 0)
		float dss_speed = current_speed;
		uint8_t dss = 0;
		while (dss_speed < DSS_CUTOFF_FREQ && dss < DSS_MAX_OVERSAMPLING)
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
			itp_cur_plan_block->entry_feed_sqr = junction_speed_sqr;
		}

		itp_cur_plan_block->steps[itp_cur_plan_block->main_stepper] = remaining_steps;

		// checks for synched motion
		if (itp_cur_plan_block->planner_flags.bit.synched)
		{
			sgm->flags |= ITP_SYNC_START;
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
	if (!cnc_get_exec_state(EXEC_HOLD | EXEC_ALARM | EXEC_RUN | EXEC_RESUMING) && !itp_sgm_is_empty()) // exec state is not hold or alarm and not already running
	{
		// check if the start is controlled by synched motion before start
		if (!(itp_sgm_data[itp_sgm_data_read].flags & ITP_SYNC_START))
		{
			cnc_set_exec_state(EXEC_RUN); // flags that it started running
			__ATOMIC__
			{
				mcu_start_itp_isr(itp_sgm_data[itp_sgm_data_read].timer_counter, itp_sgm_data[itp_sgm_data_read].timer_prescaller);
			}
		}
	}
}
#endif

void itp_update(void)
{
	// flags executing block for update
	itp_needs_update = true;
}

void itp_stop(void)
{
	// any stop command while running triggers an HALT alarm
	if (cnc_get_exec_state(EXEC_RUN))
	{
		cnc_set_exec_state(EXEC_HALT);
	}

	io_set_steps(g_settings.step_invert_mask);
#if TOOL_COUNT > 0
	if (g_settings.laser_mode)
	{
		tool_set_speed(0);
	}
#endif

	mcu_stop_itp_isr();
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

// flushes all motions from all systems (planner or interpolator)
// used to make a sync motion
uint8_t itp_sync(void)
{
	while (!planner_buffer_is_empty() || !itp_sgm_is_empty() || (itp_rt_sgm != NULL))
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

#ifdef ENABLE_LASER_PPI
// turn laser off callback
MCU_CALLBACK void laser_ppi_turnoff_cb(void)
{
#ifndef INVERT_LASER_PPI_LOGIC
	mcu_clear_output(LASER_PPI);
#else
	mcu_set_output(LASER_PPI);
#endif
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
#ifdef ENABLE_LASER_PPI
	static uint16_t new_laser_ppi = 0;
#endif

	if (!itp_busy) // prevents reentrancy
	{
		if (itp_rt_sgm != NULL)
		{
			if (itp_rt_sgm->flags)
			{
				if (itp_rt_sgm->flags & ITP_UPDATE_ISR)
				{
					mcu_change_itp_isr(itp_rt_sgm->timer_counter, itp_rt_sgm->timer_prescaller);
				}

#if TOOL_COUNT > 0
				if (itp_rt_sgm->flags & ITP_UPDATE_TOOL)
				{
#ifdef ENABLE_LASER_PPI
					if (g_settings.laser_mode & (LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE))
					{
						new_laser_ppi = itp_rt_sgm->spindle;
					}
					else
					{
#endif
						tool_set_speed(itp_rt_sgm->spindle);
#ifdef ENABLE_LASER_PPI
					}
#endif
				}
#endif
				itp_rt_sgm->flags = ITP_NOUPDATE;
			}

			// no step remaining discards current segment
			if (!itp_rt_sgm->remaining_steps)
			{
				itp_rt_sgm->block = NULL;
				itp_rt_sgm = NULL;
				itp_sgm_buffer_read();
			}
		}

		// sets step bits
#ifdef ENABLE_LASER_PPI
		if (g_settings.laser_mode & (LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE))
		{
			if (stepbits & LASER_PPI_MASK)
			{
				if (new_laser_ppi)
				{
					mcu_config_timeout(&laser_ppi_turnoff_cb, new_laser_ppi);
					new_laser_ppi = 0;
				}
				mcu_start_timeout();
#ifndef INVERT_LASER_PPI_LOGIC
				mcu_set_output(LASER_PPI);
#else
				mcu_clear_output(LASER_PPI);
#endif
			}
		}
#endif
		io_toggle_steps(stepbits);
		stepbits = 0;

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

		itp_busy = true;
		mcu_enable_global_isr();

		// is steps remaining starts calc next step bits
		if (itp_rt_sgm->remaining_steps)
		{
			bool dostep = false;
			if (itp_rt_sgm->block != NULL)
			{
// prepares the next step bits mask
#if (STEPPER_COUNT > 0)
				dostep = false;
#ifdef STEP_ISR_SKIP_MAIN
				if (itp_rt_sgm->block->main_stepper == 0)
				{
					dostep = true;
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
							dostep = true;
						}
#ifdef STEP_ISR_SKIP_IDLE
					}
#endif
#ifdef STEP_ISR_SKIP_MAIN
				}
#endif

				if (dostep)
				{
					stepbits |= STEP0_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
					if (!itp_rt_sgm->block->backlash_comp)
					{
#endif
						if (itp_rt_sgm->block->dirbits & DIR0_MASK)
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
				dostep = false;
#ifdef STEP_ISR_SKIP_MAIN
				if (itp_rt_sgm->block->main_stepper == 1)
				{
					dostep = true;
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
							dostep = true;
						}
#ifdef STEP_ISR_SKIP_IDLE
					}
#endif
#ifdef STEP_ISR_SKIP_MAIN
				}
#endif

				if (dostep)
				{
					stepbits |= STEP1_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
					if (!itp_rt_sgm->block->backlash_comp)
					{
#endif
						if (itp_rt_sgm->block->dirbits & DIR1_MASK)
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
				dostep = false;
#ifdef STEP_ISR_SKIP_MAIN
				if (itp_rt_sgm->block->main_stepper == 2)
				{
					dostep = true;
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
							dostep = true;
						}
#ifdef STEP_ISR_SKIP_IDLE
					}
#endif
#ifdef STEP_ISR_SKIP_MAIN
				}
#endif

				if (dostep)
				{
					stepbits |= STEP2_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
					if (!itp_rt_sgm->block->backlash_comp)
					{
#endif
						if (itp_rt_sgm->block->dirbits & DIR2_MASK)
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
				dostep = false;
#ifdef STEP_ISR_SKIP_MAIN
				if (itp_rt_sgm->block->main_stepper == 3)
				{
					dostep = true;
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
							dostep = true;
						}
#ifdef STEP_ISR_SKIP_IDLE
					}
#endif
#ifdef STEP_ISR_SKIP_MAIN
				}
#endif

				if (dostep)
				{
					stepbits |= STEP3_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
					if (!itp_rt_sgm->block->backlash_comp)
					{
#endif
						if (itp_rt_sgm->block->dirbits & DIR3_MASK)
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
				dostep = false;
#ifdef STEP_ISR_SKIP_MAIN
				if (itp_rt_sgm->block->main_stepper == 4)
				{
					dostep = true;
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
							dostep = true;
						}
#ifdef STEP_ISR_SKIP_IDLE
					}
#endif
#ifdef STEP_ISR_SKIP_MAIN
				}
#endif

				if (dostep)
				{
					stepbits |= STEP4_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
					if (!itp_rt_sgm->block->backlash_comp)
					{
#endif
						if (itp_rt_sgm->block->dirbits & DIR4_MASK)
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
				dostep = false;
#ifdef STEP_ISR_SKIP_MAIN
				if (itp_rt_sgm->block->main_stepper == 5)
				{
					dostep = true;
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
							dostep = true;
						}
#ifdef STEP_ISR_SKIP_IDLE
					}
#endif
#ifdef STEP_ISR_SKIP_MAIN
				}
#endif

				if (dostep)
				{
					stepbits |= STEP5_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
					if (!itp_rt_sgm->block->backlash_comp)
					{
#endif
						if (itp_rt_sgm->block->dirbits & DIR5_MASK)
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
		stepbits &= ~itp_step_lock;
#endif
	}
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
