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

#define F_INTEGRATOR 100
#define INTEGRATOR_DELTA_T (1.0f / F_INTEGRATOR)
// the amount of motion precomputed and stored for the step generator is never less then
// the size of the buffer x time window size
// in this case the buffer never holds less then 50ms of motions

// integrator calculates 10ms (minimum size) time frame windows
#define INTERPOLATOR_BUFFER_SIZE 5 // number of windows in the buffer

// Itp update flags
#define ITP_NOUPDATE 0
#define ITP_UPDATE_ISR 1
#define ITP_UPDATE_TOOL 2

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
    uint8_t update_itp;
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
#ifdef ENABLE_DUAL_DRIVE_AXIS
volatile static uint8_t itp_step_lock;
#endif

static void itp_sgm_buffer_read(void);
static void itp_sgm_buffer_write(void);
FORCEINLINE static bool itp_sgm_is_full(void);
FORCEINLINE static bool itp_sgm_is_empty(void);
FORCEINLINE static void itp_sgm_clear(void);
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
#endif
    itp_cur_plan_block = NULL;
    itp_needs_update = false;
    // initialize circular buffers
    itp_blk_clear();
    itp_sgm_clear();
}

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
            itp_blk_data[itp_blk_data_write].backlash_comp = itp_cur_plan_block->backlash_comp;
#endif
            itp_blk_data[itp_blk_data_write].dirbits = itp_cur_plan_block->dirbits;
            itp_blk_data[itp_blk_data_write].total_steps = itp_cur_plan_block->total_steps << 1;

            float total_step_inv = 1.0f / (float)itp_cur_plan_block->total_steps;
            feed_convert = 60.f / (float)g_settings.step_per_mm[itp_cur_plan_block->main_stepper];
            float sqr_step_speed = 0;

#ifdef STEP_ISR_SKIP_IDLE
            itp_blk_data[itp_blk_data_write].idle_axis = 0;
#endif
#ifdef STEP_ISR_SKIP_MAIN
            itp_blk_data[itp_blk_data_write].main_stepper = itp_cur_plan_block->main_stepper;
#endif
            for (uint8_t i = 0; i < STEPPER_COUNT; i++)
            {
                sqr_step_speed += fast_flt_pow2((float)itp_cur_plan_block->steps[i]);
                itp_blk_data[itp_blk_data_write].errors[i] = itp_cur_plan_block->total_steps;
                itp_blk_data[itp_blk_data_write].steps[i] = itp_cur_plan_block->steps[i] << 1;
#ifdef STEP_ISR_SKIP_IDLE
                if (!itp_cur_plan_block->steps[i])
                {
                    itp_blk_data[itp_blk_data_write].idle_axis |= (1 << i);
                }
#endif
            }

            sqr_step_speed *= fast_flt_pow2(total_step_inv);
            feed_convert *= fast_flt_sqrt(sqr_step_speed);

            // flags block for recalculation of speeds
            itp_needs_update = true;
            // in every new block speed update is needed
            const_speed = false;

            half_speed_change = INTEGRATOR_DELTA_T * itp_cur_plan_block->acceleration;
            half_speed_change = fast_flt_div2(half_speed_change);
        }

        uint32_t remaining_steps = itp_cur_plan_block->total_steps;

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
                this time is the reverse integrator frequency (INTEGRATOR_DELTA_T)
                for constant acceleration or deceleration the traveled distance will be equal
                to the same distance traveled at a constant speed given that
                constant_speed = 0.5 * (final_speed - initial_speed) + initial_speed

                where

                (final_speed - initial_speed) = acceleration * INTEGRATOR_DELTA_T;
            */
            speed_change = (!initial_accel_negative) ? half_speed_change : -half_speed_change;
            profile_steps_limit = accel_until;
            sgm->update_itp = ITP_UPDATE_ISR;
            const_speed = false;
        }
        else if (remaining_steps > deaccel_from)
        {
            // constant speed segment
            speed_change = 0;
            profile_steps_limit = deaccel_from;
            sgm->update_itp = (!const_speed) ? ITP_UPDATE_ISR : ITP_NOUPDATE;
            if (!const_speed)
            {
                const_speed = true;
            }
        }
        else
        {
            speed_change = -half_speed_change;
            profile_steps_limit = 0;
            sgm->update_itp = ITP_UPDATE_ISR;
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

        float partial_distance = current_speed * INTEGRATOR_DELTA_T;

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
            sgm->update_itp = ITP_UPDATE_ISR;
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
        float top_speed_inv = fast_flt_invsqrt(junction_speed_sqr);
        int16_t newspindle = planner_get_spindle_speed(MIN(1, current_speed * top_speed_inv));

        if (prev_spindle != newspindle)
        {
            prev_spindle = newspindle;
            sgm->update_itp |= ITP_UPDATE_TOOL;
        }

        sgm->spindle = newspindle;
#endif
        remaining_steps -= segm_steps;

        if (remaining_steps == accel_until) // resets float additions error
        {
            itp_cur_plan_block->entry_feed_sqr = junction_speed_sqr;
        }

        itp_cur_plan_block->total_steps = remaining_steps;

        if (itp_cur_plan_block->total_steps == 0)
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
        cnc_set_exec_state(EXEC_RUN); // flags that it started running
        __ATOMIC__
        {
            mcu_start_itp_isr(itp_sgm_data[itp_sgm_data_read].timer_counter, itp_sgm_data[itp_sgm_data_read].timer_prescaller);
        }
    }
}

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
    io_set_dirs(g_settings.dir_invert_mask);
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
}

