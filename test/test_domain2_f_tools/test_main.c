#include "../common/domain2_test.h"

void setUp(void) { d2_set_up(); }
void tearDown(void) { d2_tear_down(); }

static void test_spindle_direction_speed_and_stop(void)
{
	grbl_test_command_ok("$30=1000"); grbl_test_command_ok("$31=0");
	grbl_test_command_ok("M3S250");
	TEST_ASSERT_INT_WITHIN(2, 64, virtualmap.pwm[0]);
	TEST_ASSERT_BITS_HIGH(1U, virtualmap.outputs);
	grbl_test_command_ok("S750");
	TEST_ASSERT_INT_WITHIN(2, 191, virtualmap.pwm[0]);
	grbl_test_command_ok("M4");
	TEST_ASSERT_INT_WITHIN(2, 191, virtualmap.pwm[0]);
	TEST_ASSERT_BITS_LOW(1U, virtualmap.outputs);
	grbl_test_command_ok("M5");
	TEST_ASSERT_EQUAL_UINT8(0, virtualmap.pwm[0]);
}

static void test_spindle_clamps_and_modal_speed(void)
{
	grbl_test_command_ok("$30=1000"); grbl_test_command_ok("$31=100");
	grbl_test_command_ok("S2000M3"); TEST_ASSERT_EQUAL_UINT8(255, virtualmap.pwm[0]);
	grbl_test_command_ok("M5S500"); TEST_ASSERT_EQUAL_UINT8(0, virtualmap.pwm[0]);
	grbl_test_command_ok("M3"); TEST_ASSERT_INT_WITHIN(2, 128, virtualmap.pwm[0]);
}

static void test_program_end_disables_spindle_and_resets_modes(void)
{
	grbl_test_command_ok("G20G91M3S500");
	grbl_test_command_expect("M2", "[MSG:Pgm End]");
	TEST_ASSERT_EQUAL_UINT8(0, virtualmap.pwm[0]);
	grbl_test_command_expect("$G", "[GC:");
	grbl_test_assert_wait_for("G21"); grbl_test_assert_wait_for("G90"); grbl_test_assert_wait_for("M5");
}

static void test_coolant_outputs_when_compiled(void)
{
#ifdef ENABLE_COOLANT
	grbl_test_command_ok("M8"); TEST_ASSERT_BITS_HIGH(1U << 2, virtualmap.outputs);
	grbl_test_command_ok("M7"); TEST_ASSERT_BITS_HIGH(1U << 3, virtualmap.outputs);
	grbl_test_command_ok("M9"); TEST_ASSERT_BITS_LOW((1U << 2) | (1U << 3), virtualmap.outputs);
#else
	grbl_test_command_ok("M8");
	TEST_ASSERT_BITS_LOW((1U << 2) | (1U << 3), virtualmap.outputs);
#endif
}

D2_UNITY_MAIN(
	RUN_TEST(test_spindle_direction_speed_and_stop);
	RUN_TEST(test_spindle_clamps_and_modal_speed);
	RUN_TEST(test_program_end_disables_spindle_and_resets_modes);
	RUN_TEST(test_coolant_outputs_when_compiled))
