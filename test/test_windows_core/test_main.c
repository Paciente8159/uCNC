/*
	Name: test_main.c
	Description: Windows virtual core test for µCNC.

	Runs real µCNC G-code through the virtual UART2 RX path and asserts
	parser status, protocol output and final interpolator step position.

	This test is part of the µCNC RS274/NGC v3 conformance + motion
	endpoint regression suite. Expected positions are expressed in machine
	millimeters and converted with the active settings (step_per_mm).

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 18-08-2026

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>
*/

/*
 * cnc.c is intentionally excluded from build_src_filter and included here
 * so the test can access its private startup state without changing
 * production cnc.c.
 *
 * It must be included before <windows.h> because winnt.h redefines
 * FORCEINLINE to `extern __inline__` in C mode, which would collide with
 * the `static FORCEINLINE` declarations in atomic.h.
 */
#include "src/cnc.c"

#include <unity.h>
#include <windows.h>
#include <math.h>
#include <string.h>

extern bool mcu_uart2_inject(const char *cmd);
extern void mcu_uart2_test_capture_reset(void);
extern const char *mcu_uart2_test_capture_get(void);

#define TEST_MOTION_TIMEOUT_MS 5000

static bool test_controller_initialized;

static void test_controller_init_once(void)
{
	if (!test_controller_initialized)
	{
		cnc_init();
		test_controller_initialized = true;
	}
}

static void test_controller_prepare(void)
{
	test_controller_init_once();

	mcu_uart2_clear();
	mcu_uart2_test_capture_reset();

	cnc_reset();

	if (cnc_unlock(false) != UNLOCK_ERROR)
	{
		cnc_state.alarm = EXEC_ALARM_NOALARM;
	}

	cnc_state.loop_state = LOOP_RUNNING;

	/*
	 * Make every test start at machine step position zero.
	 */
	int32_t zero[STEPPER_COUNT] = {0};

	itp_sync_rt_position(zero);
	mc_sync_position();
	parser_sync_position();
}

static uint8_t test_execute_line(const char *cmd)
{
	TEST_ASSERT_TRUE(mcu_uart2_inject(cmd));

	uint8_t status = cnc_parse_cmd();

	TEST_ASSERT_TRUE(cnc_dotasks());

	mcu_uart2_flush();

	return status;
}

static bool test_sync_motion(uint32_t timeout_ms)
{
	DWORD start = GetTickCount();

	while (!itp_is_empty() || !planner_buffer_is_empty())
	{
		if (!cnc_dotasks())
		{
			return false;
		}

		if ((GetTickCount() - start) > timeout_ms)
		{
			return false;
		}

		Sleep(1);
	}

	return true;
}

void setUp(void)
{
	test_controller_prepare();
}

void tearDown(void)
{
}

/* ---------------------------------------------------------------------------
 * Position assertion helpers
 * ------------------------------------------------------------------------- */

static int32_t test_mm_to_steps(uint8_t axis, float mm)
{
	return (int32_t)lroundf(mm * g_settings.step_per_mm[axis]);
}

static void test_assert_position_mm(float x, float y, float z)
{
	int32_t pos[STEPPER_COUNT] = {0};

	itp_get_rt_position(pos);

#ifdef AXIS_X
	TEST_ASSERT_INT32_WITHIN(
		1,
		test_mm_to_steps(AXIS_X, x),
		pos[AXIS_X]);
#endif

#ifdef AXIS_Y
	TEST_ASSERT_INT32_WITHIN(
		1,
		test_mm_to_steps(AXIS_Y, y),
		pos[AXIS_Y]);
#endif

#ifdef AXIS_Z
	TEST_ASSERT_INT32_WITHIN(
		1,
		test_mm_to_steps(AXIS_Z, z),
		pos[AXIS_Z]);
#endif
}

static void test_assert_capture_has(const char *needle)
{
	TEST_ASSERT_NOT_NULL(strstr(mcu_uart2_test_capture_get(), needle));
}

/* ---------------------------------------------------------------------------
 * G0 / G1 motion
 * ------------------------------------------------------------------------- */

