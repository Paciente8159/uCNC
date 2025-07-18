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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cnc.h"

#define LOOP_STARTUP_RESET 0
#define LOOP_UNLOCK 1
#define LOOP_RUNNING 2
#define LOOP_FAULT 3
#define LOOP_REQUIRE_RESET 4

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

static uint8_t cnc_lock_itp;
static cnc_state_t cnc_state;
bool cnc_status_report_lock;

static void cnc_check_fault_systems(void);
static bool cnc_check_interlocking(void);
static void cnc_exec_rt_commands(void);
static void cnc_io_dotasks(void);
static void cnc_reset(void);
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

// event_cnc_parse_cmd_error_handler
WEAK_EVENT_HANDLER(cnc_parse_cmd_error)
{
	DEFAULT_EVENT_HANDLER(cnc_parse_cmd_error);
}

// event_cnc_alarm
WEAK_EVENT_HANDLER(cnc_alarm)
{
	DEFAULT_EVENT_HANDLER(cnc_alarm);
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
	mcu_init();																					// mcu
	mcu_io_reset();																			// add custom logic to set pins initial state
	io_enable_steppers(~g_settings.step_enable_invert); // disables steppers at start
	io_disable_probe();																	// forces probe isr disabling
	grbl_stream_init();																	// serial
	mod_init();																					// modules
	settings_init();																		// settings
	itp_init();																					// interpolator
	planner_init();																			// motion planner
#if TOOL_COUNT > 0
	tool_init();
#endif
}

