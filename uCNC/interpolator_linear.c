#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include "config.h"
#include "mcumap.h"
#include "mcu.h"
#include "machinedefs.h"
#include "kinematics.h"
#include "interpolator.h"
#include "planner.h"
#include "utils.h"
#include "ringbuffer.h"
#include "cnc.h"

#define INTEGRATOR_DELTA_T (1.0f/F_INTEGRATOR)

//limits segment length to 16 bits
//for now this is only used in the step counter
//must try to limit all math to 16bit vars in the future for better ISR performance
//for now 32bits segments are used beacuse of error accumulation of 16bits segments
#define MAX_STEPS_PER_SEGM 32768 //2^15 (15bits)

//flag the segment stage being calculated
#define SEGM_ACCEL 1
#define SEGM_CRUIZE 2
#define SEGM_DEACCEL 4
#define SEGM_ACCEL_CRUIZE (SEGM_ACCEL | SEGM_CRUIZE)
#define SEGM_ACCEL_DEACCEL (SEGM_ACCEL | SEGM_DEACCEL)
#define SEGM_CRUIZE_DEACCEL (SEGM_CRUIZE | SEGM_DEACCEL)
#define SEGM_ACCEL_CRUIZE_DEACCEL (SEGM_ACCEL | SEGM_CRUIZE | SEGM_DEACCEL)

//this will define the number of steps per segment
//At max frequency the full buffer will contain at least half second of pulses
//#define MAX_STEPS_PER_SEGM (F_PULSE_MAX>>1)/PULSE_BUFFER_SIZE

//contains data of the block being executed by the pulse routine
//this block has the necessary data to execute the Bresenham line algorithm
typedef struct interpolator_blk_
{
	uint8_t dirbits;
	uint32_t steps[STEPPER_COUNT];
	uint32_t totalsteps;
}INTERPOLATOR_BLOCK;

//contains data of the block segment being executed by the pulse and integrator routines
//the segment is a fragment of the motion defined in the block
//this also contains the acceleration/deacceleration info
typedef struct pulse_sgm_
{
	INTERPOLATOR_BLOCK *block;
	uint32_t errors[STEPPER_COUNT];
	uint8_t next_stepbits;
	uint16_t remaining_steps;
	uint16_t clocks_per_tick;
	uint8_t ticks_per_step;
	//bool update_speed;
}INTERPOLATOR_SEGMENT;

//circular buffers
//creates new type PULSE_BLOCK_BUFFER
static INTERPOLATOR_BLOCK interpolator_blk_data[INTERPOLATOR_BUFFER_SIZE>>1];
static buffer_t interpolator_blk_buffer;

static INTERPOLATOR_SEGMENT interpolator_sgm_data[INTERPOLATOR_BUFFER_SIZE];
static buffer_t interpolator_sgm_buffer;

//pointer to the segment being executed
static INTERPOLATOR_SEGMENT *interpolator_running_sgm;

//stores the current position of the steppers in the interpolator
static uint32_t interpolator_step_pos[STEPPER_COUNT];

//flag to force the interpolator to recalc entry and exit limit position of acceleration/deacceleration curves
static bool interpolator_needs_update;
static volatile uint8_t interpolator_dirbits;
//initial values for bresenham algorithm
//this is shared between pulse and pulsereset functions 
static uint8_t dirbitsmask[STEPPER_COUNT];

//declares functions called by the stepper ISR
void interpolator_init()
{
	//resets buffers
	memset(&interpolator_step_pos, 0, sizeof(interpolator_step_pos));
	memset(&interpolator_running_sgm, 0, sizeof(INTERPOLATOR_SEGMENT));
	
	//reset stepper positions
	cnc_unhome();

	//initialize circular buffers
	interpolator_blk_buffer = buffer_init(&interpolator_blk_data, sizeof(INTERPOLATOR_BLOCK), INTERPOLATOR_BUFFER_SIZE);
	interpolator_sgm_buffer = buffer_init(&interpolator_sgm_data, sizeof(INTERPOLATOR_SEGMENT), INTERPOLATOR_BUFFER_SIZE);
	
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
	
	mcu_attachOnStep(interpolator_step);
	mcu_attachOnStepReset(interpolator_stepReset);
	//mcu_startStepISR(65535, 1);

	interpolator_running_sgm = NULL;
	interpolator_needs_update = false;
}

