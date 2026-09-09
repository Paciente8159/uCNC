#include "../common/grbl_test.h"

void setUp(void) { grbl_test_start(); }
void tearDown(void) { grbl_test_stop(); }

static void test_jog_cancel_returns_to_idle(void)
{
	grbl_test_command_ok("$J=G91 X5 F10");
	grbl_test_realtime_expect("?", "<Jog");

	const char jog_cancel[] = {(char)0x85, '\0'};
	grbl_test_inject(jog_cancel);
	grbl_test_wait_idle();
}

GRBL_PROCESS_FIXTURE(test_jog_cancel_returns_to_idle)
