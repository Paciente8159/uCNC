/*
	Name: interpolator_linear.c
	Description: Implementation of a linear acceleration interpolator for uCNC.
		The linear acceleration interpolator generates step profiles with constant acceleration.
		
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 13/10/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
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

#define F_INTEGRATOR 100
#define INTEGRATOR_DELTA_T (1.0f/F_INTEGRATOR)
//the amount of motion precomputed and stored for the step generator is never less then
//the size of the buffer x time window size
//in this case the buffer never holds less then 100ms of motions

 //integrator calculates 10ms (minimum size) time frame windows
#define INTERPOLATOR_BUFFER_SIZE 6 //number of windows in the buffer

//contains data of the block being executed by the pulse routine
//this block has the necessary data to execute the Bresenham line algorithm
typedef struct interpolator_blk_
{
	uint8_t dirbits;
	uint32_t steps[STEPPER_COUNT];
	uint32_t totalsteps;
	uint32_t errors[STEPPER_COUNT];
}INTERPOLATOR_BLOCK;

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
	float feed;
	bool update_speed;
}INTERPOLATOR_SEGMENT;

//circular buffers
//creates new type PULSE_BLOCK_BUFFER
static INTERPOLATOR_BLOCK interpolator_blk_data[INTERPOLATOR_BUFFER_SIZE];
uint8_t interpolator_blk_data_write;
uint8_t interpolator_blk_data_read;
uint8_t interpolator_blk_data_slots;

static INTERPOLATOR_SEGMENT interpolator_sgm_data[INTERPOLATOR_BUFFER_SIZE];
volatile uint8_t interpolator_sgm_data_write;
uint8_t interpolator_sgm_data_read;
volatile uint8_t interpolator_sgm_data_slots;
//static buffer_t interpolator_sgm_buffer;

static PLANNER_BLOCK *interpolator_cur_plan_block;
//pointer to the segment being executed
static INTERPOLATOR_SEGMENT *interpolator_running_sgm;

//stores the current position of the steppers in the interpolator after processing a planner block
static uint32_t interpolator_step_pos[STEPPER_COUNT];
//keeps track of the machine realtime position
static uint32_t interpolator_rt_step_pos[STEPPER_COUNT];
static volatile float interpolator_rt_feed;
//flag to force the interpolator to recalc entry and exit limit position of acceleration/deacceleration curves
static bool interpolator_needs_update;
//static volatile uint8_t interpolator_dirbits;
//initial values for bresenham algorithm
//this is shared between pulse and pulsereset functions 
static uint8_t dirbitsmask[STEPPER_COUNT];
static volatile bool interpolator_isr_finnished;
//static volatile bool interpolator_running;

/*
	Interpolator segment buffer functions
*/
static inline void interpolator_sgm_buffer_read()
{
	interpolator_sgm_data_read++;
	interpolator_sgm_data_slots++;
	if(interpolator_sgm_data_read == INTERPOLATOR_BUFFER_SIZE)
	{
		interpolator_sgm_data_read = 0;
	}
}

static inline void interpolator_sgm_buffer_write()
{
	interpolator_sgm_data_write++;
	interpolator_sgm_data_slots--;
	if(interpolator_sgm_data_write == INTERPOLATOR_BUFFER_SIZE)
	{
		interpolator_sgm_data_write = 0;
	}
}

static inline bool interpolator_sgm_is_empty()
{
	return (interpolator_sgm_data_slots == INTERPOLATOR_BUFFER_SIZE);
}

static inline bool interpolator_sgm_is_full()
{
	return (interpolator_sgm_data_slots == 0);
}

static inline void interpolator_sgm_clear()
{
	interpolator_sgm_data_write = 0;
	interpolator_sgm_data_read = 0;
	interpolator_sgm_data_slots = INTERPOLATOR_BUFFER_SIZE;
	memset(interpolator_sgm_data, 0, sizeof(interpolator_sgm_data));
}

/*
	Interpolator block buffer functions
*/
static inline void interpolator_blk_buffer_read()
{
	interpolator_blk_data_read++;
	interpolator_blk_data_slots++;
	if(interpolator_blk_data_read == INTERPOLATOR_BUFFER_SIZE)
	{
		interpolator_blk_data_read = 0;
	}
}

