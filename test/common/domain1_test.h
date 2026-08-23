#ifndef DOMAIN1_TEST_H
#define DOMAIN1_TEST_H

#include "grbl_test.h"

typedef struct
{
	const char *id;
	const char *command;
	const char *expected;
} d1_case_t;

static volatile bool d1_keep_running;

static void *d1_controller_loop(void *argument)
{
	(void)argument;
	do
	{
		cnc_run();
	} while (d1_keep_running);
	return NULL;
}

static void d1_suite_start(void)
{
	TEST_ASSERT_EQUAL_INT(3, AXIS_COUNT);
	test_io_reset();
	cnc_init();
	grbl_test_clear_output();
	d1_keep_running = true;
	TEST_ASSERT_EQUAL_INT(0, pthread_create(&grbl_test_thread, NULL, d1_controller_loop, NULL));
	grbl_test_thread_started = true;
	grbl_test_assert_wait_for("for help]\r\n");
}

static void d1_suite_stop(void)
{
	if (!grbl_test_thread_started)
	{
		return;
	}
	d1_keep_running = false;
	const char reset[] = {0x18, '\0'};
	mcu_unit_test_inject(reset);
	pthread_join(grbl_test_thread, NULL);
	grbl_test_thread_started = false;
}

static bool d1_terminal_response_matches(const char *transcript, const char *expected)
{
	const char *end = transcript + strlen(transcript);
	while (end > transcript && (end[-1] == '\r' || end[-1] == '\n'))
	{
		--end;
	}

	const char *start = end;
	while (start > transcript && start[-1] != '\n')
	{
		--start;
	}

	size_t actual_length = (size_t)(end - start);
	if (!strcmp(expected, "error:"))
	{
		return actual_length >= 6U && !strncmp(start, "error:", 6U);
	}

	size_t expected_length = strlen(expected);
	while (expected_length && (expected[expected_length - 1U] == '\r' || expected[expected_length - 1U] == '\n'))
	{
		--expected_length;
	}
	return actual_length == expected_length && !strncmp(start, expected, expected_length);
}

static bool d1_send_line(const char *command, const char *expected)
{
	char line[192];
	int length = snprintf(line, sizeof(line), "%s\n", command);
	if (length <= 0 || length >= (int)sizeof(line))
	{
		return false;
	}

	grbl_test_clear_output();
	const char *wait_for = !strcmp(expected, "error:") ? "error:" : expected;
	if (!mcu_unit_test_inject(line) || !grbl_test_wait_for(wait_for, 1000U))
	{
		return false;
	}
	mcu_unit_test_buffer_read(grbl_test_transcript, sizeof(grbl_test_transcript));
	return d1_terminal_response_matches(grbl_test_transcript, expected);
}

static bool d1_reset_and_enter_check(void)
{
	const char reset[] = {0x18, '\0'};
	grbl_test_clear_output();
	if (!mcu_unit_test_inject(reset) || !grbl_test_wait_for("for help]\r\n", 1000U))
	{
		return false;
	}
	if (!d1_send_line("$C", "ok\r\n") || !strstr(grbl_test_transcript, "[MSG:Enabled]\r\n"))
	{
		return false;
	}
	grbl_test_clear_output();
	if (!mcu_unit_test_inject("?") || !grbl_test_wait_for(">\r\n", 1000U) || !grbl_test_snapshot_contains("<Check"))
	{
		return false;
	}
	return true;
}

static bool d1_still_in_check(void)
{
	grbl_test_clear_output();
	return mcu_unit_test_inject("?") && grbl_test_wait_for(">\r\n", 1000U) && grbl_test_snapshot_contains("<Check");
}

static void d1_append_failure(char *failures, size_t capacity, const char *id, const char *reason)
{
	size_t used = strlen(failures);
	if (used < capacity)
	{
		snprintf(failures + used, capacity - used, "%s: %s; ", id, reason);
	}
}

static bool d1_get_modal(char *destination, size_t capacity)
{
	grbl_test_clear_output();
	if (!mcu_unit_test_inject("$G\n") || !grbl_test_wait_for("ok\r\n", 1000U))
	{
		return false;
	}
	mcu_unit_test_buffer_read(destination, capacity);
	return true;
}

static void d1_run_cases(const d1_case_t *cases, size_t count)
{
	char failures[8192] = "";
	for (size_t i = 0; i < count; ++i)
	{
		char modal_before[256] = "";
		char modal_after[256] = "";
		bool rejection_case = !strncmp(cases[i].expected, "error:", 6U);
		if (!d1_reset_and_enter_check())
		{
			d1_append_failure(failures, sizeof(failures), cases[i].id, "could not establish Check state");
			continue;
		}
		if (rejection_case && !d1_get_modal(modal_before, sizeof(modal_before)))
		{
			d1_append_failure(failures, sizeof(failures), cases[i].id, "could not capture initial modal state");
			continue;
		}
		if (!d1_send_line(cases[i].command, cases[i].expected))
		{
			char reason[512];
			snprintf(reason, sizeof(reason), "`%s` expected `%s`, received `%s`", cases[i].command, cases[i].expected, grbl_test_transcript);
			d1_append_failure(failures, sizeof(failures), cases[i].id, reason);
		}
		if (rejection_case)
		{
			if (!d1_get_modal(modal_after, sizeof(modal_after)) || strcmp(modal_before, modal_after))
			{
				d1_append_failure(failures, sizeof(failures), cases[i].id, "rejected command altered parser modal state");
			}
		}
		if (!d1_still_in_check())
		{
			d1_append_failure(failures, sizeof(failures), cases[i].id, "controller left Check state");
		}
	}
	d1_suite_stop();
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", failures, "Domain 1 conformance failures");
}

#define D1_CASE(id, command, expected) {id, command, expected}
#define D1_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define D1_FIXTURE(test_function)       \
	int main(void)                    \
	{                                 \
		d1_suite_start();           \
		UNITY_BEGIN();               \
		RUN_TEST(test_function);     \
		int result = UNITY_END();    \
		d1_suite_stop();            \
		return result;               \
	}

#endif
