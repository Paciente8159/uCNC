#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "config.h"
#include "mcumap.h"
#include "mcu.h"
#include "machinedefs.h"
#include "kinematics.h"
#include "interpolator.h"
#include "planner.h"
#include "utils.h"
#include "ringbuffer.h"

#define PULSE_BUFFER_SIZE 3
#define INTEGRATOR_DELTA_T (1.0f/F_INTEGRATOR)

#define MAX_STEPS_SEGMENT 32768 //2^15 (15bits)

//flag the segment stage being calculated
#define SEGM_ACCEL 1
#define SEGM_CRUIZE 2
#define SEGM_DEACCEL 4

//this will define the number of steps per segment
//At max frequency the full buffer will contain at least half second of pulses
#define MAX_STEPS_PER_SEGM (F_PULSE_MAX>>1)/PULSE_BUFFER_SIZE

typedef struct pulse_blk_
{
	uint8_t dirbits;
	uint32_t steps[STEPPER_COUNT];
	uint32_t totalsteps;
	float step_freq;
	float step_freq_const;
}PULSE_BLOCK;

typedef struct pulse_sgm_
{
	PULSE_BLOCK *block;
	uint8_t next_pulsebits;
	uint16_t remaining_steps;
	float step_freq_inc;
}PULSE_SEGMENT;

typedef struct
{
	float steps_mm;
	float accel_until;
	float deaccel_from;
	uint32_t accel_until_steps;
	uint32_t deaccel_from_steps;
	float max_sgm_dist;
	float top_speed_sqr;
	float accel_inv;
	PLANNER_MOTION* pl_block;
	PULSE_BLOCK* pulse_block;
}INTERPOLATOR_BLOCK;

static INTERPOLATOR_BLOCK g_itp_block;
static PULSE_SEGMENT *exec_segment;
static PULSE_BLOCK *exec_block;

//these variables are shared by several ISR routines
//static volatile bool g_itp_integrator_running;
//static volatile float g_itp_step_rate_inc;
//static volatile float g_itp_step_rate;

static uint32_t step_errors[STEPPER_COUNT];

//circular buffers
//creates new type PULSE_BLOCK_BUFFER
static PULSE_BLOCK g_itp_block_data[PULSE_BUFFER_SIZE];
//static ringbuffer_t g_itp_block_buffer_ring;
static buffer_t g_itp_block_buffer;

static PULSE_SEGMENT g_itp_segment_data[PULSE_BUFFER_SIZE];
static buffer_t g_itp_segment_buffer;

static uint32_t itp_step_pos[STEPPER_COUNT];
//volatile uint16_t g_itp_pulse_tick;
//volatile bool g_itp_busy;

void interpolator_init()
{
	//g_itp_busy = false;
	//g_itp_pulse_tick = 0xFFFF;
	//resets buffers
	memset(&itp_step_pos, 0, sizeof(itp_step_pos));
	memset(&g_itp_block, 0, sizeof(INTERPOLATOR_BLOCK));
	memset(&exec_segment, 0, sizeof(PULSE_SEGMENT));

	//initialize circular buffers
	g_itp_block_buffer = buffer_init(&g_itp_block_data, sizeof(PULSE_BLOCK), PULSE_BUFFER_SIZE);
	g_itp_segment_buffer = buffer_init(&g_itp_segment_data, sizeof(PULSE_SEGMENT), PULSE_BUFFER_SIZE);
	g_itp_block.pl_block = NULL;
	
	//g_itp_integrator_running = false;
	//g_itp_step_rate_inc = 0;
	//g_itp_step_rate = 0;
	
	mcu_attachOnIntegrator(interpolator_rt_integrator);
	//start and suspend the integrator
	mcu_startIntegrator();
	mcu_pauseIntegrator();
	mcu_attachOnPulse(interpolator_rt_pulse);
	mcu_attachOnPulseReset(interpolator_rt_pulsereset);
	mcu_startPulse(10);

	//g_itp_integrator_running = false;
	exec_segment = NULL;
}

