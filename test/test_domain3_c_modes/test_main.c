#include "../common/domain3_test.h"

static void test_check_mode_parses_without_execution(void)
{
	d3_idle();
	d3_expect_command("$C", "ok\r\n"); TEST_ASSERT_TRUE(d3_contains("[MSG:Enabled]"));
	d3_expect_state("<Check", D3_TIMEOUT_MS);
	d3_expect_command("G91G1X5F300", "ok\r\n");
	char status[512]; TEST_ASSERT_TRUE(d3_status(status, sizeof(status)));
	TEST_ASSERT_NOT_NULL(strstr(status, "<Check")); TEST_ASSERT_NOT_NULL(strstr(status, "MPos:0.000,0.000,0.000"));
	d3_expect_command("G1X1F-1", "error:4\r\n");
	d3_reset(); d3_expect_state("<Idle", D3_TIMEOUT_MS);
}

static void test_unlock_idle_and_alarm(void)
{
	d3_idle();
	d3_expect_command("$X", "ok\r\n");
	d3_expect_command("$21=1", "ok\r\n");
	test_io_set(TEST_IO_LIMIT_X, true);
	grbl_test_clear_output(); grbl_test_assert_wait_for("ALARM:1");
	d3_expect_state("<Alarm", D3_TIMEOUT_MS);
	test_io_set(TEST_IO_LIMIT_X, false);
	d3_expect_command("$X", "ok\r\n"); TEST_ASSERT_TRUE(d3_contains("[MSG:Caution:"));
	d3_expect_state("<Idle", D3_TIMEOUT_MS);
}

static void test_sleep_requires_reset(void)
{
	d3_idle();
	d3_expect_command("$SLP", "ok\r\n");
	d3_expect_state("<Sleep", D3_TIMEOUT_MS);
	d3_expect_command("G0X1", "error:9\r\n");
	d3_reset();
	TEST_ASSERT_TRUE(d3_wait_state("<Alarm", D3_TIMEOUT_MS) || d3_wait_state("<Idle", D3_TIMEOUT_MS));
}

static void test_startup_blocks_store_report_and_reject_invalid(void)
{
	d3_idle();
	d3_expect_command("$N0=G21G90", "ok\r\n");
	d3_expect_command("$N1=G17G94", "ok\r\n");
	d3_expect_command("$N", "ok\r\n");
	TEST_ASSERT_NOT_NULL(strstr(grbl_test_transcript, "$N0=G21G90"));
	TEST_ASSERT_NOT_NULL(strstr(grbl_test_transcript, "$N1=G17G94"));
	d3_expect_command("$N0=G1X1", "error:");
	d3_expect_command("$N", "ok\r\n");
	TEST_ASSERT_NOT_NULL(strstr(grbl_test_transcript, "$N0=G21G90"));
}

D3_UNITY_MAIN(
	RUN_TEST(test_check_mode_parses_without_execution);
	RUN_TEST(test_unlock_idle_and_alarm);
	RUN_TEST(test_sleep_requires_reset);
	RUN_TEST(test_startup_blocks_store_report_and_reject_invalid))
