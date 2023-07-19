/*
	Name: cnc.c
	Description: µCNC main unit.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 17/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cnc.h"

#define LOOP_STARTUP_RESET 0
#define LOOP_RUNNING 1
#define LOOP_FAULT 2
#define LOOP_REQUIRE_RESET 4

#define UNLOCK_OK 0
#define UNLOCK_LOCKED 1
#define UNLOCK_ERROR 2

#define RTCMD_NORMAL_MASK (RT_CMD_FEED_100 | RT_CMD_FEED_INC_COARSE | RT_CMD_FEED_DEC_COARSE | RT_CMD_FEED_INC_FINE | RT_CMD_FEED_DEC_FINE)
#define RTCMD_RAPID_MASK (RT_CMD_RAPIDFEED_100 | RT_CMD_RAPIDFEED_OVR1 | RT_CMD_RAPIDFEED_OVR2)
#define RTCMD_SPINDLE_MASK (RT_CMD_SPINDLE_100 | RT_CMD_SPINDLE_INC_COARSE | RT_CMD_SPINDLE_DEC_COARSE | RT_CMD_SPINDLE_INC_FINE | RT_CMD_SPINDLE_DEC_FINE | RT_CMD_SPINDLE_TOGGLE)
#define RTCMD_COOLANT_MASK (RT_CMD_COOL_FLD_TOGGLE | RT_CMD_COOL_MST_TOGGLE)
typedef struct
{
	// uint8_t system_state;		//signals if CNC is system_state and gcode can run
	volatile uint8_t exec_state;
	uint8_t loop_state;
	volatile uint8_t rt_cmd;
	volatile uint8_t feed_ovr_cmd;
	volatile uint8_t tool_ovr_cmd;
	volatile int8_t alarm;
} cnc_state_t;

static bool lock_itp = false;
static cnc_state_t cnc_state;
bool cnc_status_report_lock;

static void cnc_check_fault_systems(void);
static bool cnc_check_interlocking(void);
static void cnc_exec_rt_commands(void);
static void cnc_io_dotasks(void);
static void cnc_reset(void);
static bool cnc_exec_cmd(void);
static void cnc_run_startup_blocks(void);

#ifdef ENABLE_MAIN_LOOP_MODULES
// event_cnc_reset_handler
WEAK_EVENT_HANDLER(cnc_reset)
{
	DEFAULT_EVENT_HANDLER(cnc_reset);
}

// event_rtc_tick_handler
WEAK_EVENT_HANDLER(rtc_tick)
{
	DEFAULT_EVENT_HANDLER(rtc_tick);
}

// event_cnc_dotasks_handler
WEAK_EVENT_HANDLER(cnc_dotasks)
{
	DEFAULT_EVENT_HANDLER(cnc_dotasks);
}

// event_cnc_dotasks_handler
WEAK_EVENT_HANDLER(cnc_io_dotasks)
{
	DEFAULT_EVENT_HANDLER(cnc_io_dotasks);
}

// event_cnc_stop_handler
WEAK_EVENT_HANDLER(cnc_stop)
{
	DEFAULT_EVENT_HANDLER(cnc_stop);
}

// event_cnc_exec_cmd_error_handler
WEAK_EVENT_HANDLER(cnc_exec_cmd_error)
{
	DEFAULT_EVENT_HANDLER(cnc_exec_cmd_error);
}
#endif

void cnc_init(void)
{
	// initializes cnc state
#ifdef FORCE_GLOBALS_TO_0
	memset(&cnc_state, 0, sizeof(cnc_state_t));
	cnc_status_report_lock = false;
#endif
	cnc_state.loop_state = LOOP_STARTUP_RESET;
	// initializes all systems
	mcu_init();											// mcu
	io_enable_steppers(~g_settings.step_enable_invert); // disables steppers at start
	io_disable_probe();									// forces probe isr disabling
	serial_init();										// serial
	mod_init();											// modules
	settings_init();									// settings
	itp_init();											// interpolator
	planner_init();										// motion planner
#if TOOL_COUNT > 0
	tool_init();
#endif
}

void cnc_run(void)
{
	// enters loop reset
	cnc_reset();

	// tries to reset. If fails jumps to error
	while (cnc_unlock(false) != UNLOCK_ERROR)
	{
		serial_select(SERIAL_UART);
		cnc_state.loop_state = LOOP_RUNNING;

		do
		{
		} while (cnc_exec_cmd());

		cnc_state.loop_state = LOOP_FAULT;
		int8_t alarm = cnc_state.alarm;
		if (alarm > EXEC_ALARM_NOALARM)
		{
			protocol_send_alarm(cnc_state.alarm);
		}
		if (alarm < EXEC_ALARM_PROBE_FAIL_INITIAL && alarm != EXEC_ALARM_NOALARM)
		{
			io_enable_steppers(~g_settings.step_enable_invert);
			cnc_check_fault_systems();
			cnc_state.loop_state = LOOP_REQUIRE_RESET;
			break;
		}
	}

	do
	{
		if (!serial_rx_is_empty())
		{
			if (serial_getc() == EOL)
			{
				protocol_send_feedback(MSG_FEEDBACK_1);
				protocol_send_ok();
			}
		}
		cnc_dotasks();
		// a hard/soft reset is pending
		if (cnc_state.alarm < 0)
		{
			cnc_state.loop_state = LOOP_STARTUP_RESET;
			cnc_clear_exec_state(EXEC_KILL);
		}
	} while (cnc_state.loop_state == LOOP_REQUIRE_RESET || cnc_get_exec_state(EXEC_KILL));
}

bool cnc_exec_cmd(void)
{
#ifdef ENABLE_PARSING_TIME_DEBUG
	uint32_t exec_time;
#endif
	// process gcode commands
	if (!serial_rx_is_empty())
	{
		uint8_t error = 0;
		// protocol_echo();
		uint8_t c = serial_peek();
		switch (c)
		{
		case OVF:
			serial_rx_clear();
			error = STATUS_OVERFLOW;
			break;
		case EOL: // not necessary but faster to catch empty lines and windows newline (CR+LF)
			serial_getc();
			break;
		default:
#ifdef ENABLE_PARSING_TIME_DEBUG
			if (!exec_time)
			{
				exec_time = mcu_millis();
			}
#endif
			error = parser_read_command();
#ifdef ENABLE_PARSING_TIME_DEBUG
			exec_time = mcu_millis() - exec_time;
			protocol_send_string(MSG_START);
			protocol_send_string(__romstr__("exec time "));
			serial_print_int(exec_time);
			protocol_send_string(MSG_END);
#endif
			break;
		}
		if (!error)
		{
			protocol_send_ok();
		}
		else
		{
			parser_sync_position();
			protocol_send_error(error);
#ifdef ENABLE_MAIN_LOOP_MODULES
			EVENT_INVOKE(cnc_exec_cmd_error, &error);
#endif
		}
	}

	return cnc_dotasks();
}

bool cnc_dotasks(void)
{
	// run io basic tasks
	cnc_io_dotasks();

	// let µCNC finnish startup/reset code
	if (cnc_state.loop_state == LOOP_STARTUP_RESET)
	{
		return false;
	}

	cnc_exec_rt_commands(); // executes all pending realtime commands

	if (cnc_has_alarm())
	{
		return !cnc_get_exec_state(EXEC_KILL);
	}

	// µCNC already in error loop. No point in sending the alarms
	if (cnc_state.loop_state >= LOOP_FAULT)
	{
		return !cnc_get_exec_state(EXEC_KILL);
	}

	// check security interlocking for any problem
	if (!cnc_check_interlocking())
	{
		return !cnc_get_exec_state(EXEC_INTERLOCKING_FAIL);
	}

#ifdef ENABLE_TOOL_PID_CONTROLLER
	// run the tool pid update
	tool_pid_update();
#endif

	if (!lock_itp)
	{
		lock_itp = true;
#ifdef ENABLE_MAIN_LOOP_MODULES
		EVENT_INVOKE(cnc_dotasks, NULL);
#endif

		itp_run();
		lock_itp = false;
	}

	return !cnc_get_exec_state(EXEC_KILL);
}

void cnc_store_motion(void)
{
#ifdef ENABLE_MOTION_CONTROL_PLANNER_HIJACKING
	// set hold and wait for motion to stop
	uint8_t prevholdstate = cnc_get_exec_state(EXEC_HOLD);
	cnc_set_exec_state(EXEC_HOLD);
	while (!itp_is_empty() && cnc_get_exec_state(EXEC_RUN))
	{
		if (!cnc_dotasks())
		{
			return;
		}
	}
	// store planner and motion controll data away
	planner_store();
	mc_store();
	// reset planner and sync systems
	itp_clear();
	planner_clear();
	mc_sync_position();
	// clear the current hold state (if not set previosly)
	if (!prevholdstate)
	{
		cnc_clear_exec_state(EXEC_HOLD);
	}

	lock_itp = false;
#endif
}

void cnc_restore_motion(void)
{
#ifdef ENABLE_MOTION_CONTROL_PLANNER_HIJACKING
	// set hold and wait for motion to stop
	uint8_t prevholdstate = cnc_get_exec_state(EXEC_HOLD);
	cnc_set_exec_state(EXEC_HOLD);
	while (!itp_is_empty())
	{
		if (!cnc_dotasks())
		{
			return;
		}
	}

	// reset planner and sync systems
	itp_clear();
	planner_clear();
	mc_sync_position();

	// restore the motion controller, planner and parser
	mc_restore();
	planner_restore();
	parser_sync_position();

	// clear the current hold state
	if (!prevholdstate)
	{
		cnc_clear_exec_state(EXEC_HOLD);
	}
	lock_itp = false;
#endif
}

// this function is executed every millisecond
MCU_CALLBACK void mcu_rtc_cb(uint32_t millis)
{
	static bool running = false;

	if (!running)
	{
		running = true;
		mcu_enable_global_isr();

#ifdef ENABLE_MAIN_LOOP_MODULES
		if (!cnc_get_exec_state(EXEC_ALARM))
		{
			EVENT_INVOKE(rtc_tick, NULL);
		}
#endif

		// checks any limit or control input state change (every 16ms)
#if (!defined(FORCE_SOFT_POLLING) && CTRL_SCHED_CHECK >= 0)
		uint8_t mls = (uint8_t)(0xff & millis);
		if ((mls & CTRL_SCHED_CHECK_MASK) == CTRL_SCHED_CHECK_VAL)
		{
			mcu_limits_changed_cb();
			mcu_controls_changed_cb();
		}
#endif
#if ASSERT_PIN(ACTIVITY_LED)
		// this blinks aprox. once every 1024ms
		if (!(millis & (0x200 - 1)))
		{
			io_toggle_output(ACTIVITY_LED);
		}
#endif
		mcu_disable_global_isr();
		running = false;
	}
}

void cnc_home(void)
{
	cnc_set_exec_state(EXEC_HOMING);
	uint8_t error = kinematics_home();
	// unlock expected limits
	io_lock_limits(0);
	io_invert_limits(0);
	if (error)
	{
		// disables homing and reenables alarm messages
		cnc_clear_exec_state(EXEC_HOMING);
		// cnc_alarm(error);
		return;
	}

	// sync's the motion control with the real time position
	mc_sync_position();
	cnc_run_startup_blocks();
}

void cnc_alarm(int8_t code)
{
	cnc_set_exec_state(EXEC_KILL);
	cnc_stop();
	cnc_state.alarm = code;
#ifdef ENABLE_IO_ALARM_DEBUG
	protocol_send_string(MSG_START);
	protocol_send_string(__romstr__("LIMITS:"));
	serial_print_int(io_alarm_limits);
	protocol_send_string(__romstr__("|CONTROLS:"));
	serial_print_int(io_alarm_controls);
	protocol_send_string(MSG_END);
#endif
}

bool cnc_has_alarm()
{
	return (cnc_get_exec_state(EXEC_KILL) || (cnc_state.alarm != EXEC_ALARM_NOALARM));
}

void cnc_stop(void)
{
	itp_stop();
	// stop tools
	itp_stop_tools();

#ifdef ENABLE_MAIN_LOOP_MODULES
	EVENT_INVOKE(cnc_stop, NULL);
#endif
}

uint8_t cnc_unlock(bool force)
{
	// tries to clear alarms, door or any active hold state
	cnc_clear_exec_state(EXEC_RESET_LOCKED);
	// checks all interlocking again
	cnc_check_interlocking();

	// forces to clear EXEC_UNHOMED error to allow motion after limit switch trigger
	if (force)
	{
		CLEARFLAG(cnc_state.exec_state, EXEC_UNHOMED);
		cnc_state.alarm = EXEC_ALARM_NOALARM;
	}

	// if any alarm state is still active checks system faults
	if (cnc_get_exec_state(EXEC_ALARM) || cnc_has_alarm())
	{
		if (!cnc_get_exec_state(EXEC_KILL))
		{
			protocol_send_feedback(MSG_FEEDBACK_2);
			return UNLOCK_LOCKED;
		}
		else
		{
			return UNLOCK_ERROR;
		}
	}
	else
	{
		// on unlock any alarm caused by not having homing reference or hitting a limit switch is reset at user request
		// this must be done directly because cnc_clear_exec_state will check the limit switch state
		// all other alarm flags remain active if any input is still active
		CLEARFLAG(cnc_state.exec_state, EXEC_UNHOMED);
		// clears all other locking flags
		cnc_clear_exec_state(EXEC_GCODE_LOCKED | EXEC_HOLD);
		// signals stepper enable pins

		io_set_steps(g_settings.step_invert_mask);
		io_enable_steppers(g_settings.step_enable_invert);
		parser_reset(true); // reset stop group only

		// hard reset
		// if homing not enabled run startup blocks
		if (cnc_state.loop_state == LOOP_STARTUP_RESET && !g_settings.homing_enabled)
		{
			cnc_run_startup_blocks();
		}
	}

	return UNLOCK_OK;
}

uint8_t cnc_get_exec_state(uint8_t statemask)
{
	return CHECKFLAG(cnc_state.exec_state, statemask);
}

void cnc_set_exec_state(uint8_t statemask)
{
	SETFLAG(cnc_state.exec_state, statemask);
}

void cnc_clear_exec_state(uint8_t statemask)
{
#ifndef DISABLE_ALL_CONTROLS
	uint8_t controls = io_get_controls();

#if ASSERT_PIN(ESTOP)
	if (CHECKFLAG(controls, ESTOP_MASK)) // can't clear the alarm flag if ESTOP is active
	{
		CLEARFLAG(statemask, EXEC_KILL);
		// no point in continuing
		return;
	}
#endif
#if ASSERT_PIN(SAFETY_DOOR)
	if (CHECKFLAG(controls, SAFETY_DOOR_MASK)) // can't clear the door flag if SAFETY_DOOR is active
	{
		CLEARFLAG(statemask, EXEC_DOOR | EXEC_HOLD);
	}
#endif
#if ASSERT_PIN(FHOLD)
	if (CHECKFLAG(controls, FHOLD_MASK)) // can't clear the hold flag if FHOLD is active
	{
		CLEARFLAG(statemask, EXEC_HOLD);
	}
#endif
#endif

	// has a pending (not cleared by user) alarm
	if (cnc_state.alarm || g_settings.homing_enabled)
	{
		CLEARFLAG(statemask, EXEC_UNHOMED);
	}

	uint8_t limits = 0;
#if (LIMITS_MASK != 0)
	limits = io_get_limits(); // can't clear the EXEC_UNHOMED is any limit is triggered
#endif
	if (g_settings.hard_limits_enabled && limits) // if hardlimits are enabled and limits are triggered
	{
		CLEARFLAG(statemask, EXEC_UNHOMED);
	}

	// if releasing from a HOLD state with and active delay in exec
	if (CHECKFLAG(statemask, EXEC_HOLD) && cnc_get_exec_state(EXEC_HOLD))
	{
		CLEARFLAG(cnc_state.exec_state, EXEC_HOLD);
#if TOOL_COUNT > 0
		// updated the coolant pins
		tool_set_coolant(planner_get_coolant());
#if (DELAY_ON_RESUME_COOLANT > 0)
		if (!g_settings.laser_mode)
		{
			if (!planner_buffer_is_empty())
			{
				cnc_delay_ms(DELAY_ON_RESUME_COOLANT * 1000);
			}
		}
#endif
		// tries to sync the tool
		// if something goes wrong the tool can reinstate the HOLD state
		itp_sync_spindle();
#if (DELAY_ON_RESUME_SPINDLE > 0)
		if (!g_settings.laser_mode)
		{
			if (!planner_buffer_is_empty())
			{
				cnc_delay_ms(DELAY_ON_RESUME_SPINDLE * 1000);
			}
		}
#endif
#endif
	}

	CLEARFLAG(cnc_state.exec_state, statemask);
}

void cnc_delay_ms(uint32_t miliseconds)
{
	uint32_t t_start = mcu_millis();
	while ((mcu_millis() - t_start) < miliseconds)
	{
		cnc_io_dotasks();
	}
}

void cnc_reset(void)
{
	// resets all realtime command flags
	cnc_state.rt_cmd = RT_CMD_CLEAR;
	cnc_state.feed_ovr_cmd = RT_CMD_CLEAR;
	cnc_state.tool_ovr_cmd = RT_CMD_CLEAR;
	cnc_state.exec_state = EXEC_RESET_LOCKED; // Activates all alarms, door and hold
	cnc_state.alarm = EXEC_ALARM_NOALARM;

	// clear all systems
	serial_rx_clear();
	itp_clear();
	planner_clear();
	kinematics_init();
	mc_init();
	parser_init();
	mc_sync_position();
#if ENCODERS > 0
	encoders_reset_position();
#endif
#ifdef ENABLE_MAIN_LOOP_MODULES
	EVENT_INVOKE(cnc_reset, NULL);
#endif
	protocol_send_string(MSG_STARTUP);
}

void cnc_call_rt_command(uint8_t command)
{
	uint8_t tools_cmd;
	// executes the realtime commands
	// only reset command is executed right away
	// control commands affect the exec_state directly (Abort, hold, safety door, cycle_start)
	// the effects are then propagate in the cnc_dotasks
	// uses macro to be faster
	switch (command)
	{
	case CMD_CODE_RESET:
		SETFLAG(cnc_state.rt_cmd, RT_CMD_RESET);
		break;
	case CMD_CODE_FEED_HOLD:
		SETFLAG(cnc_state.exec_state, EXEC_HOLD);
		break;
	case CMD_CODE_REPORT:
		SETFLAG(cnc_state.rt_cmd, RT_CMD_REPORT);
		break;
	case CMD_CODE_CYCLE_START:
		if (!cnc_get_exec_state(EXEC_RUN))
		{
			SETFLAG(cnc_state.rt_cmd, RT_CMD_CYCLE_START); // tries to clear hold if possible
		}
		break;
#if ASSERT_PIN(SAFETY_DOOR)
	case CMD_CODE_SAFETY_DOOR:
		SETFLAG(cnc_state.exec_state, (EXEC_HOLD | EXEC_DOOR));
		break;
#endif
	case CMD_CODE_JOG_CANCEL:
		if (cnc_get_exec_state(EXEC_JOG))
		{
			SETFLAG(cnc_state.exec_state, EXEC_HOLD);
		}
		break;
	default:
		if (command >= CMD_CODE_FEED_100 && command <= CMD_CODE_RAPIDFEED_OVR2)
		{
			cnc_state.feed_ovr_cmd = (1 << (command - CMD_CODE_FEED_100));
			break;
		}

		tools_cmd = cnc_state.tool_ovr_cmd;

		if (command >= CMD_CODE_SPINDLE_100 && command <= CMD_CODE_SPINDLE_TOGGLE)
		{
			tools_cmd &= RTCMD_COOLANT_MASK;
			tools_cmd |= (1 << (command - CMD_CODE_SPINDLE_100));
		}

		if (command >= CMD_CODE_COOL_FLD_TOGGLE && command <= CMD_CODE_COOL_MST_TOGGLE)
		{
			tools_cmd &= RTCMD_SPINDLE_MASK;
			tools_cmd |= (RT_CMD_COOL_FLD_TOGGLE << (command - CMD_CODE_COOL_FLD_TOGGLE));
		}

		cnc_state.tool_ovr_cmd = tools_cmd;
	}
}

// Executes pending realtime commands
// Realtime commands are split in to 3 groups
//   -operation commands
//   -feed override commands
//   -tools override commands
// All active flags will be executed by MSB order (higher first)
// If two flags have different effects on the same attribute the one with the LSB will run last and overwrite the other
void cnc_exec_rt_commands(void)
{
	bool update_tools = false;

	// executes feeds override rt commands
	uint8_t command = cnc_state.rt_cmd; // copies realtime flags states
	if (command)
	{
		// clear all but report. report is handled in cnc_io_dotasks
		cnc_state.rt_cmd &= RT_CMD_REPORT;
		if (command & RT_CMD_RESET)
		{
			if (cnc_get_exec_state(EXEC_HOMING))
			{
				cnc_alarm(EXEC_ALARM_HOMING_FAIL_RESET);
				return;
			}

			if (cnc_get_exec_state(EXEC_RUN))
			{
				cnc_alarm(EXEC_ALARM_ABORT_CYCLE);
				return;
			}

			cnc_alarm(EXEC_ALARM_SOFTRESET);
			return;
		}

		if (command & RT_CMD_CYCLE_START)
		{
			cnc_clear_exec_state(EXEC_HOLD | EXEC_DOOR);
		}
	}

	// executes feeds override rt commands
	command = cnc_state.feed_ovr_cmd; // copies realtime flags states
	if (command)
	{
		cnc_state.feed_ovr_cmd = RT_CMD_CLEAR; // clears command flags
		switch (command & RTCMD_NORMAL_MASK)
		{
		case RT_CMD_FEED_100:
			planner_feed_ovr_reset();
			break;
		case RT_CMD_FEED_INC_COARSE:
			planner_feed_ovr_inc(FEED_OVR_COARSE);
			break;
		case RT_CMD_FEED_DEC_COARSE:
			planner_feed_ovr_inc(-FEED_OVR_COARSE);
			break;
		case RT_CMD_FEED_INC_FINE:
			planner_feed_ovr_inc(FEED_OVR_FINE);
			break;
		case RT_CMD_FEED_DEC_FINE:
			planner_feed_ovr_inc(-FEED_OVR_FINE);
			break;
		}

		switch (command & RTCMD_RAPID_MASK)
		{
		case RT_CMD_RAPIDFEED_100:
			planner_rapid_feed_ovr_reset();
			break;
		case RT_CMD_RAPIDFEED_OVR1:
			planner_rapid_feed_ovr(RAPID_FEED_OVR1);
			break;
		case RT_CMD_RAPIDFEED_OVR2:
			planner_rapid_feed_ovr(RAPID_FEED_OVR2);
			break;
		}
	}

	// executes tools override rt commands
	command = cnc_state.tool_ovr_cmd; // copies realtime flags states
	if (command)
	{
		cnc_state.tool_ovr_cmd = RT_CMD_CLEAR; // clears command flags
		update_tools = true;
		switch (command & RTCMD_SPINDLE_MASK)
		{
#if TOOL_COUNT > 0
		case RT_CMD_SPINDLE_100:
			planner_spindle_ovr_reset();
			break;
		case RT_CMD_SPINDLE_INC_COARSE:
			planner_spindle_ovr_inc(SPINDLE_OVR_COARSE);
			break;
		case RT_CMD_SPINDLE_DEC_COARSE:
			planner_spindle_ovr_inc(-SPINDLE_OVR_COARSE);
			break;
		case RT_CMD_SPINDLE_INC_FINE:
			planner_spindle_ovr_inc(SPINDLE_OVR_FINE);
			break;
		case RT_CMD_SPINDLE_DEC_FINE:
			planner_spindle_ovr_inc(-SPINDLE_OVR_FINE);
			break;
		case RT_CMD_SPINDLE_TOGGLE:
			if (cnc_get_exec_state(EXEC_HOLD | EXEC_DOOR | EXEC_RUN) == EXEC_HOLD) // only available if a TRUE hold is active
			{
				// toogle state
				if (tool_get_speed())
				{
					update_tools = false;
					tool_set_speed(0);
				}
			}
			break;
#endif
		}

		switch (command & RTCMD_COOLANT_MASK)
		{
#if TOOL_COUNT > 0
		case RT_CMD_COOL_FLD_TOGGLE:
#ifdef COOLANT_MIST
		case RT_CMD_COOL_MST_TOGGLE:
#endif
			if (!cnc_get_exec_state(EXEC_ALARM)) // if no alarm is active
			{
				if (command == RT_CMD_COOL_FLD_TOGGLE)
				{
					planner_coolant_ovr_toggle(COOLANT_MASK);
				}
#ifdef COOLANT_MIST
				if (command == RT_CMD_COOL_MST_TOGGLE)
				{
					planner_coolant_ovr_toggle(MIST_MASK);
				}
#endif
			}
			break;
#endif
		}

		if (update_tools)
		{
			itp_update();
			if (planner_buffer_is_empty())
			{
				motion_data_t block = {0};
#if TOOL_COUNT > 0
				block.motion_flags.bit.coolant = planner_get_previous_coolant();
				block.spindle = planner_get_previous_spindle_speed();
#endif
				mc_update_tools(&block);
			}
		}
	}
}

void cnc_check_fault_systems(void)
{
	uint8_t inputs;
#ifdef CONTROLS_MASK
	inputs = io_get_controls();
#endif
#if ASSERT_PIN(ESTOP)
	if (CHECKFLAG(inputs, ESTOP_MASK)) // fault on emergency stop
	{
		protocol_send_feedback(MSG_FEEDBACK_12);
	}
#endif
#if ASSERT_PIN(SAFETY_DOOR)
	if (CHECKFLAG(inputs, SAFETY_DOOR_MASK)) // fault on safety door
	{
		protocol_send_feedback(MSG_FEEDBACK_6);
	}
#endif
#if (LIMITS_MASK != 0)
	if (g_settings.hard_limits_enabled) // fault on limits
	{
		inputs = io_get_limits();
		if (CHECKFLAG(inputs, LIMITS_MASK))
		{
			protocol_send_feedback(MSG_FEEDBACK_7);
		}
	}
#endif

	if (cnc_get_exec_state(EXEC_KILL))
	{
		switch (cnc_state.alarm)
		{
		case EXEC_ALARM_SOFTRESET:
		case EXEC_ALARM_NOALARM:
			break;
		default:
			protocol_send_feedback(MSG_FEEDBACK_1);
			break;
		}
	}
}

bool cnc_check_interlocking(void)
{
	// check all flags

	// an existing KILL condition can be due to:
	// - ESTOP trigger
	// - soft reset command
	// - any cnc_alarm call
	if (cnc_get_exec_state(EXEC_KILL))
	{
#if ASSERT_PIN(ESTOP)
		// the emergency stop is pressed.
		if (io_get_controls() & ESTOP_MASK)
		{
			cnc_alarm(EXEC_ALARM_EMERGENCY_STOP);
			return false;
		}
#endif
		// something caused the motion to stop abruptly
		if (cnc_get_exec_state(EXEC_UNHOMED))
		{
			cnc_alarm(EXEC_ALARM_ABORT_CYCLE);
		}
		return false;
	}

	// an HALT condition or a limit switch was triggered
	// this can be due to any abrupt stop while in motion
	if (cnc_get_exec_state(EXEC_LIMITS))
	{
		if (!cnc_get_exec_state(EXEC_HOMING)) // if a motion is being performed allow trigger the limit switch alarm
		{
			if (cnc_get_exec_state(EXEC_UNHOMED))
			{
				cnc_alarm(EXEC_ALARM_HARD_LIMIT);
			}
			else
			{
				cnc_alarm(EXEC_ALARM_HARD_LIMIT_NOMOTION);
			}
		}

		return false;
	}

	// an hold condition is active and motion as stopped
	if (cnc_get_exec_state(EXEC_HOLD) && !cnc_get_exec_state(EXEC_RUN))
	{
		itp_stop(); // stop motion

		if (cnc_get_exec_state(EXEC_HOMING | EXEC_JOG)) // flushes the buffers if motions was homing or jog
		{
			itp_clear();
			planner_clear();
			mc_sync_position();
			cnc_clear_exec_state(EXEC_HOMING | EXEC_JOG | EXEC_HOLD);
		}

		return false;
	}

#if ASSERT_PIN(SAFETY_DOOR)
	// the safety door condition is active
	if (cnc_get_exec_state(EXEC_DOOR))
	{
		// door opened during a homing cycle exit with alarm
		if (cnc_get_exec_state(EXEC_HOMING))
		{
			cnc_alarm(EXEC_ALARM_HOMING_FAIL_DOOR);
		}
		else if (cnc_get_exec_state(EXEC_RUN)) // if the machined is running
		{
			// with the door opened put machine on HOLD
			cnc_set_exec_state(EXEC_HOLD);
		}
		else // if the machined is not moving stop the tool too
		{
			cnc_stop();
		}

		return false;
	}
#endif

	return true;
}

static void cnc_io_dotasks(void)
{
#if STATUS_AUTOMATIC_REPORT_INTERVAL >= 100
	static uint32_t next_auto_report = STATUS_AUTOMATIC_REPORT_INTERVAL;
#endif

	// run internal mcu tasks (USB and communications)
	mcu_dotasks();

	// checks inputs and triggers ISR checks if enforced soft polling
#if defined(FORCE_SOFT_POLLING)
	mcu_limits_changed_cb();
	mcu_controls_changed_cb();
#endif
#if (DIN_ONCHANGE_MASK != 0 && ENCODERS < 1)
	// extra call in case generic inputs are running with ISR disabled. Encoders need propper ISR to work.
	mcu_inputs_changed_cb();
#endif

	// if (cnc_status_report_lock)
	// {
	// 	return;
	// }

#if STATUS_AUTOMATIC_REPORT_INTERVAL >= 100
	uint32_t current_time = mcu_millis();
	if (next_auto_report < current_time)
	{
		next_auto_report = current_time + STATUS_AUTOMATIC_REPORT_INTERVAL;
		SETFLAG(cnc_state.rt_cmd, RT_CMD_REPORT);
	}
#endif

	if (CHECKFLAG(cnc_state.rt_cmd, RT_CMD_REPORT))
	{
		// if a report request is sent, clear the respective flag
		CLEARFLAG(cnc_state.rt_cmd, RT_CMD_REPORT);
		protocol_send_status();
	}

#ifdef ENABLE_MAIN_LOOP_MODULES
	EVENT_INVOKE(cnc_io_dotasks, NULL);
#endif
}

void cnc_run_startup_blocks(void)
{
	serial_select(SERIAL_N0);
	cnc_exec_cmd();
	serial_select(SERIAL_N1);
	cnc_exec_cmd();
	serial_select(SERIAL_UART);
}