void interpolator_execute()
{
	static PLANNER_BLOCK *pl_block = NULL;
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
	
	//creates segments and fills the buffer
	while(!is_buffer_full(interpolator_sgm_buffer))
	{
		//no planner blocks has beed processed or last planner block was fully processed
		if(pl_block == NULL)
		{
			//planner is empty or interpolator block buffer full. Nothing to be done
			if(planner_buffer_empty() || is_buffer_full(interpolator_blk_buffer))
			{
				return;
			}
			//get the first block in the planner
			pl_block = planner_get_block();
			
			//creates a new interpolator block
			new_block = buffer_write(interpolator_blk_buffer, NULL);
			//new_block->step_freq = F_PULSE_MIN;
			
			uint32_t step_new_pos[STEPPER_COUNT];
			//applies the inverse kinematic to get next position in steps
			kinematics_apply_inverse(pl_block->pos, (uint32_t*)&step_new_pos);
			
			//calculates the number of steps to execute
			uint8_t dirs = pl_block->dirbits;
			new_block->dirbits = 0;
			for(uint8_t i = 0; i < STEPPER_COUNT; i++)
			{
				if((dirs & 0x01))
				{
					new_block->dirbits |= dirbitsmask[i];
				}
				
				new_block->steps[i] = (!(dirs & 0x01)) ? (step_new_pos[i]-interpolator_step_pos[i]) : (interpolator_step_pos[i]-step_new_pos[i]);
				dirs>>=1;
				if(new_block->totalsteps < new_block->steps[i])
				{
					new_block->totalsteps = new_block->steps[i];
				}
			}
			
			//copies data for interpolator step_pos
			//new_block->dirbits = pl_block->dirbits;
			memcpy(&(interpolator_step_pos), &(step_new_pos), sizeof(step_new_pos));
			
			//calculates conversion vars
			steps_per_mm = ((float)new_block->totalsteps) / pl_block->distance;
			min_step_distance = 1.0f/steps_per_mm;
			
			//initializes data for generating step segments
			unprocessed_steps = new_block->totalsteps;
			
			//flags block for recalculation of speeds
			interpolator_needs_update = true;
			
			half_speed_change = 0.5f * INTEGRATOR_DELTA_T * pl_block->acceleration;
	
		}

		uint32_t prev_unprocessed_steps = unprocessed_steps;
		float min_delta = 0;
	
	
		INTERPOLATOR_SEGMENT *sgm = buffer_get_next_free(interpolator_sgm_buffer);
		sgm->block = new_block;
		//sgm->update_speed = true;
		uint32_t error = sgm->block->totalsteps>>1;
		
		for(uint8_t i = 0; i < STEPPER_COUNT; i++)
		{
			sgm->errors[i] = error;
		}

		//forces recalculation of acceleration and deacceleration profiles
		if(interpolator_needs_update)
		{
			interpolator_needs_update = false;
			float exit_speed_sqr = planner_get_block_exit_speed_sqr();
			junction_speed_sqr = planner_get_block_top_speed(exit_speed_sqr);
			
			accel_until = unprocessed_steps;
			deaccel_from = 0;
			if(junction_speed_sqr > pl_block->entry_speed_sqr)
			{
				float accel_dist = 0.5f * (junction_speed_sqr - pl_block->entry_speed_sqr) * pl_block->accel_inv;
				accel_until -= floorf(accel_dist * steps_per_mm);	
			}
			
			if(junction_speed_sqr > exit_speed_sqr)
			{
				float deaccel_dist = 0.5f * (junction_speed_sqr - exit_speed_sqr) * pl_block->accel_inv;
				deaccel_from = floorf(deaccel_dist * steps_per_mm);
			}
		}
		
		if(g_cnc_state.rt_cmd && RT_CMD_FEED_HOLD)
		{
			//forces deacceleration by overriding the profile juntion points
			accel_until = unprocessed_steps;
			deaccel_from = unprocessed_steps;
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
			current_speed = fastsqrt(pl_block->entry_speed_sqr);
			current_speed += half_speed_change;
			partial_distance = current_speed * INTEGRATOR_DELTA_T;
			
			//computes how many steps it can perform at this speed and frame window
			uint16_t steps = (uint16_t)floorf(partial_distance * steps_per_mm);
			
			//if traveled distance is less the one step fits at least one step
			float speed_change_sqr;
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
			float new_speed_sqr = 2 * pl_block->acceleration * partial_distance + pl_block->entry_speed_sqr;
			current_speed = 0.5f * (fastsqrt(new_speed_sqr) + fastsqrt(pl_block->entry_speed_sqr));	
			//completes the segment information (step speed, steps) and updates the block
			mcu_freq2clocks(current_speed * steps_per_mm, &(sgm->clocks_per_tick), &(sgm->ticks_per_step));
			sgm->remaining_steps = steps;

			pl_block->distance -= partial_distance;	
			pl_block->entry_speed_sqr = new_speed_sqr;	
		}
		else if(unprocessed_steps > deaccel_from)
		{	
			//constant speed segment
			current_speed = fastsqrt(junction_speed_sqr);
			partial_distance = current_speed * INTEGRATOR_DELTA_T;
			
			//computes how many steps it can perform at this speed and frame window
			uint16_t steps = (uint16_t)floorf(partial_distance * steps_per_mm);
			
			//if traveled distance is less the one step fits at least one step
			float speed_change_sqr;
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
			mcu_freq2clocks(current_speed * steps_per_mm, &(sgm->clocks_per_tick), &(sgm->ticks_per_step));
			sgm->remaining_steps = steps;
			
			//if disance starts to offset to much replace by steps at constant rate * min_step_distance
			pl_block->distance -= partial_distance;	
		}
		else
		{
			current_speed = fastsqrt(pl_block->entry_speed_sqr);
			current_speed -= half_speed_change;
			
						
			if(g_cnc_state.rt_cmd && RT_CMD_FEED_HOLD)
			{
				if(current_speed < 0)
				{
					//after a feed hold if 0 speed reached exits and starves the buffer;
					g_cnc_state.exec_state = EXEC_HOLD;
					g_cnc_state.rt_cmd &= ~RT_CMD_FEED_HOLD; //clears flag
					return;
				}
			}
			
			partial_distance = current_speed * INTEGRATOR_DELTA_T;
			
			//computes how many steps it can perform at this speed and frame window
			uint16_t steps = (uint16_t)floorf(partial_distance * steps_per_mm);
			
			//if traveled distance is less the one step fits at least one step
			float speed_change_sqr;
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
			float new_speed_sqr = pl_block->entry_speed_sqr - (2 * pl_block->acceleration * partial_distance);
			new_speed_sqr = MAX(new_speed_sqr, 0); //avoids rounding errors since speed is always positive
			current_speed = 0.5f * (fastsqrt(new_speed_sqr) + fastsqrt(pl_block->entry_speed_sqr));	
			//completes the segment information (step speed, steps) and updates the block
			mcu_freq2clocks(current_speed * steps_per_mm, &(sgm->clocks_per_tick), &(sgm->ticks_per_step));
			sgm->remaining_steps = steps;
			pl_block->distance -= partial_distance;	
			pl_block->entry_speed_sqr = new_speed_sqr;	
		}
		
		unprocessed_steps -= sgm->remaining_steps;
		if(unprocessed_steps == accel_until) //resets float additions error
		{
			pl_block->entry_speed_sqr = junction_speed_sqr;
			pl_block->distance = min_step_distance * accel_until;
		}
		
		buffer_write(interpolator_sgm_buffer, NULL);
		if(g_cnc_state.exec_state == EXEC_IDLE)
		{
			g_cnc_state.exec_state |= EXEC_CYCLE;
			mcu_startStepISR(sgm->clocks_per_tick, sgm->ticks_per_step);
		}
		
		if(unprocessed_steps == 0)
		{
			pl_block = NULL;
			planner_discard_block(); //discards planner block
			//accel_profile = 0; //no updates necessary to planner
			break; 
		}
	}
}

