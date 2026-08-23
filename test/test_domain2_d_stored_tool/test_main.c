#include "../common/domain2_test.h"

void setUp(void) { d2_set_up(); }
void tearDown(void) { d2_tear_down(); }

static void test_g28_store_and_return(void)
{
	grbl_test_command_ok("G53G0X4Y2Z1");
	d2_wait_idle_timeout(3000U);
	// requires extension
	// grbl_test_command_ok("G28.1");
	// equivalent G10
	grbl_test_command_ok("G10L20P28X0Y0Z0");
	grbl_test_command_ok("G53G0X0Y0Z0");
	d2_wait_idle_timeout(3000U);
	grbl_test_command_ok("G28");
	d2_wait_idle_timeout(5000U);
	d2_sample_t p = d2_position();
	d2_assert_position(&p, 4, 2, 1, D2_POSITION_TOLERANCE);
}

static void test_g30_store_and_return(void)
{
	grbl_test_command_ok("G53G0X3Y1Z2");
	d2_wait_idle_timeout(3000U);
	// requires extension
	// grbl_test_command_ok("G30.1");
	// equivalent to G30.1
	grbl_test_command_ok("G10L20P30X0Y0Z0");
	grbl_test_command_ok("G53G0X0Y0Z0");
	d2_wait_idle_timeout(3000U);
	grbl_test_command_ok("G30");
	d2_wait_idle_timeout(5000U);
	d2_sample_t p = d2_position();
	d2_assert_position(&p, 3, 1, 2, D2_POSITION_TOLERANCE);
}

static void test_g28_intermediate_point_has_two_legs(void)
{
	grbl_test_command_ok("G53G0X4Y0");
	d2_wait_idle_timeout(3000U);
	// requires extension
	// grbl_test_command_ok("G28.1");
	// equivalent G10
	grbl_test_command_ok("G10L20P28X0Y0Z0");
	grbl_test_command_ok("G53G0X0Y0");
	d2_wait_idle_timeout(3000U);
	d2_trace_t trace;
	d2_trace_motion("G28X2Y2", &trace);
	d2_assert_trace_endpoint(&trace, 4, 0, 0);
	bool reached_intermediate = false;
	for (size_t i = 0; i < trace.count; ++i)
		reached_intermediate |= fabsf(trace.samples[i].x - 2) < 0.2f && fabsf(trace.samples[i].y - 2) < 0.2f;
	TEST_ASSERT_TRUE_MESSAGE(reached_intermediate, "G28 trace did not pass through intermediate point");
}

static void test_tool_length_offset_and_cancel(void)
{
	d2_sample_t before = d2_position();
	grbl_test_command_ok("G43.1Z2");
	d2_sample_t unchanged = d2_position();
	d2_assert_position(&unchanged, before.x, before.y, before.z, D2_POSITION_TOLERANCE);
	grbl_test_command_ok("G1Z0F300");
	d2_wait_idle_timeout(3000U);
	d2_sample_t offset = d2_position();
	TEST_ASSERT_FLOAT_WITHIN(D2_POSITION_TOLERANCE, 2, fabsf(offset.z));
	grbl_test_command_ok("G49");
	grbl_test_command_expect("$G", "G49");
}

D2_UNITY_MAIN(
	RUN_TEST(test_g28_store_and_return);
	RUN_TEST(test_g30_store_and_return);
	RUN_TEST(test_g28_intermediate_point_has_two_legs);
	RUN_TEST(test_tool_length_offset_and_cancel))
