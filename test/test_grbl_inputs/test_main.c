#include "../common/grbl_test.h"

void setUp(void) { grbl_test_start(); }
void tearDown(void) { grbl_test_stop(); }

static void test_probe_and_limit_inputs_affect_protocol_state(void)
{
	grbl_test_command_ok("G21 G90 G0 X0 Y0 Z0");
	grbl_test_wait_idle();

	test_io_set_after(TEST_IO_PROBE, 10U, false, true);
	grbl_test_command_ok("G38.2 X10 F100");
	grbl_test_wait_idle();
	grbl_test_command_expect("$#", "[PRB:");
	grbl_test_assert_wait_for(":1]");

	grbl_test_command_ok("$21=1");
	test_io_set(TEST_IO_LIMIT_X, true);
	grbl_test_assert_wait_for("ALARM:");
}

GRBL_PROCESS_FIXTURE(test_probe_and_limit_inputs_affect_protocol_state)
