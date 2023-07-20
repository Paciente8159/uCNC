/*
	Name: plasma_thc.c
	Description: Defines a plasma tool with THC for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 29/06/2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <math.h>
#include <float.h>
#include <stdint.h>
#include <stdio.h>

#include "../../../cnc.h"

#ifndef PLASMA_UP_INPUT
#define PLASMA_UP_INPUT DIN15
#endif

#ifndef PLASMA_DOWN_INPUT
#define PLASMA_DOWN_INPUT DIN14
#endif

#ifndef PLASMA_ARC_OK_INPUT
#define PLASMA_ARC_OK_INPUT DIN13
#endif

#ifndef PLASMA_ON_OUTPUT
#define PLASMA_ON_OUTPUT DOUT0
#endif

#ifndef PLASMA_STEPPERS_MASK
#define PLASMA_STEPPERS_MASK (1 << 2)
#endif

#ifdef ENABLE_PLASMA_THC

// overridable
// user can implement the plasma thc up condition based on analog voltage reading using analog the controller analog inputs and PID controllers
uint8_t __attribute__((weak)) plasma_thc_up(void)
{
#if ASSERT_PIN(PLASMA_UP_INPUT)
	return io_get_input(PLASMA_UP_INPUT);
#else
	return 0;
#endif
}

// overridable
// user can implement the plasma thc down condition based on analog voltage reading using analog the controller analog inputs and PID controllers
uint8_t __attribute__((weak)) plasma_thc_down(void)
{
#if ASSERT_PIN(PLASMA_DOWN_INPUT)
	return io_get_input(PLASMA_DOWN_INPUT);
#else
	return 0;
#endif
}

// overridable
// user can implement the plasma thc arc ok condition based on analog voltage reading using analog the controller analog inputs and PID controllers
uint8_t __attribute__((weak)) plasma_thc_arc_ok(void)
{
#if ASSERT_PIN(PLASMA_ARC_OK_INPUT)
	return io_get_input(PLASMA_ARC_OK_INPUT);
#else
	return 0;
#endif
}

#define PLASMA_ARC_OFF 0
#define PLASMA_ARC_OK 1
#define PLASMA_ARC_LOST 2

// plasma thc controller variables
static uint8_t plasma_thc_state;
static volatile int8_t plasma_step_error;

typedef struct plasma_start_params_
{
	float probe_depth;	  // I
	float probe_feed;	  // J
	float retract_height; // R
	float cut_depth;	  // K
	float cut_feed;		  // F
	uint16_t dwell;		  // P*1000
	uint8_t retries;	  // L
} plasma_start_params_t;
static plasma_start_params_t plasma_start_params;

bool plasma_thc_probe_and_start(void)
{
	static bool plasma_starting = false;
	if (plasma_starting)
	{
		// prevent reentrancy
		return false;
	}

	plasma_starting = true;
	uint8_t ret = plasma_start_params.retries;
	cnc_store_motion();

	// wait for cycle start
	while (cnc_get_exec_state(EXEC_HOLD))
	{
		cnc_dotasks();
	}

	while (ret--)
	{
		// cutoff torch
		// temporary disable
		plasma_thc_state = PLASMA_ARC_OFF;
		motion_data_t block = {0};
		block.motion_flags.bit.spindle_running = 0;
		mc_update_tools(&block);

		// get current position
		float pos[AXIS_COUNT];
		mc_get_position(pos);

		// modify target to probe depth
		pos[AXIS_Z] += plasma_start_params.probe_depth;
		// probe feed speed
		block.feed = plasma_start_params.probe_feed;
		// similar to G38.2
		if (mc_probe(pos, 0, &block) == STATUS_PROBE_SUCCESS)
		{
			// modify target to probe depth
			mc_get_position(pos);
			pos[AXIS_Z] -= plasma_start_params.probe_depth * 0.5;
			block.feed = plasma_start_params.probe_feed * 0.5f; // half speed
			// similar to G38.4
			if (mc_probe(pos, 1, &block) == STATUS_PROBE_SUCCESS)
			{
				// modify target to torch start height
				mc_get_position(pos);
				pos[AXIS_Z] += plasma_start_params.retract_height;
				// rapid feed
				block.feed = FLT_MAX;
				mc_line(pos, &block);
				// turn torch on and wait before confirm the arc on signal
				block.motion_flags.bit.spindle_running = 1;
				block.dwell = plasma_start_params.dwell;
				// updated tools and wait
				mc_dwell(&block);

				// confirm if arc is ok
				if (plasma_thc_arc_ok())
				{
					mc_get_position(pos);
					pos[AXIS_Z] -= plasma_start_params.cut_depth;
					// rapid feed
					block.feed = plasma_start_params.cut_feed;
					mc_line(pos, &block);
					// enable plasma mode
					plasma_thc_state = PLASMA_ARC_OK;
					// continues program
					plasma_starting = false;
					cnc_restore_motion();
					return true;
				}
			}
		}
	}

	cnc_restore_motion();
	plasma_starting = false;
	return false;
}

#ifdef ENABLE_RT_SYNC_MOTIONS
void itp_rt_stepbits(uint8_t *stepbits, uint8_t *dirs)
{
	int8_t step_error = plasma_step_error;

	// no error or no steps being performed
	if (!step_error || !*stepbits)
	{
		return;
	}

	if (step_error > 0)
	{
		if (step_error == 1)
		{
			*stepbits |= PLASMA_STEPPERS_MASK;
			*dirs &= ~PLASMA_STEPPERS_MASK;
		}
		step_error--;
	}

	if (step_error < 0)
	{
		if (step_error == -1)
		{
			*stepbits |= PLASMA_STEPPERS_MASK;
			*dirs |= PLASMA_STEPPERS_MASK;
		}
		step_error++;
	}

	plasma_step_error = step_error;
}
#endif

#ifdef ENABLE_PARSER_MODULES
#define M103 EXTENDED_MCODE(103)

bool m103_parse(void *args);
bool m103_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m103_parse);
CREATE_EVENT_LISTENER(gcode_exec, m103_exec);

// this just parses and acceps the code
bool m103_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'M' && ptr->code == 103)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M103;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m103_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	if (ptr->cmd->group_extended == M103)
	{
		if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_K | GCODE_WORD_R | GCODE_WORD_P)) != (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_K | GCODE_WORD_R | GCODE_WORD_P))
		{
			*(ptr->error) = STATUS_GCODE_VALUE_WORD_MISSING;
			return EVENT_HANDLED;
		}

		plasma_start_params.dwell = (uint16_t)(ptr->words->p * 1000);
		plasma_start_params.probe_depth = ptr->words->ijk[0];
		plasma_start_params.probe_feed = ptr->words->ijk[1];
		plasma_start_params.cut_depth = ptr->words->ijk[2];
		plasma_start_params.retract_height = ptr->words->r;
		if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_F)))
		{
			plasma_start_params.cut_feed = ptr->words->f;
		}
		plasma_start_params.retries = (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_L))) ? ptr->words->l : 1;

		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

static void pid_update(void)
{
	if (plasma_thc_state == PLASMA_ARC_OK)
	{
		// arc lost
		// on arc lost the plasma must enter hold
		if (!(plasma_thc_arc_ok()))
		{
			// places the machine under a HOLD and signals the arc lost
			// this requires the operator to inspect the work to see if was
			// a simple arc lost or the torch is hover a hole
			plasma_thc_state = PLASMA_ARC_LOST;
			cnc_set_exec_state(EXEC_HOLD);

			// prepares the reprobing action to be executed on cycle resume action
			if (plasma_thc_probe_and_start())
			{
			}
			else
			{
				// must restore the planner and motion to be purged
				mc_restore();
				planner_restore();
				cnc_alarm(EXEC_ALARM_PLASMA_THC_ARC_START_FAILURE);
			}
		}

		if (plasma_thc_up() && !plasma_step_error)
		{
			// option 1 - modify the planner block
			// this assumes Z is not moving in this motion
			// planner_block_t *p = planner_get_block();
			// p->steps[2] = p->steps[p->main_stepper];
			// p->dirbits &= 0xFB;

			// option 2 - mask the step bits directly
			// clamp tool max step rate according to the actual motion feed
			float feed = itp_get_rt_feed();
			float max_feed_ratio = ceilf(feed / g_settings.max_feed_rate[AXIS_TOOL]);
			plasma_step_error = 1 + (uint8_t)max_feed_ratio;
		}
		else if (plasma_thc_down() && !plasma_step_error)
		{
			// option 1 - modify the planner block
			// this assumes Z is not moving in this motion
			// planner_block_t *p = planner_get_block();
			// p->steps[2] = p->steps[p->main_stepper];
			// p->dirbits |= 4;

			// option 2 - mask the step bits directly
			float feed = itp_get_rt_feed();
			float max_feed_ratio = ceilf(feed / g_settings.max_feed_rate[AXIS_TOOL]);
			plasma_step_error = -(1 + (uint8_t)max_feed_ratio);
		}
		else
		{
			// option 1 - modify the planner block
			// this assumes Z is not moving in this motion
			// planner_block_t *p = planner_get_block();
			// p->steps[2] = 0;

			// option 2 - mask the step bits directly
			plasma_step_error = 0;
		}
	}
	return EVENT_CONTINUE;
}

DECL_MODULE(plasma_thc)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m103_parse);
	ADD_EVENT_LISTENER(gcode_exec, m103_exec);
#else
#error "Parser extensions are not enabled. M103 code extension will not work."
#endif
}

static void startup_code(void)
{
// force plasma off
#if ASSERT_PIN(PLASMA_ON_OUTPUT)
	io_clear_output(PLASMA_ON_OUTPUT);
#endif
}

static void shutdown_code(void)
{
// force plasma off
#if ASSERT_PIN(PLASMA_ON_OUTPUT)
	io_clear_output(PLASMA_ON_OUTPUT);
#endif
}

static void set_speed(int16_t value)
{
	// turn plasma on
	if (value)
	{
		// enable plasma mode
		plasma_thc_state = true;
		if (!plasma_thc_arc_ok())
		{
			if (plasma_thc_probe_and_start())
			{
				cnc_clear_exec_state(EXEC_HOLD);
#if ASSERT_PIN(PLASMA_ON_OUTPUT)
				io_set_output(PLASMA_ON_OUTPUT);
#endif
			}
		}
	}
	else
	{
		// disable plasma THC mode
		plasma_thc_state = false;
#if ASSERT_PIN(PLASMA_ON_OUTPUT)
		io_clear_output(PLASMA_ON_OUTPUT);
#endif
		mc_sync_position();
	}
}

static int16_t range_speed(int16_t value)
{
	// binary output
	value = value ? 1 : 0;
	return value;
}

static void set_coolant(uint8_t value)
{
// easy macro
#ifdef ENABLE_COOLANT
	SET_COOLANT(LASER_PWM_AIR_ASSIST, UNDEF_PIN, value);
#endif
}

const tool_t plasma_thc = {
	.startup_code = &startup_code,
	.shutdown_code = &shutdown_code,
	.pid_update = &pid_update,
	.range_speed = &range_speed,
	.get_speed = NULL,
	.set_speed = &set_speed,
	.set_coolant = &set_coolant};

#endif