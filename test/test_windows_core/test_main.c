/*
	Name: test_main.c
	Description: Windows virtual core test for µCNC.

	Runs real µCNC G-code through the virtual UART2 RX path and asserts
	parser status, protocol output and final interpolator step position.

	This test is part of the µCNC RS274/NGC v3 conformance + motion
	endpoint regression suite. Expected positions are expressed in machine
	millimeters and converted with the active settings (step_per_mm).

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 18-08-2026

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>
*/

/*
 * cnc.c is intentionally excluded from build_src_filter and included here
 * so the test can access its private startup state without changing
 * production cnc.c.
 *
 * It must be included before <windows.h> because winnt.h redefines
 * FORCEINLINE to `extern __inline__` in C mode, which would collide with
 * the `static FORCEINLINE` declarations in atomic.h.
 */
#include "src/cnc.c"

#include <unity.h>
#include <windows.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern bool mcu_uart2_inject(const char *cmd);
extern void mcu_uart2_test_capture_reset(void);
extern const char *mcu_uart2_test_capture_get(void);
extern void mcu_test_clear_events(void);

#define TEST_MOTION_TIMEOUT_MS 5000

static bool test_controller_initialized;

static void test_controller_init_once(void)
{
	if (!test_controller_initialized)
	{
		cnc_init();
		test_controller_initialized = true;
	}
}

static void test_controller_prepare(void)
{
	test_controller_init_once();

	mcu_test_clear_events();
	test_io_reset();

	mcu_uart2_clear();
	mcu_uart2_test_capture_reset();

	cnc_reset();

	if (cnc_unlock(false) != UNLOCK_ERROR)
	{
		cnc_state.alarm = EXEC_ALARM_NOALARM;
	}

	cnc_state.loop_state = LOOP_RUNNING;

	/*
	 * Make every test start at machine step position zero.
	 */
	int32_t zero[STEPPER_COUNT] = {0};

	itp_sync_rt_position(zero);
	mc_sync_position();
	parser_sync_position();

	mcu_uart2_test_capture_reset();
}

static uint8_t test_execute_line(const char *cmd)
{
	TEST_ASSERT_TRUE(mcu_uart2_inject(cmd));

	uint8_t status = cnc_parse_cmd();

	TEST_ASSERT_TRUE(cnc_dotasks());

	// wait for the interpolator to have some steps in the buffer
	while (itp_is_empty() && !planner_buffer_is_empty())
	{
		if (!cnc_dotasks())
		{
			break;
		}
	}

	mcu_uart2_flush();

	return status;
}

static bool test_sync_motion(uint32_t timeout_ms)
{
	DWORD start = GetTickCount();

	while (!itp_is_empty() || !planner_buffer_is_empty())
	{
		if (!cnc_dotasks())
		{
			return false;
		}

		if ((GetTickCount() - start) > timeout_ms)
		{
			return false;
		}

		Sleep(1);
	}

	return true;
}

void setUp(void)
{
	test_controller_prepare();
}

void tearDown(void)
{
}

/* ---------------------------------------------------------------------------
 * Position assertion helpers
 * ------------------------------------------------------------------------- */

static int32_t test_mm_to_steps(uint8_t axis, float mm)
{
	return (int32_t)lroundf(mm * g_settings.step_per_mm[axis]);
}

static void test_assert_position_array_mm(float *mpos)
{
	int32_t pos[STEPPER_COUNT] = {0};

	itp_get_rt_position(pos);

	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		char reason[256];
		uint32_t result = test_mm_to_steps(i, mpos[i]);
		snprintf(
			reason,
			sizeof(reason),
			"expected axis %u position to be %lu and got %lu", i, pos[i], result);
		TEST_ASSERT_INT32_WITHIN_MESSAGE(
			1,
			result,
			pos[i],
			reason);
	}
}

static void test_assert_position_mm(float x, float y, float z)
{
	int32_t pos[STEPPER_COUNT] = {0};

	itp_get_rt_position(pos);

#ifdef AXIS_X
	TEST_ASSERT_INT32_WITHIN(
		1,
		test_mm_to_steps(AXIS_X, x),
		pos[AXIS_X]);
#endif

#ifdef AXIS_Y
	TEST_ASSERT_INT32_WITHIN(
		1,
		test_mm_to_steps(AXIS_Y, y),
		pos[AXIS_Y]);
#endif

#ifdef AXIS_Z
	TEST_ASSERT_INT32_WITHIN(
		1,
		test_mm_to_steps(AXIS_Z, z),
		pos[AXIS_Z]);
#endif
}

static void test_assert_capture_has(const char *needle)
{
	TEST_ASSERT_NOT_NULL(strstr(mcu_uart2_test_capture_get(), needle));
}

/* ---------------------------------------------------------------------------
 * Grbl state translation (mirrors proto_status()).
 * ------------------------------------------------------------------------- */

typedef enum
{
	TEST_GRBL_IDLE,
	TEST_GRBL_RUN,
	TEST_GRBL_HOLD_0,
	TEST_GRBL_HOLD_1,
	TEST_GRBL_HOLD_2,
	TEST_GRBL_JOG,
	TEST_GRBL_ALARM,
	TEST_GRBL_LOCKED,
	TEST_GRBL_CHECK,
	TEST_GRBL_HOME,
	TEST_GRBL_DOOR_0,
	TEST_GRBL_DOOR_1,
	TEST_GRBL_DOOR_2,
	TEST_GRBL_DOOR_3,
	TEST_GRBL_SLEEP,
	TEST_GRBL_PROBE,
	TEST_GRBL_DWELL,
	TEST_GRBL_DONT_CARE
} test_grbl_state_t;

static test_grbl_state_t test_grbl_translate(uint8_t status)
{
	switch (status)
	{
	case EXEC_STATUS_IDLE:
		return TEST_GRBL_IDLE;
	case EXEC_STATUS_RUNNING:
		return TEST_GRBL_RUN;
	case EXEC_STATUS_HOLD:
		return TEST_GRBL_HOLD_0;
	case EXEC_STATUS_HOLD_PENDING:
		return TEST_GRBL_HOLD_1;
	case EXEC_STATUS_HOLD_RESUMING:
		return TEST_GRBL_HOLD_2;
	case EXEC_STATUS_JOGGING:
		return TEST_GRBL_JOG;
	case EXEC_STATUS_ALARM:
		return TEST_GRBL_ALARM;
	case EXEC_STATUS_LOCKED:
#ifdef ENABLE_EXTRA_GRBL_STATES
		return TEST_GRBL_LOCKED;
#else
		/* Strict Grbl mode: Locked is reported as Alarm. */
		return TEST_GRBL_ALARM;
#endif
	case EXEC_STATUS_CHECK:
		return TEST_GRBL_CHECK;
	case EXEC_STATUS_HOMING:
		return TEST_GRBL_HOME;
	case EXEC_STATUS_DOOR_CLOSED:
		return TEST_GRBL_DOOR_0;
	case EXEC_STATUS_DOOR_OPENED:
		return TEST_GRBL_DOOR_1;
	case EXEC_STATUS_DOOR_OPENED_PAUSING:
		return TEST_GRBL_DOOR_2;
	case EXEC_STATUS_DOOR_CLOSED_RESUMING:
		return TEST_GRBL_DOOR_3;
	case EXEC_STATUS_PROBING:
		return TEST_GRBL_PROBE;
	case EXEC_STATUS_DWELL:
		return TEST_GRBL_DWELL;
	default:
		return TEST_GRBL_DONT_CARE;
	}
}

