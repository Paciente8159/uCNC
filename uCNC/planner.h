/*
	Name: planner.h
	Description: uCNC motion planner
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 24/09/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef PLANNER_H
#define PLANNER_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "machinedefs.h"

#define PLANNER_BUFFER_SIZE 10

typedef struct
{
	uint8_t dirbits;
	float pos[AXIS_COUNT];
	float dir_vect[AXIS_COUNT];

	float distance;
	float angle_factor;

	float entry_speed_sqr;
	float entry_max_speed_sqr;

	float max_speed;
	float target_speed;
	float acceleration;
	float accel_inv;

	bool optimal;
} PLANNER_BLOCK;

void planner_init();
void planner_clear();
bool planner_buffer_is_full();
bool planner_buffer_is_empty();
PLANNER_BLOCK* planner_get_block();
float planner_get_block_exit_speed_sqr();
float planner_get_block_top_speed();
void planner_discard_block();
void planner_add_line(float* axis, float feed);
void planner_add_analog_output(uint8_t output, uint8_t value);
void planner_add_digital_output(uint8_t output, uint8_t value);
float* planner_get_position();
void planner_resync_position();

#endif