void cnc_run(void)
{
	// enters loop reset
	cnc_reset();

	cnc_state.loop_state = LOOP_UNLOCK;

	// tries to reset. If fails jumps to error
	while (cnc_unlock(false) != UNLOCK_ERROR)
	{
		cnc_state.loop_state = LOOP_RUNNING;
		do
		{
			cnc_parse_cmd();
		} while (cnc_dotasks());

		cnc_state.loop_state = LOOP_FAULT;
		int8_t alarm = cnc_state.alarm;
		if (alarm > EXEC_ALARM_NOALARM)
		{
			proto_alarm(cnc_state.alarm);
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
		if (grbl_stream_available())
		{
			if (grbl_stream_getc() == EOL)
			{
				proto_feedback(MSG_FEEDBACK_1);
				proto_error(0);
			}
		}
		cnc_dotasks();
		// a soft reset is pending
		if (cnc_state.alarm == EXEC_ALARM_SOFTRESET)
		{
			break;
		}
	} while (cnc_state.loop_state == LOOP_REQUIRE_RESET || cnc_get_exec_state(EXEC_KILL));
}

uint8_t cnc_parse_cmd(void)
{
#ifdef ENABLE_PARSING_TIME_DEBUG
	uint32_t exec_time;
#endif
	uint8_t error = STATUS_OK;
	// process gcode commands
	if (grbl_stream_available())
	{
		// protocol_echo();
		uint8_t c = grbl_stream_peek();
		switch (c)
		{
		case OVF:
			grbl_stream_overflow_flush();
			error = STATUS_OVERFLOW;
			break;
		case EOL: // not necessary but faster to catch empty lines and windows newline (CR+LF)
			grbl_stream_getc();
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
			proto_info("Exec time: %lu", exec_time);
#endif
			break;
		}
		// runs any rt command in queue
		// this catches for example a ?\n situation sent by some GUI like UGS
		cnc_exec_rt_commands();
		proto_error(error);
		if (error)
		{
			itp_sync();
			mc_sync_position();
			parser_sync_position();
#ifdef ENABLE_MAIN_LOOP_MODULES
			EVENT_INVOKE(cnc_parse_cmd_error, &error);
#endif
		}
	}

	return error;
}

bool cnc_dotasks(void)
{

	// run io basic tasks
	cnc_io_dotasks();

	cnc_exec_rt_commands(); // executes all pending realtime commands

	// let µCNC finnish startup/reset code
	if (cnc_state.loop_state == LOOP_STARTUP_RESET)
	{
		return false;
	}

	// µCNC already in error loop. No point in sending the alarms
	if (cnc_has_alarm() || (cnc_state.loop_state >= LOOP_FAULT))
	{
		return !cnc_get_exec_state(EXEC_KILL);
	}

	// check security interlocking for any problem
	if (!cnc_check_interlocking())
	{
		return !cnc_get_exec_state(EXEC_INTERLOCKING_FAIL);
	}

#ifndef ENABLE_ITP_FEED_TASK
	if (!cnc_lock_itp)
	{
		cnc_lock_itp = true;
		itp_run();
		cnc_lock_itp = false;
	}
#endif

#ifdef ENABLE_TOOL_PID_CONTROLLER
	// run the tool pid update
	tool_pid_update();
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
	EVENT_INVOKE(cnc_dotasks, NULL);
#endif

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

	cnc_lock_itp = 0;
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
	cnc_lock_itp = 0;
#endif
}

// this function is executed every millisecond
#ifndef DISABLE_RTC_CODE
MCU_CALLBACK void mcu_rtc_cb(uint32_t millis)
{
	mcu_enable_global_isr();
	uint8_t mls = (uint8_t)(0xff & millis);
	if ((mls & CTRL_SCHED_CHECK_MASK) == CTRL_SCHED_CHECK_VAL)
	{
#ifndef ENABLE_RT_PROBE_CHECKING
		mcu_probe_changed_cb();
#endif
#ifndef ENABLE_RT_LIMITS_CHECKING
		mcu_limits_changed_cb();
#endif
		mcu_controls_changed_cb();
#if (DIN_ONCHANGE_MASK != 0 && ENCODERS < 1)
		// extra call in case generic inputs are running with ISR disabled. Encoders need propper ISR to work.
		mcu_inputs_changed_cb();
#endif
	}

#ifdef ENABLE_ITP_FEED_TASK
	static uint8_t itp_feed_counter = (uint8_t)CLAMP(1, (1000 / INTERPOLATOR_FREQ), 255);
	mls = itp_feed_counter;
	if (!cnc_lock_itp && !mls--)
	{
		cnc_lock_itp = 1;
		if ((cnc_state.loop_state == LOOP_RUNNING) && (cnc_state.alarm == EXEC_ALARM_NOALARM) && !cnc_get_exec_state(EXEC_INTERLOCKING_FAIL))
		{
			itp_run();
		}
		mls = (uint8_t)CLAMP(1, (1000 / INTERPOLATOR_FREQ), 255);
		cnc_lock_itp = 0;
	}

	itp_feed_counter = mls;
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
	if (!cnc_get_exec_state(EXEC_ALARM))
	{
		EVENT_INVOKE(rtc_tick, NULL);
	}
#endif

#if ASSERT_PIN(ACTIVITY_LED)
	// this blinks aprox. once every 1024ms
	if (!(millis & (0x200 - 1)))
	{
		io_toggle_output(ACTIVITY_LED);
	}
#endif
}
#endif

uint8_t cnc_home(void)
{
	cnc_set_exec_state(EXEC_HOMING);
	uint8_t error = kinematics_home();
// unlock expected limits
#ifdef ENABLE_MULTI_STEP_HOMING
	io_lock_limits(0);
#endif
	io_invert_limits(0);
	// sync's the motion control with the real time position
	// this flushes the homing motion before returning from error or home success
	itp_clear();
	planner_clear();
	mc_sync_position();

	// disables homing and reenables limits alarm messages
	cnc_clear_exec_state(EXEC_HOMING);

	if (error == STATUS_OK)
	{
		cnc_run_startup_blocks();
	}

	return error;
}

void cnc_alarm(int8_t code)
{
	cnc_set_exec_state(EXEC_KILL);
	cnc_stop();
	if (!cnc_state.alarm || code < 0)
	{
		cnc_state.alarm = code;
#ifdef ENABLE_MAIN_LOOP_MODULES
		if (code > 0)
		{
			EVENT_INVOKE(cnc_alarm, NULL);
		}
#endif
#ifdef ENABLE_IO_ALARM_DEBUG
		proto_info("LIMITS:%hd|CONTROLS:%hd", io_alarm_limits, io_alarm_controls);
#endif
	}
}

bool cnc_has_alarm()
{
	return (cnc_get_exec_state(EXEC_KILL) || (cnc_state.alarm != EXEC_ALARM_NOALARM));
}

uint8_t cnc_get_alarm(void)
{
	// force interlocking check to set alarm code in case this as not yet been set
	cnc_check_interlocking();
	return cnc_state.alarm;
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

#ifndef DISABLE_SAFE_SETTINGS
		// on settins error prevent unlock until settings error is cleared
		if (!(g_settings_error & SETTINGS_READ_ERROR))
		{
#endif
			CLEARFLAG(cnc_state.exec_state, EXEC_UNHOMED);
			cnc_state.alarm = EXEC_ALARM_NOALARM;
#ifndef DISABLE_SAFE_SETTINGS
		}
#endif
	}

	// if any alarm state is still active checks system faults
	if (cnc_get_exec_state(EXEC_ALARM) || cnc_has_alarm())
	{
		if (!cnc_get_exec_state(EXEC_KILL))
		{
#ifndef DISABLE_SAFE_SETTINGS
			// on settins error prevent unlock until settings error is cleared
			if ((g_settings_error & SETTINGS_READ_ERROR))
			{
				proto_feedback(MSG_FEEDBACK_16);
			}
#endif
			proto_feedback(MSG_FEEDBACK_2);
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
		parser_reset(false); // reset parser

		// hard reset
		// if homing not enabled run startup blocks
		if (cnc_state.loop_state < LOOP_RUNNING && !g_settings.homing_enabled)
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

#ifndef DISABLE_SAFE_SETTINGS
	// on settins error prevent unlock
	if (g_settings_error & SETTINGS_READ_ERROR)
	{
		CLEARFLAG(statemask, EXEC_UNHOMED);
	}
#endif

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
		// skip this if the hold release is for a jog cancel
		if (!cnc_get_exec_state(EXEC_JOG))
		{
#if TOOL_COUNT > 0
			planner_spindle_ovr_reset();
			// updated the coolant pins
			tool_set_coolant(planner_get_coolant());
#if (DELAY_ON_RESUME_COOLANT > 0)
			if (!g_settings.laser_mode)
			{
				if (!planner_buffer_is_empty())
				{
					cnc_dwell_ms(DELAY_ON_RESUME_COOLANT * 1000);
				}
			}
#endif
			// tries to sync the tool
			// if something goes wrong the tool can reinstate the HOLD state
			itp_sync_spindle();
#if (DELAY_ON_RESUME_SPINDLE > 0)
			if (!g_settings.laser_mode && cnc_state.loop_state == LOOP_RUNNING)
			{
				if (!planner_buffer_is_empty())
				{
					cnc_dwell_ms(DELAY_ON_RESUME_SPINDLE * 1000);
				}
			}
#endif
#endif
		}
	}

	CLEARFLAG(cnc_state.exec_state, statemask);
}

// executes delay
void cnc_delay_ms(uint32_t milliseconds)
{
	milliseconds += mcu_millis();
	do
	{
		cnc_dotasks();
	} while (mcu_millis() < milliseconds);
}

// executes delay (resumes earlier on error)
void cnc_dwell_ms(uint32_t milliseconds)
{
	milliseconds += mcu_millis();
	do
	{
	} while ((mcu_millis() < milliseconds) && cnc_dotasks());
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
	grbl_stream_clear();
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
	grbl_stream_start_broadcast();
	proto_print(MSG_STARTUP);
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
		__FALL_THROUGH__
	case CMD_CODE_JOG_CANCEL:
		if (cnc_get_exec_state(EXEC_JOG))
		{
			SETFLAG(cnc_state.exec_state, EXEC_HOLD);
			SETFLAG(cnc_state.rt_cmd, RT_CMD_JOG_CANCEL);
		}
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

#ifdef ENABLE_COOLANT
		if (command >= CMD_CODE_COOL_FLD_TOGGLE && command <= CMD_CODE_COOL_MST_TOGGLE)
		{
			tools_cmd &= RTCMD_SPINDLE_MASK;
			tools_cmd |= (RT_CMD_COOL_FLD_TOGGLE << (command - CMD_CODE_COOL_FLD_TOGGLE));
		}
#endif
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

#if STATUS_AUTOMATIC_REPORT_INTERVAL >= 100
	static uint32_t next_auto_report = STATUS_AUTOMATIC_REPORT_INTERVAL;
	uint32_t current_time = mcu_millis();
	if (next_auto_report < current_time)
	{
		next_auto_report = current_time + STATUS_AUTOMATIC_REPORT_INTERVAL;
		command |= RT_CMD_REPORT;
	}
#endif

	if (command)
	{
		// clear all but report. report is handled in cnc_io_dotasks
		__ATOMIC__
		{
			cnc_state.rt_cmd = RT_CMD_CLEAR;
		}
		if (CHECKFLAG(command, RT_CMD_RESET))
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
			cnc_state.loop_state = LOOP_STARTUP_RESET;
			return;
		}

		if (CHECKFLAG(command, RT_CMD_JOG_CANCEL))
		{
			while (grbl_stream_available())
			{
				char c = grbl_stream_getc();
				if (c == EOL)
				{
					proto_error(STATUS_JOG_CANCELED);
				}
			}
			return;
		}

		if (CHECKFLAG(command, RT_CMD_CYCLE_START))
		{
			cnc_clear_exec_state(EXEC_HOLD | EXEC_DOOR);
		}

		if (CHECKFLAG(command, RT_CMD_REPORT))
		{
			proto_status();
		}
	}

	// let µCNC finnish startup/reset code
	if (cnc_state.loop_state == LOOP_STARTUP_RESET)
	{
		return;
	}

	// executes feeds override rt commands
	command = cnc_state.feed_ovr_cmd; // copies realtime flags states
	if (command)
	{
		cnc_state.feed_ovr_cmd = RT_CMD_CLEAR; // clears command flags
		uint8_t ovr = g_planner_state.feed_override;
		switch (command & RTCMD_NORMAL_MASK)
		{
		case RT_CMD_FEED_100:
			planner_feed_ovr(100);
			break;
		case RT_CMD_FEED_INC_COARSE:
			planner_feed_ovr(ovr + FEED_OVR_COARSE);
			break;
		case RT_CMD_FEED_DEC_COARSE:
			planner_feed_ovr(ovr - FEED_OVR_COARSE);
			break;
		case RT_CMD_FEED_INC_FINE:
			planner_feed_ovr(ovr + FEED_OVR_FINE);
			break;
		case RT_CMD_FEED_DEC_FINE:
			planner_feed_ovr(ovr - FEED_OVR_FINE);
			break;
		}

		switch (command & RTCMD_RAPID_MASK)
		{
		case RT_CMD_RAPIDFEED_100:
			planner_rapid_feed_ovr(100);
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
#if TOOL_COUNT > 0
		uint8_t ovr = g_planner_state.spindle_speed_override;
		update_tools = true;
		switch (command & RTCMD_SPINDLE_MASK)
		{
		case RT_CMD_SPINDLE_100:
			planner_spindle_ovr(100);
			break;
		case RT_CMD_SPINDLE_INC_COARSE:
			planner_spindle_ovr(ovr + SPINDLE_OVR_COARSE);
			break;
		case RT_CMD_SPINDLE_DEC_COARSE:
			planner_spindle_ovr(ovr - SPINDLE_OVR_COARSE);
			break;
		case RT_CMD_SPINDLE_INC_FINE:
			planner_spindle_ovr(ovr + SPINDLE_OVR_FINE);
			break;
		case RT_CMD_SPINDLE_DEC_FINE:
			planner_spindle_ovr(ovr - SPINDLE_OVR_FINE);
			break;
		case RT_CMD_SPINDLE_TOGGLE:
			planner_spindle_ovr_toggle();
			break;
		}
#endif

#ifdef ENABLE_COOLANT
		switch (command & RTCMD_COOLANT_MASK)
		{
#if TOOL_COUNT > 0
		case RT_CMD_COOL_FLD_TOGGLE:
#ifndef M7_SAME_AS_M8
		case RT_CMD_COOL_MST_TOGGLE:
#endif
			if (!cnc_get_exec_state(EXEC_ALARM)) // if no alarm is active
			{
				if (command == RT_CMD_COOL_FLD_TOGGLE)
				{
					planner_coolant_ovr_toggle(COOLANT_MASK);
				}
#ifndef M7_SAME_AS_M8
				if (command == RT_CMD_COOL_MST_TOGGLE)
				{
					planner_coolant_ovr_toggle(MIST_MASK);
				}
#endif
			}
			break;
#endif
		}
#endif

		if (update_tools)
		{
			itp_update();
			if (planner_buffer_is_empty())
			{
				mc_update_tools(NULL);
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
		proto_feedback(MSG_FEEDBACK_12);
	}
#endif
#if ASSERT_PIN(SAFETY_DOOR)
	if (CHECKFLAG(inputs, SAFETY_DOOR_MASK)) // fault on safety door
	{
		proto_feedback(MSG_FEEDBACK_6);
	}
#endif
#if (LIMITS_MASK != 0)
	if (g_settings.hard_limits_enabled) // fault on limits
	{
		inputs = io_get_limits();
		if (CHECKFLAG(inputs, LIMITS_MASK))
		{
			proto_feedback(MSG_FEEDBACK_7);
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
			proto_feedback(MSG_FEEDBACK_1);
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

#if ASSERT_PIN(SAFETY_DOOR)
	// the safety door condition is active
	if (cnc_get_exec_state(EXEC_DOOR))
	{
		// door opened during a homing cycle exit with alarm
		if (cnc_get_exec_state(EXEC_HOMING))
		{
			cnc_alarm(EXEC_ALARM_HOMING_FAIL_DOOR);
			return false;
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
	}
#endif

	// an hold condition is active and motion as stopped
	if (cnc_get_exec_state(EXEC_HOLD) && !cnc_get_exec_state(EXEC_RUN))
	{
		itp_stop(); // stop motion

		if (cnc_get_exec_state(EXEC_HOMING | EXEC_JOG)) // flushes the buffers if motions was homing or jog
		{
			itp_clear();
			// clears the buffer but conserves the tool data
			while (!planner_buffer_is_empty())
			{
				planner_discard_block();
			}
			mc_sync_position();
			parser_sync_position();
			// flush all pending commands and motions
			mc_flush_pending_motion();
			// homing will be cleared inside homing cycle
			cnc_clear_exec_state(EXEC_HOLD | EXEC_JOG);
		}
	}

	// end of JOG
	if (cnc_get_exec_state(EXEC_JOG | EXEC_HOLD) == EXEC_JOG)
	{
		if (itp_is_empty() && planner_buffer_is_empty())
		{
			cnc_clear_exec_state(EXEC_JOG);
		}
	}

	return true;
}

static void cnc_io_dotasks(void)
{
	// run internal mcu tasks (USB and communications)
	mcu_dotasks();
#if IC74HC595_COUNT > 0 || IC74HC165_COUNT > 0
	io_extended_pins_update(); // update extended IO
#endif
	mcu_limits_changed_cb();
	mcu_controls_changed_cb();

#if (DIN_ONCHANGE_MASK != 0 && ENCODERS < 1)
	// extra call in case generic inputs are running with ISR disabled. Encoders need propper ISR to work.
	mcu_inputs_changed_cb();
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
	EVENT_INVOKE(cnc_io_dotasks, NULL);
#endif

#ifdef ENABLE_STEPPERS_DISABLE_TIMEOUT
	static uint32_t stepper_timeout = 0;

	if (g_settings.step_disable_timeout)
	{
		// is idle check the timeout
		if (cnc_get_exec_state(EXEC_RUN | EXEC_HOLD) == EXEC_IDLE)
		{
			if (stepper_timeout < mcu_millis())
			{
				io_enable_steppers(~g_settings.step_enable_invert); // disables steppers after idle timeout
				stepper_timeout = UINT32_MAX;
			}
		}
		else
		{
			stepper_timeout = mcu_millis() + g_settings.step_disable_timeout;
		}
	}
#endif
}

void cnc_run_startup_blocks(void)
{
	for (uint8_t i = 0; i < STARTUP_BLOCKS_COUNT; i++)
	{
		uint16_t address = STARTUP_BLOCK_ADDRESS_OFFSET(i);
		if (settings_check_startup_gcode(address))
		{
			grbl_stream_eeprom(address);
			cnc_parse_cmd();
		}
	}

	// reset streams
	grbl_stream_change(NULL);
}
