/*
	Name: itp_linear.c
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
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include "config.h"
#include "mcu.h"
#include "machinedefs.h"
#include "kinematics.h"
#include "interpolator.h"
#include "settings.h"
#include "planner.h"
#include "utils.h"
#include "cnc.h"
#include "io_control.h"

#define F_INTEGRATOR 100
#define INTEGRATOR_DELTA_T (1.0f / F_INTEGRATOR)
//the amount of motion precomputed and stored for the step generator is never less then
//the size of the buffer x time window size
//in this case the buffer never holds less then 100ms of motions

//integrator calculates 10ms (minimum size) time frame windows
#define INTERPOLATOR_BUFFER_SIZE 5 //number of windows in the buffer

//contains data of the block being executed by the pulse routine
//this block has the necessary data to execute the Bresenham line algorithm
typedef struct itp_blk_
{
    uint8_t dirbits;
    uint32_t steps[STEPPER_COUNT];
    uint32_t totalsteps;
    uint32_t errors[STEPPER_COUNT];
    #ifdef GCODE_PROCESS_LINE_NUMBERS
    uint32_t line;
    #endif
} INTERPOLATOR_BLOCK;

//contains data of the block segment being executed by the pulse and integrator routines
//the segment is a fragment of the motion defined in the block
//this also contains the acceleration/deacceleration info
typedef struct pulse_sgm_
{
    INTERPOLATOR_BLOCK *block;
    uint8_t next_stepbits;
    uint16_t remaining_steps;
    uint16_t clocks_per_tick;
    uint8_t ticks_per_step;
    #ifdef USE_SPINDLE
    uint8_t spindle;
    bool spindle_inv;
    #endif
    float feed;
    bool update_speed;
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
static INTERPOLATOR_SEGMENT *itp_running_sgm;

//stores the current position of the steppers in the interpolator after processing a planner block
static uint32_t itp_step_pos[STEPPER_COUNT];
//keeps track of the machine realtime position
static uint32_t itp_rt_step_pos[STEPPER_COUNT];
volatile static uint8_t itp_rt_spindle;
//flag to force the interpolator to recalc entry and exit limit position of acceleration/deacceleration curves
static bool itp_needs_update;
static volatile bool itp_isr_finnished;

volatile static bool itp_busy;

/*
	Interpolator segment buffer functions
*/
static inline void itp_sgm_buffer_read()
{
    itp_sgm_data_slots++;
    if (++itp_sgm_data_read == INTERPOLATOR_BUFFER_SIZE)
    {
        itp_sgm_data_read = 0;
    }
}

static inline void itp_sgm_buffer_write()
{
    itp_sgm_data_slots--;
    if (++itp_sgm_data_write == INTERPOLATOR_BUFFER_SIZE)
    {
        itp_sgm_data_write = 0;
    }
}

static inline bool itp_sgm_is_empty()
{
    return (itp_sgm_data_slots == INTERPOLATOR_BUFFER_SIZE);
}

static inline bool itp_sgm_is_full()
{
    return (itp_sgm_data_slots == 0);
}

static inline void itp_sgm_clear()
{
    itp_sgm_data_write = 0;
    itp_sgm_data_read = 0;
    itp_sgm_data_slots = INTERPOLATOR_BUFFER_SIZE;
    memset(itp_sgm_data, 0, sizeof(itp_sgm_data));
}

/*
	Interpolator block buffer functions
*/
//NOT NEEDED
/*
static inline void itp_blk_buffer_read()
{
	itp_blk_data_read++;
	itp_blk_data_slots++;
	if (itp_blk_data_read == INTERPOLATOR_BUFFER_SIZE)
	{
		itp_blk_data_read = 0;
	}
}*/

static inline void itp_blk_buffer_write()
{
    //itp_blk_data_slots--; //AUTOMATIC LOOP
    if (++itp_blk_data_write == INTERPOLATOR_BUFFER_SIZE)
    {
        itp_blk_data_write = 0;
    }
}
/* NOT NECESSARY
static inline bool itp_blk_is_empty()
{
	return (itp_blk_data_slots == INTERPOLATOR_BUFFER_SIZE);
}*/

/*static inline bool itp_blk_is_full()
{
	return (itp_blk_data_slots == 0);
}*/