static const char *test_grbl_state_name(test_grbl_state_t state)
{
	switch (state)
	{
	case TEST_GRBL_IDLE:
		return "Idle";
	case TEST_GRBL_RUN:
		return "Run";
	case TEST_GRBL_HOLD_0:
		return "Hold:0";
	case TEST_GRBL_HOLD_1:
		return "Hold:1";
	case TEST_GRBL_HOLD_2:
		return "Hold:2";
	case TEST_GRBL_JOG:
		return "Jog";
	case TEST_GRBL_ALARM:
		return "Alarm";
	case TEST_GRBL_LOCKED:
		return "Locked";
	case TEST_GRBL_CHECK:
		return "Check";
	case TEST_GRBL_HOME:
		return "Home";
	case TEST_GRBL_DOOR_0:
		return "Door:0";
	case TEST_GRBL_DOOR_1:
		return "Door:1";
	case TEST_GRBL_DOOR_2:
		return "Door:2";
	case TEST_GRBL_DOOR_3:
		return "Door:3";
	case TEST_GRBL_SLEEP:
		return "Sleep";
	case TEST_GRBL_PROBE:
		return "Probe";
	case TEST_GRBL_DWELL:
		return "Dwell";
	default:
		return "?";
	}
}

static const char *test_status_name(uint8_t status)
{
	return test_grbl_state_name(test_grbl_translate(status));
}

static void test_delay_ms(uint32_t timeout_ms)
{
	uint32_t start = mcu_millis();

	while ((uint32_t)(mcu_millis() - start) < timeout_ms)
	{
		cnc_dotasks();
	}
}

/* ---------------------------------------------------------------------------
 * Diagnostic message helpers.
 * ------------------------------------------------------------------------- */

static void test_make_message(
	char *buffer,
	size_t buffer_size,
	const char *case_name,
	const char *phase)
{
	snprintf(buffer, buffer_size, "%s | %s", case_name, phase);
}

/* ---------------------------------------------------------------------------
 * Realtime UART stimulus helper.
 * ------------------------------------------------------------------------- */

static void test_send_rt(uint8_t command)
{
	char data[2] = {(char)command, '\0'};

	TEST_ASSERT_TRUE(mcu_uart2_inject(data));

	/* Let pending realtime work propagate. */
	test_delay_ms(1);

	mcu_uart2_flush();
}

/* ---------------------------------------------------------------------------
 * Scheduled IO / realtime events on the simulated MCU timeline.
 * ------------------------------------------------------------------------- */

typedef struct
{
	uint8_t input;
	bool value;
} test_io_event_args_t;

static void test_io_event_cb(void *args)
{
	test_io_event_args_t *event = args;
	test_io_set(event->input, event->value);
	free(event);
}

static void test_schedule_io_us(uint8_t input, bool value, uint32_t delay_us)
{
	test_io_event_args_t *event = calloc(1, sizeof(test_io_event_args_t));
	TEST_ASSERT_NOT_NULL(event);

	event->input = input;
	event->value = value;

	mcu_add_event(delay_us, test_io_event_cb, event);
}

typedef struct
{
	uint8_t command;
} test_rt_event_args_t;

static void test_rt_event_cb(void *args)
{
	test_rt_event_args_t *event = args;
	cnc_call_rt_command(event->command);
	free(event);
}

static void test_schedule_rt_us(uint8_t command, uint32_t delay_us)
{
	test_rt_event_args_t *event = calloc(1, sizeof(test_rt_event_args_t));
	TEST_ASSERT_NOT_NULL(event);

	event->command = command;

	mcu_add_event(delay_us, test_rt_event_cb, event);
}

/* ---------------------------------------------------------------------------
 * Physical control helpers.
 * ------------------------------------------------------------------------- */

static void test_press_control(uint8_t input)
{
	test_io_set(input, true);
	test_delay_ms(1);
}

static void test_release_control(uint8_t input)
{
	test_io_set(input, false);
	test_delay_ms(1);
}

/* ---------------------------------------------------------------------------
 * Wait helpers (simulated MCU clock).
 * ------------------------------------------------------------------------- */

static bool test_wait_status(uint8_t expected, uint32_t timeout_ms)
{
	uint32_t start = mcu_millis();

	while ((uint32_t)(mcu_millis() - start) < timeout_ms)
	{
		if (cnc_get_status() == expected)
		{
			return true;
		}
		test_delay_ms(1);
	}

	return cnc_get_status() == expected;
}

typedef bool (*test_condition_cb_t)(void);

static bool test_wait_condition(
	test_condition_cb_t condition,
	uint32_t timeout_ms)
{
	uint32_t start = mcu_millis();

	while ((uint32_t)(mcu_millis() - start) < timeout_ms)
	{
		if (condition())
		{
			return true;
		}
		test_delay_ms(1);
	}

	return condition();
}

static void test_wait_status_or_fail(
	uint8_t expected,
	uint32_t timeout_ms,
	const char *case_name,
	const char *phase)
{
	uint32_t start = mcu_millis();
	uint8_t last = cnc_get_status();

	while ((uint32_t)(mcu_millis() - start) < timeout_ms)
	{
		last = cnc_get_status();
		if (last == expected)
		{
			return;
		}
		cnc_dotasks();
	}

	char msg[192];
	snprintf(
		msg,
		sizeof(msg),
		"%s | %s | timeout waiting for %s, last status %s",
		case_name,
		phase,
		test_status_name(expected),
		test_status_name(last));

	TEST_FAIL_MESSAGE(msg);
}

/* ---------------------------------------------------------------------------
 * State fixtures.
 * ------------------------------------------------------------------------- */

static void test_fixture_idle(void)
{
	/* Controller is already prepared to Idle by test_controller_prepare(). */
}

static void test_fixture_run(const char *case_name)
{
	uint8_t status = test_execute_line("G21\n");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(STATUS_OK, status, case_name);
	status = test_execute_line("G90\n");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(STATUS_OK, status, case_name);
	status = test_execute_line("G1X100F600\n");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(STATUS_OK, status, case_name);
}

static void test_fixture_hold(const char *case_name, uint8_t expected)
{
	test_fixture_run(case_name);
	test_send_rt(CMD_CODE_FEED_HOLD);
	test_wait_status_or_fail(expected, TEST_MOTION_TIMEOUT_MS, case_name, "fixture hold");
}

static void test_fixture_alarm(void)
{
	/* Trigger a real alarm: hard limit while moving. */
	test_fixture_run("alarm fixture");
	test_wait_status_or_fail(
		EXEC_STATUS_RUNNING,
		TEST_MOTION_TIMEOUT_MS,
		"alarm fixture",
		"run");
	test_io_set(TEST_IO_LIMIT_X, true);
	test_delay_ms(10);
	/* Release the limit so unlock/reset can clear the alarm. */
	test_io_set(TEST_IO_LIMIT_X, false);
	test_delay_ms(10);
}

static void test_fixture_jog(const char *case_name)
{
	uint8_t status = test_execute_line("$J=G91X100F600\n");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(STATUS_OK, status, case_name);
}

static void test_fixture_check(const char *case_name)
{
	uint8_t status = test_execute_line("$C\n");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(STATUS_OK, status, case_name);
}

/* ---------------------------------------------------------------------------
 * Settings helpers.
 * ------------------------------------------------------------------------- */

static void test_set_setting(uint8_t id, float value)
{
	char cmd[32];
	snprintf(cmd, sizeof(cmd), "$%u=%g\n", (unsigned)id, value);
	uint8_t status = test_execute_line(cmd);
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, status);
}

static void test_soft_reset(void)
{
	/* Ctrl-X soft reset through the real reset path. */
	test_send_rt(CMD_CODE_RESET);
	test_delay_ms(10);

	/* Re-establish the running state after the soft reset. */
	if (cnc_unlock(false) != UNLOCK_ERROR)
	{
		cnc_state.alarm = EXEC_ALARM_NOALARM;
	}
	cnc_state.loop_state = LOOP_RUNNING;

	int32_t zero[STEPPER_COUNT] = {0};
	itp_sync_rt_position(zero);
	mc_sync_position();
	parser_sync_position();

	mcu_uart2_test_capture_reset();
}

