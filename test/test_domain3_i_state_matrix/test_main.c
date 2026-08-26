#include "../common/domain3_test.h"

static void test_idle_acceptance(void)
{
	d3_idle();
	d3_expect_command("G0X0", "ok\r\n");
	d3_expect_command("$$", "ok\r\n"); d3_expect_command("$#", "ok\r\n");
	d3_expect_command("$G", "ok\r\n"); d3_expect_command("$I", "ok\r\n"); d3_expect_command("$N", "ok\r\n");
	d3_expect_command("$X", "ok\r\n");
	d3_expect_command("$J=G91X1F300", "ok\r\n"); d3_expect_state("<Jog", D3_TIMEOUT_MS);
	d3_realtime(0x85); d3_expect_state("<Idle", D3_TIMEOUT_MS);
}

static void test_run_acceptance_matrix(void)
{
	d3_idle(); d3_start_long_motion();
	d3_expect_command("G1X4F300", "ok\r\n");
	d3_expect_command("$$", "error:8\r\n"); d3_expect_command("$#", "error:8\r\n");
	d3_expect_command("$H", "error:8\r\n"); d3_expect_command("$SLP", "error:8\r\n"); d3_expect_command("$C", "error:8\r\n");
	d3_expect_command("$J=G91X1F300", "error:8\r\n");
	d3_expect_command("$G", "ok\r\n");
	d3_cancel_motion();
}

static void test_hold_acceptance_matrix(void)
{
	d3_reset();d3_idle(); d3_start_long_motion(); d3_realtime('!'); d3_expect_state("<Hold:0", D3_MOTION_TIMEOUT_MS);
	d3_expect_command("G1X4F300", "ok\r\n");
	d3_expect_command("$J=G91X1F300", "error:8\r\n");
	d3_expect_command("$G", "ok\r\n");
	char status[512]; TEST_ASSERT_TRUE(d3_status(status, sizeof(status))); TEST_ASSERT_NOT_NULL(strstr(status, "<Hold"));
	d3_cancel_motion();
}

static void test_jog_acceptance_matrix(void)
{
	d3_idle(); d3_expect_command("$J=G91X5F60", "ok\r\n"); d3_expect_state("<Jog", D3_TIMEOUT_MS);
	d3_expect_command("G0X0", "error:9\r\n");
	d3_expect_command("$J=G91X1F60", "ok\r\n");
	d3_realtime(0x85); d3_expect_state("<Idle", D3_MOTION_TIMEOUT_MS);
}

static void test_alarm_acceptance_matrix(void)
{
	d3_idle(); d3_expect_command("$21=1", "ok\r\n");
	test_io_set(TEST_IO_LIMIT_X, true); grbl_test_assert_wait_for("ALARM:1"); d3_expect_state("<Alarm", D3_TIMEOUT_MS);
	d3_expect_command("G0X0", "error:9\r\n");
	d3_expect_command("$G", "ok\r\n"); d3_expect_command("$#", "ok\r\n"); d3_expect_command("$I", "ok\r\n");
	test_io_set(TEST_IO_LIMIT_X, false); d3_expect_command("$X", "ok\r\n"); d3_expect_state("<Idle", D3_TIMEOUT_MS);
}

static void test_check_acceptance_matrix(void)
{
	d3_idle(); d3_expect_command("$C", "ok\r\n"); d3_expect_state("<Check", D3_TIMEOUT_MS);
	d3_expect_command("G1X1F300", "ok\r\n");
	d3_expect_command("$J=G91X1F300", "error:8\r\n");
	d3_expect_command("$G", "ok\r\n");
	d3_reset();
}

static void test_pin_state_reporting(void)
{
	d3_idle();
	test_io_set(TEST_IO_PROBE, true);
	char status[512]; TEST_ASSERT_TRUE(d3_status(status, sizeof(status)));
	TEST_ASSERT_NOT_NULL_MESSAGE(strstr(status, "Pn:P"), status);
	test_io_set(TEST_IO_PROBE, false);
}

D3_UNITY_MAIN(
	RUN_TEST(test_idle_acceptance);
	RUN_TEST(test_run_acceptance_matrix);
	RUN_TEST(test_hold_acceptance_matrix);
	RUN_TEST(test_jog_acceptance_matrix);
	RUN_TEST(test_alarm_acceptance_matrix);
	RUN_TEST(test_check_acceptance_matrix);
	RUN_TEST(test_pin_state_reporting))