static inline void itp_blk_clear()
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
void itp_init()
{
#ifdef FORCE_GLOBALS_TO_0
    //resets buffers
    memset(&itp_step_pos, 0, sizeof(itp_step_pos));
    memset(&itp_rt_step_pos, 0, sizeof(itp_rt_step_pos));
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

void itp_run()
{
    //static INTERPOLATOR_BLOCK *new_block = NULL;
    //static bool read_new_block = true;
    //conversion vars
    static float min_step_distance = 0;
    static float steps_per_mm = 0;
    //limits of the speed profile
    static uint32_t accel_until = 0;
    static uint32_t deaccel_from = 0;
    static float junction_speed_sqr = 0;
    static float half_speed_change = 0;
    static bool initial_accel_negative = false;

    //accel profile vars
    //static float processed_steps = 0;
    static uint32_t unprocessed_steps = 0;
    //static uint8_t accel_profile = 0;

    INTERPOLATOR_SEGMENT *sgm = NULL;

    //creates segments and fills the buffer
    while (!itp_sgm_is_full())
    {
        if(cnc_get_exec_state(EXEC_ALARM))
        {
            //on any active alarm exits
            return;
        }
        /* NOT NECESSARY
        /* BLOCKS CAN BE OVERWRITTEN SINCE THEY NEVER WILL BE LARGER THEN THE NUMBER OF SEGMENTS
        //flushes completed blocks
        /*
        if (itp_sgm_data[itp_sgm_data_read].block != NULL)
        {
        	if (!itp_blk_is_empty() && (itp_sgm_data[itp_sgm_data_read].block != &itp_blk_data[itp_blk_data_read]))
        	{
        		itp_blk_buffer_read();
        	}
        }*/

        //no planner blocks has beed processed or last planner block was fully processed
        if (itp_cur_plan_block == NULL)
        {
            //planner is empty or interpolator block buffer full. Nothing to be done
            if (planner_buffer_is_empty()/* || itp_blk_is_full()*/)
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

            if (itp_cur_plan_block->distance == 0)
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

            //erases previous values
            itp_blk_data[itp_blk_data_write].dirbits = 0;
            itp_blk_data[itp_blk_data_write].totalsteps = 0;
            //itp_blk_data[itp_blk_data_write].step_freq = F_STEP_MIN;

            uint32_t step_new_pos[STEPPER_COUNT];
            //applies the inverse kinematic to get next position in steps
            kinematics_apply_inverse(itp_cur_plan_block->pos, (uint32_t *)&step_new_pos);

            //calculates the number of steps to execute
            itp_blk_data[itp_blk_data_write].dirbits = 0;
            for (uint8_t i = STEPPER_COUNT; i != 0;)
            {
                i--;
                itp_blk_data[itp_blk_data_write].steps[i] = step_new_pos[i] - itp_step_pos[i];

                if(itp_blk_data[itp_blk_data_write].steps[i] > (uint32_t)INT32_MAX)
                {
                    itp_blk_data[itp_blk_data_write].dirbits |= (1<<i);
                    itp_blk_data[itp_blk_data_write].steps[i] = ~itp_blk_data[itp_blk_data_write].steps[i] + 1;
                }

                itp_blk_data[itp_blk_data_write].totalsteps = MAX(itp_blk_data[itp_blk_data_write].totalsteps, itp_blk_data[itp_blk_data_write].steps[i]);
            }

            //copies data for interpolator step_pos
            //itp_blk_data[itp_blk_data_write].dirbits = itp_cur_plan_block->dirbits;
            memcpy(&(itp_step_pos), &(step_new_pos), sizeof(step_new_pos));

            //calculates conversion vars
            steps_per_mm = ((float)itp_blk_data[itp_blk_data_write].totalsteps) / itp_cur_plan_block->distance;
            min_step_distance = 1.0f / steps_per_mm;

            //initializes data for generating step segments
            unprocessed_steps = itp_blk_data[itp_blk_data_write].totalsteps;

            //flags block for recalculation of speeds
            itp_needs_update = true;

            half_speed_change = 0.5f * INTEGRATOR_DELTA_T * itp_cur_plan_block->acceleration;

            uint32_t error = itp_blk_data[itp_blk_data_write].totalsteps >> 1;
            for (uint8_t i = 0; i < STEPPER_COUNT; i++)
            {
                itp_blk_data[itp_blk_data_write].errors[i] = error;
            }
        }

        if(itp_sgm_is_full()) //re-checks in case an injected dweel filled the buffer
        {
            break;
        }

        //uint32_t prev_unprocessed_steps = unprocessed_steps;
        //float min_delta = 0;

        sgm = &itp_sgm_data[itp_sgm_data_write];
        sgm->block = &itp_blk_data[itp_blk_data_write];
        //sgm->update_speed = true;

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
            junction_speed_sqr = planner_get_block_top_speed(exit_speed_sqr);

            accel_until = unprocessed_steps;
            deaccel_from = 0;
            if (junction_speed_sqr != itp_cur_plan_block->entry_feed_sqr)
            {
                float accel_dist = 0.5f * ABS(junction_speed_sqr - itp_cur_plan_block->entry_feed_sqr) * itp_cur_plan_block->accel_inv;
                accel_until -= floorf(accel_dist * steps_per_mm);
                initial_accel_negative = (junction_speed_sqr < itp_cur_plan_block->entry_feed_sqr);
            }
            
            //if entry speed already a junction speed updates it.
            if(accel_until == unprocessed_steps)
            {
            	itp_cur_plan_block->entry_feed_sqr = junction_speed_sqr;
			}

            if (junction_speed_sqr > exit_speed_sqr)
            {
                float deaccel_dist = 0.5f * (junction_speed_sqr - exit_speed_sqr) * itp_cur_plan_block->accel_inv;
                deaccel_from = floorf(deaccel_dist * steps_per_mm);
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
            sgm->update_speed = true;

        }
        else if (unprocessed_steps > deaccel_from)
        {
            /*if(unprocessed_steps == accel_until)	//first time in const step speed updates current speed
            {
            	itp_cur_plan_block->entry_feed_sqr = junction_speed_sqr;
            }*/
            //constant speed segment
            speed_change = 0;
            profile_steps_limit = deaccel_from;
            sgm->update_speed = false;
        }
        else
        {
            speed_change = -half_speed_change;
            profile_steps_limit = 0;
            sgm->update_speed = true;
        }

        float current_speed = fast_sqrt(itp_cur_plan_block->entry_feed_sqr);
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

        float partial_distance = current_speed * INTEGRATOR_DELTA_T;
        //computes how many steps it can perform at this speed and frame window
        uint16_t steps = (uint16_t)floorf(partial_distance * steps_per_mm);
        //if traveled distance is less the one step fits at least one step
        if (steps == 0)
        {
            steps = 1;
        }
        //if computed steps exceed the remaining steps for the motion shortens the distance
        if (steps > (unprocessed_steps - profile_steps_limit))
        {
            steps = (uint16_t)(unprocessed_steps - profile_steps_limit);
        }

        //recalculates the precise distance to travel the given amount os steps
        partial_distance = steps * min_step_distance;

        if(sgm->update_speed)
        {
            float new_speed_sqr;
            if(speed_change>0)
            {
                //calculates the final speed at the end of this position
                new_speed_sqr = 2 * itp_cur_plan_block->acceleration * partial_distance + itp_cur_plan_block->entry_feed_sqr;
            }
            else
            {
                //calculates the final speed at the end of this position
                new_speed_sqr = itp_cur_plan_block->entry_feed_sqr - (2 * itp_cur_plan_block->acceleration * partial_distance);
                new_speed_sqr = MAX(new_speed_sqr, 0); //avoids rounding errors since speed is always positive
            }
            current_speed = 0.5f * (fast_sqrt(new_speed_sqr) + fast_sqrt(itp_cur_plan_block->entry_feed_sqr));
            itp_cur_plan_block->entry_feed_sqr = new_speed_sqr;
        }

        //completes the segment information (step speed, steps) and updates the block
        mcu_freq_to_clocks(current_speed * steps_per_mm, &(sgm->clocks_per_tick), &(sgm->ticks_per_step));
        sgm->remaining_steps = steps;
        itp_cur_plan_block->distance -= partial_distance;

        sgm->feed = current_speed;
        float top_speed_inv = fast_inv_sqrt(junction_speed_sqr);
        #ifdef LASER_MODE
        planner_get_spindle_speed(MIN(1, current_speed * top_speed_inv), &(sgm->spindle), &(sgm->spindle_inv));
        #else
        planner_get_spindle_speed(1, &(sgm->spindle), &(sgm->spindle_inv));
        #endif
        unprocessed_steps -= sgm->remaining_steps;

        if (unprocessed_steps == accel_until) //resets float additions error
        {
            itp_cur_plan_block->entry_feed_sqr = junction_speed_sqr;
            itp_cur_plan_block->distance = min_step_distance * accel_until;
        }
        else if (unprocessed_steps == deaccel_from) //resets float additions error
        {
            itp_cur_plan_block->distance = min_step_distance * deaccel_from;
        }

        itp_sgm_buffer_write();

        if (unprocessed_steps == 0)
        {
            itp_blk_buffer_write();
            itp_cur_plan_block = NULL;
            planner_discard_block(); //discards planner block
            //accel_profile = 0; //no updates necessary to planner
            //break;
        }
    }

    //starts the step isr if is stopped and there are segments to execute
    if (!cnc_get_exec_state(EXEC_HOLD | EXEC_ALARM | EXEC_RUN) && (itp_sgm_data_slots != INTERPOLATOR_BUFFER_SIZE)) //exec state is not hold or alarm and not already running
    {
#ifdef STEPPER_ENABLE
        io_set_outputs(STEPPER_ENABLE);
#endif
        cnc_set_exec_state(EXEC_RUN); //flags that it started running
        mcu_start_step_ISR(itp_sgm_data[itp_sgm_data_read].clocks_per_tick, itp_sgm_data[itp_sgm_data_read].ticks_per_step);
    }
}

void itp_update()
{
    //flags executing block for update
    itp_needs_update = true;
}

void itp_stop()
{
    mcu_step_stop_ISR();
    cnc_clear_exec_state(EXEC_RUN);
}

void itp_clear()
{
    itp_cur_plan_block = NULL;
    itp_running_sgm = NULL;
    //syncs the stored position and the real position
    memcpy(&itp_step_pos, &itp_rt_step_pos, sizeof(itp_step_pos));
    itp_sgm_data_write = 0;
    itp_sgm_data_read = 0;
    itp_sgm_data_slots = INTERPOLATOR_BUFFER_SIZE;
    itp_blk_clear();
}

void itp_get_rt_position(float *axis)
{
    uint32_t step_pos[STEPPER_COUNT];
    memcpy(step_pos, itp_rt_step_pos, sizeof(step_pos));
    kinematics_apply_forward((uint32_t *)&step_pos, axis);
}

void itp_reset_rt_position()
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

        kinematics_apply_inverse((float *)&origin, (uint32_t *)&itp_rt_step_pos);
    }
    else
    {
        memset(&itp_rt_step_pos, 0, sizeof(itp_rt_step_pos));
    }
}