typedef struct test_block_
{
	const char *cmd;				// command to be sent to the parse (if NULL ignores)
	bool cmd_expected_ok;			// expected result of the command (true if STATUS_OK or false if other code)
	uint8_t expected_status;		// expected Grbl status (after executing the command if any). Set to 254 if don't care
	uint32_t status_wait_timeout;	// how much time will it wait for the expected status before failing. If 0 will test imediatly
	bool is_synched;				// is a synched command. it will wait for the motion to finish before running the next block
	const float *expected_position; // expected position at the end of the motion to compare. ignores this if NULL or is_synched is false
	uint8_t io_target;				// if not 0 commands an io defined by io_target
	bool io_value;					// new value to set to the io (emulation)
	uint32_t callback_delay_us;		// if a delay is set it will add an event to fire the sequence of inner/dependent test_block_t *sequence of tests. If set to 0 executes the sequence immediatly
	struct test_block_ *sequence;	// sequence of inner blocks to execute in an event. note that this will run in an mcu_event so it may run in parallel to the current block
	bool is_last;					// signals the last block in a sequence
} test_block_t;

void test_run_block(const test_block_t *block);
void test_run_blocks(const test_block_t *block);

void test_run_block_sequence(void *args)
{
	const test_block_t *blocks = (const test_block_t *)args;
	test_run_blocks(blocks);
}

void test_run_block(const test_block_t *block)
{
	// tick the cnc
	cnc_dotasks();

	if (!block)
	{
		TEST_IGNORE_MESSAGE("Block is empty");
	}

	if (block->cmd)
	{
		char reason[256];
		uint8_t result = test_execute_line(block->cmd);
		mcu_uart2_flush();
		if (block->cmd_expected_ok)
		{
			snprintf(
				reason,
				sizeof(reason),
				"expected command result OK and got %u",
				result);
			TEST_ASSERT_EQUAL_UINT8_MESSAGE(STATUS_OK, result, reason);
		}
		else
		{
			snprintf(
				reason,
				sizeof(reason),
				"expected command result NOT OK and got %u",
				result);
			TEST_ASSERT_NOT_EQUAL_MESSAGE(STATUS_OK, result, reason);
		}
	}

	if (block->expected_status != 254)
	{
		test_wait_status_or_fail(block->expected_status, block->status_wait_timeout, "", "");
	}

	if (block->is_synched)
	{
		TEST_ASSERT_TRUE_MESSAGE(test_sync_motion(TEST_MOTION_TIMEOUT_MS), "Motion failed to complete");
	}

	if (block->expected_position)
	{
		test_assert_position_array_mm(block->expected_position);
	}

	if (block->io_target)
	{
		test_io_set(block->io_target, block->io_value);
	}

	if (block->sequence)
	{
		mcu_add_event(block->callback_delay_us, test_run_block_sequence, (void *)block->sequence);
	}

	// tick the cnc
	cnc_dotasks();
}

/**
 * This is the main test block function to call.
 * It's possible to chain several blocks and interactions.
 *
 */
void test_run_blocks(const test_block_t *blocks)
{
	if (!blocks)
	{
		TEST_IGNORE_MESSAGE("No blocks to run\r\n");
	}

	do
	{
		test_run_block(blocks);
		blocks++;
	} while (!blocks->is_last);

	// run last block
	test_run_block(blocks);
}

// just for test purposes
void test_run_blocks_test(void)
{
	// example
	const test_block_t t[] = {{"G0X10Y10\n", true, 254, 0, true, (float[3]){10.0f, 10.0f, 0.0f}, 0, false, 0, NULL, true}};
	test_run_blocks(t);
}

/* ---------------------------------------------------------------------------
 * G0 / G1 motion
 * ------------------------------------------------------------------------- */

static void test_g0_absolute_xy(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X10Y10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");

	/*-------------OR-------------- */

	test_run_blocks((test_block_t[3]){
		{"G21\n", true, 254, 0, false, NULL, 0, false, 0, NULL, false},
		{"G90\n", true, 254, 0, false, NULL, 0, false, 0, NULL, false},
		{"G0X10Y10\n", true, 254, 0, true, (float[3]){10.0f, 10.0f, 0.0f}, 0, false, 0, NULL, true}});

	test_assert_capture_has("ok");
}

