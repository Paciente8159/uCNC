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
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef PLANNER_H
#define PLANNER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "motion_control.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef PLANNER_BUFFER_SIZE
#define PLANNER_BUFFER_SIZE 20
#endif

#define PLANNER_MOTION_EXACT_PATH 32 // default (not used)
#define PLANNER_MOTION_EXACT_STOP 64
#define PLANNER_MOTION_CONTINUOUS 128

#define TOOL_STATE_COPY_FLAG_MASK 0x78
	typedef motion_flags_t planner_flags_t;

	typedef struct planner_block_
	{
#ifdef GCODE_PROCESS_LINE_NUMBERS
		uint32_t line;
#endif
		uint8_t dirbits;
		step_t steps[STEPPER_COUNT];
		uint8_t main_stepper;
		float feed_conversion;
		float entry_feed_sqr;
		float entry_max_feed_sqr;
		float feed_sqr;
		float rapid_feed_sqr;
		float acceleration;

#if TOOL_COUNT > 0
		int16_t spindle;
#endif
		// uint8_t action;
		planner_flags_t planner_flags;
	} planner_block_t;

	typedef struct
	{
		uint8_t ovr_counter;
		uint8_t feed_override;
		uint8_t rapid_feed_override;
		uint8_t planner_ovr_counter;
#if TOOL_COUNT > 0
		int16_t spindle_speed;
		uint8_t spindle_speed_override;
		planner_flags_t state_flags;
#endif
	} planner_state_t;

	extern planner_state_t g_planner_state;

	void planner_init(void);
	void planner_clear(void);
	bool planner_buffer_is_full(void);
	bool planner_buffer_is_empty(void);
	planner_block_t *planner_get_block(void);
	planner_block_t *planner_get_last_block(void);
	float planner_get_block_exit_speed_sqr(void);
	float planner_get_block_top_speed(float exit_speed_sqr);
#if TOOL_COUNT > 0
	int16_t planner_get_spindle_speed(float scale);
	uint8_t planner_get_coolant(void);
#endif
	void planner_discard_block(void);
	void planner_add_line(motion_data_t *block_data);
	void planner_add_analog_output(uint8_t output, uint8_t value);
	void planner_add_digital_output(uint8_t output, uint8_t value);
	void planner_sync_tools(motion_data_t *block_data);

	// overrides
	void planner_feed_ovr(uint8_t value);
	void planner_rapid_feed_ovr(uint8_t value);
#if TOOL_COUNT > 0
	void planner_spindle_ovr(uint8_t value);
	void planner_spindle_ovr_toggle(void);
	void planner_spindle_ovr_reset(void);
	uint8_t planner_coolant_ovr_toggle(uint8_t value);
	void planner_coolant_ovr_reset(void);
#endif

	uint8_t planner_get_buffer_freeblocks();

#ifdef ENABLE_MOTION_CONTROL_PLANNER_HIJACKING
	// creates a full copy of the planner state
	void planner_store(void);
	// restores the planner to it's previous saved state
	void planner_restore(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
