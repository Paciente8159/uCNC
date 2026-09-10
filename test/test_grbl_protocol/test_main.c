#include "../common/grbl_test.h"

void setUp(void) { grbl_test_start(); }
void tearDown(void) { grbl_test_stop(); }

static void test_protocol_reports_and_default_configuration(void)
{
	grbl_test_realtime_expect("?", "<Idle");
	grbl_test_assert_wait_for("MPos:0.000,0.000,0.000");

	grbl_test_command_expect("$", "[HLP:");
	grbl_test_assert_wait_for("ok\r\n");

	grbl_test_command_expect("$G", "[GC:");
	grbl_test_assert_wait_for("G54");
	grbl_test_assert_wait_for("G21");
	grbl_test_assert_wait_for("G90");
	grbl_test_assert_wait_for("ok\r\n");

	grbl_test_command_expect("$#", "[G54:");
	grbl_test_assert_wait_for("[G59:");
	grbl_test_assert_wait_for("[G28:");
	grbl_test_assert_wait_for("[G30:");
	grbl_test_assert_wait_for("[G92:");
	grbl_test_assert_wait_for("[PRB:");
	grbl_test_assert_wait_for("ok\r\n");

	grbl_test_command_expect("$$", "$0=");
	grbl_test_assert_wait_for("$100=200.000");
	grbl_test_assert_wait_for("$101=200.000");
	grbl_test_assert_wait_for("$102=200.000");
	grbl_test_assert_wait_for("ok\r\n");
}

GRBL_PROCESS_FIXTURE(test_protocol_reports_and_default_configuration)