void interpolator_exec_planner_block()
{
	uint32_t step_new_pos[STEPPER_COUNT];
	static float mm_per_step = 0;
	static float speed_var_factor = 0;
	
	//static float steps_mm = 0;
	static float accel_rate = 0;
	static uint32_t processed_steps = 0;
	
	//no planner blocks has beed processed or last planner block was fully processed
	if(g_itp_block.pl_block == NULL)
	{
		//planner is empty or interpolator block buffer full. Nothing to be done
		if(planner_buffer_empty() || is_buffer_full(g_itp_block_buffer))
		{
			return;
		}
		//get the first block in the planner
		g_itp_block.pl_block = planner_get_block();
		
		//creates a new interpolator block
		g_itp_block.pulse_block = buffer_write(g_itp_block_buffer, NULL);
		
		//applies the inverse kinematic to get next position in steps
		kinematics_apply_inverse(g_itp_block.pl_block->pos, (uint32_t*)&step_new_pos);
		
		//calculates the number of steps to execute
		uint8_t dirs = g_itp_block.pl_block->dirbits;
		for(uint8_t i = 0; i < STEPPER_COUNT; i++)
		{
			g_itp_block.pulse_block->steps[i] = (!(dirs & 0x01)) ? (step_new_pos[i]-itp_step_pos[i]) : (itp_step_pos[i]-step_new_pos[i]);
			dirs>>=1;
			if(g_itp_block.pulse_block->totalsteps < g_itp_block.pulse_block->steps[i])
			{
				g_itp_block.pulse_block->totalsteps = g_itp_block.pulse_block->steps[i];
			}
		}
		
		//copies data for direction bits and updates interpolator step_pos
		g_itp_block.pulse_block->dirbits = g_itp_block.pl_block->dirbits;
		memcpy(&(itp_step_pos), &(step_new_pos), sizeof(step_new_pos));
		
		//calculates the invert acceleration for the current block
		g_itp_block.accel_inv = 1.0f / g_itp_block.pl_block->acceleration;
		mm_per_step = g_itp_block.pl_block->distance / (float)g_itp_block.pulse_block->totalsteps;
		speed_var_factor = 2 * g_itp_block.pl_block->acceleration * mm_per_step;
		
		//initializes data for generating step segments
		processed_steps = 0;
		g_itp_block.steps_mm = ((float)g_itp_block.pulse_block->totalsteps) / g_itp_block.pl_block->distance;
		
		//this is used to compute the maximum distance contained inside each segment
		//this will limit the number of steps per segment to 16bits
		g_itp_block.max_sgm_dist = ((float)MAX_STEPS_PER_SEGM)/g_itp_block.steps_mm;
		
		//calculates the step rate increment (frequency of pulse) on each integrator cycle
		accel_rate = g_itp_block.pl_block->acceleration * g_itp_block.steps_mm * INTEGRATOR_DELTA_T;

		//calculates the accel and deaccel profiles for the current block
		//this is also calculated every time the planner updates the current block being interpolated
		interpolator_update_block();
		
		//initial steps per second (step frequency)
		//this is updated in the step integrator (acceleration)
		g_itp_block.pulse_block->step_freq = (g_itp_block.pl_block->entry_speed_sqr != 0) ? sqrtf(g_itp_block.pl_block->entry_speed_sqr) * g_itp_block.steps_mm : 0;

	}
	
	uint8_t segment_stage = 0;
	uint32_t completed_steps = processed_steps;
	bool block_completed = (processed_steps == g_itp_block.pulse_block->totalsteps);
	
	//creates segments and fills the buffer
	while(!is_buffer_full(g_itp_segment_buffer))
	{
		PULSE_SEGMENT *sgm = buffer_get_next_free(g_itp_segment_buffer);
		sgm->block = g_itp_block.pulse_block;
		
		if(block_completed)
		{
			g_itp_block.pl_block = NULL;
			planner_discard_block();
			segment_stage = 0; //no updates necessary to planner
			break; //fetches next planner block
		}
		else
		{
			if(processed_steps < g_itp_block.accel_until_steps) //accel
			{
				segment_stage = SEGM_ACCEL;
				uint32_t remaining = g_itp_block.accel_until_steps - processed_steps;
				
				if(remaining > MAX_STEPS_PER_SEGM)
				{
					sgm->remaining_steps = MAX_STEPS_PER_SEGM;
					processed_steps += MAX_STEPS_PER_SEGM;
					//g_itp_block.pl_block->distance += g_itp_block.max_sgm_dist;
					//g_itp_block.pl_block->entry_speed_sqr += 2 * g_itp_block.pl_block->acceleration * g_itp_block.max_sgm_dist;
				}
				else
				{
					sgm->remaining_steps = remaining;
					processed_steps = g_itp_block.accel_until_steps;
					//g_itp_block.pl_block->distance -= g_itp_block.accel_until;
					g_itp_block.pl_block->entry_speed_sqr = g_itp_block.top_speed_sqr;
					segment_stage = SEGM_CRUIZE;
				}
				
				sgm->step_freq_inc = accel_rate;
				
			}
			else if(processed_steps < g_itp_block.deaccel_from_steps) //cruize
			{
				uint32_t remaining = g_itp_block.deaccel_from_steps - processed_steps;
				segment_stage = SEGM_CRUIZE;
				
				if(remaining > MAX_STEPS_PER_SEGM)
				{
					sgm->remaining_steps = MAX_STEPS_PER_SEGM;
					processed_steps += MAX_STEPS_PER_SEGM;
					//g_itp_block.pl_block->distance -= g_itp_block.max_sgm_dist;
				}
				else
				{
					sgm->remaining_steps = remaining;
					processed_steps = g_itp_block.deaccel_from_steps;
					//g_itp_block.pl_block->distance = g_itp_block.deaccel_from;
					segment_stage = SEGM_DEACCEL;
				}
				
				sgm->step_freq_inc = 0;
			}
			else//deaccel
			{
				uint32_t remaining = g_itp_block.pulse_block->totalsteps - processed_steps;
				segment_stage = SEGM_DEACCEL;
				
				if(remaining > MAX_STEPS_PER_SEGM)
				{
					sgm->remaining_steps = MAX_STEPS_PER_SEGM;
					processed_steps += MAX_STEPS_PER_SEGM;
					//g_itp_block.pl_block->distance -= g_itp_block.max_sgm_dist;
					//g_itp_block.pl_block->entry_speed_sqr -= 2 * g_itp_block.pl_block->acceleration * g_itp_block.max_sgm_dist;
				}
				else
				{
					sgm->remaining_steps = remaining;
					processed_steps = g_itp_block.pulse_block->totalsteps;
					block_completed = true;
				}
				
				sgm->step_freq_inc = -accel_rate;
			}
		
			buffer_write(g_itp_segment_buffer, NULL);
		}
	}
	
	if(segment_stage != 0)
	{
		//updates the planner block
		completed_steps = processed_steps - completed_steps;
		
		switch(segment_stage)
		{
			case SEGM_ACCEL:
				g_itp_block.pl_block->distance += completed_steps * mm_per_step;
				g_itp_block.pl_block->entry_speed_sqr += completed_steps * speed_var_factor;
				break;
			case SEGM_CRUIZE:
				g_itp_block.pl_block->distance += completed_steps * mm_per_step;
				break;
			case SEGM_DEACCEL:
				g_itp_block.pl_block->distance -= completed_steps * mm_per_step;
				g_itp_block.pl_block->entry_speed_sqr -= completed_steps * speed_var_factor;
				break;
		}
	}
}