void itp_reset_rt_position(void)
{
    if (g_settings.homing_enabled)
    {
        float origin[AXIS_COUNT];
        for (uint8_t i = AXIS_COUNT; i != 0;)
        {
            i--;
            if (g_settings.homing_dir_invert_mask & (1 << i))
            {
                origin[i] = g_settings.max_distance[i];
            }
            else
            {
                origin[i] = 0;
            }
        }

        kinematics_apply_inverse(origin, itp_rt_step_pos);
#if STEPPER_COUNT > 0
#ifdef STEP0_ENCODER
        encoder_reset_position(STEP0_ENCODER, origin[0]);
#endif
#endif
#if STEPPER_COUNT > 1
#ifdef STEP1_ENCODER
        encoder_reset_position(STEP1_ENCODER, origin[1]);
#endif
#endif
#if STEPPER_COUNT > 2
#ifdef STEP2_ENCODER
        encoder_reset_position(STEP2_ENCODER, origin[2]);
#endif
#endif
#if STEPPER_COUNT > 3
#ifdef STEP3_ENCODER
        encoder_reset_position(STEP3_ENCODER, origin[3]);
#endif
#endif
#if STEPPER_COUNT > 4
#ifdef STEP4_ENCODER
        encoder_reset_position(STEP4_ENCODER, origin[4]);
#endif
#endif
#if STEPPER_COUNT > 5
#ifdef STEP5_ENCODER
        encoder_reset_position(STEP5_ENCODER, origin[5]);
#endif
#endif
    }
    else
    {
        memset(itp_rt_step_pos, 0, sizeof(itp_rt_step_pos));
#if STEPPER_COUNT > 0
#ifdef STEP0_ENCODER
        encoder_reset_position(STEP0_ENCODER, 0);
#endif
#endif
#if STEPPER_COUNT > 1
#ifdef STEP1_ENCODER
        encoder_reset_position(STEP1_ENCODER, 0);
#endif
#endif
#if STEPPER_COUNT > 2
#ifdef STEP2_ENCODER
        encoder_reset_position(STEP2_ENCODER, 0);
#endif
#endif
#if STEPPER_COUNT > 3
#ifdef STEP3_ENCODER
        encoder_reset_position(STEP3_ENCODER, 0);
#endif
#endif
#if STEPPER_COUNT > 4
#ifdef STEP4_ENCODER
        encoder_reset_position(STEP4_ENCODER, 0);
#endif
#endif
#if STEPPER_COUNT > 5
#ifdef STEP5_ENCODER
        encoder_reset_position(STEP5_ENCODER, 0);
#endif
#endif
    }
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
    while (!planner_buffer_is_empty() || !itp_sgm_is_empty() || cnc_get_exec_state(EXEC_RUN))
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

#if TOOL_COUNT > 0
uint16_t itp_get_rt_spindle(void)
{
    float spindle = (float)tool_get_speed();
    spindle *= g_settings.spindle_max_rpm * UINT8_MAX_INV;

    return (uint16_t)roundf(spindle);
}
#endif

#ifdef ENABLE_DUAL_DRIVE_AXIS
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
void mcu_step_reset_cb(void)
{
    // always resets all stepper pins
    io_set_steps(g_settings.step_invert_mask);
}

void mcu_step_cb(void)
{
    static uint8_t stepbits = 0;
    static bool itp_busy = false;

    if (!itp_busy) // prevents reentrancy
    {
        if (itp_rt_sgm != NULL)
        {
            if (itp_rt_sgm->update_itp)
            {
                if (itp_rt_sgm->update_itp & ITP_UPDATE_ISR)
                {
                    mcu_change_itp_isr(itp_rt_sgm->timer_counter, itp_rt_sgm->timer_prescaller);
                }

#if TOOL_COUNT > 0
                if (itp_rt_sgm->update_itp & ITP_UPDATE_TOOL)
                {
                    tool_set_speed(itp_rt_sgm->spindle);
                }
#endif
                itp_rt_sgm->update_itp = ITP_NOUPDATE;
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
                itp_stop();                     // the buffer is empty. The ISR can stop
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

#ifdef ENABLE_DUAL_DRIVE_AXIS
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
//         itp_sgm_data[itp_sgm_data_write].update_itp = type;
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
