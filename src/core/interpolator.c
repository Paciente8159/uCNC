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

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include "cnc.h"
#include "core/interpolator.h"
#include "interface/settings.h"
#include "core/planner.h"
#include "core/io_control.h"

#if (DSS_MAX_OVERSAMPLING < 0 || DSS_MAX_OVERSAMPLING > 3)
#error DSS_MAX_OVERSAMPLING invalid value! Should be set between 0 and 3
#endif

#define F_INTEGRATOR 100
#define INTEGRATOR_DELTA_T (1.0f / F_INTEGRATOR)
//the amount of motion precomputed and stored for the step generator is never less then
//the size of the buffer x time window size
//in this case the buffer never holds less then 50ms of motions

//integrator calculates 10ms (minimum size) time frame windows
#define INTERPOLATOR_BUFFER_SIZE 5 //number of windows in the buffer

    //contains data of the block being executed by the pulse routine
    //this block has the necessary data to execute the Bresenham line algorithm
    typedef struct itp_blk_
    {
        uint8_t main_axis;
        uint8_t idle_axis;
        uint8_t dirbits;
        uint32_t steps[STEPPER_COUNT];
        uint32_t total_steps;
        uint32_t errors[STEPPER_COUNT];
#ifdef GCODE_PROCESS_LINE_NUMBERS
        uint32_t line;
#endif
#ifdef ENABLE_BACKLASH_COMPENSATION
        bool backlash_comp;
#endif
    } INTERPOLATOR_BLOCK;

    //contains data of the block segment being executed by the pulse and integrator routines
    //the segment is a fragment of the motion defined in the block
    //this also contains the acceleration/deacceleration info
    typedef struct pulse_sgm_
    {
        INTERPOLATOR_BLOCK *block;
        uint8_t main_axis;
        uint16_t remaining_steps;
        uint16_t timer_counter;
        uint16_t timer_prescaller;
#if (DSS_MAX_OVERSAMPLING != 0)
        uint8_t next_dss;
#endif
#ifdef USE_SPINDLE
        uint8_t spindle;
        bool spindle_inv;
#endif
        float feed;
        uint8_t update_speed;
    } INTERPOLATOR_SEGMENT;

    //circular buffers
    //creates new type PULSE_BLOCK_BUFFER
    static INTERPOLATOR_BLOCK itp_blk_data[INTERPOLATOR_BUFFER_SIZE];
    static uint8_t itp_blk_data_write;
    static uint8_t itp_blk_data_read;
    static uint8_t itp_blk_data_slots;

    static INTERPOLATOR_SEGMENT itp_sgm_data[INTERPOLATOR_BUFFER_SIZE];
    static volatile uint8_t itp_sgm_data_write;
    static uint8_t itp_sgm_data_read;
    static volatile uint8_t itp_sgm_data_slots;
    //static buffer_t itp_sgm_buffer;

    static planner_block_t *itp_cur_plan_block;
    //pointer to the segment being executed
    static volatile INTERPOLATOR_SEGMENT volatile *itp_running_sgm;

    //stores the current position of the steppers in the interpolator after processing a planner block
    static uint32_t itp_step_pos[STEPPER_COUNT];
    //keeps track of the machine realtime position
    static uint32_t itp_rt_step_pos[STEPPER_COUNT];
    static volatile uint8_t itp_rt_spindle;
    //flag to force the interpolator to recalc entry and exit limit position of acceleration/deacceleration curves
    static bool itp_needs_update;
    static volatile bool itp_isr_finnished;
#ifdef ENABLE_DUAL_DRIVE_AXIS
    volatile static uint8_t itp_step_lock;