void interpolator_update_block()
{
	float exit_speed_sqr = planner_get_block_exit_speed_sqr();
	float acc_deacc_speed_sqr = 4.0f * g_itp_block.pl_block->acceleration * g_itp_block.pl_block->distance;
	g_itp_block.top_speed_sqr = (exit_speed_sqr + g_itp_block.pl_block->entry_speed_sqr + acc_deacc_speed_sqr) * 0.25;
	float target_speed_sqr = g_itp_block.pl_block->target_speed*g_itp_block.pl_block->target_speed;
	g_itp_block.top_speed_sqr = MIN(g_itp_block.top_speed_sqr, target_speed_sqr);
	
	//g_itp_block.pulse_block->step_freq_const = sqrtf(v_cruize_sqr)*g_itp_block.steps_mm;
	float step_dist_const = g_itp_block.accel_inv * 0.5;
	g_itp_block.accel_until = (g_itp_block.top_speed_sqr - g_itp_block.pl_block->entry_speed_sqr);
	g_itp_block.deaccel_from = (g_itp_block.top_speed_sqr - exit_speed_sqr);
	
	if(g_itp_block.accel_until > 0)
	{
		g_itp_block.accel_until *= step_dist_const;
		g_itp_block.accel_until_steps = (uint32_t)floor(g_itp_block.accel_until * g_itp_block.steps_mm);
	}
	
	if(g_itp_block.deaccel_from > 0)
	{
		g_itp_block.deaccel_from = g_itp_block.deaccel_from * step_dist_const;
		g_itp_block.deaccel_from_steps = g_itp_block.pulse_block->totalsteps - (uint32_t)floor(g_itp_block.deaccel_from * g_itp_block.steps_mm);
	}
}

void interpolator_sleep()
{
	//mcu_stopPulse();
}

