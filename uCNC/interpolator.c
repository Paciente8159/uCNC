#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "machinedefs.h"
#include "interpolator.h"
#include "planner.h"
#include "board.h"
#include "utils.h"

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
	float current_speed_sqr;
	float max_speed;
	float target_speed_sqr;
	float double_accel;
	float double_accel_inv;
	float distance;
	
	float accel_until;
	float deaccel_from;

} INTERPOLATOR_BLOCK;

static INTERPOLATOR_BLOCK g_itp_block;

void interpolator_init()
{
	memset(&g_itp_block, 0, sizeof(INTERPOLATOR_BLOCK));
	board_attachOnIntegrator(interpolator_rt_integrator);
	board_startIntegrator(INTEGRATOR_FREQ);
	board_startPulse(30000);
}

void interpolator_exec_planner_block()
{
	//nothing to be done
	if(planner_buffer_empty())
	{
		return;
	}
	
	PLANNER_MOTION_BLOCK* block = planner_get_block();
	g_itp_block.dirbits = block->dirbits;
	memcpy(&(g_itp_block.steps), &(block->steps), sizeof(g_itp_block.steps));
	g_itp_block.totalsteps = block->totalsteps;
	
	g_itp_block.steps_mm = g_itp_block.totalsteps / block->distance;
	g_itp_block.current_speed_sqr = block->entry_speed_sqr;

	g_itp_block.target_speed_sqr = block->target_speed * block->target_speed;
	g_itp_block.double_accel = 2.0f * block->acceleration;
	g_itp_block.double_accel_inv = 1.0f/g_itp_block.double_accel;
	g_itp_block.distance = block->distance;
	
	interpolator_update_profile();
}

void interpolator_update_profile()
{
	float exit_speed_sqr = planner_get_block_exit_speed_sqr();
	float v_acc_deacc_sqr = 2.0f * g_itp_block.double_accel * g_itp_block.distance;
	float v_cruize_sqr = (exit_speed_sqr + g_itp_block.current_speed_sqr + v_acc_deacc_sqr) * 0.25;
	v_cruize_sqr = MIN(v_cruize_sqr, g_itp_block.target_speed_sqr);
	
	g_itp_block.accel_until = g_itp_block.distance;
	g_itp_block.accel_until -= (v_cruize_sqr - g_itp_block.current_speed_sqr) * g_itp_block.double_accel_inv;
	g_itp_block.deaccel_from = (v_cruize_sqr - exit_speed_sqr) * g_itp_block.double_accel_inv;
}

void interpolator_rt_integrator()
{
	//g_itp_block.current_speed += g_itp_block.acceleration;
	
}

