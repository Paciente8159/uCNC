#include "../common/domain3_test.h"

static void test_jog_validation(void)
{
	d3_idle();
	d3_expect_command("$J=G91X1F300", "ok\r\n"); d3_expect_state("<Jog", D3_TIMEOUT_MS);
	d3_realtime(0x85); d3_expect_state("<Idle", D3_TIMEOUT_MS);
	d3_expect_command("$J=G91X1", "error:22\r\n");
	d3_expect_command("$J=G91F100", "ok\r\n");
	d3_expect_command("$J=G91G2X1I0.5F100", "error:16\r\n");
	d3_expect_command("$JG91X1F100", "error:3\r\n");
}

static void test_jog_lock_cancel_and_hold(void)
{
	d3_idle();
	d3_expect_command("$J=G91X5F60", "ok\r\n"); d3_expect_state("<Jog", D3_TIMEOUT_MS);
	d3_expect_command("G0X0", "error:9\r\n");
	d3_realtime('!'); d3_expect_state("<Idle", D3_MOTION_TIMEOUT_MS);
	d3_expect_command("$J=G91X5F60", "ok\r\n"); d3_expect_state("<Jog", D3_TIMEOUT_MS);
	d3_realtime(0x85); d3_expect_state("<Idle", D3_MOTION_TIMEOUT_MS);
}

static void test_jog_does_not_change_parser_modes(void)
{
	d3_idle();
	d3_expect_command("G20G90", "ok\r\n");
	d3_expect_command("$J=G21G91X1F300", "ok\r\n"); d3_expect_state("<Jog", D3_TIMEOUT_MS);
	d3_realtime(0x85); d3_expect_state("<Idle", D3_TIMEOUT_MS);
	d3_expect_command("$G", "ok\r\n");
	TEST_ASSERT_NOT_NULL(strstr(grbl_test_transcript, "G20")); TEST_ASSERT_NOT_NULL(strstr(grbl_test_transcript, "G90"));
}

static void test_second_jog_is_accepted_while_jogging(void)
{
	d3_idle();
	d3_expect_command("$J=G91X2F60", "ok\r\n"); d3_expect_state("<Jog", D3_TIMEOUT_MS);
	d3_expect_command("$J=G91X2F60", "ok\r\n");
	d3_realtime(0x85); d3_expect_state("<Idle", D3_MOTION_TIMEOUT_MS);
}

D3_UNITY_MAIN(
	RUN_TEST(test_jog_validation);
	RUN_TEST(test_jog_lock_cancel_and_hold);
	RUN_TEST(test_jog_does_not_change_parser_modes);
	RUN_TEST(test_second_jog_is_accepted_while_jogging))
