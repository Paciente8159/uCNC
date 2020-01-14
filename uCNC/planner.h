/*
	Name: planner.h
	Description: Chain planner for linear motions and acceleration/deacceleration profiles.
        It uses a similar algorithm to Grbl.
			
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
	float feed;
	float spindle;
	uint8_t coolant;
	uint16_t dwell;
	bool no_motion;
} planner_block_data_t;

typedef struct
{
	uint8_t dirbits;
	float pos[AXIS_COUNT];
	float dir_vect[AXIS_COUNT];

	float distance;
	float angle_factor;

	float entry_feed_sqr;
	float entry_max_feed_sqr;
	float feed_sqr;
	float rapid_feed_sqr;

	float acceleration;
	float accel_inv;

	#ifdef USE_SPINDLE
	float spindle;
	#endif
	#ifdef USE_COOLANT
	uint8_t coolant;
	#endif
	uint16_t dwell;

	bool optimal;
} planner_block_t;

void planner_init();
void planner_clear();
bool planner_buffer_is_full();
bool planner_buffer_is_empty();
planner_block_t *planner_get_block();
float planner_get_block_exit_speed_sqr();
float planner_get_block_top_speed();
#ifdef USE_SPINDLE
float planner_get_spindle_speed(uint8_t *pwm, bool *spindle_ccw);
#endif
#ifdef USE_COOLANT
uint8_t planner_get_coolant();
#endif
void planner_discard_block();
void planner_add_line(float *target, planner_block_data_t block_data);
void planner_add_analog_output(uint8_t output, uint8_t value);
void planner_add_digital_output(uint8_t output, uint8_t value);
void planner_get_position(float *axis);
void planner_resync_position();

//overrides
void planner_toggle_overrides();
bool planner_get_overrides();

void planner_feed_ovr_reset();
void planner_feed_ovr_inc(float value);

void planner_rapid_feed_ovr_reset();
void planner_rapid_feed_ovr(float value);
#ifdef USE_SPINDLE
void planner_spindle_ovr_reset();
void planner_spindle_ovr_inc(float value);
#endif


#endif
