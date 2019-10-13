#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "machinedefs.h"
#include "interpolator.h"
#include "planner.h"
#include "board.h"

#define INTEGRATOR_FREQ 600
#define INTEGRATOR_DELTA_T 1/INTEGRATOR_FREQ

typedef struct
{
	uint8_t dirbits;
	uint32_t steps[STEPPER_COUNT];
	uint32_t totalsteps;
	uint16_t step_per_second;
	uint8_t next_stepbits;
	uint16_t tick_count;	
}INTERPOLATOR_SEGMENT;

typedef struct
{
	uint8_t dirbits;
	uint32_t steps[STEPPER_COUNT];
	uint32_t totalsteps;

	float steps_mm;
	float current_speed;
	float max_speed;
	float target_speed;
	float acceleration;
	float exit_speed;
	
	float accel_until;
	float deaccel_from;

} INTERPOLATOR_BLOCK;

static INTERPOLATOR_BLOCK g_itp_block;

void interpolator_init()
{
	memset(&g_itp_block, 0, sizeof(INTERPOLATOR_BLOCK));
	board_attachOnIntegrator(interpolator_rt_integrator);
	board_startIntegrator(INTEGRATOR_FREQ);
}

void interpolator_update_segments()
{
	//nothing to be done
	if(planner_buffer_empty())
	{
		return;
	}
	
	PLANNER_MOTION_BLOCK* block = planner_get_block();
	g_itp_block.dirbits = block->dirbits;
	g_itp_block.steps = block->steps;
	g_itp_block.totalsteps = block->totalsteps;
	g_itp_block.current_speed = block->entry_speed_sqr;
	g_itp_block.exit_speed = planner_get_block_exit_speed_sqr();
	g_itp_block.steps_mm = g_itp_block.totalsteps / block->distance;
	g_itp_block.acceleration = block->acceleration * INTEGRATOR_DELTA_T;
	
	float v_acc = block->acceleration * block->distance;
	float v_deacc_sqr = planner_get_block_exit_speed_sqr() + v_acc;
	float v_acc_sqr = block->entry_speed_sqr + v_acc;
	float v_final = sqrtf(MIN(v_acc_sqr, v_deacc_sqr));
	v_final = MIN(v_final, block->target_speed);
	
	accel_until = 
}

void interpolator_rt_integrator()
{
	g_itp_block.current_speed += g_itp_block.acceleration;
	
}

