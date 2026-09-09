#include "../common/domain2_test.h"

void setUp(void) { d2_set_up(); }
void tearDown(void) { d2_tear_down(); }

static void run_xy_arc(const char *command, bool clockwise)
{
	d2_trace_t trace;
	d2_trace_motion(command, &trace);
	d2_assert_trace_endpoint(&trace, 10, 0, 0);
	d2_assert_arc(&trace, 0, 1, -1, 5, 0, 5, clockwise, 0, 0);
}

static void test_xy_ijk_cw_and_ccw(void)
{
	run_xy_arc("G17G2X10Y0I5J0F600", true);
	d2_trace_t reset; d2_trace_motion("G0X0Y0", &reset);
	run_xy_arc("G17G3X10Y0I5J0F600", false);
}

static void test_xy_radius_cw_and_ccw(void)
{
	run_xy_arc("G17G2X10Y0R5F600", true);
	d2_trace_t reset; d2_trace_motion("G0X0Y0", &reset);
	run_xy_arc("G17G3X10Y0R5F600", false);
}

static void test_zx_and_yz_planes(void)
{
	d2_trace_t trace;
	d2_trace_motion("G18G2X10Z0I5K0F600", &trace);
	d2_assert_trace_endpoint(&trace, 10, 0, 0);
	d2_assert_arc(&trace, 2, 0, -1, 0, 5, 5, true, 0, 0);
	d2_trace_motion("G0X0Z0", &trace);
	d2_trace_motion("G19G3Y10Z0J5K0F600", &trace);
	d2_assert_trace_endpoint(&trace, 0, 10, 0);
	d2_assert_arc(&trace, 1, 2, -1, 5, 0, 5, false, 0, 0);
}

static void test_xy_helical_motion(void)
{
	d2_trace_t trace;
	d2_trace_motion("G17G2X10Y0Z3I5J0F600", &trace);
	d2_assert_trace_endpoint(&trace, 10, 0, 3);
	d2_assert_arc(&trace, 0, 1, 2, 5, 0, 5, true, 0, 3);
}

D2_UNITY_MAIN(
	RUN_TEST(test_xy_ijk_cw_and_ccw);
	RUN_TEST(test_xy_radius_cw_and_ccw);
	RUN_TEST(test_zx_and_yz_planes);
	RUN_TEST(test_xy_helical_motion))