static inline void interpolator_blk_buffer_write()
{
	interpolator_blk_data_write++;
	interpolator_blk_data_slots--;
	if(interpolator_blk_data_write == INTERPOLATOR_BUFFER_SIZE)
	{
		interpolator_blk_data_write = 0;
	}
}

static inline bool interpolator_blk_is_empty()
{
	return (interpolator_blk_data_slots == INTERPOLATOR_BUFFER_SIZE);
}

static inline bool interpolator_blk_is_full()
{
	return (interpolator_blk_data_slots == 0);
}

static inline void interpolator_blk_clear()
{
	interpolator_blk_data_write = 0;
	interpolator_blk_data_read = 0;
	interpolator_blk_data_slots = INTERPOLATOR_BUFFER_SIZE;
	memset(interpolator_blk_data, 0, sizeof(interpolator_blk_data));
}

/*
	Interpolator functions
*/
//declares functions called by the stepper ISR
void interpolator_init()
{
	//resets buffers
	memset(&interpolator_step_pos, 0, sizeof(interpolator_step_pos));
	memset(&interpolator_rt_step_pos, 0, sizeof(interpolator_rt_step_pos));
	//memset(&interpolator_running_sgm, 0, sizeof(INTERPOLATOR_SEGMENT));
	interpolator_rt_feed = 0;

	//initialize circular buffers
	interpolator_blk_clear();
	interpolator_sgm_clear();
	
	//initializes bit masks
	#ifdef DIR0
	dirbitsmask[0] = DIR0_MASK;
	#endif
	#ifdef DIR1
	dirbitsmask[1] = DIR1_MASK;
	#endif
	#ifdef DIR2
	dirbitsmask[2] = DIR2_MASK;
	#endif
	#ifdef DIR3
	dirbitsmask[3] = DIR3_MASK;
	#endif
	#ifdef DIR4
	dirbitsmask[4] = DIR4_MASK;
	#endif
	#ifdef DIR5
	dirbitsmask[5] = DIR5_MASK;
	#endif 
	
	interpolator_running_sgm = NULL;
	interpolator_cur_plan_block = NULL;
	interpolator_needs_update = false;
	interpolator_isr_finnished = true;
}

bool interpolator_is_buffer_full()
{
	return interpolator_sgm_is_full();
}

