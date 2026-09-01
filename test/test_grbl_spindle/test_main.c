#include "../common/grbl_test.h"

void setUp(void) { grbl_test_start(); }
void tearDown(void) { grbl_test_stop(); }

static void test_spindle_commands_drive_default_virtual_outputs(void)
{
	grbl_test_command_ok("M3 S500");
	TEST_ASSERT_INT_WITHIN_MESSAGE(1, 127, virtualmap.pwm[0], "M3 S500 must produce 50% PWM");
	TEST_ASSERT_NOT_EQUAL_MESSAGE(0U, virtualmap.outputs & 1U, "M3 must select clockwise direction");

	grbl_test_command_ok("M4 S500");
	TEST_ASSERT_INT_WITHIN_MESSAGE(1, 127, virtualmap.pwm[0], "M4 S500 must preserve 50% PWM magnitude");
	TEST_ASSERT_EQUAL_MESSAGE(0U, virtualmap.outputs & 1U, "M4 must select counter-clockwise direction");

	grbl_test_command_ok("M5");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, virtualmap.pwm[0], "M5 must disable spindle PWM");
	TEST_ASSERT_EQUAL_MESSAGE(0U, virtualmap.outputs & 1U, "M5 must clear spindle direction output");
}

GRBL_PROCESS_FIXTURE(test_spindle_commands_drive_default_virtual_outputs)
