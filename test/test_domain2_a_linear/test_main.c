#include "../common/domain2_test.h"

void setUp(void) { d2_set_up(); }
void tearDown(void) { d2_tear_down(); }

static void test_reporting_and_zero_distance(void)
{
	d2_sample_t before = d2_position();
	TEST_ASSERT_EQUAL_STRING("Idle", before.state);
	grbl_test_command_ok("G0X0Y0Z0");
	d2_wait_idle_timeout(GRBL_TEST_TIMEOUT_MS);
	d2_sample_t after = d2_position();
	d2_assert_position(&after, before.x, before.y, before.z, D2_POSITION_TOLERANCE);
}

static void test_absolute_and_incremental_axis_moves(void)
{
	d2_trace_t trace;
	d2_trace_motion("G90G1X3Y-2Z1F600", &trace);
	d2_assert_trace_endpoint(&trace, 3, -2, 1);
	d2_trace_motion("G91G1X-1Y3Z-0.5F600", &trace);
	d2_assert_trace_endpoint(&trace, 2, 1, 0.5f);
	d2_trace_motion("G90G1X0Y0Z0F600", &trace);
	d2_assert_trace_endpoint(&trace, 0, 0, 0);
}

static void test_xyz_linear_path_geometry(void)
{
	d2_sample_t start = d2_position();
	d2_sample_t end = start;
	end.x = 6; end.y = 4; end.z = 2;
	d2_trace_t trace;
	d2_trace_motion("G1X6Y4Z2F600", &trace);
	d2_assert_trace_endpoint(&trace, 6, 4, 2);
	d2_assert_line(&trace, start, end);
}

static void test_inch_coordinates_and_modal_motion(void)
{
	d2_trace_t trace;
	d2_trace_motion("G20G90G1X0.2F20", &trace);
	d2_assert_trace_endpoint(&trace, 5.08f, 0, 0);
	d2_trace_motion("X0.1", &trace);
	d2_assert_trace_endpoint(&trace, 2.54f, 0, 0);
}

static void test_g80_cancels_coordinate_motion(void)
{
	grbl_test_command_ok("G1X1F300");
	d2_wait_idle_timeout(GRBL_TEST_TIMEOUT_MS);
	grbl_test_command_ok("G80");
	d2_sample_t before = d2_position();
	grbl_test_command_error("X2");
	d2_sample_t after = d2_position();
	d2_assert_position(&after, before.x, before.y, before.z, D2_POSITION_TOLERANCE);
}

D2_UNITY_MAIN(
	RUN_TEST(test_reporting_and_zero_distance);
	RUN_TEST(test_absolute_and_incremental_axis_moves);
	RUN_TEST(test_xyz_linear_path_geometry);
	RUN_TEST(test_inch_coordinates_and_modal_motion);
	RUN_TEST(test_g80_cancels_coordinate_motion))