void interpolator_run()
{
	
	static INTERPOLATOR_BLOCK *new_block = NULL;
	//conversion vars
	static float min_step_distance = 0;
	static float steps_per_mm = 0;
	//limits of the speed profile
	static uint32_t accel_until = 0;
	static uint32_t deaccel_from = 0;
	static float junction_speed_sqr = 0;
	static float half_speed_change = 0;
	
	//accel profile vars
	//static float processed_steps = 0;
	static uint32_t unprocessed_steps = 0;
	//static uint8_t accel_profile = 0;
	
	INTERPOLATOR_SEGMENT *sgm = NULL;

	//creates segments and fills the buffer
	while(!interpolator_sgm_is_full())
	{
		//flushes completed blocks
		if(interpolator_sgm_data[interpolator_sgm_data_read].block != NULL)
		{
			if(!interpolator_blk_is_empty() && (interpolator_sgm_data[interpolator_sgm_data_read].block != &interpolator_blk_data[interpolator_blk_data_read]))
			{
				interpolator_blk_buffer_read();
			}
		}
		
		//no planner blocks has beed processed or last planner block was fully processed
		if(interpolator_cur_plan_block == NULL)
		{
			//planner is empty or interpolator block buffer full. Nothing to be done
			if(planner_buffer_is_empty() || interpolator_blk_is_full())
			{
				return;
			}
			//get the first block in the planner
			interpolator_cur_plan_block = planner_get_block();
			
			//creates a new interpolator block
			new_block = &interpolator_blk_data[interpolator_blk_data_write];
			interpolator_blk_buffer_write();
			//erases previous values
			new_block->dirbits = 0;
			new_block->totalsteps = 0;
			//new_block->step_freq = F_STEP_MIN;
			
			uint32_t step_new_pos[STEPPER_COUNT];
			//applies the inverse kinematic to get next position in steps
			kinematics_apply_inverse(interpolator_cur_plan_block->pos, (uint32_t*)&step_new_pos);
			
			//calculates the number of steps to execute
			new_block->dirbits = 0;
			for(uint8_t i = 0; i < STEPPER_COUNT; i++)
			{
				if((int32_t)step_new_pos[i] >= (int32_t)interpolator_step_pos[i])
				{
					new_block->steps[i] = step_new_pos[i]-interpolator_step_pos[i];
				}
				else
				{
					new_block->dirbits |= dirbitsmask[i];
					new_block->steps[i] = interpolator_step_pos[i]-step_new_pos[i];
				}
				
				if(new_block->totalsteps < new_block->steps[i])
				{
					new_block->totalsteps = new_block->steps[i];
				}
			}
			
			//copies data for interpolator step_pos
			//new_block->dirbits = interpolator_cur_plan_block->dirbits;
			memcpy(&(interpolator_step_pos), &(step_new_pos), sizeof(step_new_pos));
			
			//calculates conversion vars
			steps_per_mm = ((float)new_block->totalsteps) / interpolator_cur_plan_block->distance;
			min_step_distance = 1.0f/steps_per_mm;
			
			//initializes data for generating step segments
			unprocessed_steps = new_block->totalsteps;
			
			//flags block for recalculation of speeds
			interpolator_needs_update = true;
			
			half_speed_change = 0.5f * INTEGRATOR_DELTA_T * interpolator_cur_plan_block->acceleration;
			
			uint32_t error = new_block->totalsteps>>1;
			for(uint8_t i = 0; i < STEPPER_COUNT; i++)
			{
				new_block->errors[i] = error;
			}
		}

		//uint32_t prev_unprocessed_steps = unprocessed_steps;
		//float min_delta = 0;

		sgm = &interpolator_sgm_data[interpolator_sgm_data_write];
		sgm->block = new_block;
		//sgm->update_speed = true;

		//if an hold is active forces to deaccelerate
		if(cnc_get_exec_state(EXEC_HOLD))
		{
			//forces deacceleration by overriding the profile juntion points
			accel_until = unprocessed_steps;
			deaccel_from = unprocessed_steps;
			interpolator_needs_update = true;
		}
		else if(interpolator_needs_update) //forces recalculation of acceleration and deacceleration profiles
		{
			interpolator_needs_update = false;
			float exit_speed_sqr = planner_get_block_exit_speed_sqr();
			junction_speed_sqr = planner_get_block_top_speed(exit_speed_sqr);
			
			accel_until = unprocessed_steps;
			deaccel_from = 0;
			if(junction_speed_sqr > interpolator_cur_plan_block->entry_speed_sqr)
			{
				float accel_dist = 0.5f * (junction_speed_sqr - interpolator_cur_plan_block->entry_speed_sqr) * interpolator_cur_plan_block->accel_inv;
				accel_until -= floorf(accel_dist * steps_per_mm);	
			}
			
			if(junction_speed_sqr > exit_speed_sqr)
			{
				float deaccel_dist = 0.5f * (junction_speed_sqr - exit_speed_sqr) * interpolator_cur_plan_block->accel_inv;
				deaccel_from = floorf(deaccel_dist * steps_per_mm);
			}
		}
		
		
		
		float partial_distance;
		float current_speed;
		//acceleration profile
		if(unprocessed_steps > accel_until)
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
			current_speed = fast_sqrt(interpolator_cur_plan_block->entry_speed_sqr);
			current_speed += half_speed_change;
			partial_distance = current_speed * INTEGRATOR_DELTA_T;
			
			//computes how many steps it can perform at this speed and frame window
			uint16_t steps = (uint16_t)floorf(partial_distance * steps_per_mm);
			
			//if traveled distance is less the one step fits at least one step
			//float speed_change_sqr;
			if(steps == 0)
			{
				steps = 1;
			}
			
			//if computed steps exceed the remaining steps for the motion shortens the distance
			if(steps > (unprocessed_steps - accel_until))
			{
				steps = (uint16_t) (unprocessed_steps - accel_until);
			}
			
			//recalculates the precise distance to travel the given amount of steps
			partial_distance = steps * min_step_distance;
			//calculates the final speed at the end of this position
			float new_speed_sqr = 2 * interpolator_cur_plan_block->acceleration * partial_distance + interpolator_cur_plan_block->entry_speed_sqr;
			current_speed = 0.5f * (fast_sqrt(new_speed_sqr) + fast_sqrt(interpolator_cur_plan_block->entry_speed_sqr));	
			//completes the segment information (step speed, steps) and updates the block
			mcu_freq_to_clocks(current_speed * steps_per_mm, &(sgm->clocks_per_tick), &(sgm->ticks_per_step));
			sgm->remaining_steps = steps;
			sgm->update_speed = true;
			interpolator_cur_plan_block->distance -= partial_distance;	
			interpolator_cur_plan_block->entry_speed_sqr = new_speed_sqr;	
		}
		else if(unprocessed_steps > deaccel_from)
		{	
			//constant speed segment
			current_speed = fast_sqrt(junction_speed_sqr);
			partial_distance = current_speed * INTEGRATOR_DELTA_T;
			
			//computes how many steps it can perform at this speed and frame window
			uint16_t steps = (uint16_t)floorf(partial_distance * steps_per_mm);
			
			//if traveled distance is less the one step fits at least one step
			//float speed_change_sqr;
			if(steps == 0)
			{
				steps = 1;
			}
			
			//if computed steps exceed the remaining steps for the motion shortens the distance
			if(steps > (unprocessed_steps - deaccel_from))
			{
				steps = (uint16_t) (unprocessed_steps - deaccel_from);
			}
			
			//recalculates the precise distance to travel the given amount os steps
			partial_distance = steps * min_step_distance;
	
			//completes the segment information (step speed, steps) and updates the block
			mcu_freq_to_clocks(current_speed * steps_per_mm, &(sgm->clocks_per_tick), &(sgm->ticks_per_step));
			sgm->remaining_steps = steps;
			sgm->update_speed = false;
			//if disance starts to offset to much replace by steps at constant rate * min_step_distance
			interpolator_cur_plan_block->distance -= partial_distance;	
		}
		else
		{
			current_speed = fast_sqrt(interpolator_cur_plan_block->entry_speed_sqr);
			current_speed -= half_speed_change;
						
			//if on active hold state
			if(cnc_get_exec_state(EXEC_HOLD))
			{
				if(current_speed < 0)
				{
					//after a feed hold if 0 speed reached exits and starves the buffer
					return;
				}
			}
			
			
			partial_distance = current_speed * INTEGRATOR_DELTA_T;
			
			//computes how many steps it can perform at this speed and frame window
			uint16_t steps = (uint16_t)floorf(partial_distance * steps_per_mm);
			
			//if traveled distance is less the one step fits at least one step
			//float speed_change_sqr;
			if(steps == 0)
			{
				steps = 1;
			}
			
			//if computed steps exceed the remaining steps for the motion shortens the distance
			if(steps > unprocessed_steps)
			{
				steps = (uint16_t)unprocessed_steps;
			}
			
			//recalculates the precise distance to travel the given amount os steps
			partial_distance = steps * min_step_distance;
			//calculates the final speed at the end of this position
			float new_speed_sqr = interpolator_cur_plan_block->entry_speed_sqr - (2 * interpolator_cur_plan_block->acceleration * partial_distance);
			new_speed_sqr = MAX(new_speed_sqr, 0); //avoids rounding errors since speed is always positive
			current_speed = 0.5f * (fast_sqrt(new_speed_sqr) + fast_sqrt(interpolator_cur_plan_block->entry_speed_sqr));	
			//completes the segment information (step speed, steps) and updates the block
			mcu_freq_to_clocks(current_speed * steps_per_mm, &(sgm->clocks_per_tick), &(sgm->ticks_per_step));
			sgm->remaining_steps = steps;
			sgm->update_speed = true;
			
			interpolator_cur_plan_block->distance -= partial_distance;	
			interpolator_cur_plan_block->entry_speed_sqr = new_speed_sqr;	
		}
		
		sgm->feed = current_speed;
		unprocessed_steps -= sgm->remaining_steps;
		if(unprocessed_steps == accel_until) //resets float additions error
		{
			interpolator_cur_plan_block->entry_speed_sqr = junction_speed_sqr;
			interpolator_cur_plan_block->distance = min_step_distance * accel_until;
		}
		
		if(unprocessed_steps == deaccel_from) //resets float additions error
		{
			interpolator_cur_plan_block->distance = min_step_distance * deaccel_from;
		}
		
		interpolator_sgm_buffer_write();

		if(unprocessed_steps == 0)
		{
			interpolator_cur_plan_block = NULL;
			planner_discard_block(); //discards planner block
			//accel_profile = 0; //no updates necessary to planner
			//break; 
		}
	}
	
	//starts the step isr if is stoped and there are segments to execute
	if(!cnc_get_exec_state(EXEC_HOLD|EXEC_ALARM|EXEC_RUN) && (interpolator_sgm_data_slots != INTERPOLATOR_BUFFER_SIZE)) //exec state is not hold or alarm and not already running
	{
		#ifdef STEPPER_ENABLE
		dio_set_outputs(STEPPER_ENABLE);
		#endif
		cnc_set_exec_state(EXEC_RUN); //flags that it started running
		mcu_start_step_ISR(interpolator_sgm_data[interpolator_sgm_data_read].clocks_per_tick, interpolator_sgm_data[interpolator_sgm_data_read].ticks_per_step);
	}
}

