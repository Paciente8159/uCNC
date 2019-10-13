#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "machinedefs.h"
#include "boarddefs.h"
#include "interpolator.h"
#include "planner.h"
//#include "board.h"
#include "utils.h"

#define PULSE_BUFFER_SIZE 10
#define INTEGRATOR_DELTA_T 1.0f / F_INTEGRATOR

typedef struct pulse_blk_
{
	uint8_t dirbits;
	uint32_t steps[STEPPER_COUNT];
	uint32_t totalsteps;
	float step_freq;
	float step_freq_const;
	struct pulse_blk_ *next;
	struct pulse_blk_ *prev;
}PULSE_BLOCK;

typedef struct pulse_sgm_
{
	PULSE_BLOCK *block;
	uint16_t next_pulsebits;
	uint16_t remaining_steps;
	float step_freq_inc;
	struct pulse_sgm_ *next;
	struct pulse_sgm_ *prev;
}PULSE_SEGMENT;

typedef struct
{
	uint32_t remaining_steps;

	float steps_mm;
	float current_speed_sqr;
	float max_speed;
	float target_speed_sqr;
	float accel;
	float distance;
	
	float accel_until;
	float deaccel_from;

}INTERPOLATOR_BLOCK;

static INTERPOLATOR_BLOCK g_itp_block;

PLANNER_MOTION_BLOCK* g_planner_block;

//circular buffers
PULSE_BLOCK g_itp_block_buffer[PULSE_BUFFER_SIZE];
PULSE_BLOCK *g_itp_block_wr;
PULSE_BLOCK *g_itp_block_rd;
PULSE_SEGMENT g_itp_segment_buffer[PULSE_BUFFER_SIZE];
PULSE_SEGMENT *g_itp_segment_wr;
PULSE_SEGMENT *g_itp_segment_rd;

volatile uint16_t g_itp_pulse_tick;
volatile bool g_itp_busy;

bool itp_block_buffer_empty()
{
	return (g_itp_block_wr == g_itp_block_rd);
}

bool itp_block_buffer_full()
{
	return (g_itp_block_wr->next == g_itp_block_rd);
}

bool itp_segment_buffer_empty()
{
	return (g_itp_segment_wr == g_itp_segment_rd);
}

bool itp_segment_buffer_full()
{
	return (g_itp_segment_wr->next == g_itp_segment_rd);
}

void interpolator_init()
{
	g_itp_busy = false;
	g_itp_pulse_tick = 0xFFFF;
	
	//resets buffers
	memset(&g_itp_block, 0, sizeof(INTERPOLATOR_BLOCK));
	memset(&g_itp_block_buffer, 0, PULSE_BUFFER_SIZE * sizeof(PULSE_BLOCK));
	memset(&g_itp_segment_buffer, 0, PULSE_BUFFER_SIZE * sizeof(PULSE_SEGMENT));
	
	board_attachOnIntegrator(interpolator_rt_integrator);
	board_startIntegrator(F_INTEGRATOR);
	board_attachOnPulse(interpolator_rt_pulse);
	board_startPulse(F_PULSE);
	
	//initialize circular buffers
	CIRC_BUFFER_INIT(g_itp_block_buffer, PULSE_BUFFER_SIZE);
	g_itp_block_wr = (PULSE_BLOCK*)&g_itp_block_buffer;
	g_itp_block_rd = g_itp_block_wr;
	CIRC_BUFFER_INIT(g_itp_segment_buffer, PULSE_BUFFER_SIZE);
	g_itp_segment_wr = (PULSE_SEGMENT*)&g_itp_segment_buffer;
	g_itp_segment_rd = g_itp_segment_wr;
	
	g_planner_block = NULL;
}

void interpolator_exec_planner_block()
{
	//no planner blocks has beed processed or last planner block was fully processed
	if(g_planner_block==NULL)
	{
		//planner is empty. Nothing to be done
		if(planner_buffer_empty())
		{
			return;
		}
		
		//interpolator block buffer full
		if(itp_block_buffer_full())
		{
			return;
		}
		
		//starts to process the buffer block
		PLANNER_MOTION_BLOCK* block = planner_get_block();
		g_itp_block_wr->dirbits = block->dirbits;
		memcpy(&(g_itp_block_wr->steps), &(block->steps), sizeof(g_itp_block_wr->steps));
		g_itp_block_wr->totalsteps = block->totalsteps;
		g_itp_block.remaining_steps = block->totalsteps;
		
		g_itp_block.steps_mm = roundf(block->totalsteps / block->distance);
		g_itp_block.current_speed_sqr = block->entry_speed_sqr;
	
		g_itp_block.target_speed_sqr = block->target_speed * block->target_speed;
		g_itp_block.accel = block->acceleration;
		g_itp_block.distance = block->distance;
		
		//calculates the accel and deaccel profiles
		interpolator_update_profile();
		
		//initial steps per second (step frequency)
		//this is updated in the step integrator (acceleration)
		g_itp_block_wr->step_freq = sqrtf(g_itp_block.current_speed_sqr)*g_itp_block.steps_mm;
		g_itp_segment_wr->step_freq_inc = block->acceleration * g_itp_block.steps_mm * INTEGRATOR_DELTA_T;
	}
	
	//creates segments and fills the buffer
	while(!itp_segment_buffer_full())
	{
		g_itp_segment_wr->block = g_itp_block_wr;
		
		//fill ramp accel
		if(g_itp_block.accel_until != 0)
		{
			uint32_t steps_to_accel = roundf(g_itp_block.accel_until * g_itp_block.steps_mm);
			do{
				if(steps_to_accel > 0XFFFF)
				{
					g_itp_segment_wr->remaining_steps = 0XFFFF;
					steps_to_accel -= 0XFFFF;
				}
			}while(steps_to_accel != 0);
			
		}		
	}
	
}

void interpolator_update_profile()
{
	float double_acc = g_itp_block.accel * 2;
	float double_acc_inv = 1.0f / double_acc;
	float exit_speed_sqr = planner_get_block_exit_speed_sqr();
	float v_acc_deacc_sqr = 2.0f * double_acc * g_itp_block.distance;
	float v_cruize_sqr = (exit_speed_sqr + g_itp_block.current_speed_sqr + v_acc_deacc_sqr) * 0.25;
	v_cruize_sqr = MIN(v_cruize_sqr, g_itp_block.target_speed_sqr);
	
	g_itp_block.accel_until = (v_cruize_sqr - g_itp_block.current_speed_sqr) * double_acc_inv;
	g_itp_block.deaccel_from = (v_cruize_sqr - exit_speed_sqr) * double_acc_inv;
}

void interpolator_rt_integrator()
{
	//g_itp_segment_rd->block->step_freq += g_itp_segment_rd->step_freq_inc;
	//board.changePulse(roundf(g_itp_segment_wr->block->step_freq));
}

void interpolator_rt_pulse()
{
	static uint16_t error;
	static uint32_t step_counter[STEPPER_COUNT];
	uint8_t stepbits;
	uint8_t i = STEPPER_COUNT;
	
	board_setStepDirs(g_itp_segment_rd->next_pulsebits);

	
	//do loop unroll
    do{
    	i--;
        step_counter[i] += g_itp_block_wr->steps[i];
        if (step_counter[i] > g_itp_block_wr->totalsteps)
        {
            step_counter[i] -= g_itp_block_wr->totalsteps;
            SETBIT(stepbits,i<<1);
        }
    }while (i);	
}

