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
//initial values for bresenham algorithm
//this is shared between pulse and pulsereset functions 
static uint8_t dirbitsmask[STEPPER_COUNT];

//declares functions called by the stepper ISR
void interpolator_init()
{
	//resets buffers
	memset(&interpolator_step_pos, 0, sizeof(interpolator_step_pos));
	memset(&interpolator_running_sgm, 0, sizeof(INTERPOLATOR_SEGMENT));

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
	
	mcu_startStepISR(65535, 1);
	/*mcu_attachOnIntegrator(interpolator_rt_integrator);
	//start and suspend the integrator
	mcu_changeStepISR(interpolator_running_sgm->clocks_per_tick, interpolator_running_sgm->ticks_per_step);
	mcu_startIntegrator();
	mcu_pauseIntegrator();*/
	//mcu_attachOnStep(interpolator_step);
	//mcu_attachOnStepReset(interpolator_stepReset);
	//mcu_startPulse(F_PULSE_MIN);*/

	interpolator_running_sgm = NULL;
	interpolator_needs_update = false;
}

void interpolator_execute()
{
	static PLANNER_BLOCK *pl_block = NULL;
	static INTERPOLATOR_BLOCK *new_block = NULL;
	//conversion vars
	static float mm_per_step = 0;
	static float steps_per_mm = 0;
	//limits of the speed profile
	static float accel_until = 0;
	static float deaccel_from = 0;
	static uint32_t accel_until_steps = 0;
	static uint32_t deaccel_from_steps = 0;
	static float junction_speed_sqr = 0;
	static float junction_step_speed = 0;
	//static float speed_var_factor = 0;
	static float step_speed_var = 0;
	
	//accel profile vars
	//static float processed_steps = 0;
	static uint32_t unprocessed_steps = 0;
	static uint8_t accel_profile = 0;
	
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
		mm_per_step = 1.0f/steps_per_mm;
		
		//calculates the invert acceleration for the current block
		//speed_var_factor = 2 * pl_block->acceleration * mm_per_step;
		
		//initializes data for generating step segments
		unprocessed_steps = new_block->totalsteps;
		
		//calculates the step rate increment on each integration cycle
		step_speed_var = 0.5 * pl_block->acceleration * steps_per_mm * INTEGRATOR_DELTA_T;

		//flags block for recalculation of speeds
		interpolator_needs_update = true;
		
		//initial steps per second (step frequency)
		//this is updated in the step integrator (acceleration)
		//new_block->step_freq = sqrtf(pl_block->entry_speed_sqr) * steps_per_mm;
		//new_block->step_freq = MAX(F_PULSE_MIN,new_block->step_freq);
		//speed change
		float speed_var = pl_block->acceleration * INTEGRATOR_DELTA_T;
	}

	float step_speed = sqrtf(pl_block->entry_speed_sqr) * steps_per_mm;
	uint32_t prev_unprocessed_steps = unprocessed_steps;
	//creates segments and fills the buffer
	while(!is_buffer_full(interpolator_sgm_buffer))
	{
		uint16_t steps_computed = 0;
		INTERPOLATOR_SEGMENT *sgm = buffer_get_next_free(interpolator_sgm_buffer);
		sgm->block = new_block;
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
			
			junction_step_speed = sqrtf(junction_speed_sqr) * steps_per_mm;
			
			float step_dist_const = 0.5f / pl_block->acceleration;
			accel_until = (junction_speed_sqr - pl_block->entry_speed_sqr);
			deaccel_from = (junction_speed_sqr - exit_speed_sqr);
			
			if(accel_until > 0)
			{
				accel_until *= step_dist_const;
				accel_until_steps = unprocessed_steps - (uint32_t)floorf(accel_until * steps_per_mm);
			}
			else
			{
				accel_until_steps = unprocessed_steps;
			}
			
			if(deaccel_from > 0)
			{
				deaccel_from = deaccel_from * step_dist_const;
				deaccel_from_steps = (uint32_t)floorf(deaccel_from * steps_per_mm);
			}
			else
			{
				deaccel_from_steps = 0;
			}
		}

		if(unprocessed_steps > accel_until_steps)
		{
			uint32_t remaining = unprocessed_steps - accel_until_steps;
			step_speed += step_speed_var;
			steps_computed = roundf(INTEGRATOR_DELTA_T * step_speed);
			mcu_freq2clocks(step_speed, &(sgm->clocks_per_tick), &(sgm->ticks_per_step));
			accel_profile |= SEGM_ACCEL;

			if(steps_computed < remaining)
			{
				sgm->remaining_steps = steps_computed;
				step_speed += step_speed_var;
			}
			else
			{
				steps_computed = (uint16_t)remaining;
				sgm->remaining_steps = steps_computed;
				step_speed = junction_step_speed;
			}		
		}
		else if(unprocessed_steps>deaccel_from_steps)
		{	
			uint32_t remaining = unprocessed_steps - deaccel_from_steps;
			steps_computed = roundf(INTEGRATOR_DELTA_T * step_speed);
			mcu_freq2clocks(step_speed, &(sgm->clocks_per_tick), &(sgm->ticks_per_step));
			accel_profile |= SEGM_CRUIZE;

			if(steps_computed < remaining)
			{
				sgm->remaining_steps = steps_computed;
			}
			else
			{
				steps_computed = (uint16_t)remaining;
				sgm->remaining_steps = steps_computed;
			}	
		}
		else
		{
			step_speed -= step_speed_var;
			steps_computed = roundf(INTEGRATOR_DELTA_T * step_speed);
			mcu_freq2clocks(step_speed, &(sgm->clocks_per_tick), &(sgm->ticks_per_step));
			accel_profile |= SEGM_DEACCEL;
			if(steps_computed < unprocessed_steps)
			{
				sgm->remaining_steps = steps_computed;
				step_speed -= step_speed_var;
			}
			else
			{
				steps_computed = (uint16_t)(unprocessed_steps);
				sgm->remaining_steps = steps_computed;
			}
		}

		unprocessed_steps -= steps_computed;
		if(sgm->remaining_steps != 0)
		{
			buffer_write(interpolator_sgm_buffer, NULL);
		}
		
		if(unprocessed_steps == 0)
		{
			pl_block = NULL;
			planner_discard_block(); //discards planner block
			accel_profile = 0; //no updates necessary to planner
			break; 
		}
	}

	if(accel_profile != 0)
	{
		//updates the planner block
		float completed_distance = (prev_unprocessed_steps - unprocessed_steps) * mm_per_step;
		pl_block->distance -= completed_distance;
		
		switch(accel_profile)
		{
			case SEGM_ACCEL:
				pl_block->entry_speed_sqr += 2.0f * completed_distance * pl_block->acceleration;
				break;
			case SEGM_ACCEL_CRUIZE:
			case SEGM_CRUIZE:
				pl_block->entry_speed_sqr = junction_speed_sqr;
				break;
			case SEGM_ACCEL_DEACCEL:
			case SEGM_CRUIZE_DEACCEL:
			case SEGM_ACCEL_CRUIZE_DEACCEL:
				pl_block->entry_speed_sqr = junction_speed_sqr - 2.0f * (deaccel_from - pl_block->distance) * pl_block->acceleration;
				break;
			case SEGM_DEACCEL:
				pl_block->entry_speed_sqr -= 2.0f * completed_distance * pl_block->acceleration;
				break;
		}
		
		//prevents unecessary reentrancy
		accel_profile = 0;
	}
}