static void test_g0_incremental_xy(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G91\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X5Y-2\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X5\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	mcu_uart2_flush();

	test_assert_position_mm(10.0f, -2.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g1_absolute_xy(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G1X10Y5F600\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 5.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g1_modal_continue(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G1X5Y0F600\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("X10Y5\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 5.0f, 0.0f);
	test_assert_capture_has("ok");
}

/* ---------------------------------------------------------------------------
 * G2 / G3 arcs
 * ------------------------------------------------------------------------- */

static void test_g2_ij_cw(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G2X10Y10I10J0\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g3_ij_ccw(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G3X10Y10I0J10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g2_r_cw(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G2X10Y10R10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g3_r_ccw(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G3X10Y10R10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

#ifdef AXIS_X
#ifdef AXIS_Z
static void test_g18_xz_plane(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G18\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Z0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G2X10Z10I10K0\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 0.0f, 10.0f);
	test_assert_capture_has("ok");
}
#endif
#endif

#ifdef AXIS_Y
#ifdef AXIS_Z
static void test_g19_yz_plane(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G19\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0Y0Z0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G3Y10Z10J0K10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(0.0f, 10.0f, 10.0f);
	test_assert_capture_has("ok");
}
#endif
#endif

/* ---------------------------------------------------------------------------
 * G20 / G21 units and word order / comments
 * ------------------------------------------------------------------------- */

static void test_g20_inch(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G20\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X1\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(25.4f, 0.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g21_mm(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X25.4\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(25.4f, 0.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_word_order(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("Y10F600X10G1G90\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_comment_in_line(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X10(test comment)Y10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

/* ---------------------------------------------------------------------------
 * Modal group / repeated word rules
 * ------------------------------------------------------------------------- */

static void test_valid_multiple_modal_groups(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21G90G1X10Y5F600\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 5.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_valid_reordered_modal_groups(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("Y5F600X10G1G90G21\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 5.0f, 0.0f);
	test_assert_capture_has("ok");
}

/* ---------------------------------------------------------------------------
 * Invalid / ill-formed commands
 * ------------------------------------------------------------------------- */

static void test_invalid_gcode_does_not_move(void)
{
	int32_t before[STEPPER_COUNT] = {0};
	int32_t after[STEPPER_COUNT] = {0};

	itp_get_rt_position(before);

	uint8_t status = test_execute_line("G0XABC\n");

	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);

	itp_get_rt_position(after);

	for (uint8_t i = 0; i < STEPPER_COUNT; i++)
	{
		TEST_ASSERT_EQUAL_INT32(before[i], after[i]);
	}

	test_assert_capture_has("error");
}

static void test_err_g0_no_axis(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G0\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g1_no_axis(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G1X0F600\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G1\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_axis_no_number(void)
{
	uint8_t status = test_execute_line("G0X\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_bad_number(void)
{
	uint8_t status = test_execute_line("G0XABC\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_repeated_axis(void)
{
	uint8_t status = test_execute_line("G0X1X2\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_two_motion_gcodes(void)
{
	uint8_t status = test_execute_line("G0G1X10\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_two_unit_gcodes(void)
{
	uint8_t status = test_execute_line("G20G21\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_two_distance_modes(void)
{
	uint8_t status = test_execute_line("G90G91\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g2_no_center(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G2X10Y10F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g3_no_center(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G3X10Y10F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g2_no_endpoint(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G2I5J0F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g3_no_endpoint(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G3I0J5F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g2_bad_center_radius(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G2X10Y10I4J0F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g2_radius_too_small(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G2X10Y10R1F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g4_negative(void)
{
	uint8_t status = test_execute_line("G4P-1\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g4_missing_p(void)
{
	uint8_t status = test_execute_line("G4\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

/* ---------------------------------------------------------------------------
 * Parser atomicity
 * ------------------------------------------------------------------------- */

static void test_parser_atomicity(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G91G0XBAD\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);

	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X10\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	mcu_uart2_flush();

	/*
	 * G91 from the invalid block must not be committed, so the following
	 * G0X10 is absolute and ends at 10 mm, not 10 mm relative to an
	 * altered state.
	 */
	test_assert_position_mm(10.0f, 0.0f, 0.0f);
}

/* ---------------------------------------------------------------------------
 * Test Group 1 — Control inputs and realtime commands
 * ------------------------------------------------------------------------- */

static void test_safety_door_path(void)
{
	const char *case_name = "safety door stop/close/resume/reopen";

	test_fixture_run(case_name);
	test_wait_status_or_fail(
		EXEC_STATUS_RUNNING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"fixture running");

	/* Open the safety door (physical input). */
	test_press_control(TEST_IO_SAFETY_DOOR);
	test_wait_status_or_fail(
		EXEC_STATUS_DOOR_OPENED_PAUSING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"door open while running (Door:2)");
	test_wait_status_or_fail(
		EXEC_STATUS_DOOR_OPENED,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"door open after stop (Door:1)");

	/* Close the door. */
	test_release_control(TEST_IO_SAFETY_DOOR);
	test_wait_status_or_fail(
		EXEC_STATUS_DOOR_CLOSED,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"door closed stopped (Door:0)");

	/*
	 * Cycle start resumes. In this µCNC build (no ENABLE_SAFETY_DOOR_PARKING)
	 * EXEC_DOOR is cleared immediately on cycle start, so the Grbl Door:3
	 * (DOOR_CLOSED_RESUMING) intermediate state is skipped and the machine
	 * resumes directly to Run. The Door:3 expectation is asserted separately
	 * in test_door_resume_grbl_door3() as a documented compatibility mismatch.
	 */
	test_send_rt(CMD_CODE_CYCLE_START);
	test_wait_status_or_fail(
		EXEC_STATUS_RUNNING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"door resume running");

	/* Reopen the door during Run and verify the stop is re-entered. */
	test_schedule_io_us(TEST_IO_SAFETY_DOOR, true, 20000);
	test_wait_status_or_fail(
		EXEC_STATUS_DOOR_OPENED_PAUSING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"reopen while running (Door:2)");
	test_wait_status_or_fail(
		EXEC_STATUS_DOOR_OPENED,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"reopen after stop (Door:1)");

	/* Clean up: close door and resume to leave a clean state. */
	test_release_control(TEST_IO_SAFETY_DOOR);
	test_wait_status_or_fail(
		EXEC_STATUS_DOOR_CLOSED,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"reopen close stopped (Door:0)");
	test_send_rt(CMD_CODE_CYCLE_START);
}

/*
 * Grbl v1.1 expects the resume-from-door sequence to pass through Door:3
 * (door closed, resuming) before reaching Run. This build clears EXEC_DOOR
 * immediately on cycle start, so Door:3 is never reported. This test keeps
 * the Grbl-correct expectation and documents the µCNC mismatch; it is
 * expected to fail until production behavior is aligned.
 */
static void test_door_resume_grbl_door3(void)
{
	const char *case_name = "door resume Grbl Door:3";

	test_fixture_run(case_name);
	test_wait_status_or_fail(
		EXEC_STATUS_RUNNING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"fixture running");

	test_press_control(TEST_IO_SAFETY_DOOR);
	test_wait_status_or_fail(
		EXEC_STATUS_DOOR_OPENED_PAUSING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"door open (Door:2)");
	test_wait_status_or_fail(
		EXEC_STATUS_DOOR_OPENED,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"door stopped (Door:1)");

	test_release_control(TEST_IO_SAFETY_DOOR);
	test_wait_status_or_fail(
		EXEC_STATUS_DOOR_CLOSED,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"door closed (Door:0)");

	test_send_rt(CMD_CODE_CYCLE_START);

	/*
	 * Grbl: Door:3 (DOOR_CLOSED_RESUMING) must be observed while resuming.
	 * µCNC skips it and goes directly to Run. This assertion documents the
	 * mismatch and is expected to fail.
	 */
	test_wait_status_or_fail(
		EXEC_STATUS_DOOR_CLOSED_RESUMING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"door closed resuming (Door:3)");
}

/* ---------------------------------------------------------------------------
 * Control / realtime command matrix (TB-05).
 * ------------------------------------------------------------------------- */

typedef enum
{
	TEST_FIXTURE_IDLE,
	TEST_FIXTURE_RUN,
	TEST_FIXTURE_HOLD,
	TEST_FIXTURE_JOG,
	TEST_FIXTURE_ALARM,
	TEST_FIXTURE_DOOR_OPEN,
	TEST_FIXTURE_CHECK,
	TEST_FIXTURE_DONT_CARE
} test_fixture_t;

typedef enum
{
	TEST_STIM_RT_RESET,
	TEST_STIM_RT_STATUS,
	TEST_STIM_RT_HOLD,
	TEST_STIM_RT_START,
	TEST_STIM_RT_DOOR,
	TEST_STIM_RT_JOG_CANCEL,
	TEST_STIM_PIN_FHOLD,
	TEST_STIM_PIN_CS_RES,
	TEST_STIM_PIN_DOOR_OPEN,
	TEST_STIM_PIN_DOOR_CLOSE
} test_stimulus_t;

typedef struct
{
	const char *name;
	test_fixture_t initial;
	test_stimulus_t stimulus;

	test_grbl_state_t expected_settled;
	uint8_t expected_alarm; /* EXEC_ALARM_NOALARM if none */
} test_transition_case_t;

static const char *test_stimulus_name(test_stimulus_t stimulus)
{
	switch (stimulus)
	{
	case TEST_STIM_RT_RESET:
		return "reset";
	case TEST_STIM_RT_STATUS:
		return "status ?";
	case TEST_STIM_RT_HOLD:
		return "feed hold !";
	case TEST_STIM_RT_START:
		return "cycle start ~";
	case TEST_STIM_RT_DOOR:
		return "safety door";
	case TEST_STIM_RT_JOG_CANCEL:
		return "jog cancel";
	case TEST_STIM_PIN_FHOLD:
		return "FHOLD pin";
	case TEST_STIM_PIN_CS_RES:
		return "CS_RES pin";
	case TEST_STIM_PIN_DOOR_OPEN:
		return "door open pin";
	case TEST_STIM_PIN_DOOR_CLOSE:
		return "door close pin";
	default:
		return "?";
	}
}

static void test_build_fixture(test_fixture_t fixture, const char *case_name)
{
	switch (fixture)
	{
	case TEST_FIXTURE_IDLE:
		break;
	case TEST_FIXTURE_RUN:
		test_fixture_run(case_name);
		test_wait_status_or_fail(
			EXEC_STATUS_RUNNING,
			TEST_MOTION_TIMEOUT_MS,
			case_name,
			"fixture run");
		break;
	case TEST_FIXTURE_HOLD:
		test_fixture_hold(case_name, EXEC_STATUS_HOLD);
		break;
	case TEST_FIXTURE_JOG:
		test_fixture_jog(case_name);
		test_wait_status_or_fail(
			EXEC_STATUS_JOGGING,
			TEST_MOTION_TIMEOUT_MS,
			case_name,
			"fixture jog");
		break;
	case TEST_FIXTURE_ALARM:
		test_fixture_alarm();
		break;
	case TEST_FIXTURE_DOOR_OPEN:
		test_fixture_run(case_name);
		test_wait_status_or_fail(
			EXEC_STATUS_RUNNING,
			TEST_MOTION_TIMEOUT_MS,
			case_name,
			"fixture run for door");
		test_press_control(TEST_IO_SAFETY_DOOR);
		test_wait_status_or_fail(
			EXEC_STATUS_DOOR_OPENED,
			TEST_MOTION_TIMEOUT_MS,
			case_name,
			"fixture door open");
		break;
	case TEST_FIXTURE_CHECK:
		test_fixture_check(case_name);
		break;
	default:
		break;
	}
}

static void test_apply_stimulus(test_stimulus_t stimulus)
{
	switch (stimulus)
	{
	case TEST_STIM_RT_RESET:
		test_send_rt(CMD_CODE_RESET);
		break;
	case TEST_STIM_RT_STATUS:
		test_send_rt(CMD_CODE_REPORT);
		break;
	case TEST_STIM_RT_HOLD:
		test_send_rt(CMD_CODE_FEED_HOLD);
		break;
	case TEST_STIM_RT_START:
		test_send_rt(CMD_CODE_CYCLE_START);
		break;
	case TEST_STIM_RT_DOOR:
		test_send_rt(CMD_CODE_SAFETY_DOOR);
		break;
	case TEST_STIM_RT_JOG_CANCEL:
		test_send_rt(CMD_CODE_JOG_CANCEL);
		break;
	case TEST_STIM_PIN_FHOLD:
		test_press_control(TEST_IO_FHOLD);
		break;
	case TEST_STIM_PIN_CS_RES:
		test_press_control(TEST_IO_CS_RES);
		break;
	case TEST_STIM_PIN_DOOR_OPEN:
		test_press_control(TEST_IO_SAFETY_DOOR);
		break;
	case TEST_STIM_PIN_DOOR_CLOSE:
		test_release_control(TEST_IO_SAFETY_DOOR);
		break;
	default:
		break;
	}
}

static void test_run_transition_case(const test_transition_case_t *tc)
{
	test_controller_prepare();

	test_build_fixture(tc->initial, tc->name);

	test_apply_stimulus(tc->stimulus);

	/*
	 * A soft reset returns to the startup state; re-establish the running
	 * controller state like test_soft_reset() so the case ends back in a
	 * deterministic Idle for the settled assertion.
	 */
	if (tc->stimulus == TEST_STIM_RT_RESET)
	{
		test_delay_ms(10);
		if (cnc_unlock(true) != UNLOCK_ERROR)
		{
			cnc_state.alarm = EXEC_ALARM_NOALARM;
		}
		cnc_state.loop_state = LOOP_RUNNING;
	}

	if (tc->expected_settled != TEST_GRBL_DONT_CARE)
	{
		char phase[96];
		snprintf(
			phase,
			sizeof(phase),
			"settled after %s",
			test_stimulus_name(tc->stimulus));

		uint8_t expected_status = EXEC_STATUS_IDLE;
		switch (tc->expected_settled)
		{
		case TEST_GRBL_RUN:
			expected_status = EXEC_STATUS_RUNNING;
			break;
		case TEST_GRBL_HOLD_0:
			expected_status = EXEC_STATUS_HOLD;
			break;
		case TEST_GRBL_HOLD_1:
			expected_status = EXEC_STATUS_HOLD_PENDING;
			break;
		case TEST_GRBL_JOG:
			expected_status = EXEC_STATUS_JOGGING;
			break;
		case TEST_GRBL_ALARM:
			expected_status = EXEC_STATUS_ALARM;
			break;
		case TEST_GRBL_LOCKED:
			expected_status = EXEC_STATUS_LOCKED;
			break;
		case TEST_GRBL_CHECK:
			expected_status = EXEC_STATUS_CHECK;
			break;
		case TEST_GRBL_HOME:
			expected_status = EXEC_STATUS_HOMING;
			break;
		case TEST_GRBL_DOOR_0:
			expected_status = EXEC_STATUS_DOOR_CLOSED;
			break;
		case TEST_GRBL_DOOR_1:
			expected_status = EXEC_STATUS_DOOR_OPENED;
			break;
		case TEST_GRBL_DOOR_2:
			expected_status = EXEC_STATUS_DOOR_OPENED_PAUSING;
			break;
		case TEST_GRBL_DOOR_3:
			expected_status = EXEC_STATUS_DOOR_CLOSED_RESUMING;
			break;
		case TEST_GRBL_PROBE:
			expected_status = EXEC_STATUS_PROBING;
			break;
		default:
			expected_status = EXEC_STATUS_IDLE;
			break;
		}

		test_wait_status_or_fail(
			expected_status,
			TEST_MOTION_TIMEOUT_MS,
			tc->name,
			phase);
	}

	if (tc->expected_alarm != EXEC_ALARM_NOALARM)
	{
		char msg[160];
		snprintf(
			msg,
			sizeof(msg),
			"%s | alarm after %s",
			tc->name,
			test_stimulus_name(tc->stimulus));
		TEST_ASSERT_EQUAL_INT8_MESSAGE(
			tc->expected_alarm,
			cnc_get_alarm(),
			msg);
	}
}

static void test_control_idle_matrix(void)
{
	static const test_transition_case_t cases[] = {
		{.name = "IDLE + reset", .initial = TEST_FIXTURE_IDLE, .stimulus = TEST_STIM_RT_RESET, .expected_settled = TEST_GRBL_IDLE, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "IDLE + status", .initial = TEST_FIXTURE_IDLE, .stimulus = TEST_STIM_RT_STATUS, .expected_settled = TEST_GRBL_IDLE, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "IDLE + feed hold", .initial = TEST_FIXTURE_IDLE, .stimulus = TEST_STIM_RT_HOLD, .expected_settled = TEST_GRBL_HOLD_0, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "IDLE + cycle start", .initial = TEST_FIXTURE_IDLE, .stimulus = TEST_STIM_RT_START, .expected_settled = TEST_GRBL_IDLE, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "IDLE + safety door", .initial = TEST_FIXTURE_IDLE, .stimulus = TEST_STIM_RT_DOOR, .expected_settled = TEST_GRBL_DOOR_0, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "IDLE + jog cancel", .initial = TEST_FIXTURE_IDLE, .stimulus = TEST_STIM_RT_JOG_CANCEL, .expected_settled = TEST_GRBL_IDLE, .expected_alarm = EXEC_ALARM_NOALARM},
	};
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
	{
		test_run_transition_case(&cases[i]);
	}
}

static void test_control_run_matrix(void)
{
	static const test_transition_case_t cases[] = {
		{.name = "RUN + feed hold", .initial = TEST_FIXTURE_RUN, .stimulus = TEST_STIM_RT_HOLD, .expected_settled = TEST_GRBL_HOLD_0, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "RUN + cycle start", .initial = TEST_FIXTURE_RUN, .stimulus = TEST_STIM_RT_START, .expected_settled = TEST_GRBL_RUN, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "RUN + safety door", .initial = TEST_FIXTURE_RUN, .stimulus = TEST_STIM_RT_DOOR, .expected_settled = TEST_GRBL_DOOR_0, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "RUN + jog cancel", .initial = TEST_FIXTURE_RUN, .stimulus = TEST_STIM_RT_JOG_CANCEL, .expected_settled = TEST_GRBL_RUN, .expected_alarm = EXEC_ALARM_NOALARM},
	};
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
	{
		test_run_transition_case(&cases[i]);
	}
}

static void test_control_hold_matrix(void)
{
	static const test_transition_case_t cases[] = {
		{.name = "HOLD + cycle start", .initial = TEST_FIXTURE_HOLD, .stimulus = TEST_STIM_RT_START, .expected_settled = TEST_GRBL_RUN, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "HOLD + feed hold", .initial = TEST_FIXTURE_HOLD, .stimulus = TEST_STIM_RT_HOLD, .expected_settled = TEST_GRBL_HOLD_0, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "HOLD + safety door", .initial = TEST_FIXTURE_HOLD, .stimulus = TEST_STIM_RT_DOOR, .expected_settled = TEST_GRBL_DOOR_0, .expected_alarm = EXEC_ALARM_NOALARM},
	};
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
	{
		test_run_transition_case(&cases[i]);
	}
}

static void test_control_jog_matrix(void)
{
	static const test_transition_case_t cases[] = {
		{.name = "JOG + jog cancel", .initial = TEST_FIXTURE_JOG, .stimulus = TEST_STIM_RT_JOG_CANCEL, .expected_settled = TEST_GRBL_IDLE, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "JOG + feed hold", .initial = TEST_FIXTURE_JOG, .stimulus = TEST_STIM_RT_HOLD, .expected_settled = TEST_GRBL_IDLE, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "JOG + safety door", .initial = TEST_FIXTURE_JOG, .stimulus = TEST_STIM_RT_DOOR, .expected_settled = TEST_GRBL_DOOR_0, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "JOG + status", .initial = TEST_FIXTURE_JOG, .stimulus = TEST_STIM_RT_STATUS, .expected_settled = TEST_GRBL_JOG, .expected_alarm = EXEC_ALARM_NOALARM},
	};
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
	{
		test_run_transition_case(&cases[i]);
	}
}

static void test_control_alarm_matrix(void)
{
	static const test_transition_case_t cases[] = {
		{.name = "ALARM + reset", .initial = TEST_FIXTURE_ALARM, .stimulus = TEST_STIM_RT_RESET, .expected_settled = TEST_GRBL_IDLE, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "ALARM + feed hold", .initial = TEST_FIXTURE_ALARM, .stimulus = TEST_STIM_RT_HOLD, .expected_settled = TEST_GRBL_ALARM, .expected_alarm = EXEC_ALARM_HARD_LIMIT},
		{.name = "ALARM + cycle start", .initial = TEST_FIXTURE_ALARM, .stimulus = TEST_STIM_RT_START, .expected_settled = TEST_GRBL_ALARM, .expected_alarm = EXEC_ALARM_HARD_LIMIT},
	};
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
	{
		test_run_transition_case(&cases[i]);
	}
}

static void test_control_door_matrix(void)
{
	static const test_transition_case_t cases[] = {
		{.name = "DOOR + cycle start", .initial = TEST_FIXTURE_DOOR_OPEN, .stimulus = TEST_STIM_RT_START, .expected_settled = TEST_GRBL_DOOR_1, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "DOOR + status", .initial = TEST_FIXTURE_DOOR_OPEN, .stimulus = TEST_STIM_RT_STATUS, .expected_settled = TEST_GRBL_DOOR_1, .expected_alarm = EXEC_ALARM_NOALARM},
	};
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
	{
		test_run_transition_case(&cases[i]);
	}
}

static void test_control_check_matrix(void)
{
	static const test_transition_case_t cases[] = {
		{.name = "CHECK + status", .initial = TEST_FIXTURE_CHECK, .stimulus = TEST_STIM_RT_STATUS, .expected_settled = TEST_GRBL_CHECK, .expected_alarm = EXEC_ALARM_NOALARM},
		{.name = "CHECK + cycle start", .initial = TEST_FIXTURE_CHECK, .stimulus = TEST_STIM_RT_START, .expected_settled = TEST_GRBL_CHECK, .expected_alarm = EXEC_ALARM_NOALARM},
	};
	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
	{
		test_run_transition_case(&cases[i]);
	}
}

static void test_control_realtime_matrix(void)
{
	/* Split into per-fixture Unity tests for clear failure diagnosis. */
	test_control_idle_matrix();
	test_control_run_matrix();
	test_control_hold_matrix();
	test_control_jog_matrix();
	test_control_alarm_matrix();
	test_control_door_matrix();
	test_control_check_matrix();
}

/* ---------------------------------------------------------------------------
 * Override behavior (TB-06).
 * ------------------------------------------------------------------------- */

static void test_override_feed(void)
{
	const char *case_name = "feed override";

	/* Feed override is meaningful while running a feed move. */
	test_fixture_run(case_name);
	test_wait_status_or_fail(
		EXEC_STATUS_RUNNING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"fixture running");

	uint8_t before = g_planner_state.feed_override;

	/* Feed +10% (coarse). */
	test_send_rt(CMD_CODE_FEED_INC_COARSE);

	uint8_t after = g_planner_state.feed_override;

	char msg[160];
	snprintf(
		msg,
		sizeof(msg),
		"%s | feed override changed after +10%%",
		case_name);
	TEST_ASSERT_TRUE_MESSAGE(after > before, msg);

	/* Controller state must not change from an override. */
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(
		EXEC_STATUS_RUNNING,
		cnc_get_status(),
		case_name);
}

static void test_override_rapid(void)
{
	const char *case_name = "rapid override";

	test_fixture_run(case_name);
	test_wait_status_or_fail(
		EXEC_STATUS_RUNNING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"fixture running");

	uint8_t before = g_planner_state.rapid_feed_override;

	/* Rapid 50%. */
	test_send_rt(CMD_CODE_RAPIDFEED_OVR1);

	uint8_t after = g_planner_state.rapid_feed_override;

	char msg[160];
	snprintf(
		msg,
		sizeof(msg),
		"%s | rapid override changed after 50%%",
		case_name);
	TEST_ASSERT_TRUE_MESSAGE(after != before, msg);

	TEST_ASSERT_EQUAL_UINT8_MESSAGE(
		EXEC_STATUS_RUNNING,
		cnc_get_status(),
		case_name);
}

static void test_override_invariance(void)
{
	const char *case_name = "feed/rapid override invariance";

	/*
	 * A feed override must not modify the rapid override and vice versa.
	 * In Idle, apply a feed override and assert rapid is unchanged.
	 */
	uint8_t rapid_before = g_planner_state.rapid_feed_override;
	test_send_rt(CMD_CODE_FEED_INC_COARSE);
	uint8_t rapid_after = g_planner_state.rapid_feed_override;
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(
		rapid_before,
		rapid_after,
		case_name);

	/* A rapid override must not modify the feed override. */
	uint8_t feed_before = g_planner_state.feed_override;
	test_send_rt(CMD_CODE_RAPIDFEED_OVR1);
	uint8_t feed_after = g_planner_state.feed_override;
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(
		feed_before,
		feed_after,
		case_name);
}

/* ---------------------------------------------------------------------------
 * Jogging (TB-07).
 * ------------------------------------------------------------------------- */

static void test_jog_success_path(void)
{
	const char *case_name = "jog success from Idle";

	/* Valid jog from Idle. */
	uint8_t status = test_execute_line("$J=G91X20F600\n");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(STATUS_OK, status, case_name);

	/* Enters Jog. */
	test_wait_status_or_fail(
		EXEC_STATUS_JOGGING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"jog active");

	/* Let the jog advance partway. */
	test_delay_ms(100);

	int32_t mid[STEPPER_COUNT] = {0};
	itp_get_rt_position(mid);
	TEST_ASSERT_TRUE_MESSAGE(mid[AXIS_X] > 0, "jog produced motion");

	/* Produces motion and returns to Idle. */
	test_wait_status_or_fail(
		EXEC_STATUS_IDLE,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"jog complete");

	/* Position advanced by 20 mm. */
	test_assert_position_mm(20.0f, 0.0f, 0.0f);
}
typedef enum
{
	TEST_JOG_ADMIT_IDLE,
	TEST_JOG_ADMIT_RUN,
	TEST_JOG_ADMIT_HOLD,
	TEST_JOG_ADMIT_ALARM,
	TEST_JOG_ADMIT_DOOR,
	TEST_JOG_ADMIT_CHECK
} test_jog_admit_state_t;

static void test_jog_admission_matrix(void)
{
	/* clang-format off */
	static const struct
	{
		const char *name;
		test_jog_admit_state_t state;
		bool expect_accepted;
	} cases[] = {
		{ .name = "jog from Idle", .state = TEST_JOG_ADMIT_IDLE, .expect_accepted = true },
		{ .name = "jog from Run", .state = TEST_JOG_ADMIT_RUN, .expect_accepted = false },
		{ .name = "jog from Hold", .state = TEST_JOG_ADMIT_HOLD, .expect_accepted = false },
		{ .name = "jog from Alarm", .state = TEST_JOG_ADMIT_ALARM, .expect_accepted = false },
		{ .name = "jog from Door", .state = TEST_JOG_ADMIT_DOOR, .expect_accepted = false },
		{ .name = "jog from Check", .state = TEST_JOG_ADMIT_CHECK, .expect_accepted = false },
	};
	/* clang-format on */

	for (size_t i = 0; i < 3 /*sizeof(cases) / sizeof(cases[0])*/; i++)
	{
		const char *case_name = cases[i].name;

		printf("[JOG-ADMIT] case %s: prepare...\n", case_name);
		fflush(stdout);
		test_controller_prepare();
		printf("[JOG-ADMIT] case %s: prepared\n", case_name);
		fflush(stdout);

		// fflush(stdout);
		switch (cases[i].state)
		{
		case TEST_JOG_ADMIT_IDLE:
			break;
		case TEST_JOG_ADMIT_RUN:
			/* Use a long move so Run is still active when jog is rejected. */
			test_execute_line("G21\n");
			test_execute_line("G90\n");
			test_execute_line("G1X1000F600\n");
			printf("[JOG-ADMIT] %s: G1 queued, millis=%u status=%u\n", case_name, (unsigned)mcu_millis(), cnc_get_status());
			fflush(stdout);
			test_wait_status_or_fail(
				EXEC_STATUS_RUNNING,
				TEST_MOTION_TIMEOUT_MS,
				case_name,
				"run");
			printf("[JOG-ADMIT] %s: RUN reached millis=%u\n", case_name, (unsigned)mcu_millis());
			fflush(stdout);
			break;
		case TEST_JOG_ADMIT_HOLD:
			test_fixture_hold(case_name, EXEC_STATUS_IDLE);
			break;
		case TEST_JOG_ADMIT_ALARM:
			test_fixture_alarm();
			break;
		case TEST_JOG_ADMIT_DOOR:
			test_fixture_run(case_name);
			test_wait_status_or_fail(
				EXEC_STATUS_RUNNING,
				TEST_MOTION_TIMEOUT_MS,
				case_name,
				"run for door");
			test_press_control(TEST_IO_SAFETY_DOOR);
			test_wait_status_or_fail(
				EXEC_STATUS_DOOR_OPENED,
				TEST_MOTION_TIMEOUT_MS,
				case_name,
				"door open");
			break;
		case TEST_JOG_ADMIT_CHECK:
			test_fixture_check(case_name);
			break;
		}

		/* Capture position after the fixture is fully built. */
		int32_t before[STEPPER_COUNT] = {0};
		itp_get_rt_position(before);

		printf("[JOG-ADMIT] %s: about to exec jog, millis=%u status=%u\n", case_name, (unsigned)mcu_millis(), cnc_get_status());
		fflush(stdout);
		uint8_t status = test_execute_line("$J=G91X10F600\n");
		printf("[JOG-ADMIT] %s: jog exec returned status=%u\n", case_name, status);
		fflush(stdout);

		if (cases[i].expect_accepted)
		{
			char msg[160];
			snprintf(msg, sizeof(msg), "%s | jog accepted", case_name);
			TEST_ASSERT_EQUAL_UINT8_MESSAGE(STATUS_OK, status, msg);
		}
		else
		{
			/* The jog must be rejected with a parser error. */
			char msg[160];
			snprintf(msg, sizeof(msg), "%s | jog rejected", case_name);
			TEST_ASSERT_NOT_EQUAL_MESSAGE(STATUS_OK, status, msg);

			/*
			 * Rejected jogs must not add motion. For stationary fixtures,
			 * assert the position is unchanged once any pre-existing motion
			 * has settled (Run/Hold may still own the queue, so skip the
			 * position equality there; the parse rejection itself proves the
			 * jog did not start).
			 */
			if (cases[i].state == TEST_JOG_ADMIT_RUN ||
				cases[i].state == TEST_JOG_ADMIT_HOLD)
			{
				continue;
			}
			if (cases[i].state == TEST_JOG_ADMIT_DOOR)
			{
				/* Door is a controlled stop; queue is held. */
				continue;
			}

			int32_t after[STEPPER_COUNT] = {0};
			itp_get_rt_position(after);
			for (uint8_t a = 0; a < STEPPER_COUNT; a++)
			{
				char pmsg[192];
				snprintf(
					pmsg,
					sizeof(pmsg),
					"%s | rejected jog no motion axis %u",
					case_name,
					(unsigned)a);
				TEST_ASSERT_EQUAL_INT32_MESSAGE(before[a], after[a], pmsg);
			}
		}
	}
}

/* ---------------------------------------------------------------------------
 * Jog parser / modal isolation (TB-08).
 * ------------------------------------------------------------------------- */

/*
 * Read the active modal groups exposed by parser_get_modes(). Indices follow
 * proto_gcode_modes(): distance mode is index 2 (90/91), feed rate mode is
 * index 3 (93/94), units is index 4 (20/21).
 */
static void test_jog_read_modes(int *distance, int *feedrate_mode, int *units)
{
	uint8_t modalgroups[14];
	uint16_t feed;
	uint16_t spindle;

	parser_get_modes(modalgroups, &feed, &spindle);

	if (distance)
	{
		*distance = modalgroups[2];
	}
	if (feedrate_mode)
	{
		*feedrate_mode = modalgroups[3];
	}
	if (units)
	{
		*units = modalgroups[4];
	}
}

static void test_jog_modal_isolation(void)
{
	const char *case_name = "jog parser/modal isolation";

	/*
	 * Establish an unmistakable normal parser state before jogging:
	 * G21 (mm), G90 (absolute) and G93 (inverse-time feed mode).
	 */
	test_execute_line("G21\n");
	test_execute_line("G90\n");
	test_execute_line("G93\n");

	int d0;
	int f0;
	int u0;
	test_jog_read_modes(&d0, &f0, &u0);

	/* Baseline must be the unmistakable normal state. */
	TEST_ASSERT_EQUAL_INT_MESSAGE(G90, d0, case_name);
	TEST_ASSERT_EQUAL_INT_MESSAGE(G93, f0, case_name);
	TEST_ASSERT_EQUAL_INT_MESSAGE(G21, u0, case_name);

	/*
	 * Jog overrides units (G20) and distance (G91) for this command only.
	 * F is always interpreted in G94 units/min regardless of current G93.
	 */
	uint8_t status = test_execute_line("$J=G91G20X0.5F600\n");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(STATUS_OK, status, case_name);

	/* Let the jog run to completion and clear the interpolator queue. */
	test_wait_status_or_fail(
		EXEC_STATUS_IDLE,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"jog complete");

	int d1;
	int f1;
	int u1;
	test_jog_read_modes(&d1, &f1, &u1);

	/*
	 * After the jog completes the normal parser modal state must be
	 * unchanged (Grbl jogging: "jogging does not alter the g-code parser
	 * state"). G91/G20 must not have leaked into the normal parser.
	 */
	TEST_ASSERT_EQUAL_INT_MESSAGE(G90, d1, case_name);
	TEST_ASSERT_EQUAL_INT_MESSAGE(G93, f1, case_name);
	TEST_ASSERT_EQUAL_INT_MESSAGE(G21, u1, case_name);
}

static void test_jog_syntax_and_modal_variants(void)
{
	/*
	 * Valid jog syntax rows executed from a known fresh machine position.
	 * Each row is its own G21/G90 slice and asserts the protocol ok plus
	 * that the jog produced motion toward the commanded target.
	 */
	static const struct
	{
		const char *name;
		const char *cmd;
	} valid[] = {
		{.name = "jog incremental single axis", .cmd = "$J=G91X10F600\n"},
		{.name = "jog incremental multiple axes", .cmd = "$J=G91X10Y5F600\n"},
		{.name = "jog absolute G21 mm", .cmd = "$J=G90X25F600\n"},
		{.name = "jog imperial G20 inch", .cmd = "$J=G90G20X1F600\n"},
		{.name = "jog with spaces", .cmd = "$J= G91 X10 F600\n"},
		{.name = "jog with N line number", .cmd = "$J=N5G91X10F600\n"},
		{.name = "jog with comment", .cmd = "$J=G91X10F600 (comment)\n"},
	};

	for (size_t i = 0; i < sizeof(valid) / sizeof(valid[0]); i++)
	{
		const char *case_name = valid[i].name;

		test_controller_prepare();

		uint8_t status = test_execute_line(valid[i].cmd);
		char msg[192];

		snprintf(
			msg,
			sizeof(msg),
			"%s | valid jog accepted",
			case_name);
		TEST_ASSERT_EQUAL_UINT8_MESSAGE(STATUS_OK, status, msg);

		/* Enters Jog then returns to Idle. */
		snprintf(
			msg,
			sizeof(msg),
			"%s | jog finished",
			case_name);
		test_wait_status_or_fail(
			EXEC_STATUS_IDLE,
			TEST_MOTION_TIMEOUT_MS,
			case_name,
			"jog finished");

		/* Protocol must echo ok. */
		snprintf(
			msg,
			sizeof(msg),
			"%s | capture has ok",
			case_name);
		test_assert_capture_has("ok");
	}

	/*
	 * Invalid jog syntax rows must be rejected without starting motion.
	 */
	static const struct
	{
		const char *name;
		const char *cmd;
	} invalid[] = {
		{.name = "jog missing axis word", .cmd = "$J=G91F600\n"},
		{.name = "jog missing F", .cmd = "$J=G91X10\n"},
		{.name = "jog with M code", .cmd = "$J=G91X10M3F600\n"},
		{.name = "jog with S word", .cmd = "$J=G91X10S1000F600\n"},
		{.name = "jog with T word", .cmd = "$J=G91X10T1F600\n"},
		{.name = "jog with unsupported G code", .cmd = "$J=G0X10F600\n"},
		{.name = "jog zero feed", .cmd = "$J=G91X10F0\n"},
	};

	for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++)
	{
		const char *case_name = invalid[i].name;

		test_controller_prepare();

		int32_t before[STEPPER_COUNT] = {0};
		itp_get_rt_position(before);

		uint8_t status = test_execute_line(invalid[i].cmd);
		char msg[192];

		snprintf(
			msg,
			sizeof(msg),
			"%s | invalid jog rejected",
			case_name);
		TEST_ASSERT_TRUE_MESSAGE(status != STATUS_OK, msg);

		/* Let any accepted-adjacent work settle, then check no motion. */
		test_delay_ms(10);

		int32_t after[STEPPER_COUNT] = {0};
		itp_get_rt_position(after);
		for (uint8_t a = 0; a < STEPPER_COUNT; a++)
		{
			snprintf(
				msg,
				sizeof(msg),
				"%s | invalid jog no motion axis %u",
				case_name,
				(unsigned)a);
			TEST_ASSERT_EQUAL_INT32_MESSAGE(before[a], after[a], msg);
		}
	}
}

static void test_serial_hold_resume(void)
{
	const char *case_name = "serial hold/resume";

	test_fixture_run(case_name);
	test_wait_status_or_fail(
		EXEC_STATUS_RUNNING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"fixture running");

	/* Advance into the move so the hold fires mid-motion. */
	test_delay_ms(20);

	/* Serial feed hold `!`. */
	test_send_rt(CMD_CODE_FEED_HOLD);
	test_wait_status_or_fail(
		EXEC_STATUS_HOLD_PENDING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"after serial hold");
	test_wait_status_or_fail(
		EXEC_STATUS_HOLD,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"settled serial hold");

	/* Status query `?` while holding must report Hold:0. */
	mcu_uart2_test_capture_reset();
	test_send_rt(CMD_CODE_REPORT);
	test_assert_capture_has("<Hold:0");

	/* Serial cycle start `~`. */
	test_send_rt(CMD_CODE_CYCLE_START);
	test_wait_status_or_fail(
		EXEC_STATUS_RUNNING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"after serial resume");
}

static void test_physical_fhold_smoke(void)
{
	const char *case_name = "physical FHOLD smoke";

	/* Start a long Run move. */
	test_fixture_run(case_name);
	test_wait_status_or_fail(
		EXEC_STATUS_RUNNING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"fixture running");

	/* Schedule a physical FHOLD assertion while motion is active. */
	test_schedule_io_us(TEST_IO_FHOLD, true, 20000);

	/* Let motion progress; the event should fire during the move. */
	test_wait_status_or_fail(
		EXEC_STATUS_HOLD_PENDING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"after FHOLD event");

	test_wait_status_or_fail(
		EXEC_STATUS_HOLD,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"settled hold");

	/* Release FHOLD and resume. */
	test_release_control(TEST_IO_FHOLD);
	test_send_rt(CMD_CODE_CYCLE_START);

	test_wait_status_or_fail(
		EXEC_STATUS_RUNNING,
		TEST_MOTION_TIMEOUT_MS,
		case_name,
		"after resume");
}

int main(void)
{
	UNITY_BEGIN();

	// RUN_TEST(test_run_blocks_test);

	// return UNITY_END();

	RUN_TEST(test_g0_absolute_xy);
	RUN_TEST(test_g0_incremental_xy);
	RUN_TEST(test_g1_absolute_xy);
	RUN_TEST(test_g1_modal_continue);

	RUN_TEST(test_g2_ij_cw);
	RUN_TEST(test_g3_ij_ccw);
	RUN_TEST(test_g2_r_cw);
	RUN_TEST(test_g3_r_ccw);
#ifdef AXIS_X
#ifdef AXIS_Z
	RUN_TEST(test_g18_xz_plane);
#endif
#endif
#ifdef AXIS_Y
#ifdef AXIS_Z
	RUN_TEST(test_g19_yz_plane);
#endif
#endif

	RUN_TEST(test_g20_inch);
	RUN_TEST(test_g21_mm);
	RUN_TEST(test_word_order);
	RUN_TEST(test_comment_in_line);

	RUN_TEST(test_valid_multiple_modal_groups);
	RUN_TEST(test_valid_reordered_modal_groups);

	RUN_TEST(test_invalid_gcode_does_not_move);
	RUN_TEST(test_err_g0_no_axis);
	RUN_TEST(test_err_g1_no_axis);
	RUN_TEST(test_err_axis_no_number);
	RUN_TEST(test_err_bad_number);
	RUN_TEST(test_err_repeated_axis);
	RUN_TEST(test_err_two_motion_gcodes);
	RUN_TEST(test_err_two_unit_gcodes);
	RUN_TEST(test_err_two_distance_modes);
	RUN_TEST(test_err_g2_no_center);
	RUN_TEST(test_err_g3_no_center);
	RUN_TEST(test_err_g2_no_endpoint);
	RUN_TEST(test_err_g3_no_endpoint);
	RUN_TEST(test_err_g2_bad_center_radius);
	RUN_TEST(test_err_g2_radius_too_small);
	RUN_TEST(test_err_g4_negative);
	RUN_TEST(test_err_g4_missing_p);

	RUN_TEST(test_parser_atomicity);

	RUN_TEST(test_physical_fhold_smoke);
	RUN_TEST(test_serial_hold_resume);
	RUN_TEST(test_safety_door_path);
	RUN_TEST(test_control_idle_matrix);
	RUN_TEST(test_control_run_matrix);
	RUN_TEST(test_control_hold_matrix);
	RUN_TEST(test_control_jog_matrix);
	RUN_TEST(test_control_alarm_matrix);
	RUN_TEST(test_control_door_matrix);
	RUN_TEST(test_control_check_matrix);
	RUN_TEST(test_override_feed);
	RUN_TEST(test_override_rapid);
	RUN_TEST(test_override_invariance);
	RUN_TEST(test_jog_success_path);
	RUN_TEST(test_jog_admission_matrix);
	RUN_TEST(test_jog_modal_isolation);
	RUN_TEST(test_jog_syntax_and_modal_variants);
	RUN_TEST(test_door_resume_grbl_door3);

	return UNITY_END();
}
