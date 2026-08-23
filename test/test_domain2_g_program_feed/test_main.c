#include "../common/domain2_test.h"

void setUp(void) { d2_set_up(); }
void tearDown(void) { d2_tear_down(); }

static void test_dwell_has_no_position_effect(void)
{
	d2_sample_t before = d2_position();
	uint32_t started = mcu_millis();
	grbl_test_command_ok("G4P0.05");
	uint32_t elapsed = mcu_millis() - started;
	d2_sample_t after = d2_position();
	d2_assert_position(&after, before.x, before.y, before.z, D2_POSITION_TOLERANCE);
	TEST_ASSERT_GREATER_OR_EQUAL_UINT32_MESSAGE(30U, elapsed, "G4 dwell completed too early");
}

static void test_m0_holds_following_motion_until_resume(void)
{
	grbl_test_command_ok("M0");
	grbl_test_wait_for_state("<Hold", 3000U);
	d2_sample_t held = d2_position();
	d2_assert_position(&held, 0, 0, 0, D2_POSITION_TOLERANCE);
	grbl_test_inject("~");
	d2_wait_idle_timeout(5000U);
}

static void test_feed_is_reported_and_endpoint_is_unchanged(void)
{
	d2_trace_t slow, fast;
	d2_trace_motion("G1X4F240", &slow);
	d2_assert_trace_endpoint(&slow, 4, 0, 0);
	d2_trace_motion("G1X8F480", &fast);
	d2_assert_trace_endpoint(&fast, 8, 0, 0);
	float slow_max = 0, fast_max = 0;
	for (size_t i = 0; i < slow.count; ++i) slow_max = fmaxf(slow_max, slow.samples[i].feed);
	for (size_t i = 0; i < fast.count; ++i) fast_max = fmaxf(fast_max, fast.samples[i].feed);
	TEST_ASSERT_TRUE_MESSAGE(slow_max > 0, "status never reported active feed");
	TEST_ASSERT_TRUE_MESSAGE(fast_max > slow_max, "higher programmed feed did not increase reported feed");
}

static void test_axis_max_rate_does_not_change_endpoint(void)
{
	grbl_test_command_ok("G4P0.05");
	grbl_test_command_ok("$110=120");
	d2_trace_t trace;
	d2_trace_motion("G1X2F600", &trace);
	d2_assert_trace_endpoint(&trace, 2, 0, 0);
	TEST_ASSERT_TRUE_MESSAGE(trace.saw_run, "rate-limited move never entered Run");
}

D2_UNITY_MAIN(
	RUN_TEST(test_dwell_has_no_position_effect);
	RUN_TEST(test_feed_is_reported_and_endpoint_is_unchanged);
	RUN_TEST(test_axis_max_rate_does_not_change_endpoint);
	RUN_TEST(test_m0_holds_following_motion_until_resume))