//improve
//+1000 cycles to change speed
//all improvements in the mcu_changePulse function
void interpolator_rt_integrator()
{
	if(exec_segment != NULL)
	{
		if(exec_segment->step_freq_inc != 0)
		{
			exec_segment->block->step_freq += exec_segment->step_freq_inc;
			mcu_changePulse(exec_segment->block->step_freq);
		}
		else
		{
			mcu_changePulse(exec_segment->block->step_freq_const);
			mcu_pauseIntegrator();
		}
	}
}

//always fires before pulse

//with pointer
void interpolator_rt_pulsereset()
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
	
	if(exec_segment == NULL)
	{
		exec_segment = buffer_get_first(g_itp_segment_buffer);
		//if buffer is empty return null
		if(exec_segment == NULL)
		{
			return;
		}
		
		//signals the integrator to run
		mcu_resumeIntegrator();
		//g_itp_integrator_running = true;
		#ifdef STEP0
		step_errors[0] = exec_segment->block->totalsteps;
		#endif
		#ifdef STEP1
		step_errors[1] = exec_segment->block->totalsteps;
		#endif
		#ifdef STEP2
		step_errors[2] = exec_segment->block->totalsteps;
		#endif
		#ifdef STEP3
		step_errors[3] = exec_segment->block->totalsteps;
		#endif
		#ifdef STEP4
		step_errors[4] = exec_segment->block->totalsteps;
		#endif
		#ifdef STEP5
		step_errors[5] = exec_segment->block->totalsteps;
		#endif
		
		exec_segment->block->totalsteps<<=1;
		
		#ifdef DIR0
		if(exec_segment->block->dirbits & 1)
		{
			dirbits |= DIR0_MASK;
		}
		#endif
		#ifdef DIR1
		if(exec_segment->block->dirbits & 2)
		{
			dirbits |= DIR1_MASK;
		}
		#endif
		#ifdef DIR2
		if(exec_segment->block->dirbits & 4)
		{
			dirbits |= DIR2_MASK;
		}
		#endif
		#ifdef DIR3
		if(exec_segment->block->dirbits & 8)
		{
			dirbits |= DIR3_MASK;
		}
		#endif
		#ifdef DIR4
		if(exec_segment->block->dirbits & 16)
		{
			dirbits |= DIR4_MASK;
		}
		#endif
		#ifdef DIR5
		if(exec_segment->block->dirbits & 32)
		{
			dirbits |= DIR5_MASK;
		}
		#endif
	}
}


void interpolator_rt_pulse()
{
	static uint8_t stepbits = 0;
	
	//sets step bits
	mcu_setSteps(stepbits);

	//is steps remaining starts calc next step bits
	if(exec_segment != NULL)
	{
		stepbits = 0;
		#ifdef STEP0
		step_errors[0] += exec_segment->block->steps[0];
		if (step_errors[0] > exec_segment->block->totalsteps)
		{
			step_errors[0] -= exec_segment->block->totalsteps;
			stepbits |= STEP0_MASK;
		}
		#endif
		#ifdef STEP1
		step_errors[1] += exec_segment->block->steps[1];
		if (step_errors[1] > exec_segment->block->totalsteps)
		{
			step_errors[1] -= exec_segment->block->totalsteps;
			stepbits |= STEP1_MASK;
		}
		#endif
		#ifdef STEP2
		step_errors[2] += exec_segment->block->steps[2];
		if (step_errors[2] > exec_segment->block->totalsteps)
		{
			step_errors[2] -= exec_segment->block->totalsteps;
			stepbits |= STEP2_MASK;
		}
		#endif
		#ifdef STEP3
		step_errors[3] += exec_segment->block->steps[3];
		if (step_errors[3] > exec_segment->block->totalsteps)
		{
			step_errors[3] -= exec_segment->block->totalsteps;
			stepbits |= STEP3_MASK;
		}
		#endif
		#ifdef STEP4
		step_errors[4] += exec_segment->block->steps[4];
		if (step_errors[4] > exec_segment->block->totalsteps)
		{
			step_errors[4] -= exec_segment->block->totalsteps;
			stepbits |= STEP4_MASK;
		}
		#endif
		#ifdef STEP5
		step_errors[5] += exec_segment->block->steps[5];
		if (step_errors[5] > exec_segment->block->totalsteps)
		{
			step_errors[5] -= exec_segment->block->totalsteps;
			stepbits |= STEP5_MASK;
		}
		#endif

		if(--(exec_segment->remaining_steps) == 0)
		{
			exec_segment=NULL;
			//advance buffer
			buffer_read(g_itp_segment_buffer, NULL);
		}
	}
}

