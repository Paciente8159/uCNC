#include "../common/domain3_test.h"

static void test_reset_help_and_queries(void)
{
	d3_reset();
	TEST_ASSERT_TRUE(d3_contains("Grbl "));
	d3_expect_command("$", "ok\r\n"); TEST_ASSERT_TRUE(d3_contains("[HLP:"));
	d3_expect_command("$$", "ok\r\n"); TEST_ASSERT_TRUE(d3_contains("$0=")); TEST_ASSERT_TRUE(d3_contains("$132="));
	d3_expect_command("$#", "ok\r\n"); TEST_ASSERT_TRUE(d3_contains("[G54:")); TEST_ASSERT_TRUE(d3_contains("[PRB:"));
	d3_expect_command("$G", "ok\r\n"); TEST_ASSERT_TRUE(d3_contains("[GC:"));
	d3_expect_command("$I", "ok\r\n"); TEST_ASSERT_TRUE(d3_contains("[VER:")); TEST_ASSERT_TRUE(d3_contains("[OPT:"));
	d3_expect_command("$N", "ok\r\n"); TEST_ASSERT_TRUE(d3_contains("$N0=")); TEST_ASSERT_TRUE(d3_contains("$N1="));
	d3_expect_command("$Q", "error:3\r\n");
}

static void test_status_framing_and_fields(void)
{
	d3_idle();
	char status[512];
	TEST_ASSERT_TRUE(d3_status(status, sizeof(status)));
	TEST_ASSERT_NOT_NULL(strstr(status, "<Idle"));
	TEST_ASSERT_NOT_NULL(strstr(status, "MPos:"));
	TEST_ASSERT_NOT_NULL(strstr(status, "FS:"));
	TEST_ASSERT_EQUAL_CHAR('<', status[0]);
	TEST_ASSERT_NOT_NULL(strstr(status, ">"));
}

static void test_stream_framing_and_partial_lines(void)
{
	d3_idle();
	grbl_test_clear_output();
	TEST_ASSERT_TRUE(mcu_unit_test_inject("G21\rG90\nG17\r\n"));
	TEST_ASSERT_TRUE(grbl_test_wait_for("ok\r\nok\r\nok\r\n", D3_TIMEOUT_MS));
	grbl_test_clear_output();
	TEST_ASSERT_TRUE(mcu_unit_test_inject("G1X"));
	d3_realtime('?');
	grbl_test_assert_wait_for("<Idle");
	TEST_ASSERT_TRUE(mcu_unit_test_inject("0F100\n"));
	grbl_test_assert_wait_for("ok\r\n");
}

static void test_realtime_status_has_no_ok(void)
{
	d3_idle();
	grbl_test_clear_output(); d3_realtime('?'); grbl_test_assert_wait_for(">");
	mcu_unit_test_buffer_read(grbl_test_transcript, sizeof(grbl_test_transcript));
	TEST_ASSERT_NULL(strstr(grbl_test_transcript, "ok\r\n"));
}

D3_UNITY_MAIN(
	RUN_TEST(test_reset_help_and_queries);
	RUN_TEST(test_status_framing_and_fields);
	RUN_TEST(test_stream_framing_and_partial_lines);
	RUN_TEST(test_realtime_status_has_no_ok))
