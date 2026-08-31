#include "../common/domain2_test.h"

void setUp(void) { d2_set_up(); }
void tearDown(void) { d2_tear_down(); }

static void test_g54_through_g59_offsets(void)
{
	const char *set[] = {"G10L2P1X1", "G10L2P2X2", "G10L2P3X3", "G10L2P4X4", "G10L2P5X5", "G10L2P6X6"};
	const char *select[] = {"G54G0X0.5", "G55G0X0.5", "G56G0X0.5", "G57G0X0.5", "G58G0X0.5", "G59G0X0.5"};
	for (size_t i = 0; i < 6; ++i) grbl_test_command_ok(set[i]);
	for (size_t i = 0; i < 6; ++i)
	{
		d2_trace_t trace; d2_trace_motion(select[i], &trace);
		d2_sample_t p = d2_position();
		TEST_ASSERT_FLOAT_WITHIN(D2_POSITION_TOLERANCE, (float)i + 1.5f, p.x);
	}
}

static void test_g92_and_cancel_do_not_move(void)
{
	d2_sample_t before = d2_position();
	grbl_test_command_ok("G92X10Y20Z30");
	d2_sample_t after = d2_position();
	d2_assert_position(&after, before.x, before.y, before.z, D2_POSITION_TOLERANCE);
	d2_trace_t trace; d2_trace_motion("G1X11Y21Z31F600", &trace);
	after = d2_position(); d2_assert_position(&after, 1, 1, 1, D2_POSITION_TOLERANCE);
	grbl_test_command_ok("G92.1");
	d2_trace_motion("G1X0Y0Z0F600", &trace);
	after = d2_position(); d2_assert_position(&after, 0, 0, 0, D2_POSITION_TOLERANCE);
}

static void test_g53_bypasses_offset_and_is_nonmodal(void)
{
	grbl_test_command_ok("G10L2P1X5");
	d2_trace_t trace; d2_trace_motion("G54G53G0X2", &trace);
	d2_sample_t p = d2_position(); TEST_ASSERT_FLOAT_WITHIN(D2_POSITION_TOLERANCE, 2, p.x);
	d2_trace_motion("G0X0", &trace);
	p = d2_position(); TEST_ASSERT_FLOAT_WITHIN(D2_POSITION_TOLERANCE, 5, p.x);
}

static void test_g10_l20_maps_current_position(void)
{
	d2_trace_t trace; d2_trace_motion("G53G0X3Y2Z1", &trace);
	grbl_test_command_ok("G10L20P1X10Y10Z10");
	d2_trace_motion("G54G0X11Y11Z11", &trace);
	d2_sample_t p = d2_position(); d2_assert_position(&p, 4, 3, 2, D2_POSITION_TOLERANCE);
}

D2_UNITY_MAIN(
	RUN_TEST(test_g54_through_g59_offsets);
	RUN_TEST(test_g92_and_cancel_do_not_move);
	RUN_TEST(test_g53_bypasses_offset_and_is_nonmodal);
	RUN_TEST(test_g10_l20_maps_current_position))
