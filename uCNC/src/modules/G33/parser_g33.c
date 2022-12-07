/*
	Name: parser_g33.c
	Description: Implements a parser extension for LinuxCNC G33 for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 25/11/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_SETTINGS_MODULES
static float tool_at_speed_tolerance;
#endif

#ifdef ENABLE_PARSER_MODULES

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
#error "This module is not compatible with the current version of µCNC"
#endif

#define SYNC_DISABLED 0
#define SYNC_READY 1
#define SYNC_RUNNING 2
#define SYNC_EVAL 4

static volatile uint8_t synched_motion_status;
static volatile int32_t spindle_index_counter;
static uint32_t steps_per_index;
static uint32_t motion_total_steps;

void spindle_index_cb_handler(void)
{
	switch (synched_motion_status)
	{
	case SYNC_READY:
		itp_start(false);
		synched_motion_status = SYNC_RUNNING;
		break;
	case SYNC_RUNNING:
		spindle_index_counter++;
		synched_motion_status |= SYNC_EVAL;
		break;
	}
}

// this ID must be unique for each code
#define G33 33

uint8_t g33_parse(void *args, bool *handled);
uint8_t g33_exec(void *args, bool *handled);

CREATE_EVENT_LISTENER(gcode_parse, g33_parse);
CREATE_EVENT_LISTENER(gcode_exec, g33_exec);

// this just parses and accepts the code
uint8_t g33_parse(void *args, bool *handled)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'G' && ptr->code == 33)
	{
		// stops event propagation
		*handled = true;
		if (ptr->cmd->group_extended != 0 || CHECKFLAG(ptr->cmd->groups, GCODE_GROUP_MOTION))
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		// checks if it's G5 or G5.1
		// check mantissa
		uint8_t mantissa = (uint8_t)lroundf(((ptr->value - ptr->code) * 100.0f));

		if (mantissa != 0)
		{
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}

		ptr->new_state->groups.motion = G33;
		ptr->new_state->groups.motion_mantissa = 0;
		SETFLAG(ptr->cmd->groups, GCODE_GROUP_MOTION);
		ptr->cmd->group_extended = EXTENDED_MOTION_GCODE(33);
		return STATUS_OK;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return ptr->error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t g33_exec(void *args, bool *handled)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	if (ptr->cmd->group_extended == EXTENDED_MOTION_GCODE(33))
	{
		// stops event propagation
		*handled = true;

		if (!CHECKFLAG(ptr->cmd->words, GCODE_XYZ_AXIS))
		{
			// it's an error no axis word is specified
			return STATUS_GCODE_NO_AXIS_WORDS;
		}

		if (!CHECKFLAG(ptr->cmd->words, GCODE_WORD_K))
		{
			// it's an error no distance per rev word is specified
			return STATUS_GCODE_VALUE_WORD_MISSING;
		}

		// syncs motions and sets spindle
		if (mc_update_tools(ptr->block_data) != STATUS_OK)
		{
			return STATUS_CRITICAL_FAIL;
		}

		encoder_attach_index_cb(&spindle_index_cb_handler);

		// calculates travel distance

		// gets the starting point
		float prev_target[AXIS_COUNT];
		mc_get_position(prev_target);
		kinematics_apply_transform(prev_target);
		int32_t prev_step_pos[STEPPER_COUNT];
		kinematics_apply_inverse(prev_target, prev_step_pos);

		// gets the exit point (copies to prevent modifying target vector)
		float line_dist = 0;
		float dir_vect[AXIS_COUNT];
		memcpy(dir_vect, ptr->target, sizeof(dir_vect));
		kinematics_apply_transform(dir_vect);
		int32_t next_step_pos[STEPPER_COUNT];
		kinematics_apply_inverse(dir_vect, next_step_pos);

		// calculates amount of motion vector
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			dir_vect[i] -= prev_target[i];
			line_dist += dir_vect[i] * dir_vect[i];
		}

		line_dist = sqrtf(line_dist);

		// determines the normalized direction vector
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			dir_vect[i] /= line_dist;
		}

		// calculates the total number of steps in the motion
		uint32_t total_steps = 0;
		for (uint8_t i = AXIS_TO_STEPPERS; i != 0;)
		{
			i--;
			int32_t steps = next_step_pos[i] - prev_step_pos[i];

			steps = ABS(steps);
			if (total_steps < (uint32_t)steps)
			{
				total_steps = steps;
			}
		}

		motion_total_steps = total_steps;

		// calculates the feedrate based in the K factor and the programmed spindle RPM
		// spindle is in Rev/min and K is in units(mm) per Rev Rev/min * mm/Rev = mm/min
		float total_revs = line_dist / ptr->words->ijk[2];
		float feed = ptr->words->ijk[2] * ptr->block_data->spindle;
		ptr->block_data->feed = feed;
		ptr->block_data->motion_flags.bit.synched = 1;

		// convert feed to mm/s
		feed *= MIN_SEC_MULT;

		// calculates the needed distance to reach the desired feed
		float needed_distance = feed * feed * 0.5f / ptr->block_data->max_accel;

		// calculates the next number of complete rev in which the desired speed can be reached in sync to index pulse
		float revs_to_sync = ceilf(needed_distance / ptr->words->ijk[2]);

		// with the needed revs readjusts the acceleration to meet the specs
		float distance_to_accel = ptr->words->ijk[2] * revs_to_sync;
		float new_accel = feed * feed * 0.5f / distance_to_accel;

		// given the feed and acceleration calculates how many extra full revs will be executed by the spindle before the motions sync
		float elapsed_time = feed / new_accel;

		// initialize the index counter (this number is always negative unless sync distance is 0)
		spindle_index_counter = (int32_t)(revs_to_sync - (ptr->block_data->spindle * MIN_SEC_MULT * elapsed_time));

		// calculates the expected number of steps per revolution
		float steps_per_rev = (float)motion_total_steps / total_revs;
		steps_per_index = lroundf(steps_per_rev);

		// initializes the sync step counter (starts negative - the unsynched region)
		// itp_sync_step_counter should reach 0 at the same time spindle_index_counter reaches 0 too
		itp_sync_step_counter = -lroundf(steps_per_rev * revs_to_sync);

		// now queue the acceleration phase in the planner
		// calculate the intermediate target where speeds sync
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			prev_target[i] += (dir_vect[i] * distance_to_accel);
		}

		// create a copy of the original block and adjust the accel
		motion_data_t accel_block;
		memcpy(&accel_block, ptr->block_data, sizeof(motion_data_t));
		accel_block.max_accel = new_accel;

		// queue motions in planner
		if (mc_line(prev_target, &accel_block) != STATUS_OK)
		{
			return STATUS_CRITICAL_FAIL;
		}

		if (mc_line(ptr->target, ptr->block_data) != STATUS_OK)
		{
			return STATUS_CRITICAL_FAIL;
		}

		// wait for spindle to reach the desired speed
		uint16_t programmed_speed = planner_get_spindle_speed(1);
		uint16_t at_speed_threshold = lroundf(tool_at_speed_tolerance * programmed_speed);

		// wait for tool at speed
		while (ABS(programmed_speed - tool_get_speed()) > at_speed_threshold)
		{
			if (cnc_dotasks() != STATUS_OK)
			{
				return STATUS_CRITICAL_FAIL;
			}
		}

		// flag the spindle index callback that it can start the threading motion
		synched_motion_status = SYNC_READY;

		// wait for the motion to end
		if (itp_sync() != STATUS_OK)
		{
			return STATUS_CRITICAL_FAIL;
		}

		encoder_dettach_index_cb();
	}

	return STATUS_GCODE_EXTENDED_UNSUPPORTED;
}

#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
uint8_t spindle_sync_update_loop(void *ptr, bool *handled)
{
#ifndef RPM_SYNC_UPDATE_ON_INDEX_ONLY
	if (encoder_rpm_updated)
	{
		// clear update flag
		encoder_rpm_updated = false;

		uint32_t index_counter;
		uint32_t rt_pulse_counter;
		__ATOMIC__
		{
			index_counter = spindle_index_counter;
			rt_pulse_counter = itp_sync_step_counter;
		}

		// still in accel phase (leave)
		if (index_counter < 0 && rt_pulse_counter < 0)
		{
			return STATUS_OK;
		}

		// calculate the spindle position
		uint32_t spindle_pulse_counter = (encoder_get_position(RPM_ENCODER) * steps_per_index / RPM_PPR) + (index_counter * steps_per_index);
		int32_t error = spindle_pulse_counter - itp_sync_step_counter;
	}
#endif

	return STATUS_OK;
}

CREATE_EVENT_LISTENER(cnc_dotasks, spindle_sync_update_loop);
#endif

DECL_MODULE(g33)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, g33_parse);
	ADD_EVENT_LISTENER(gcode_exec, g33_exec);
#else
#error "Parser extensions are not enabled. G33 code extension will not work."
#endif
#ifndef ENABLE_IO_MODULES
#error "IO extensions are not enabled. G33 code extension will not work."
#endif
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, spindle_sync_update_loop);
#else
#error "Main loop extensions are not enabled. G33 code extension will not work."
#endif
#ifndef ENABLE_RT_SYNC_MOTIONS
#error "ENABLE_RT_SYNC_MOTIONS must be enabled to allow realtime step counting in sync motions."
#endif
}
