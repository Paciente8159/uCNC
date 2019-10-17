#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "config.h"
#include "machinedefs.h"
#include "boarddefs.h"
#include "interpolator.h"
#include "planner.h"
#include "board.h"
#include "utils.h"
#include "ringbuffer.h"

#define PULSE_BUFFER_SIZE 10
#define INTEGRATOR_DELTA_T 1.0f / F_INTEGRATOR

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
	uint16_t next_pulsebits;
	uint16_t remaining_steps;
	float step_freq_inc;
}PULSE_SEGMENT;

typedef struct
{
	uint32_t processed_steps;
	float steps_mm;
	uint32_t accel_until;
	uint32_t deaccel_from;
	float accel_inc;
	float accel_inv;
	PLANNER_MOTION* pl_block;
	PULSE_BLOCK* pulse_block;
}INTERPOLATOR_BLOCK;

static INTERPOLATOR_BLOCK g_itp_block;
static PULSE_SEGMENT exec_segment;

//circular buffers
//creates new type PULSE_BLOCK_BUFFER
static PULSE_BLOCK g_itp_block_data[PULSE_BUFFER_SIZE];
static buffer_t g_itp_block_buffer;

static PULSE_SEGMENT g_itp_segment_data[PULSE_BUFFER_SIZE];
static buffer_t g_itp_segment_buffer;

//TYPEDEF_BUFFER(PULSE_BLOCK, PULSE_BUFFER_SIZE);
//PULSE_BLOCK_BUFFER g_itp_block_buffer;
//creates new type PULSE_BLOCK_BUFFER
//TYPEDEF_BUFFER(PULSE_SEGMENT, PULSE_BUFFER_SIZE);
//PULSE_SEGMENT_BUFFER g_itp_segment_buffer;

volatile uint16_t g_itp_pulse_tick;
volatile bool g_itp_busy;

void interpolator_init()
{
	g_itp_busy = false;
	g_itp_pulse_tick = 0xFFFF;
	
	//resets buffers
	memset(&g_itp_block, 0, sizeof(INTERPOLATOR_BLOCK));
	memset(&exec_segment, 0, sizeof(PULSE_SEGMENT));
	
	
	
	//board_stopPulse();
	//board_stopIntegrator();
	//board_enableInterrupts();
	
	//initialize circular buffers
	g_itp_block_buffer = buffer_init(&g_itp_block_data, sizeof(PULSE_BLOCK), PULSE_BUFFER_SIZE);
	g_itp_segment_buffer = buffer_init(&g_itp_segment_data, sizeof(PULSE_SEGMENT), PULSE_BUFFER_SIZE);
	g_itp_block.pl_block = NULL;
	
	board_attachOnIntegrator(interpolator_rt_integrator);
	board_startIntegrator(F_INTEGRATOR);
	board_attachOnPulse(interpolator_rt_pulse);
	board_attachOnPulseReset(interpolator_rt_pulsereset);
	board_startPulse(10);
}

void interpolator_exec_planner_block()
{
	static float steps_mm = 0;
	static uint32_t remaining_steps = 0;
	
	//no planner blocks has beed processed or last planner block was fully processed
	if(g_itp_block.pl_block==NULL)
	{
		//planner is empty or interpolator block buffer full. Nothing to be done
		if(planner_buffer_empty() || is_buffer_full(g_itp_block_buffer))
		{
			return;
		}
		
		//starts to process the planner block
		g_itp_block.pl_block = planner_get_block();
		
		//returns the new block
		g_itp_block.pulse_block = buffer_write(g_itp_block_buffer, NULL);
		
		g_itp_block.pulse_block->dirbits = g_itp_block.pl_block->dirbits;
		memcpy(&(g_itp_block.pulse_block->steps), &(g_itp_block.pl_block->steps), sizeof(g_itp_block.pulse_block->steps));
		g_itp_block.pulse_block->totalsteps = g_itp_block.pl_block->totalsteps;
		
		g_itp_block.processed_steps = 0;
		g_itp_block.steps_mm = g_itp_block.pl_block->totalsteps / g_itp_block.pl_block->distance;
		g_itp_block.accel_inv = 1.0f / g_itp_block.pl_block->acceleration;
		//calculates the accel and deaccel profiles
		interpolator_update_profile();
		
		//initial steps per second (step frequency)
		//this is updated in the step integrator (acceleration)
		g_itp_block.pulse_block->step_freq = (g_itp_block.pl_block->entry_speed_sqr > 0) ? sqrtf(g_itp_block.pl_block->entry_speed_sqr)*g_itp_block.steps_mm : 0;
		g_itp_block.accel_inc = g_itp_block.pl_block->acceleration * g_itp_block.steps_mm * INTEGRATOR_DELTA_T;

	}
	
	//creates segments and fills the buffer
	while(!is_buffer_full(g_itp_segment_buffer))
	{
		PULSE_SEGMENT sgm;
		sgm.block = g_itp_block.pulse_block;
		
		//fill ramp accel
		if(g_itp_block.processed_steps == g_itp_block.pulse_block->totalsteps)
		{
			g_itp_block.pl_block=NULL;
			planner_discard_block();
			return;
		}
		else
		{
			if(g_itp_block.processed_steps<g_itp_block.accel_until)//accel
			{
				uint32_t steps = g_itp_block.accel_until - g_itp_block.processed_steps;
				if(steps>F_INTEGRATOR)
				{
					g_itp_block.processed_steps+=F_INTEGRATOR;
					sgm.remaining_steps = F_INTEGRATOR;
					sgm.step_freq_inc = g_itp_block.accel_inc;
				}
				else
				{
					g_itp_block.processed_steps=g_itp_block.accel_until;
					sgm.remaining_steps = (uint16_t)steps;
					sgm.step_freq_inc = g_itp_block.accel_inc;
				}
			}
			else if(g_itp_block.processed_steps<g_itp_block.deaccel_from) //cruize
			{
				uint32_t steps = g_itp_block.deaccel_from - g_itp_block.processed_steps;
				if(steps>F_INTEGRATOR)
				{
					g_itp_block.processed_steps+=F_INTEGRATOR;
					sgm.remaining_steps = F_INTEGRATOR;
					sgm.step_freq_inc = 0;
				}
				else
				{
					g_itp_block.processed_steps=g_itp_block.deaccel_from;
					sgm.remaining_steps = (uint16_t)steps;
					sgm.step_freq_inc = 0;
				}
			}
			else//deaccel
			{
				uint32_t steps = g_itp_block.pulse_block->totalsteps - g_itp_block.processed_steps;
				if(steps>F_INTEGRATOR)
				{
					g_itp_block.processed_steps+=F_INTEGRATOR;
					sgm.remaining_steps = F_INTEGRATOR;
					sgm.step_freq_inc = -g_itp_block.accel_inc;
				}
				else
				{
					sgm.remaining_steps = (uint16_t)steps;
					g_itp_block.processed_steps = g_itp_block.pulse_block->totalsteps;
					sgm.step_freq_inc = -g_itp_block.accel_inc;
				}
			}
			
			buffer_write(g_itp_segment_buffer, &sgm);
			//BUFFER_WRITE_PTR_INC(g_itp_segment_buffer);
		}	
	}
	
}