void interpolator_update()
{
	//flags executing block for update
	interpolator_needs_update = true;
}

void interpolator_sleep()
{
	//mcu_stopPulse();
}

//always fires before pulse
void interpolator_stepReset()
{
	static uint8_t dirbits = 0;
	static uint8_t prev_dirbits = 0;
	//resets all stepper pins
	mcu_setSteps(0);
	
	if(prev_dirbits != dirbits)
	{
		mcu_setDirs(dirbits);
		prev_dirbits = dirbits;
	}
	
	if(interpolator_running_sgm == NULL)
	{
		interpolator_running_sgm = buffer_get_first(interpolator_sgm_buffer);
		//if buffer is empty return null
		if(interpolator_running_sgm == NULL)
		{
			return;
		}
		
		//adjusts step frequency
		mcu_changeStepISR(interpolator_running_sgm->clocks_per_tick, interpolator_running_sgm->ticks_per_step);
		if(interpolator_running_sgm->remaining_steps == 0) //empty cycle
		{
			interpolator_running_sgm=NULL;
			buffer_read(interpolator_sgm_buffer, NULL);
			return;
		}
		
		dirbits = interpolator_running_sgm->block->dirbits;
	}
}

void interpolator_step()
{
	static uint8_t stepbits = 0;
	
	//sets step bits
	mcu_setSteps(stepbits);
	stepbits = 0;
	
	//is steps remaining starts calc next step bits
	if(interpolator_running_sgm != NULL)
	{
		
		#ifdef STEP0
		interpolator_running_sgm->errors[0] += interpolator_running_sgm->block->steps[0];
		if (interpolator_running_sgm->errors[0] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[0] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP0_MASK;
		}
		#endif
		#ifdef STEP1
		interpolator_running_sgm->errors[1] += interpolator_running_sgm->block->steps[1];
		if (interpolator_running_sgm->errors[1] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[1] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP1_MASK;
		}
		#endif
		#ifdef STEP2
		interpolator_running_sgm->errors[2] += interpolator_running_sgm->block->steps[2];
		if (interpolator_running_sgm->errors[2] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[2] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP2_MASK;
		}
		#endif
		#ifdef STEP3
		interpolator_running_sgm->errors[3] += interpolator_running_sgm->block->steps[3];
		if (interpolator_running_sgm->errors[3] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[3] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP3_MASK;
		}
		#endif
		#ifdef STEP4
		interpolator_running_sgm->errors[4] += interpolator_running_sgm->block->steps[4];
		if (interpolator_running_sgm->errors[4] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[4] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP4_MASK;
		}
		#endif
		#ifdef STEP5
		interpolator_running_sgm->errors[5] += interpolator_running_sgm->block->steps[5];
		if (interpolator_running_sgm->errors[5] > interpolator_running_sgm->block->totalsteps)
		{
			interpolator_running_sgm->errors[5] -= interpolator_running_sgm->block->totalsteps;
			stepbits |= STEP5_MASK;
		}
		#endif

		if(--(interpolator_running_sgm->remaining_steps) == 0)
		{
			interpolator_running_sgm=NULL;
			//advance buffer
			buffer_read(interpolator_sgm_buffer, NULL);
		}
	}
}

