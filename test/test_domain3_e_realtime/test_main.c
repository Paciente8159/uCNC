#include "../common/domain3_test.h"

static void test_hold_resume_full_transition(void)
{
	d3_idle(); d3_start_long_motion();
	d3_realtime('!');
	TEST_ASSERT_TRUE(d3_wait_either_state("<Hold:1", "<Hold:0", D3_TIMEOUT_MS));
	d3_expect_state("<Hold:0", D3_MOTION_TIMEOUT_MS);
	d3_realtime('~');
	TEST_ASSERT_TRUE(d3_wait_either_state("<Run", "<Idle", D3_MOTION_TIMEOUT_MS));
	d3_expect_state("<Idle", D3_MOTION_TIMEOUT_MS);
}

static void test_idle_hold_and_cycle_start(void)
{
	d3_idle();
	d3_realtime('~'); d3_expect_state("<Idle", D3_TIMEOUT_MS);
	d3_realtime('!');
	TEST_ASSERT_TRUE(d3_wait_either_state("<Hold", "<Idle", D3_TIMEOUT_MS));
	d3_realtime('~'); d3_expect_state("<Idle", D3_TIMEOUT_MS);
}

static void test_reset_idle_and_run(void)
{
	d3_idle(); d3_reset(); d3_expect_state("<Idle", D3_TIMEOUT_MS);
	d3_expect_command("$X", "ok\r\n"); d3_start_long_motion();
	grbl_test_clear_output(); d3_realtime(0x18); grbl_test_assert_wait_for("Grbl "); grbl_test_assert_wait_for("ALARM:3");
	d3_expect_state("<Alarm", D3_TIMEOUT_MS);
}

static void test_safety_door_realtime_suspends_and_requires_resume(void)
{
	d3_idle(); d3_start_long_motion();
	d3_realtime(0x84); d3_expect_state("<Door", D3_MOTION_TIMEOUT_MS);
	d3_realtime('~');
	TEST_ASSERT_FALSE_MESSAGE(d3_wait_state("<Run", 500U), "cycle start resumed while door was logically open");
	d3_cancel_motion();
}

static void test_physical_hold_and_cycle_start_inputs(void)
{
	d3_idle(); d3_start_long_motion();
	test_io_set(TEST_IO_FHOLD, true); d3_expect_state("<Hold", D3_TIMEOUT_MS);
	test_io_set(TEST_IO_FHOLD, false); d3_expect_state("<Hold:0", D3_MOTION_TIMEOUT_MS);
	test_io_set(TEST_IO_CS_RES, true);
	TEST_ASSERT_TRUE(d3_wait_either_state("<Run", "<Idle", D3_MOTION_TIMEOUT_MS));
	test_io_set(TEST_IO_CS_RES, false);
	d3_expect_state("<Idle", D3_MOTION_TIMEOUT_MS);
}

D3_UNITY_MAIN(
	RUN_TEST(test_hold_resume_full_transition);
	RUN_TEST(test_idle_hold_and_cycle_start);
	RUN_TEST(test_reset_idle_and_run);
	RUN_TEST(test_safety_door_realtime_suspends_and_requires_resume);
	RUN_TEST(test_physical_hold_and_cycle_start_inputs))
