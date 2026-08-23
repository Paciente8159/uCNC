#include "../common/grbl_test.h"

void setUp(void) { grbl_test_start(); }
void tearDown(void) { grbl_test_stop(); }

static void test_feed_hold_and_cycle_start(void)
{
	/* Two seconds at nominal feed: long enough to observe Run/Hold, but fast in CI. */
	grbl_test_command_ok("G91 G1 X2 F60");
	grbl_test_wait_for_state("<Run", 1000U);
	grbl_test_inject("!");
	grbl_test_wait_for_state("<Hold:0", 2000U);
	/* Confirm the completed hold on a subsequent controller service cycle. */
	grbl_test_wait_for_state("<Hold:0", 500U);
	grbl_test_inject("~");
	grbl_test_wait_for_state("<Run", 1000U);
	grbl_test_wait_for_state("<Idle", 5000U);
}

GRBL_PROCESS_FIXTURE(test_feed_hold_and_cycle_start)