#endif

    static volatile bool itp_busy;

    /*
	Interpolator segment buffer functions
*/
    static inline void itp_sgm_buffer_read(void)
    {
        itp_sgm_data_slots++;
        if (++itp_sgm_data_read == INTERPOLATOR_BUFFER_SIZE)
        {
            itp_sgm_data_read = 0;
        }
    }

    static inline void itp_sgm_buffer_write(void)
    {
        itp_sgm_data_slots--;
        if (++itp_sgm_data_write == INTERPOLATOR_BUFFER_SIZE)
        {
            itp_sgm_data_write = 0;
        }
    }

    static inline bool itp_sgm_is_empty(void)
    {
        return (itp_sgm_data_slots == INTERPOLATOR_BUFFER_SIZE);
    }

    static inline bool itp_sgm_is_full(void)
    {
        return (itp_sgm_data_slots == 0);
    }

    static inline void itp_sgm_clear(void)
    {
        itp_sgm_data_write = 0;
        itp_sgm_data_read = 0;
        itp_sgm_data_slots = INTERPOLATOR_BUFFER_SIZE;
        memset(itp_sgm_data, 0, sizeof(itp_sgm_data));
    }

    static inline void itp_blk_buffer_write(void)
    {
        //itp_blk_data_slots--; //AUTOMATIC LOOP
        if (++itp_blk_data_write == INTERPOLATOR_BUFFER_SIZE)
        {
            itp_blk_data_write = 0;
        }
    }

    static inline void itp_blk_clear(void)
    {
        itp_blk_data_write = 0;
        itp_blk_data_read = 0;
        itp_blk_data_slots = INTERPOLATOR_BUFFER_SIZE;
        memset(itp_blk_data, 0, sizeof(itp_blk_data));
    }

    /*
	Interpolator functions
*/
    //declares functions called by the stepper ISR
    void itp_init(void)
    {
#ifdef FORCE_GLOBALS_TO_0
        //resets buffers
        memset(itp_step_pos, 0, sizeof(itp_step_pos));
        memset(itp_rt_step_pos, 0, sizeof(itp_rt_step_pos));
        itp_running_sgm = NULL;
        itp_cur_plan_block = NULL;
        itp_needs_update = false;
#endif
        itp_busy = false;
        itp_isr_finnished = true;
        //initialize circular buffers
        itp_blk_clear();
        itp_sgm_clear();
    }

    void itp_run(void)
    {
        //conversion vars
        static uint32_t accel_until = 0;
        static uint32_t deaccel_from = 0;
        static float junction_speed_sqr = 0;
        static float half_speed_change = 0;
        static bool initial_accel_negative = false;
        static float feed_convert = 0;
        static bool is_initial_transition = true;

        //accel profile vars
        static uint32_t unprocessed_steps = 0;

        INTERPOLATOR_SEGMENT *sgm = NULL;

        //creates segments and fills the buffer
        while (!itp_sgm_is_full())
        {
            if (cnc_get_exec_state(EXEC_ALARM))
            {
                //on any active alarm exits
                return;
            }

            //no planner blocks has beed processed or last planner block was fully processed
            if (itp_cur_plan_block == NULL)
            {
                //planner is empty or interpolator block buffer full. Nothing to be done
                if (planner_buffer_is_empty() /* || itp_blk_is_full()*/)
                {
                    break;
                }
                //get the first block in the planner
                itp_cur_plan_block = planner_get_block();
#ifdef GCODE_PROCESS_LINE_NUMBERS
                itp_blk_data[itp_blk_data_write].line = itp_cur_plan_block->line;
#endif

                if (itp_cur_plan_block->dwell != 0)
                {
                    itp_delay(itp_cur_plan_block->dwell);
                }

                if (itp_cur_plan_block->total_steps == 0)
                {
#ifdef USE_SPINDLE
                    if (itp_cur_plan_block->dwell == 0) //if dwell is 0 then run a single loop to updtate outputs (spindle)
                    {
                        itp_delay(1);
                    }
#endif
                    //no motion action (doesn't need a interpolator block = NULL)
                    itp_cur_plan_block = NULL;
                    planner_discard_block();
                    break; //exits after adding the dwell segment if motion is 0 (empty motion block)
                }

//overwrites previous values
#ifdef ENABLE_BACKLASH_COMPENSATION
                itp_blk_data[itp_blk_data_write].backlash_comp = itp_cur_plan_block->backlash_comp;
#endif
                itp_blk_data[itp_blk_data_write].dirbits = itp_cur_plan_block->dirbits;
                itp_blk_data[itp_blk_data_write].total_steps = itp_cur_plan_block->total_steps << 1;

                float total_step_inv = 1.0f / (float)itp_cur_plan_block->total_steps;
                feed_convert = 60.f / (float)g_settings.step_per_mm[itp_cur_plan_block->step_indexer];
                float sqr_step_speed = 0;
                itp_blk_data[itp_blk_data_write].idle_axis = 0;
                for (uint8_t i = STEPPER_COUNT; i != 0;)
                {
                    i--;
                    sqr_step_speed += fast_flt_pow2((float)itp_cur_plan_block->steps[i]);
                    itp_blk_data[itp_blk_data_write].errors[i] = itp_cur_plan_block->total_steps;
                    itp_blk_data[itp_blk_data_write].steps[i] = itp_cur_plan_block->steps[i] << 1;
                    if (itp_cur_plan_block->steps[i] == itp_cur_plan_block->total_steps)
                    {
                        itp_blk_data[itp_blk_data_write].main_axis = i;
                    }
                    if (!itp_cur_plan_block->steps[i])
                    {
                        itp_blk_data[itp_blk_data_write].idle_axis |= (1 << i);
                    }
                }

                sqr_step_speed *= fast_flt_pow2(total_step_inv);
                feed_convert *= fast_flt_sqrt(sqr_step_speed);

                //initializes data for generating step segments
                unprocessed_steps = itp_cur_plan_block->total_steps;

                //flags block for recalculation of speeds
                itp_needs_update = true;
                //in every new block speed update is needed
                is_initial_transition = true;

                half_speed_change = INTEGRATOR_DELTA_T * itp_cur_plan_block->acceleration;
                half_speed_change = fast_flt_div2(half_speed_change);
            }

            if (itp_sgm_is_full()) //re-checks in case an injected dweel filled the buffer
            {
                break;
            }

            sgm = &itp_sgm_data[itp_sgm_data_write];
            sgm->block = &itp_blk_data[itp_blk_data_write];

            //if an hold is active forces to deaccelerate
            if (cnc_get_exec_state(EXEC_HOLD))
            {
                //forces deacceleration by overriding the profile juntion points
                accel_until = unprocessed_steps;
                deaccel_from = unprocessed_steps;
                itp_needs_update = true;
            }
            else if (itp_needs_update) //forces recalculation of acceleration and deacceleration profiles
            {
                itp_needs_update = false;
                float exit_speed_sqr = planner_get_block_exit_speed_sqr();
                junction_speed_sqr = planner_get_block_top_speed();

                accel_until = unprocessed_steps;
                deaccel_from = 0;
                if (junction_speed_sqr != itp_cur_plan_block->entry_feed_sqr)
                {
                    float accel_dist = ABS(junction_speed_sqr - itp_cur_plan_block->entry_feed_sqr) / itp_cur_plan_block->acceleration;
                    accel_dist = fast_flt_div2(accel_dist);
                    accel_until -= floorf(accel_dist);
                    initial_accel_negative = (junction_speed_sqr < itp_cur_plan_block->entry_feed_sqr);
                }

                //if entry speed already a junction speed updates it.
                if (accel_until == unprocessed_steps)
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
            //acceleration profile
            if (unprocessed_steps > accel_until)
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
                sgm->update_speed = 1;
                is_initial_transition = true;
            }
            else if (unprocessed_steps > deaccel_from)
            {
                //constant speed segment
                speed_change = 0;
                profile_steps_limit = deaccel_from;
                sgm->update_speed = is_initial_transition ? 2 : 0;
                is_initial_transition = false;
            }
            else
            {
                speed_change = -half_speed_change;
                profile_steps_limit = 0;
                sgm->update_speed = 1;
                is_initial_transition = true;
            }

            float current_speed = fast_flt_sqrt(itp_cur_plan_block->entry_feed_sqr);
            /*
        	common calculations for all three profiles (accel, constant and deaccel)
        */
            current_speed += speed_change;
            //if on active hold state
            if (cnc_get_exec_state(EXEC_HOLD))
            {
                if (current_speed < 0)
                {
                    //after a feed hold if 0 speed reached exits and starves the buffer
                    return;
                }
            }

            float partial_distance = MIN(current_speed * INTEGRATOR_DELTA_T, 65535.0f);
            //computes how many steps it will perform at this speed and frame window
            uint16_t segm_steps = (uint16_t)floorf(partial_distance);
            //if traveled distance is less the one step fits at least one step
            if (segm_steps == 0)
            {
                segm_steps = 1;
            }
            //if computed steps exceed the remaining steps for the motion shortens the distance
            if (segm_steps > (unprocessed_steps - profile_steps_limit))
            {
                segm_steps = (uint16_t)(unprocessed_steps - profile_steps_limit);
            }

            if (speed_change)
            {
                float new_speed_sqr = itp_cur_plan_block->acceleration * segm_steps;
                new_speed_sqr = fast_flt_mul2(new_speed_sqr);
                if (speed_change > 0)
                {
                    //calculates the final speed at the end of this position
                    new_speed_sqr += itp_cur_plan_block->entry_feed_sqr;
                }
                else
                {
                    //calculates the final speed at the end of this position
                    new_speed_sqr = itp_cur_plan_block->entry_feed_sqr - new_speed_sqr;
                    new_speed_sqr = MAX(new_speed_sqr, 0); //avoids rounding errors since speed is always positive
                }
                current_speed = (fast_flt_sqrt(new_speed_sqr) + fast_flt_sqrt(itp_cur_plan_block->entry_feed_sqr));
                current_speed = fast_flt_div2(current_speed);
                itp_cur_plan_block->entry_feed_sqr = new_speed_sqr;
            }

//The DSS (Dynamic Step Spread) algorithm reduces stepper vibration by spreading step distribution at lower speads.
//This is done by oversampling the Bresenham line algorithm by multiple factors of 2.
//This way stepping actions fire in different moments in order to reduce vibration caused by the stepper internal mechanics.
//This works in a similar way to Grbl's AMASS but has a modified implementation to minimize the processing penalty on the ISR and also take less static memory.
//DSS never loads the step generating ISR with a frequency above half of the absolute maximum frequency
#if (DSS_MAX_OVERSAMPLING != 0)
            uint32_t step_speed = (uint32_t)round(current_speed);
            static uint8_t prev_dss = 0;
            uint8_t dss = 0;
            while (step_speed < (F_STEP_MAX >> 2) && dss < DSS_MAX_OVERSAMPLING && segm_steps > 1)
            {
                step_speed <<= 1;
                dss++;
            }

            sgm->next_dss = dss - prev_dss;
            prev_dss = dss;

            //completes the segment information (step speed, steps) and updates the block
            sgm->remaining_steps = segm_steps << dss;
            mcu_freq_to_clocks((float)step_speed, &(sgm->timer_counter), &(sgm->timer_prescaller));
#else
        sgm->remaining_steps = segm_steps;
        mcu_freq_to_clocks(current_speed, &(sgm->timer_counter), &(sgm->timer_prescaller));
#endif
            itp_cur_plan_block->total_steps -= segm_steps;

            sgm->feed = current_speed * feed_convert;
#ifdef USE_SPINDLE
#ifdef LASER_MODE
            float top_speed_inv = fast_flt_invsqrt(junction_speed_sqr);
            planner_get_spindle_speed(MIN(1, current_speed * top_speed_inv), &(sgm->spindle), &(sgm->spindle_inv));
#else
            planner_get_spindle_speed(1, &(sgm->spindle), &(sgm->spindle_inv));
#endif
#endif
            unprocessed_steps -= segm_steps;

            if (unprocessed_steps == accel_until) //resets float additions error
            {
                itp_cur_plan_block->entry_feed_sqr = junction_speed_sqr;
                itp_cur_plan_block->total_steps = accel_until;
            }
            else if (unprocessed_steps == deaccel_from) //resets float additions error
            {
                itp_cur_plan_block->total_steps = deaccel_from;
            }

            //finally write the segment
            itp_sgm_buffer_write();

            if (unprocessed_steps == 0)
            {
                itp_blk_buffer_write();
                itp_cur_plan_block = NULL;
                planner_discard_block(); //discards planner block
#if (DSS_MAX_OVERSAMPLING != 0)
                prev_dss = 0;
#endif
                //accel_profile = 0; //no updates necessary to planner
                //break;
            }
        }

#ifdef USE_COOLANT
        //updated the coolant pins
        io_set_coolant(planner_get_coolant());
#endif

        //starts the step isr if is stopped and there are segments to execute
        if (!cnc_get_exec_state(EXEC_HOLD | EXEC_ALARM | EXEC_RUN) && (itp_sgm_data_slots != INTERPOLATOR_BUFFER_SIZE)) //exec state is not hold or alarm and not already running
        {
#ifdef STEPPER_ENABLE
            mcu_clear_output(STEPPER_ENABLE);
#endif
            cnc_set_exec_state(EXEC_RUN); //flags that it started running
            mcu_start_itp_isr(itp_sgm_data[itp_sgm_data_read].timer_counter, itp_sgm_data[itp_sgm_data_read].timer_prescaller);
        }
    }

    void itp_update(void)
    {
        //flags executing block for update
        itp_needs_update = true;
    }

    void itp_stop(void)
    {
        mcu_stop_itp_isr();
        cnc_clear_exec_state(EXEC_RUN);
#ifdef LASER_MODE
        if (g_settings.laser_mode)
        {
            io_set_spindle(0, false);
            itp_rt_spindle = 0;
        }
#endif
    }

    void itp_clear(void)
    {
        itp_cur_plan_block = NULL;
        itp_running_sgm = NULL;
        //syncs the stored position and the real position
        memcpy(itp_step_pos, itp_rt_step_pos, sizeof(itp_step_pos));
        itp_sgm_data_write = 0;
        itp_sgm_data_read = 0;
        itp_sgm_data_slots = INTERPOLATOR_BUFFER_SIZE;
        itp_blk_clear();
    }

    void itp_get_rt_position(uint32_t *position)
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
        }
        else
        {
            memset(itp_rt_step_pos, 0, sizeof(itp_rt_step_pos));
        }
    }

    float itp_get_rt_feed(void)
    {
        float feed = 0;
        if (!cnc_get_exec_state(EXEC_RUN))
        {
            return feed;
        }

        if (itp_sgm_data_slots != INTERPOLATOR_BUFFER_SIZE)
        {
            feed = itp_sgm_data[itp_sgm_data_read].feed;
        }

        return feed;
    }

