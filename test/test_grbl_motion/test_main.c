#include "../common/grbl_test.h"

void setUp(void) { grbl_test_start(); }
void tearDown(void) { grbl_test_stop(); }

static void test_cartesian_motion_and_modal_state(void)
{
	grbl_test_command_ok("G21 G90 G0 X1 Y2 Z3");
	grbl_test_command_ok("G4 P0.01");
	grbl_test_realtime_expect("?", "MPos:1.000,2.000,3.000");

	grbl_test_command_ok("G91 G1 X1 Y-1 Z0.5 F100");
	grbl_test_command_ok("G4 P0.01");
	grbl_test_realtime_expect("?", "MPos:2.000,1.000,3.500");

	grbl_test_command_ok("G90 G2 X3 Y1 I0.5 J0 F100");
	grbl_test_command_ok("G4 P0.01");
	grbl_test_realtime_expect("?", "MPos:3.000,1.000,3.500");

	grbl_test_command_expect("$G", "[GC:");
	grbl_test_assert_wait_for("G90");
	grbl_test_assert_wait_for("G21");
	grbl_test_assert_wait_for("G94");
}

GRBL_PROCESS_FIXTURE(test_cartesian_motion_and_modal_state)