void interpolator_update()
{
	//flags executing block for update
	interpolator_needs_update = true;
}

void interpolator_stop()
{
	mcu_step_stop_ISR();
	cnc_clear_exec_state(EXEC_RUN);
}

void interpolator_clear()
{
	interpolator_cur_plan_block = NULL;
	interpolator_running_sgm = NULL;
	//syncs the stored position and the real position
	memcpy(&interpolator_step_pos, &interpolator_rt_step_pos, sizeof(interpolator_step_pos));
	interpolator_sgm_data_write = 0;
	interpolator_sgm_data_read = 0;
	interpolator_sgm_data_slots = INTERPOLATOR_BUFFER_SIZE;
	interpolator_blk_clear();
}

void interpolator_get_rt_position(float* axis)
{
	uint32_t step_pos[STEPPER_COUNT];
	memcpy(step_pos, interpolator_rt_step_pos, sizeof(step_pos));
	kinematics_apply_forward((uint32_t*)&step_pos, axis);
}

void interpolator_reset_rt_position()
{
	if(g_settings.homing_enabled)
	{
		float origin[AXIS_COUNT];
		for(uint8_t i = AXIS_COUNT;i != 0;)
		{
			i--;
			if(g_settings.homing_dir_invert_mask & (1<<i))
			{
				origin[i] = g_settings.max_distance[i];
			}
			else
			{
				origin[i] = 0;
			}
		}
		
		kinematics_apply_inverse((float*)&origin, (uint32_t*)&interpolator_rt_step_pos);
	}
	else
	{
		memset(&interpolator_rt_step_pos, 0, sizeof(interpolator_rt_step_pos));
	}
}

