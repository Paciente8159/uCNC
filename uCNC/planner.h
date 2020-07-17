/*
	Name: planner.h
	Description: Chain planner for linear motions and acceleration/deacceleration profiles.
        It uses a similar algorithm to Grbl.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 24/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef PLANNER_H
#define PLANNER_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

#define PLANNER_BUFFER_SIZE 15

#define PLANNER_MOTION_MODE_NOMOTION 1
#define PLANNER_MOTION_MODE_FEED 2
#define PLANNER_MOTION_MODE_INVERSEFEED 4

#define PLANNER_MOTION_EXACT_PATH 32 //default (not used)
#define PLANNER_MOTION_EXACT_STOP 64
#define PLANNER_MOTION_CONTINUOUS 128

typedef struct
{
    #ifdef GCODE_PROCESS_LINE_NUMBERS
    uint32_t line;
    #endif
    float dir_vect[AXIS_COUNT];
    float distance;
    float feed;
    int16_t spindle;
    uint16_t dwell;
    uint8_t motion_mode;
} planner_block_data_t;

typedef struct
{
    #ifdef GCODE_PROCESS_LINE_NUMBERS
    uint32_t line;
    #endif
    uint8_t dirbits;
    float pos[AXIS_COUNT];

    float distance;
    //float angle_factor;

    float entry_feed_sqr;
    float entry_max_feed_sqr;
    float feed_sqr;
    float rapid_feed_sqr;

    float acceleration;
    float accel_inv;

#ifdef USE_SPINDLE
    int16_t spindle;
#endif
#ifdef USE_COOLANT
    uint8_t coolant;
#endif
    uint16_t dwell;

    bool optimal;
} planner_block_t;

void planner_init(void);
void planner_clear(void);
bool planner_buffer_is_full(void);
bool planner_buffer_is_empty(void);
planner_block_t *planner_get_block(void);
float planner_get_block_exit_speed_sqr(void);
float planner_get_block_top_speed(void);
#ifdef USE_SPINDLE
void planner_get_spindle_speed(float scale, uint8_t* pwm,bool* invert);
float planner_get_previous_spindle_speed(void);
#endif
void planner_discard_block(void);
void planner_add_line(float *target, planner_block_data_t block_data);
void planner_add_analog_output(uint8_t output, uint8_t value);
void planner_add_digital_output(uint8_t output, uint8_t value);
void planner_get_position(float *axis);
void planner_set_position(float *axis);
void planner_resync_position(void);

//overrides
void planner_toggle_overrides(void);
bool planner_get_overrides(void);

void planner_feed_ovr_reset(void);
void planner_feed_ovr_inc(uint8_t value);

void planner_rapid_feed_ovr_reset();
void planner_rapid_feed_ovr(uint8_t value);
#ifdef USE_SPINDLE
void planner_spindle_ovr_reset(void);
void planner_spindle_ovr_inc(uint8_t value);
#endif

bool planner_get_overflows(uint8_t *overflows);

#endif