static void test_g0_absolute_xy(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X10Y10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g0_incremental_xy(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G91\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X5Y-2\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X5\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	mcu_uart2_flush();

	test_assert_position_mm(10.0f, -2.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g1_absolute_xy(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G1X10Y5F600\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 5.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g1_modal_continue(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G1X5Y0F600\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("X10Y5\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 5.0f, 0.0f);
	test_assert_capture_has("ok");
}

/* ---------------------------------------------------------------------------
 * G2 / G3 arcs
 * ------------------------------------------------------------------------- */

static void test_g2_ij_cw(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G2X10Y10I10J0\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g3_ij_ccw(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G3X10Y10I0J10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g2_r_cw(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G2X10Y10R10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g3_r_ccw(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G3X10Y10R10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

#ifdef AXIS_X
#ifdef AXIS_Z
static void test_g18_xz_plane(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G18\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Z0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G2X10Z10I10K0\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 0.0f, 10.0f);
	test_assert_capture_has("ok");
}
#endif
#endif

#ifdef AXIS_Y
#ifdef AXIS_Z
static void test_g19_yz_plane(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G19\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0Y0Z0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G3Y10Z10J0K10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(0.0f, 10.0f, 10.0f);
	test_assert_capture_has("ok");
}
#endif
#endif

/* ---------------------------------------------------------------------------
 * G20 / G21 units and word order / comments
 * ------------------------------------------------------------------------- */

static void test_g20_inch(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G20\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X1\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(25.4f, 0.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_g21_mm(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X25.4\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(25.4f, 0.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_word_order(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("Y10F600X10G1G90\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_comment_in_line(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X10(test comment)Y10\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 10.0f, 0.0f);
	test_assert_capture_has("ok");
}

/* ---------------------------------------------------------------------------
 * Modal group / repeated word rules
 * ------------------------------------------------------------------------- */

static void test_valid_multiple_modal_groups(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21G90G1X10Y5F600\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 5.0f, 0.0f);
	test_assert_capture_has("ok");
}

static void test_valid_reordered_modal_groups(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("Y5F600X10G1G90G21\n"));

	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));
	mcu_uart2_flush();

	test_assert_position_mm(10.0f, 5.0f, 0.0f);
	test_assert_capture_has("ok");
}

/* ---------------------------------------------------------------------------
 * Invalid / ill-formed commands
 * ------------------------------------------------------------------------- */

static void test_invalid_gcode_does_not_move(void)
{
	int32_t before[STEPPER_COUNT] = {0};
	int32_t after[STEPPER_COUNT] = {0};

	itp_get_rt_position(before);

	uint8_t status = test_execute_line("G0XABC\n");

	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);

	itp_get_rt_position(after);

	for (uint8_t i = 0; i < STEPPER_COUNT; i++)
	{
		TEST_ASSERT_EQUAL_INT32(before[i], after[i]);
	}

	test_assert_capture_has("error");
}

static void test_err_g0_no_axis(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G0\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g1_no_axis(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G1X0F600\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G1\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_axis_no_number(void)
{
	uint8_t status = test_execute_line("G0X\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_bad_number(void)
{
	uint8_t status = test_execute_line("G0XABC\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_repeated_axis(void)
{
	uint8_t status = test_execute_line("G0X1X2\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_two_motion_gcodes(void)
{
	uint8_t status = test_execute_line("G0G1X10\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_two_unit_gcodes(void)
{
	uint8_t status = test_execute_line("G20G21\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_two_distance_modes(void)
{
	uint8_t status = test_execute_line("G90G91\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g2_no_center(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G2X10Y10F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g3_no_center(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G3X10Y10F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g2_no_endpoint(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G2I5J0F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g3_no_endpoint(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G3I0J5F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g2_bad_center_radius(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G2X10Y10I4J0F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g2_radius_too_small(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G17\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("F600\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0Y0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G2X10Y10R1F600\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g4_negative(void)
{
	uint8_t status = test_execute_line("G4P-1\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

static void test_err_g4_missing_p(void)
{
	uint8_t status = test_execute_line("G4\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);
}

/* ---------------------------------------------------------------------------
 * Parser atomicity
 * ------------------------------------------------------------------------- */

static void test_parser_atomicity(void)
{
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G21\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G90\n"));
	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X0\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	uint8_t status = test_execute_line("G91G0XBAD\n");
	TEST_ASSERT_NOT_EQUAL(STATUS_OK, status);

	TEST_ASSERT_EQUAL_UINT8(STATUS_OK, test_execute_line("G0X10\n"));
	TEST_ASSERT_TRUE(test_sync_motion(TEST_MOTION_TIMEOUT_MS));

	mcu_uart2_flush();

	/*
	 * G91 from the invalid block must not be committed, so the following
	 * G0X10 is absolute and ends at 10 mm, not 10 mm relative to an
	 * altered state.
	 */
	test_assert_position_mm(10.0f, 0.0f, 0.0f);
}

/* ---------------------------------------------------------------------------
 * Main
 * ------------------------------------------------------------------------- */

int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(test_g0_absolute_xy);
	RUN_TEST(test_g0_incremental_xy);
	RUN_TEST(test_g1_absolute_xy);
	RUN_TEST(test_g1_modal_continue);

	RUN_TEST(test_g2_ij_cw);
	RUN_TEST(test_g3_ij_ccw);
	RUN_TEST(test_g2_r_cw);
	RUN_TEST(test_g3_r_ccw);
#ifdef AXIS_X
#ifdef AXIS_Z
	RUN_TEST(test_g18_xz_plane);
#endif
#endif
#ifdef AXIS_Y
#ifdef AXIS_Z
	RUN_TEST(test_g19_yz_plane);
#endif
#endif

	RUN_TEST(test_g20_inch);
	RUN_TEST(test_g21_mm);
	RUN_TEST(test_word_order);
	RUN_TEST(test_comment_in_line);

	RUN_TEST(test_valid_multiple_modal_groups);
	RUN_TEST(test_valid_reordered_modal_groups);

	RUN_TEST(test_invalid_gcode_does_not_move);
	RUN_TEST(test_err_g0_no_axis);
	RUN_TEST(test_err_g1_no_axis);
	RUN_TEST(test_err_axis_no_number);
	RUN_TEST(test_err_bad_number);
	RUN_TEST(test_err_repeated_axis);
	RUN_TEST(test_err_two_motion_gcodes);
	RUN_TEST(test_err_two_unit_gcodes);
	RUN_TEST(test_err_two_distance_modes);
	RUN_TEST(test_err_g2_no_center);
	RUN_TEST(test_err_g3_no_center);
	RUN_TEST(test_err_g2_no_endpoint);
	RUN_TEST(test_err_g3_no_endpoint);
	RUN_TEST(test_err_g2_bad_center_radius);
	RUN_TEST(test_err_g2_radius_too_small);
	RUN_TEST(test_err_g4_negative);
	RUN_TEST(test_err_g4_missing_p);

	RUN_TEST(test_parser_atomicity);

	return UNITY_END();
}