void interpolator_update_profile()
{
	float exit_speed_sqr = planner_get_block_exit_speed_sqr();
	float v_acc_deacc_sqr = 4.0f * g_itp_block.pl_block->acceleration * g_itp_block.pl_block->distance;
	float v_cruize_sqr = (exit_speed_sqr + g_itp_block.pl_block->entry_speed_sqr + v_acc_deacc_sqr) * 0.25;
	float target_speed_sqr = g_itp_block.pl_block->target_speed*g_itp_block.pl_block->target_speed;
	v_cruize_sqr = MIN(v_cruize_sqr, target_speed_sqr);
	
	g_itp_block.pulse_block->step_freq_const = sqrtf(v_cruize_sqr)*g_itp_block.steps_mm;
	float step_dist_const = g_itp_block.accel_inv * 0.5 * g_itp_block.steps_mm;
	g_itp_block.accel_until = (uint32_t)(v_cruize_sqr - g_itp_block.pl_block->entry_speed_sqr) * step_dist_const;
	g_itp_block.deaccel_from = g_itp_block.pulse_block->totalsteps - (uint32_t)(v_cruize_sqr - exit_speed_sqr) * step_dist_const;
}

void interpolator_sleep()
{
	//board_stopPulse();
}

void interpolator_rt_integrator()
{
	static bool const_speed = false;
	
	if(exec_segment.remaining_steps!=0)
	{
		if(exec_segment.step_freq_inc != 0)
		{
			exec_segment.block->step_freq += exec_segment.step_freq_inc;
			board_changePulse(exec_segment.block->step_freq);
			const_speed = false;
		}
		else if(!const_speed)
		{
			exec_segment.block->step_freq = exec_segment.block->step_freq_const;
			board_changePulse(exec_segment.block->step_freq);
			const_speed = true;
		}
	}
}

void interpolator_rt_pulsereset()
{
	board_setStepDirs(exec_segment.next_pulsebits & 0x0555);
}

void interpolator_rt_pulse()
{
	static uint16_t error;
	static uint32_t step_counter[STEPPER_COUNT];
	uint8_t dirbits;
	uint8_t i = STEPPER_COUNT;
	
	//resets step bits and pulse
	board_setStepDirs(exec_segment.next_pulsebits);

	if(exec_segment.remaining_steps!=0)
	{
		exec_segment.next_pulsebits &= 0x0555;

		//do loop unroll
	    do{
	    	i--;
	        step_counter[i] += exec_segment.block->steps[i];
	        if (step_counter[i] > exec_segment.block->totalsteps)
	        {
	            step_counter[i] -= exec_segment.block->totalsteps;
	            SETBIT(exec_segment.next_pulsebits,i<<2 + 1);
	        }
	    } while (i);
	    
	    exec_segment.remaining_steps--;
	}
	
	if(exec_segment.remaining_steps==0)
	{
		if(is_buffer_empty(g_itp_segment_buffer))
		{
			//interpolator_sleep();
			return;
		}
		
		buffer_read(g_itp_segment_buffer, &exec_segment);
		//BUFFER_READ_PTR_INC(g_itp_segment_buffer);
		
		exec_segment.next_pulsebits &= 0x0AAA;
		i = 0;
		dirbits = exec_segment.block->dirbits;
		do{
			if(dirbits&1)
			{
				SETBIT(exec_segment.next_pulsebits, i<<2);
			}
			dirbits>>=1;
			step_counter[i] = exec_segment.block->steps[i];
		} while(++i != STEPPER_COUNT);
	}
}

