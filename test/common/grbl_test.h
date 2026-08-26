#ifndef GRBL_TEST_H
#define GRBL_TEST_H

#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <unity.h>
#include "src/cnc.h"

#define GRBL_TEST_TIMEOUT_MS 3000U
#define GRBL_TEST_TRANSCRIPT_SIZE 4096U

static char grbl_test_transcript[GRBL_TEST_TRANSCRIPT_SIZE];
static uint64_t grbl_test_output_cursor;

static size_t grbl_test_snapshot(void)
{
	return mcu_unit_test_buffer_read_since(grbl_test_output_cursor, grbl_test_transcript, sizeof(grbl_test_transcript));
}

static void grbl_test_step(void)
{
	if (!cnc_unit_test_run_once())
	{
		cnc_unit_test_start();
	}
}

static bool grbl_test_snapshot_contains(const char *needle)
{
	grbl_test_snapshot();
	return strstr(grbl_test_transcript, needle) != NULL;
}

static bool grbl_test_wait_for(const char *needle, uint32_t timeout_ms)
{
	uint32_t started = mcu_millis();
	do
	{
		if (grbl_test_snapshot_contains(needle))
		{
			return true;
		}
		grbl_test_step();
		mcu_unit_test_advance_time(5000U);
	} while ((uint32_t)(mcu_millis() - started) < timeout_ms);

	return false;
}

static void grbl_test_assert_wait_for(const char *needle)
{
	if (!grbl_test_wait_for(needle, GRBL_TEST_TIMEOUT_MS))
	{
		grbl_test_snapshot();
		TEST_FAIL_MESSAGE(grbl_test_transcript);
	}
	TEST_ASSERT_FALSE_MESSAGE(mcu_unit_test_buffer_overflowed(), "Serial test transcript overflowed");
}

static void grbl_test_clear_output(void)
{
	grbl_test_output_cursor = mcu_unit_test_output_cursor();
	grbl_test_transcript[0] = '\0';
}

static void grbl_test_inject(const char *bytes)
{
	TEST_ASSERT_TRUE_MESSAGE(mcu_unit_test_inject(bytes), "Unable to inject bytes into Grbl stream");
}

static void grbl_test_command_expect(const char *command, const char *expected)
{
	char line[192];
	int written = snprintf(line, sizeof(line), "%s\n", command);
	TEST_ASSERT_GREATER_THAN_INT_MESSAGE(0, written, "Unable to format test command");
	TEST_ASSERT_LESS_THAN_INT_MESSAGE((int)sizeof(line), written, "Test command is too long");

	grbl_test_clear_output();
	grbl_test_inject(line);
	if (!grbl_test_wait_for(expected, GRBL_TEST_TIMEOUT_MS))
	{
		char message[GRBL_TEST_TRANSCRIPT_SIZE + 256U];
		grbl_test_snapshot();
		snprintf(message, sizeof(message), "command `%s` expected `%s`, received: %s", command, expected, grbl_test_transcript);
		TEST_FAIL_MESSAGE(message);
	}
}

static __attribute__((unused)) void grbl_test_command_ok(const char *command)
{
	grbl_test_command_expect(command, "ok\r\n");
}

static __attribute__((unused)) void grbl_test_command_error(const char *command)
{
	grbl_test_command_expect(command, "error:");
}

static __attribute__((unused)) bool grbl_test_command_matches(const char *command, const char *expected)
{
	char line[192];
	int written = snprintf(line, sizeof(line), "%s\n", command);
	if (written <= 0 || written >= (int)sizeof(line))
	{
		return false;
	}

	grbl_test_clear_output();
	if (!mcu_unit_test_inject(line))
	{
		return false;
	}
	return grbl_test_wait_for(expected, 500U);
}

static __attribute__((unused)) void grbl_test_realtime_expect(const char *bytes, const char *expected)
{
	grbl_test_clear_output();
	grbl_test_inject(bytes);
	if (!grbl_test_wait_for(expected, GRBL_TEST_TIMEOUT_MS))
	{
		char message[GRBL_TEST_TRANSCRIPT_SIZE + 128U];
		grbl_test_snapshot();
		snprintf(message, sizeof(message), "realtime input expected `%s`, received: %s", expected, grbl_test_transcript);
		TEST_FAIL_MESSAGE(message);
	}
}

static __attribute__((unused)) void grbl_test_start(void)
{
	TEST_ASSERT_EQUAL_INT_MESSAGE(3, AXIS_COUNT, "Conformity fixtures require the default 3-axis build");
	static bool grbl_initialized;
	if (!grbl_initialized)
	{
		cnc_init();
		grbl_initialized = true;
	}
	// mcu_stop_itp_isr();
	cnc_stop(true);
	mc_clear(true);
	itp_clear();
	mcu_unit_test_runtime_reset();
	grbl_test_clear_output();
	settings_reset(true);
	parser_parameters_reset();

	// cnc_reset();

	// if (cnc_unlock(true) != UNLOCK_ERROR)
	// {
	// 	cnc_state.alarm = EXEC_ALARM_NOALARM;
	// }

	// cnc_state.loop_state = LOOP_RUNNING;

	/*
	 * Make every test start at machine step position zero.
	 */
	int32_t zero[STEPPER_COUNT] = {0};

	itp_sync_rt_position(zero);
	mc_sync_position();
	parser_sync_position();
	cnc_unit_test_start();
	grbl_test_assert_wait_for("for help]\r\n");
}

static __attribute__((unused)) void grbl_test_stop(void)
{
	cnc_stop(true);
}

static __attribute__((unused)) void grbl_test_wait_for_state(const char *state, uint32_t timeout_ms)
{
	uint32_t started = mcu_millis();
	do
	{
		grbl_test_clear_output();
		grbl_test_inject("?");
		if (grbl_test_wait_for(state, 100U))
		{
			return;
		}
		sched_yield();
	} while ((uint32_t)(mcu_millis() - started) < timeout_ms);

	grbl_test_snapshot();
	char message[GRBL_TEST_TRANSCRIPT_SIZE + 64U];
	snprintf(message, sizeof(message), "Machine did not reach `%s`; last report: %s", state, grbl_test_transcript);
	TEST_FAIL_MESSAGE(message);
}

static __attribute__((unused)) void grbl_test_wait_idle(void)
{
	grbl_test_wait_for_state("<Idle", GRBL_TEST_TIMEOUT_MS);
}

#define GRBL_PROCESS_FIXTURE(test_function) \
	int main(void)                          \
	{                                       \
		UNITY_BEGIN();                      \
		RUN_TEST(test_function);            \
		return UNITY_END();                 \
	}

#endif