#ifdef USE_SPINDLE
    uint16_t itp_get_rt_spindle(void)
    {
        float spindle = (float)itp_rt_spindle;
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

    //always fires after pulse
    void itp_step_reset_isr(void)
    {
        //always resets all stepper pins
        io_set_steps(g_settings.step_invert_mask);

        if (itp_isr_finnished)
        {
            itp_stop(); //the buffer is empty. The ISR can stop
            return;     //itp_sgm is null
        }

        if (itp_running_sgm == NULL) //just in case it reenters
        {
            return;
        }

        //if segment needs to update the step ISR (after preloading first step byte)
        if (itp_running_sgm->update_speed)
        {
            if (itp_running_sgm->update_speed & 0x01)
            {
                mcu_change_itp_isr(itp_running_sgm->timer_counter, itp_running_sgm->timer_prescaller);
            }
            io_set_dirs(itp_running_sgm->block->dirbits);
#ifdef USE_SPINDLE
            io_set_spindle(itp_running_sgm->spindle, itp_running_sgm->spindle_inv);
            itp_rt_spindle = itp_running_sgm->spindle;
#endif
            itp_running_sgm->update_speed = 0;
        }

        //one step remaining discards current segment
        if (!itp_running_sgm->remaining_steps)
        {
            itp_running_sgm = NULL;
            itp_sgm_buffer_read();
        }
    }

    void itp_step_isr(void)
    {
        static uint8_t stepbits = 0;
        if (itp_busy) //prevents reentrancy
        {
            return;
        }

        //sets step bits
        io_toggle_steps(stepbits);
        stepbits = 0;

        itp_busy = true;
        mcu_enable_global_isr();

        //if buffer empty loads one
        if (itp_running_sgm == NULL)
        {
            //if buffer is not empty
            if (itp_sgm_data_slots < INTERPOLATOR_BUFFER_SIZE)
            {
                //loads a new segment
                itp_running_sgm = &itp_sgm_data[itp_sgm_data_read];
                cnc_set_exec_state(EXEC_RUN);
                itp_isr_finnished = false;
                if (itp_running_sgm->block != NULL)
                {
#if (DSS_MAX_OVERSAMPLING != 0)
                    if (itp_running_sgm->next_dss != 0)
                    {
                        itp_running_sgm->block->main_axis = 0; //disables direct step increment to force step calculation
                        if (!(itp_running_sgm->next_dss & 0xF8))
                        {
                            itp_running_sgm->block->total_steps <<= itp_running_sgm->next_dss;
#if (STEPPER_COUNT > 0 && defined(STEP0))
                            itp_running_sgm->block->errors[0] <<= itp_running_sgm->next_dss;
#endif
#if (STEPPER_COUNT > 1 && defined(STEP1))
                            itp_running_sgm->block->errors[1] <<= itp_running_sgm->next_dss;
#endif
#if (STEPPER_COUNT > 2 && defined(STEP2))
                            itp_running_sgm->block->errors[2] <<= itp_running_sgm->next_dss;
#endif
#if (STEPPER_COUNT > 3 && defined(STEP3))
                            itp_running_sgm->block->errors[3] <<= itp_running_sgm->next_dss;
#endif
#if (STEPPER_COUNT > 4 && defined(STEP4))
                            itp_running_sgm->block->errors[4] <<= itp_running_sgm->next_dss;
#endif
#if (STEPPER_COUNT > 5 && defined(STEP5))
                            itp_running_sgm->block->errors[5] <<= itp_running_sgm->next_dss;
#endif
                        }
                        else
                        {
                            itp_running_sgm->next_dss = -itp_running_sgm->next_dss;
                            itp_running_sgm->block->total_steps >>= itp_running_sgm->next_dss;
#if (STEPPER_COUNT > 0 && defined(STEP0))
                            itp_running_sgm->block->errors[0] >>= itp_running_sgm->next_dss;
#endif
#if (STEPPER_COUNT > 1 && defined(STEP1))
                            itp_running_sgm->block->errors[1] >>= itp_running_sgm->next_dss;
#endif
#if (STEPPER_COUNT > 2 && defined(STEP2))
                            itp_running_sgm->block->errors[2] >>= itp_running_sgm->next_dss;
#endif
#if (STEPPER_COUNT > 3 && defined(STEP3))
                            itp_running_sgm->block->errors[3] >>= itp_running_sgm->next_dss;
#endif
#if (STEPPER_COUNT > 4 && defined(STEP4))
                            itp_running_sgm->block->errors[4] >>= itp_running_sgm->next_dss;
#endif
#if (STEPPER_COUNT > 5 && defined(STEP5))
                            itp_running_sgm->block->errors[5] >>= itp_running_sgm->next_dss;
#endif
                        }
                    }
#endif
                }
            }
            else
            {
                itp_isr_finnished = true;
            }
        }

        //is steps remaining starts calc next step bits
        if (itp_running_sgm != NULL)
        {
            itp_running_sgm->remaining_steps--;
            bool dostep;
            if (itp_running_sgm->block != NULL)
            {
//prepares the next step bits mask
#if (STEPPER_COUNT > 0 && defined(STEP0))
                dostep = false;
                if (itp_running_sgm->block->main_axis == 0)
                {
                    dostep = true;
                }
                else if (!(itp_running_sgm->block->idle_axis & STEP0_MASK))
                {
                    itp_running_sgm->block->errors[0] += itp_running_sgm->block->steps[0];
                    if (itp_running_sgm->block->errors[0] > itp_running_sgm->block->total_steps)
                    {
                        itp_running_sgm->block->errors[0] -= itp_running_sgm->block->total_steps;
                        dostep = true;
                    }
                }

                if (dostep)
                {
                    stepbits |= STEP0_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
                    if (!itp_running_sgm->block->backlash_comp)
                    {
#endif
                        if (itp_running_sgm->block->dirbits & DIR0_MASK)
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
#if (STEPPER_COUNT > 1 && defined(STEP1))
                dostep = false;
                if (itp_running_sgm->block->main_axis == 1)
                {
                    dostep = true;
                }
                else if (!(itp_running_sgm->block->idle_axis & STEP1_MASK))
                {
                    itp_running_sgm->block->errors[1] += itp_running_sgm->block->steps[1];
                    if (itp_running_sgm->block->errors[1] > itp_running_sgm->block->total_steps)
                    {
                        itp_running_sgm->block->errors[1] -= itp_running_sgm->block->total_steps;
                        dostep = true;
                    }
                }

                if (dostep)
                {
                    stepbits |= STEP1_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
                    if (!itp_running_sgm->block->backlash_comp)
                    {
#endif
                        if (itp_running_sgm->block->dirbits & DIR1_MASK)
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
#if (STEPPER_COUNT > 2 && defined(STEP2))
                dostep = false;
                if (itp_running_sgm->block->main_axis == 2)
                {
                    dostep = true;
                }
                else if (!(itp_running_sgm->block->idle_axis & STEP2_MASK))
                {
                    itp_running_sgm->block->errors[2] += itp_running_sgm->block->steps[2];
                    if (itp_running_sgm->block->errors[2] > itp_running_sgm->block->total_steps)
                    {
                        itp_running_sgm->block->errors[2] -= itp_running_sgm->block->total_steps;
                        dostep = true;
                    }
                }

                if (dostep)
                {
                    stepbits |= STEP2_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
                    if (!itp_running_sgm->block->backlash_comp)
                    {
#endif
                        if (itp_running_sgm->block->dirbits & DIR2_MASK)
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
#if (STEPPER_COUNT > 3 && defined(STEP3))
                dostep = false;
                if (itp_running_sgm->block->main_axis == 3)
                {
                    dostep = true;
                }
                else if (!(itp_running_sgm->block->idle_axis & STEP3_MASK))
                {
                    itp_running_sgm->block->errors[3] += itp_running_sgm->block->steps[3];
                    if (itp_running_sgm->block->errors[3] > itp_running_sgm->block->total_steps)
                    {
                        itp_running_sgm->block->errors[3] -= itp_running_sgm->block->total_steps;
                        dostep = true;
                    }
                }

                if (dostep)
                {
                    stepbits |= STEP3_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
                    if (!itp_running_sgm->block->backlash_comp)
                    {
#endif
                        if (itp_running_sgm->block->dirbits & DIR3_MASK)
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
#if (STEPPER_COUNT > 4 && defined(STEP4))
                dostep = false;
                if (itp_running_sgm->block->main_axis == 4)
                {
                    dostep = true;
                }
                else if (!(itp_running_sgm->block->idle_axis & STEP4_MASK))
                {
                    itp_running_sgm->block->errors[4] += itp_running_sgm->block->steps[4];
                    if (itp_running_sgm->block->errors[4] > itp_running_sgm->block->total_steps)
                    {
                        itp_running_sgm->block->errors[4] -= itp_running_sgm->block->total_steps;
                        dostep = true;
                    }
                }

                if (dostep)
                {
                    stepbits |= STEP4_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
                    if (!itp_running_sgm->block->backlash_comp)
                    {
#endif
                        if (itp_running_sgm->block->dirbits & DIR4_MASK)
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
#if (STEPPER_COUNT > 5 && defined(STEP5))
                dostep = false;
                if (itp_running_sgm->block->main_axis == 5)
                {
                    dostep = true;
                }
                else if (!(itp_running_sgm->block->idle_axis & STEP5_MASK))
                {
                    itp_running_sgm->block->errors[5] += itp_running_sgm->block->steps[5];
                    if (itp_running_sgm->block->errors[5] > itp_running_sgm->block->total_steps)
                    {
                        itp_running_sgm->block->errors[5] -= itp_running_sgm->block->total_steps;
                        dostep = true;
                    }
                }

                if (dostep)
                {
                    stepbits |= STEP5_ITP_MASK;
#ifdef ENABLE_BACKLASH_COMPENSATION
                    if (!itp_running_sgm->block->backlash_comp)
                    {
#endif
                        if (itp_running_sgm->block->dirbits & DIR5_MASK)
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
        }

#ifdef ENABLE_DUAL_DRIVE_AXIS
        stepbits &= ~itp_step_lock;
#endif
        mcu_disable_global_isr(); //lock isr before clearin busy flag
        itp_busy = false;
    }

    void itp_delay(uint16_t delay)
    {
        itp_sgm_data[itp_sgm_data_write].block = NULL;
        //clicks every 100ms (10Hz)
        mcu_freq_to_clocks(10, &(itp_sgm_data[itp_sgm_data_write].timer_counter), &(itp_sgm_data[itp_sgm_data_write].timer_prescaller));
        itp_sgm_data[itp_sgm_data_write].remaining_steps = delay;
        itp_sgm_data[itp_sgm_data_write].update_speed = true;
        itp_sgm_data[itp_sgm_data_write].feed = 0;
#ifdef USE_SPINDLE
#ifdef LASER_MODE
        if (g_settings.laser_mode)
        {
            itp_sgm_data[itp_sgm_data_write].spindle = 0;
            itp_sgm_data[itp_sgm_data_write].spindle_inv = false;
        }
        else
        {
            planner_get_spindle_speed(1, &(itp_sgm_data[itp_sgm_data_write].spindle), &(itp_sgm_data[itp_sgm_data_write].spindle_inv));
        }
#else
        planner_get_spindle_speed(1, &(itp_sgm_data[itp_sgm_data_write].spindle), &(itp_sgm_data[itp_sgm_data_write].spindle_inv));
#endif
#endif
        itp_sgm_buffer_write();
    }

#ifdef __cplusplus
}
#endif
