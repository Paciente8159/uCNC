/*
	Name: 
	Copyright: 
	Author: 
	Date: 23/09/19 23:14
	Description: 
*/

#ifndef PLANNER_H
#define PLANNER_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "machinedefs.h"

#define PLANNER_BUFFER_SIZE 5

typedef struct
{
	/*uint8_t type;
	uint8_t dirs;
	uint32_t steps[STEPPER_COUNT];
	uint32_t totalsteps;
	uint32_t accel_until;
	uint32_t deaccel_from;
	uint16_t accel_rate;
	uint16_t speed_rate;*/
	uint8_t dirbits;
	/*uint32_t steps[STEPPER_COUNT];
	uint32_t totalsteps;*/
	float pos[AXIS_COUNT];
	float dir_vect[AXIS_COUNT];

	float distance;
	float angle_factor;

	float entry_speed_sqr;
	float entry_max_speed_sqr;
	//float exit_speed_sqr;	//same as next entry_speed

	float max_speed;
	float target_speed;
	float acceleration;

	bool optimal;
} PLANNER_MOTION;

typedef struct pl_motion_blk_
{
	uint8_t dirbits;
	uint32_t steps[STEPPER_COUNT];
	uint32_t totalsteps;

	float dir_vect[AXIS_COUNT];

	float distance;
	float angle_factor;

	float entry_speed_sqr;
	float entry_max_speed_sqr;
	//float exit_speed_sqr;	//same as next entry_speed

	float max_speed;
	float target_speed;
	float acceleration;

	bool optimal;

	struct pl_motion_blk_ *next;
	struct pl_motion_blk_ *prev;
} PLANNER_MOTION_BLOCK;

void planner_init();
bool planner_buffer_full();
bool planner_buffer_empty();
PLANNER_MOTION* planner_get_block();
float planner_get_block_exit_speed_sqr();
void planner_discard_block();
void planner_add_line(float* axis, float feed);
void planner_add_analog_output(uint8_t output, uint8_t value);
void planner_add_digital_output(uint8_t output, uint8_t value);
void planner_add_arc(float* axis, float* center, uint8_t clockwise, uint8_t plane, float feed);

#endif