float itp_get_rt_feed()
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
uint16_t itp_get_rt_spindle()
{
	float spindle = (float)itp_rt_spindle;
	spindle *= g_settings.spindle_max_rpm * UINT8_MAX_INV;
	
	return (uint16_t)roundf(spindle);
}
#endif

#ifdef GCODE_PROCESS_LINE_NUMBERS
uint32_t itp_get_rt_line_number()
{
    return ((itp_sgm_data[itp_sgm_data_read].block!=NULL) ? itp_sgm_data[itp_sgm_data_read].block->line : 0);
}
#endif

//always fires after pulse
void itp_step_reset_isr()
{
    //always resets all stepper pins
    io_set_steps(g_settings.step_invert_mask);
    
    if (itp_isr_finnished)
    {
        itp_stop(); //the buffer is empty. The ISR can stop
        return; //itp_sgm is null
    }
    
    if(itp_running_sgm == NULL) //just in case it reenters
    {
    	return;
	}
    
    //if segment needs to update the step ISR (after preloading first step byte
    if(itp_running_sgm->update_speed)
	{
		//set dir bits
		if(itp_running_sgm->block != NULL)
		{
			io_set_dirs(itp_running_sgm->block->dirbits);
		}
		
		mcu_change_step_ISR(itp_running_sgm->clocks_per_tick, itp_running_sgm->ticks_per_step);
		#ifdef USE_SPINDLE
		mcu_set_pwm(SPINDLE_PWM, itp_running_sgm->spindle);
		if(!itp_running_sgm->spindle_inv)
		{
			mcu_clear_output(SPINDLE_DIR);
		}
		else
		{
			mcu_set_output(SPINDLE_DIR);
		}
		
		itp_rt_spindle = itp_running_sgm->spindle;
		#endif
		itp_running_sgm->update_speed = false;
	}
	
    //one step remaining discards current segment
    if (itp_running_sgm->remaining_steps == 0)
    {
        itp_running_sgm = NULL;
        itp_sgm_buffer_read();
    }
}