float interpolator_get_rt_feed()
{
	float feed = 0;
	if(!cnc_get_exec_state(EXEC_RUN))
	{
		return feed;
	}
	
	if(interpolator_sgm_data_slots != INTERPOLATOR_BUFFER_SIZE)
	{
		feed = interpolator_sgm_data[interpolator_sgm_data_read].feed;
	}
	
	return feed;
}

//always fires before pulse
void interpolator_step_reset_isr()
{
	static uint8_t dirbits = 0;
	static uint8_t prev_dirbits = 0;
	static bool update_step_rate = false;
	static uint16_t clock = 0;
	static uint8_t pres = 0;
	//static bool busy = false;

	//always resets all stepper pins
	mcu_set_steps(0);
	
	if(update_step_rate)
	{
		update_step_rate = false;
		mcu_change_step_ISR(clock, pres);
	}
	
	if(prev_dirbits != dirbits)
	{
		mcu_set_dirs(dirbits);
		prev_dirbits = dirbits;
	}

	mcu_enable_interrupts();
	//if no segment running tries to load one
	if(interpolator_running_sgm == NULL)
	{
		//if buffer is not empty
		if(interpolator_sgm_data_slots < INTERPOLATOR_BUFFER_SIZE)
		{
			//loads a new segment
			interpolator_running_sgm = &interpolator_sgm_data[interpolator_sgm_data_read];
			cnc_set_exec_state(EXEC_RUN);
			interpolator_isr_finnished = false;
			dirbits = interpolator_running_sgm->block->dirbits;
			update_step_rate = interpolator_running_sgm->update_speed;
			clock = interpolator_running_sgm->clocks_per_tick;
			pres = interpolator_running_sgm->ticks_per_step;		
		}
		else
		{
			//if buffer is empty and last step has been sent
			if(interpolator_isr_finnished)
			{
				interpolator_stop(); //the buffer is empty. The ISR can stop
			}
		}
		
	}

}

