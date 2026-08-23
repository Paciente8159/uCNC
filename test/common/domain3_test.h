#ifndef DOMAIN3_TEST_H
#define DOMAIN3_TEST_H

#include "grbl_test.h"
#include <stdlib.h>

#define D3_TIMEOUT_MS 3000U
#define D3_MOTION_TIMEOUT_MS 15000U

static volatile bool d3_running;

void setUp(void) {}
void tearDown(void) {}

static void *d3_controller(void *unused)
{
	(void)unused;
	do
	{
		cnc_run();
	} while (d3_running);
	return NULL;
}

static void d3_start(void)
{
	TEST_ASSERT_EQUAL_INT_MESSAGE(3, AXIS_COUNT, "Domain 3 requires the default three-axis build");
	test_io_reset();
	cnc_init();
	grbl_test_clear_output();
	d3_running = true;
	TEST_ASSERT_EQUAL_INT(0, pthread_create(&grbl_test_thread, NULL, d3_controller, NULL));
	grbl_test_thread_started = true;
	grbl_test_assert_wait_for("Grbl ");
}

static void d3_stop(void)
{
	if (!grbl_test_thread_started) return;
	d3_running = false;
	const char reset[] = {0x18, '\0'};
	mcu_unit_test_inject(reset);
	pthread_join(grbl_test_thread, NULL);
	grbl_test_thread_started = false;
}

static bool d3_final_line_matches(const char *text, const char *expected)
{
	const char *end = text + strlen(text);
	while (end > text && (end[-1] == '\r' || end[-1] == '\n')) --end;
	const char *start = end;
	while (start > text && start[-1] != '\n') --start;
	size_t actual = (size_t)(end - start);
	if (!strcmp(expected, "error:")) return actual >= 6U && !strncmp(start, "error:", 6U);
	size_t wanted = strlen(expected);
	while (wanted && (expected[wanted - 1U] == '\r' || expected[wanted - 1U] == '\n')) --wanted;
	return actual == wanted && !strncmp(start, expected, wanted);
}

static bool d3_command(const char *command, const char *terminal, uint32_t timeout_ms)
{
	char line[192];
	int length = snprintf(line, sizeof(line), "%s\n", command);
	if (length <= 0 || length >= (int)sizeof(line)) return false;
	grbl_test_clear_output();
	if (!mcu_unit_test_inject(line)) return false;
	const char *wait = !strcmp(terminal, "error:") ? "error:" : terminal;
	if (!grbl_test_wait_for(wait, timeout_ms)) return false;
	mcu_unit_test_buffer_read(grbl_test_transcript, sizeof(grbl_test_transcript));
	return d3_final_line_matches(grbl_test_transcript, terminal);
}

static void d3_expect_command(const char *command, const char *terminal)
{
	if (!d3_command(command, terminal, D3_TIMEOUT_MS))
	{
		char message[1024];
		mcu_unit_test_buffer_read(grbl_test_transcript, sizeof(grbl_test_transcript));
		snprintf(message, sizeof(message), "`%s` expected terminal `%s`, received `%s`", command, terminal, grbl_test_transcript);
		TEST_FAIL_MESSAGE(message);
	}
}

static bool d3_status(char *destination, size_t capacity)
{
	grbl_test_clear_output();
	if (!mcu_unit_test_inject("?") || !grbl_test_wait_for(">", 500U)) return false;
	mcu_unit_test_buffer_read(destination, capacity);
	return strchr(destination, '<') && strchr(destination, '>');
}

static bool d3_wait_state(const char *state, uint32_t timeout_ms)
{
	uint32_t start = mcu_millis();
	char status[512];
	do
	{
		if (d3_status(status, sizeof(status)) && strstr(status, state)) return true;
		sched_yield();
	} while ((uint32_t)(mcu_millis() - start) < timeout_ms);
	strncpy(grbl_test_transcript, status, sizeof(grbl_test_transcript) - 1U);
	return false;
}

static void d3_expect_state(const char *state, uint32_t timeout_ms)
{
	if (!d3_wait_state(state, timeout_ms))
	{
		char message[768];
		snprintf(message, sizeof(message), "expected state `%s`, last status `%s`", state, grbl_test_transcript);
		TEST_FAIL_MESSAGE(message);
	}
}

static __attribute__((unused)) bool d3_wait_either_state(const char *first, const char *second, uint32_t timeout_ms)
{
	uint32_t start = mcu_millis();
	char status[512];
	do
	{
		if (d3_status(status, sizeof(status)) && (strstr(status, first) || strstr(status, second))) return true;
		sched_yield();
	} while ((uint32_t)(mcu_millis() - start) < timeout_ms);
	return false;
}

static void d3_realtime(uint8_t byte)
{
	char input[2] = {(char)byte, '\0'};
	TEST_ASSERT_TRUE(mcu_unit_test_inject(input));
}

static void d3_reset(void)
{
	grbl_test_clear_output();
	d3_realtime(0x18);
	grbl_test_assert_wait_for("Grbl ");
}

static void d3_idle(void)
{
	d3_reset();
	d3_expect_command("$X", "ok\r\n");
	d3_expect_command("$20=0", "ok\r\n");
	d3_expect_command("G21G90G17G94G54G49G92.1M5M9", "ok\r\n");
	d3_expect_state("<Idle", D3_TIMEOUT_MS);
}

static __attribute__((unused)) bool d3_contains(const char *needle)
{
	return strstr(grbl_test_transcript, needle) != NULL;
}

static __attribute__((unused)) void d3_start_long_motion(void)
{
	d3_expect_command("G91G1X20F300", "ok\r\n");
	d3_expect_state("<Run", D3_TIMEOUT_MS);
}

static __attribute__((unused)) void d3_cancel_motion(void)
{
	d3_realtime(0x18);
	grbl_test_assert_wait_for("Grbl ");
}

static __attribute__((unused)) void d3_expect_no_serial_response(uint8_t byte, uint32_t observation_ms)
{
	grbl_test_clear_output();
	d3_realtime(byte);
	uint32_t start = mcu_millis();
	while ((uint32_t)(mcu_millis() - start) < observation_ms) sched_yield();
	mcu_unit_test_buffer_read(grbl_test_transcript, sizeof(grbl_test_transcript));
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", grbl_test_transcript, "realtime command unexpectedly emitted a serial response");
}

#define D3_MAIN(test_function)       \
	int main(void)                    \
	{                                 \
		d3_start();                   \
		UNITY_BEGIN();                 \
		RUN_TEST(test_function);       \
		int result = UNITY_END();      \
		d3_stop();                    \
		return result;                 \
	}

#define D3_UNITY_MAIN(...)           \
	int main(void)                    \
	{                                 \
		d3_start();                   \
		UNITY_BEGIN();                 \
		__VA_ARGS__;                   \
		int result = UNITY_END();      \
		d3_stop();                    \
		return result;                 \
	}

#endif