void itp_step_isr()
{
    static uint8_t stepbits = 0;
    if(itp_busy) //prevents reentrancy
    {
    	return;
    }

    //sets step bits
    io_toggle_steps(stepbits);
    stepbits = 0;

    itp_busy = true;
    mcu_enable_interrupts();
    
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
        if(itp_running_sgm->block!=NULL)
        {
//prepares the next step bits mask
#ifdef STEP0
            itp_running_sgm->block->errors[0] += itp_running_sgm->block->steps[0];
            if (itp_running_sgm->block->errors[0] > itp_running_sgm->block->totalsteps)
            {
                itp_running_sgm->block->errors[0] -= itp_running_sgm->block->totalsteps;
                stepbits |= STEP0_MASK;
                if (itp_running_sgm->block->dirbits & DIR0_MASK)
                {
                    itp_rt_step_pos[0]--;
                }
                else
                {
                    itp_rt_step_pos[0]++;
                }
            }
#endif
#ifdef STEP1
            itp_running_sgm->block->errors[1] += itp_running_sgm->block->steps[1];
            if (itp_running_sgm->block->errors[1] > itp_running_sgm->block->totalsteps)
            {
                itp_running_sgm->block->errors[1] -= itp_running_sgm->block->totalsteps;
                stepbits |= STEP1_MASK;
                if (itp_running_sgm->block->dirbits & DIR1_MASK)
                {
                    itp_rt_step_pos[1]--;
                }
                else
                {
                    itp_rt_step_pos[1]++;
                }
            }
#endif
#ifdef STEP2
            itp_running_sgm->block->errors[2] += itp_running_sgm->block->steps[2];
            if (itp_running_sgm->block->errors[2] > itp_running_sgm->block->totalsteps)
            {
                itp_running_sgm->block->errors[2] -= itp_running_sgm->block->totalsteps;
                stepbits |= STEP2_MASK;
                if (itp_running_sgm->block->dirbits & DIR2_MASK)
                {
                    itp_rt_step_pos[2]--;
                }
                else
                {
                    itp_rt_step_pos[2]++;
                }
            }
#endif
#ifdef STEP3
            itp_running_sgm->block->errors[3] += itp_running_sgm->block->steps[3];
            if (itp_running_sgm->block->errors[3] > itp_running_sgm->block->totalsteps)
            {
                itp_running_sgm->block->errors[3] -= itp_running_sgm->block->totalsteps;
                stepbits |= STEP3_MASK;
                if (itp_running_sgm->block->dirbits & DIR3_MASK)
                {
                    itp_rt_step_pos[3]--;
                }
                else
                {
                    itp_rt_step_pos[3]++;
                }
            }
#endif
#ifdef STEP4
            itp_running_sgm->block->errors[4] += itp_running_sgm->block->steps[4];
            if (itp_running_sgm->block->errors[4] > itp_running_sgm->block->totalsteps)
            {
                itp_running_sgm->block->errors[4] -= itp_running_sgm->block->totalsteps;
                stepbits |= STEP4_MASK;
                if (itp_running_sgm->block->dirbits & DIR4_MASK)
                {
                    itp_rt_step_pos[4]--;
                }
                else
                {
                    itp_rt_step_pos[4]++;
                }
            }
#endif
#ifdef STEP5
            itp_running_sgm->block->errors[5] += itp_running_sgm->block->steps[5];
            if (itp_running_sgm->block->errors[5] > itp_running_sgm->block->totalsteps)
            {
                itp_running_sgm->block->errors[5] -= itp_running_sgm->block->totalsteps;
                stepbits |= STEP5_MASK;
                if (itp_running_sgm->block->dirbits & DIR5_MASK)
                {
                    itp_rt_step_pos[5]--;
                }
                else
                {
                    itp_rt_step_pos[5]++;
                }
            }
#endif
        }

    }

	mcu_disable_interrupts();//lock isr before clearin busy flag
    itp_busy = false;
}

void itp_delay(uint16_t delay)
{
    itp_sgm_data[itp_sgm_data_write].block = NULL;
    //clicks every 100ms (10Hz)
    mcu_freq_to_clocks(10, &(itp_sgm_data[itp_sgm_data_write].clocks_per_tick), &(itp_sgm_data[itp_sgm_data_write].ticks_per_step));
    itp_sgm_data[itp_sgm_data_write].remaining_steps = delay;
    itp_sgm_data[itp_sgm_data_write].update_speed = true;
    itp_sgm_data[itp_sgm_data_write].feed = 0;
    #ifdef LASER_MODE
    itp_sgm_data[itp_sgm_data_write].spindle = 0;
    itp_sgm_data[itp_sgm_data_write].spindle_inv = false;
    #else
    planner_get_spindle_speed(1, &(itp_sgm_data[itp_sgm_data_write].spindle), &(itp_sgm_data[itp_sgm_data_write].spindle_inv));
    #endif
    itp_sgm_buffer_write();
}