void interpolator_step_isr()
{
	static uint8_t stepbits = 0;
	//static bool busy = false;
/*
	if(busy) //prevents reentrancy
	{	
		return;
	}
	*/
	//sets step bits
	mcu_set_steps(stepbits);
	stepbits = 0;

	//busy = true;
	//mcu_enableInterrupts();
	mcu_enable_interrupts();
	//is steps remaining starts calc next step bits
	if(interpolator_running_sgm != NULL)
	{
		interpolator_running_sgm->remaining_steps--;
		//prepares the next step bits mask
		#ifdef STEP0
		interpolator_running_sgm->block->errors[0] += interpolator_running_sgm->block->steps[0];
		if (interpolator_running_sgm->block->errors[0] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->block->errors[0] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP0_MASK;
			if(interpolator_running_sgm->block->dirbits & DIR0_MASK)
			{
				interpolator_rt_step_pos[0]--; 
			}
			else
			{
				interpolator_rt_step_pos[0]++;
			}
		}
		#endif
		#ifdef STEP1
		interpolator_running_sgm->block->errors[1] += interpolator_running_sgm->block->steps[1];
		if (interpolator_running_sgm->block->errors[1] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->block->errors[1] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP1_MASK;
			if(interpolator_running_sgm->block->dirbits & DIR1_MASK)
			{
				interpolator_rt_step_pos[1]--; 
			}
			else
			{
				interpolator_rt_step_pos[1]++;
			}
		}
		#endif
		#ifdef STEP2
		interpolator_running_sgm->block->errors[2] += interpolator_running_sgm->block->steps[2];
		if (interpolator_running_sgm->block->errors[2] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->block->errors[2] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP2_MASK;
			if(interpolator_running_sgm->block->dirbits & DIR2_MASK)
			{
				interpolator_rt_step_pos[2]--; 
			}
			else
			{
				interpolator_rt_step_pos[2]++;
			}
		}
		#endif
		#ifdef STEP3
		interpolator_running_sgm->block->errors[3] += interpolator_running_sgm->block->steps[3];
		if (interpolator_running_sgm->block->errors[3] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->block->errors[3] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP3_MASK;
			if(interpolator_running_sgm->block->dirbits & DIR3_MASK)
			{
				interpolator_rt_step_pos[3]--; 
			}
			else
			{
				interpolator_rt_step_pos[3]++;
			}
		}
		#endif
		#ifdef STEP4
		interpolator_running_sgm->block->errors[4] += interpolator_running_sgm->block->steps[4];
		if (interpolator_running_sgm->block->errors[4] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->block->errors[4] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP4_MASK;
			if(interpolator_running_sgm->block->dirbits & DIR4_MASK)
			{
				interpolator_rt_step_pos[4]--; 
			}
			else
			{
				interpolator_rt_step_pos[4]++;
			}
		}
		#endif
		#ifdef STEP5
		interpolator_running_sgm->block->errors[5] += interpolator_running_sgm->block->steps[5];
		if (interpolator_running_sgm->block->errors[5] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->block->errors[5] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP5_MASK;
			if(interpolator_running_sgm->block->dirbits & DIR5_MASK)
			{
				interpolator_rt_step_pos[5]--; 
			}
			else
			{
				interpolator_rt_step_pos[5]++;
			}
		}
		#endif
		
		//one step remaining discards current segment
		if(interpolator_running_sgm->remaining_steps == 0)
		{
			interpolator_running_sgm = NULL;
			interpolator_sgm_buffer_read();
		}
	}
	else
	{
		//signals isr to stop after processing last step
		interpolator_isr_finnished = true;
	}
	
	//busy = false;
}

