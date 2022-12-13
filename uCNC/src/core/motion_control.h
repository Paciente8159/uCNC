/*
		Name: motion_control.h
		Description: Contains the building blocks for performing motions/actions in µCNC.

		Copyright: Copyright (c) João Martins
		Author: João Martins
		Date: 19/11/2019

		µCNC is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version. Please see <http://www.gnu.org/licenses/>

		µCNC is distributed WITHOUT ANY WARRANTY;
		Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
		See the	GNU General Public License for more details.
*/

#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../module.h"
#include <stdint.h>
#include <stdbool.h>

#define MOTIONCONTROL_MODE_FEED 0
#define MOTIONCONTROL_MODE_INVERSEFEED 1
#define MOTIONCONTROL_MODE_BACKLASH_COMPENSATION 2
#define MOTIONCONTROL_MODE_PAUSEPROGRAM 4
#define MOTIONCONTROL_MODE_PAUSEPROGRAM_CONDITIONAL 8

#define MOTIONCONTROL_PROBE_INVERT 1
#define MOTIONCONTROL_PROBE_NOALARM_ONFAIL 2

	typedef union
	{
		/* data */
		uint8_t reg;
		struct
		{
			uint8_t feed_override : 1;
			uint8_t optimal : 1;
			uint8_t synched : 1;
#if TOOL_COUNT > 0
			uint8_t spindle_running : 2;
			uint8_t coolant : 2;
#else
		uint8_t : 4; // unused
#endif
#ifdef ENABLE_BACKLASH_COMPENSATION
			uint8_t backlash_comp : 1;
#else
		uint8_t : 1; // unused
#endif
		} bit;
	} motion_flags_t;

	typedef struct
	{
#ifdef GCODE_PROCESS_LINE_NUMBERS
		uint32_t line;
#endif
		step_t steps[STEPPER_COUNT];
		uint8_t dirbits;
#ifdef ENABLE_LINACT_PLANNER
		uint32_t full_steps; // number of steps of all linear actuators
#endif
		float feed;
		float max_feed;
		float max_accel;
		float feed_conversion;
		float cos_theta; // angle between current and previous motion
		uint8_t main_stepper;
		uint16_t spindle;
		uint16_t dwell;
		uint8_t motion_mode;
		motion_flags_t motion_flags;
	} motion_data_t;

	void mc_init(void);
	bool mc_get_checkmode(void);
	bool mc_toogle_checkmode(void);

	// async motions
	uint8_t mc_line(float *target, motion_data_t *block_data);
	uint8_t mc_arc(float *target, float center_offset_a, float center_offset_b, float radius, uint8_t axis_0, uint8_t axis_1, bool isclockwise, motion_data_t *block_data);

	// sync motions
	uint8_t mc_dwell(motion_data_t *block_data);
	uint8_t mc_pause(void);
	uint8_t mc_update_tools(motion_data_t *block_data);

	// mixed/special motions
	uint8_t mc_home_axis(uint8_t axis, uint8_t axis_limit);
	uint8_t mc_probe(float *target, uint8_t flags, motion_data_t *block_data);

	void mc_get_position(float *target);
	void mc_sync_position(void);

#ifdef ENABLE_MOTION_CONTROL_MODULES
	// event_mc_line_segment_handler
	DECL_EVENT_HANDLER(mc_line_segment);
#endif

#ifdef __cplusplus
}
#endif

#endif
