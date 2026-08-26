#ifndef DOMAIN1_TEST_H
#define DOMAIN1_TEST_H

#include "grbl_test.h"

typedef struct
{
	const char *id;
	const char *command;
	const char *expected;
	bool ignore;
} d1_case_t;

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
	grbl_test_snapshot();
	return d1_terminal_response_matches(grbl_test_transcript, expected);
}

static bool d1_enter_check(void)
{
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

static bool d1_get_modal(char *destination, size_t capacity)
{
	grbl_test_clear_output();
	if (!mcu_unit_test_inject("$G\n") || !grbl_test_wait_for("ok\r\n", 1000U))
	{
		return false;
	}
	mcu_unit_test_buffer_read_since(grbl_test_output_cursor, destination, capacity);
	return true;
}

static const d1_case_t *d1_current_case;

static __attribute__((unused)) void d1_run_current_case(void)
{
	char modal_before[256] = "";
	char modal_after[256] = "";
	bool rejection_case = !strncmp(d1_current_case->expected, "error:", 6U);
	if (d1_current_case->ignore)
	{
		TEST_IGNORE_MESSAGE("Case explicitly disabled");
	}

	TEST_ASSERT_TRUE_MESSAGE(d1_enter_check(), "could not establish Check state");
	if (rejection_case)
	{
		TEST_ASSERT_TRUE_MESSAGE(d1_get_modal(modal_before, sizeof(modal_before)), "could not capture initial modal state");
	}

	if (!d1_send_line(d1_current_case->command, d1_current_case->expected))
	{
		char reason[GRBL_TEST_TRANSCRIPT_SIZE + 256U];
		snprintf(reason, sizeof(reason), "`%s` expected `%s`, received `%s`", d1_current_case->command, d1_current_case->expected, grbl_test_transcript);
		TEST_FAIL_MESSAGE(reason);
	}

	if (rejection_case)
	{
		TEST_ASSERT_TRUE_MESSAGE(d1_get_modal(modal_after, sizeof(modal_after)), "could not capture final modal state");
		TEST_ASSERT_EQUAL_STRING_MESSAGE(modal_before, modal_after, "rejected command altered parser modal state");
	}
	TEST_ASSERT_TRUE_MESSAGE(d1_still_in_check(), "controller left Check state");
}

#define D1_CASE(id, command, expected, ignore) {id, command, expected, ignore}
#define D1_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define D1_CASE_FIXTURE(cases)                                      \
	int main(void)                                                  \
	{                                                               \
		UNITY_BEGIN();                                               \
		for (size_t i = 0; i < D1_COUNT(cases); ++i)                \
		{                                                           \
			d1_current_case = &(cases)[i];                           \
			UnityDefaultTestRun(d1_run_current_case, (cases)[i].id, __LINE__); \
		}                                                           \
		return UNITY_END();                                         \
	}

#endif
