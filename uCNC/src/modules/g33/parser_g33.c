/*
	Name: parser_g33.c
	Description: Implements a parser extension for LinuxCNC G33 for ÂµCNC.

	Copyright: Copyright (c) JoÃ£o Martins
	Author: JoÃ£o Martins
	Date: 25/11/2022

	ÂµCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	ÂµCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "../encoder.h"
#include <stdint.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>

#ifdef ENABLE_PARSER_MODULES

#if (UCNC_MODULE_VERSION < 11501 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of ÂµCNC"
#endif

#ifndef AXIS_DIR_VECTORS
#ifdef ABC_INDEP_FEED_CALC
#define AXIS_DIR_VECTORS MIN(AXIS_COUNT, 3)
#else
#define AXIS_DIR_VECTORS AXIS_COUNT
#endif
#endif

#ifndef G33_ENCODER
#error "G33 requires to have an assigned encoder"
#endif

#ifndef G33_SYNCHRONIZATION_SPEED
#define G33_SYNCHRONIZATION_SPEED 0.5f
#endif
// enable this to use the encoder pulse as the feedback loop marker/trigger
//  #define G33_FEEDBACK_LOOP_USE_ENC_PULSE

// uncomment to allow data verbose of sync constants
// the message output is
// [MSG:<spindle index counter>:<expected_step_position>:<current_step_position>:<error>:<encoder_rpm>]
 #define G33_DEBUG

#define SYNC_DISABLED 0
#define SYNC_READY 1
#define SYNC_STARTING 2
#define SYNC_RUNNING 4
#define SYNC_UPDATED 8

static volatile int32_t itp_sync_step_counter;		// step distance counter for synched motions
static volatile uint8_t synched_motion_status;		// synched motion status/phase
static volatile int32_t spindle_index_counter;		// spindle index pulse counter
static int32_t spindle_index_counter_start;			// spindle index pulse initial offset when motion starts
static volatile int32_t spindle_index_step_counter; // step distance counter when the spindle index pulses
static volatile int32_t spindle_index_time;			// index pulse timestamp in us
static volatile int32_t spindle_index_last_time;	// index pulse previous timestamp in us
static uint32_t steps_per_index;					// motion steps per index pulse
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
static uint32_t update_loop_index_counter; // keeps the last update loop index counter
#endif
static uint32_t motion_total_steps;
static float motion_total_distance;
static int32_t current_error;
static float rpm_to_stepfeed_constant;
static uint32_t enc_res;

// #if (MCU == MCU_VIRTUAL_WIN)
// // used with the virtual emulator to simulate pulses
// void mcu_stimul_inputs(volatile VIRTUAL_MAP *virtualmap, uint64_t micros)
// {
// 	static uint64_t last_stim = 0, last_stimsync = 0;
// 	uint64_t next_stim = last_stim + 120000;			 // 120RPM
// 	uint64_t next_stimsync = last_stimsync + 120000 / 4; // 120RPM

// 	if (micros >= next_stimsync)
// 	{
// 		last_stimsync = next_stimsync;
// 		virtualmap->inputs ^= 1; // index pin
// 		mcu_inputs_changed_cb();
// 	}

// 	if (micros >= next_stim)
// 	{
// 		last_stim = next_stim;
// 		virtualmap->inputs ^= 2; // index pin
// 		mcu_inputs_changed_cb();
// 	}
// }
// #endif

void itp_rt_stepcount_cb_handler(uint8_t stepbits, uint8_t itp_flags)
{
	if (itp_flags & ITP_SYNC)
	{
		if (itp_flags & ITP_CONST)
		{
			synched_motion_status |= SYNC_RUNNING;
		}
		else
		{
			synched_motion_status &= ~SYNC_RUNNING;
		}
		itp_sync_step_counter++;
	}
}

#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
#define _g33_enc_pulse_(X) enc##X##_pulse(void)
#define _g33_enc_pulse(X) _g33_enc_pulse_(X)
#define g33_enc_pulse(X) _g33_enc_pulse(X)

void g33_enc_pulse(G33_ENCODER)
{
	if (synched_motion_status >= SYNC_RUNNING)
	{
		spindle_index_step_counter = itp_sync_step_counter;
		synched_motion_status |= SYNC_UPDATED;
	}
}
#endif

void spindle_index_cb_handler(void)
{
	// this measures the amount of time it took to do X full turns of the tool
	// allow to measure RPM (using the index pin instead of the encoder)
	uint32_t now = mcu_micros();
	int32_t index = spindle_index_counter;
	spindle_index_last_time = spindle_index_time;
	spindle_index_time = now;
	index++;

	switch (synched_motion_status)
	{
	case SYNC_READY:
		// the spindle index starts synchronized motion
		itp_start(false);
		synched_motion_status = SYNC_STARTING;
		index = spindle_index_counter_start;
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
		encoder_reset_position(G33_ENCODER, index * enc_res); // syncs the pulse counter with the index counter
#endif
		break;
	default:
#ifndef G33_FEEDBACK_LOOP_USE_ENC_PULSE
		if (index > 0 && synched_motion_status >= SYNC_RUNNING)
		{
			synched_motion_status |= SYNC_UPDATED;
			// store the step position at the time the index pulse happens
			spindle_index_step_counter = itp_sync_step_counter;
		}
#endif
		break;
	}

	spindle_index_counter = index;
}

#ifdef G33_INDEX_PIN
CREATE_EVENT_LISTENER(input_change, spindle_index_cb_handler);
#endif

// this ID must be unique for each code
#define G33 33

bool g33_parse(void *args);
bool g33_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, g33_parse);
CREATE_EVENT_LISTENER(gcode_exec, g33_exec);

// this just parses and accepts the code
bool g33_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'G' && ptr->code == 33)
	{
		// stops event propagation
		if (ptr->cmd->group_extended != 0 || CHECKFLAG(ptr->cmd->groups, GCODE_GROUP_MOTION))
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// checks if it's G5 or G5.1
		// check mantissa
		uint8_t mantissa = (uint8_t)lroundf(((ptr->value - ptr->code) * 100.0f));

		if (mantissa != 0)
		{
			*(ptr->error) = STATUS_GCODE_UNSUPPORTED_COMMAND;
			return EVENT_HANDLED;
		}

		ptr->new_state->groups.motion = G33;
		ptr->new_state->groups.motion_mantissa = 0;
		SETFLAG(ptr->cmd->groups, GCODE_GROUP_MOTION);
		ptr->cmd->group_extended = EXTENDED_MOTION_GCODE(33);
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool g33_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	if (ptr->cmd->group_extended == EXTENDED_MOTION_GCODE(33))
	{
		if (!CHECKFLAG(ptr->cmd->words, GCODE_XYZ_AXIS))
		{
			// it's an error no axis word is specified
			*(ptr->error) = STATUS_GCODE_NO_AXIS_WORDS;
			return EVENT_HANDLED;
		}

		if (!CHECKFLAG(ptr->cmd->words, GCODE_WORD_K))
		{
			// it's an error no distance per rev word is specified
			*(ptr->error) = STATUS_GCODE_VALUE_WORD_MISSING;
			return EVENT_HANDLED;
		}

		// syncs motions and sets spindle
		if (mc_update_tools(ptr->block_data) != STATUS_OK)
		{
			*(ptr->error) = STATUS_CRITICAL_FAIL;
			return EVENT_HANDLED;
		}

		enc_res = ((uint32_t)g_settings.encoders_resolution[G33_ENCODER]);

		// attach the index event callback
#if (G33_ENCODER == ENC0)
		HOOK_ATTACH_CALLBACK(enc0_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC1)
		HOOK_ATTACH_CALLBACK(enc1_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC2)
		HOOK_ATTACH_CALLBACK(enc2_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC3)
		HOOK_ATTACH_CALLBACK(enc3_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC4)
		HOOK_ATTACH_CALLBACK(enc4_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC5)
		HOOK_ATTACH_CALLBACK(enc5_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC6)
		HOOK_ATTACH_CALLBACK(enc6_index, spindle_index_cb_handler);
#elif (G33_ENCODER == ENC7)
		HOOK_ATTACH_CALLBACK(enc7_index, spindle_index_cb_handler);
#endif

		// this code can be removed as the initial reading of the G33 RPM ensures the spindle is running

		// if (!ptr->block_data->motion_flags.bit.spindle_running)
		// {
		// 	*(ptr->error) = STATUS_SPINDLE_RPM_ERROR;
		// 	return EVENT_HANDLED;
		// }

		// // update tool
		// mc_update_tools(ptr->block_data);

#ifdef TOOL_WAIT_FOR_SPEED
		// wait for spindle to reach the desired speed
		uint16_t programmed_speed = ptr->block_data->spindle;
		uint16_t at_speed_threshold = lroundf(TOOL_WAIT_FOR_SPEED_MAX_ERROR * 0.01f * programmed_speed);

		// wait for tool at speed
		uint32_t start_spindle_time = mcu_millis();
		while (ABS(programmed_speed - encoder_get_rpm(G33_ENCODER)) > at_speed_threshold)
		{
			if (!cnc_dotasks() || (mcu_millis() - start_spindle_time) > (DELAY_ON_RESUME_SPINDLE * 1000))
			{
				*(ptr->error) = STATUS_SPINDLE_RPM_ERROR;
				return EVENT_HANDLED;
			}
		}
#endif
		uint32_t t = 0, delta_t = 0;

		for (;;)
		{
			ATOMIC_CODEBLOCK
			{
				delta_t = spindle_index_time;
				t = spindle_index_last_time;
			}
			if (t)
			{
				break;
			}
			cnc_dotasks();
		}

		delta_t -= t;
		float index_rpm = 1000000.0f / ((float)delta_t * MIN_SEC_MULT);

		// spindle speed ins not valid
		if (index_rpm < 1)
		{
			*(ptr->error) = STATUS_SPINDLE_RPM_ERROR;
			return EVENT_HANDLED;
		}

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
		motion_total_distance = line_dist;
		float inv_dist = fast_flt_inv(line_dist);

		// determines the normalized direction vector
		// and the maximum acceleration
		float max_feed = FLT_MAX;
		float max_accel = FLT_MAX;

		for (uint8_t i = 0; i < AXIS_DIR_VECTORS; i++)
		{
			float normal_vect = dir_vect[i] * inv_dist;
			dir_vect[i] = normal_vect;
			normal_vect = ABS(normal_vect);
			// denormalize max feed rate for each axis
			float denorm_param = fast_flt_div(g_settings.max_feed_rate[i], normal_vect);
			max_feed = MIN(max_feed, denorm_param);
			max_feed = MIN(max_feed, F_STEP_MAX);
			denorm_param = fast_flt_div(g_settings.acceleration[i], normal_vect);
			max_accel = MIN(max_accel, denorm_param);
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

		// from this the factor to convert from RPM to step feed can be obtained
		// step rate = rpm_to_stepfeed_constant * RPM
		rpm_to_stepfeed_constant = ptr->words->ijk[2] * total_steps * MIN_SEC_MULT / line_dist;

		// calculates the feedrate based in the K factor and the programmed spindle RPM
		// spindle is in Rev/min and K is in units(mm) per Rev Rev/min * mm/Rev = mm/min
		float total_revs = line_dist / ptr->words->ijk[2];
		float feed = ptr->words->ijk[2] * index_rpm;
		if (feed > max_feed)
		{
			*(ptr->error) = STATUS_MAX_STEP_RATE_EXCEEDED;
			return EVENT_HANDLED;
		}

		// calculates the expected number of steps per revolution
		float steps_per_rev = (float)total_steps / total_revs;
		steps_per_index = lroundf(steps_per_rev);

		ptr->block_data->feed = feed;
		ptr->block_data->motion_flags.bit.synched = 1;
		ptr->block_data->max_accel = max_accel;

		// convert feed to mm/s
		feed *= MIN_SEC_MULT;

		// The thread feed is given by:
		// vf = (RPM / 60) * K
		// and the thread position at any given time t(s) is expressed as
		// x = vf * t
		// on a linear acceleration the motion will ALWAYS stay behind because of the acceleration phase
		// the real thread path is given by:
		// xreal = (vf^2) / (2 * a) + vf * (t - tacc)
		// we can also express this as the ideal position version minus the acceleration triangle area (valid for constant acceleration)
		// xreal = vf * t - (vf^2) / (2 * a)
		// the error is expressed as
		// e = x - xreal = (vf^2) / (2 * a)
		// the thread correct position is a multiple of pitch K
		// the motion will always lag behind a bit. The acceleration can be tuned so that the lag is exctly P multiples of pitch K
		// e = P * K
		// replacing the value of error
		// (vf^2) / (2 * a) = P * K
		// solving this for acceleration we get
		// a = (vf^2) / (2 * P * K)
		// replacing vf from the first equation we get
		// a = (K * RPM^2) / (7200 * P)

		// calculate the minimum acceleration time
		float accel_time = feed / max_accel;
		// calculate the time per revolution
		float rev_time = 60 / index_rpm;

		// calculate the minimum amount of revs needed to reach the target speed
		float p_revs = ceilf(accel_time / rev_time);
		// calculate the new acceleration given an additional revolution to compensate for the lag
		float new_accel = ptr->words->ijk[2] * index_rpm * index_rpm / (p_revs * 7200);
		ptr->block_data->max_accel = new_accel;

		spindle_index_counter_start = -(uint32_t)(p_revs);

		// resets indexes
		spindle_index_counter = 0;
		itp_sync_step_counter = 0;

// resets the correction loop
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
		update_loop_index_counter = 0;
#endif

		if (mc_line(ptr->target, ptr->block_data) != STATUS_OK)
		{
			*(ptr->error) = STATUS_CRITICAL_FAIL;
			return EVENT_HANDLED;
		}

		// attach the stepcounter callback
		HOOK_ATTACH_CALLBACK(itp_rt_stepbits, itp_rt_stepcount_cb_handler);

		// flag the spindle index callback that it can start the threading motion
		synched_motion_status = SYNC_READY;

		// wait for the motion to end
		if (itp_sync() != STATUS_OK)
		{
			*(ptr->error) = STATUS_CRITICAL_FAIL;
			return EVENT_HANDLED;
		}

		synched_motion_status = SYNC_DISABLED;

// encoder_dettach_index_cb();
#if (G33_ENCODER == ENC0)
		HOOK_RELEASE(enc0_index);
#elif (G33_ENCODER == ENC1)
		HOOK_RELEASE(enc1_index);
#elif (G33_ENCODER == ENC2)
		HOOK_RELEASE(enc2_index);
#elif (G33_ENCODER == ENC3)
		HOOK_RELEASE(enc3_index);
#elif (G33_ENCODER == ENC4)
		HOOK_RELEASE(enc4_index);
#elif (G33_ENCODER == ENC5)
		HOOK_RELEASE(enc5_index);
#elif (G33_ENCODER == ENC6)
		HOOK_RELEASE(enc6_index);
#elif (G33_ENCODER == ENC7)
		HOOK_RELEASE(enc7_index);
#endif
		HOOK_RELEASE(itp_rt_stepbits);

		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
bool g33_proto_status(void *args)
{
	if ((g_settings.status_report_mask & 4))
	{
		if ((synched_motion_status > SYNC_DISABLED))
		{
			float error = motion_total_distance * current_error;
			error /= (float)motion_total_steps;
			proto_printf("|Se:%f", error);
		}
	}

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(proto_status, g33_proto_status);

bool spindle_sync_update_loop(void *ptr)
{
	if ((synched_motion_status & SYNC_UPDATED))
	{

		int32_t error, index_step_counter, index_counter;
		uint32_t t = 0, delta_t = 0;
		// gets a snapshot of the current spindle index position, and the step position at the time of the index pulse
		ATOMIC_CODEBLOCK
		{
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
			index_counter = encoder_get_position(G33_ENCODER);
#else
			index_counter = spindle_index_counter;
#endif
			synched_motion_status &= ~SYNC_UPDATED;
			index_step_counter = spindle_index_step_counter;
			delta_t = spindle_index_time;
			t = spindle_index_last_time;
		}

#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
		delta_t = encoder_get_delta(G33_ENCODER) * g_settings.encoders_resolution[G33_ENCODER];
#else
		delta_t -= t;
#endif
		float index_rpm = 1000000.0f / ((float)delta_t * MIN_SEC_MULT);
		if (index_rpm < 1)
		{
			cnc_alarm(EXEC_ALARM_SPINDLE_SYNC_FAIL);
			return STATUS_CRITICAL_FAIL;
		}

		// calculate the spindle position
		int32_t expected_position = index_counter * steps_per_index;
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
		expected_position /= g_settings.encoders_resolution[G33_ENCODER];
#endif

		// if negative the axis are ahead of spindle and need to slow down
		// if positive the axis are behind the spindle and need to speed up.
		error = expected_position - index_step_counter;
		current_error = error;

		// #ifdef G33_DEBUG
		//  cnc_call_rt_command(CMD_CODE_REPORT);
		// #endif

		if (error)
		{
			float new_step_rate = rpm_to_stepfeed_constant * index_rpm;
#ifdef G33_FEEDBACK_LOOP_USE_ENC_PULSE
			new_step_rate += error * G33_SYNCHRONIZATION_SPEED * g_settings.encoders_resolution[G33_ENCODER];
#else
			new_step_rate += error * G33_SYNCHRONIZATION_SPEED;
#endif
			// this updates the interpolator right on the next step and the current motion in the planner
			itp_update_feed(new_step_rate);
		}

#ifdef G33_DEBUG
		proto_info("MSG:Spindle turns %ld, expected pos %ld, real pos %ld, error: %ld, rpm %f", index_counter, expected_position, index_step_counter, error, index_rpm);
#endif
	}
	else if (synched_motion_status)
	{
#ifdef G33_DEBUG
		static uint32_t prev_print = 0;
		uint32_t elapsed = mcu_millis() - prev_print;
		if (elapsed > 1000)
		{
			uint32_t t = 0, delta_t = 0;
			ATOMIC_CODEBLOCK
			{
				delta_t = spindle_index_time;
				t = spindle_index_last_time;
			}
			delta_t -= t;
			if (delta_t)
			{
				float index_rpm = 1000000.0f / ((float)delta_t * MIN_SEC_MULT);
				proto_info("MSG:G33 TOOL RPM %f", index_rpm);
			}
			prev_print = mcu_millis();
		}
#endif
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, spindle_sync_update_loop);
#endif

#if (MCU == MCU_VIRTUAL_WIN)
#define INDEX_MULTIPLIER 8

void emulate_tool_encoder(volatile VIRTUAL_MAP *virtualmap, uint64_t micros)
{
    static uint64_t last_pulse = 0;
    static uint64_t last_index = 0;

    uint32_t rpm = tool_get_setpoint();
    if (synched_motion_status >= SYNC_RUNNING)
    {
    	// emulate a 20% rpm drop
    	rpm = (uint32_t)(0.8f * rpm);
	}
    uint32_t maxrpm = g_settings.spindle_max_rpm;

    if (rpm == 0 || maxrpm == 0)
        return; // spindle parado → sem pulsos

    // clamp para evitar valores absurdos
    if (rpm > maxrpm)
        rpm = maxrpm;

    /*
        Cálculo físico real:
        período de 1 rotação = 60.000.000us / RPM
        pulse_interval = período
        index_interval = período * INDEX_MULTIPLIER
    */
    uint64_t period_us = 30000000ULL / rpm;

    uint64_t index_interval = period_us;
    uint64_t pulse_interval = period_us / g_settings.encoders_resolution[G33_ENCODER];

    uint64_t next_pulse = last_pulse + pulse_interval;
    uint64_t next_index = last_index + index_interval;

    // index pulse
    if (micros >= next_index)
    {
        last_index = next_index;
        virtualmap->inputs ^= 2;   // index pin
        mcu_inputs_changed_cb();
    }

    // main pulse
    if (micros >= next_pulse)
    {
        last_pulse = next_pulse;
        virtualmap->inputs ^= 1;   // pulse pin
        mcu_inputs_changed_cb();
    }
}
#endif

DECL_MODULE(g33)
{
#if (MCU == MCU_VIRTUAL_WIN)
	mcu_stimul_inputs = emulate_tool_encoder;
#endif
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, g33_parse);
	ADD_EVENT_LISTENER(gcode_exec, g33_exec);
#else
#error "Parser extensions are not enabled. G33 code extension will not work."
#endif
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(proto_status, g33_proto_status);
	ADD_EVENT_LISTENER(cnc_dotasks, spindle_sync_update_loop);
#else
#error "Main loop extensions are not enabled. G33 code extension will not work."
#endif
#ifndef ENABLE_RT_SYNC_MOTIONS
#error "ENABLE_RT_SYNC_MOTIONS must be enabled to allow realtime step counting in sync motions."
#endif
}