void interpolator_update()
{
	//flags executing block for update
	interpolator_needs_update = true;
}

void interpolator_stop()
{
	g_cnc_state.exec_state = EXEC_IDLE;
	mcu_stopStepISR();
}

void interpolator_clear()
{
	buffer_clear(interpolator_sgm_buffer);
	buffer_clear(interpolator_blk_buffer);
}

//always fires before pulse
void interpolator_stepReset()
{
	static uint8_t dirbits = 0;
	static uint8_t prev_dirbits = 0;

	//resets all stepper pins
	mcu_setSteps(0);
	
	if(prev_dirbits != interpolator_dirbits)
	{
		mcu_setDirs(interpolator_dirbits);
		prev_dirbits = interpolator_dirbits;
	}
}

void interpolator_step()
{
	static uint8_t stepbits = 0;
	static bool update_step_rate = false; //update delay flag
	static uint16_t clock = 0;
	static uint8_t pres = 0;
	static uint16_t remaining_steps = 0;

	if(g_cnc_state.halt) //in case of any failure or stop trigger activated stops motion
	{
		if(interpolator_running_sgm != NULL) //if halt in the middle of a motion considers that homing position was lost
		{
			g_cnc_state.is_homed = false;
		}
		interpolator_stop();
		return;
	}
	
	//sets step bits
	mcu_setSteps(stepbits);
	stepbits = 0;
	remaining_steps--;
	
	if(update_step_rate)
	{
		mcu_changeStepISR(clock, pres);
		update_step_rate = false;
	}
	
	//if no segment running tries to load one
	if(interpolator_running_sgm == NULL)
	{
		//loads a new segment
		interpolator_running_sgm = buffer_get_first(interpolator_sgm_buffer);
		//if buffer is empty return null
		if(interpolator_running_sgm == NULL)
		{
			interpolator_stop(); //the buffer is empty. The ISR can stop
			return;
		}
		
		interpolator_dirbits = interpolator_running_sgm->block->dirbits;
		remaining_steps = interpolator_running_sgm->remaining_steps;
		update_step_rate = true;
		clock = interpolator_running_sgm->clocks_per_tick;
		pres = interpolator_running_sgm->ticks_per_step;
	}

	//is steps remaining starts calc next step bits
	if(interpolator_running_sgm != NULL)
	{
		//prepares the next step bits mask
		#ifdef STEP0
		interpolator_running_sgm->errors[0] += interpolator_running_sgm->block->steps[0];
		if (interpolator_running_sgm->errors[0] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[0] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP0_MASK;
			if(interpolator_dirbits && DIR0_MASK)
			{
				g_cnc_state.rt_position[0]--; 
			}
			else
			{
				g_cnc_state.rt_position[0]++;
			}
		}
		#endif
		#ifdef STEP1
		interpolator_running_sgm->errors[1] += interpolator_running_sgm->block->steps[1];
		if (interpolator_running_sgm->errors[1] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[1] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP1_MASK;
			if(interpolator_dirbits && DIR1_MASK)
			{
				g_cnc_state.rt_position[1]--; 
			}
			else
			{
				g_cnc_state.rt_position[1]++;
			}
		}
		#endif
		#ifdef STEP2
		interpolator_running_sgm->errors[2] += interpolator_running_sgm->block->steps[2];
		if (interpolator_running_sgm->errors[2] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[2] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP2_MASK;
			if(interpolator_dirbits && DIR1_MASK)
			{
				g_cnc_state.rt_position[2]--; 
			}
			else
			{
				g_cnc_state.rt_position[2]++;
			}
		}
		#endif
		#ifdef STEP3
		interpolator_running_sgm->errors[3] += interpolator_running_sgm->block->steps[3];
		if (interpolator_running_sgm->errors[3] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[3] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP3_MASK;
			if(interpolator_dirbits && DIR1_MASK)
			{
				g_cnc_state.rt_position[3]--; 
			}
			else
			{
				g_cnc_state.rt_position[3]++;
			}
		}
		#endif
		#ifdef STEP4
		interpolator_running_sgm->errors[4] += interpolator_running_sgm->block->steps[4];
		if (interpolator_running_sgm->errors[4] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[4] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP4_MASK;
			if(interpolator_dirbits && DIR1_MASK)
			{
				g_cnc_state.rt_position[4]--; 
			}
			else
			{
				g_cnc_state.rt_position[4]++;
			}
		}
		#endif
		#ifdef STEP5
		interpolator_running_sgm->errors[5] += interpolator_running_sgm->block->steps[5];
		if (interpolator_running_sgm->errors[5] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[5] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP5_MASK;
			if(interpolator_dirbits && DIR1_MASK)
			{
				g_cnc_state.rt_position[5]--; 
			}
			else
			{
				g_cnc_state.rt_position[5]++;
			}
		}
		#endif
		
		//one step remaining discards current segment
		if(remaining_steps == 1)
		{
			interpolator_running_sgm = NULL;
			//advances buffer
			buffer_read(interpolator_sgm_buffer, NULL);
		}
	}
}

