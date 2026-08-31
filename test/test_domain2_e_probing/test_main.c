#include "../common/domain2_test.h"

void setUp(void) { d2_set_up(); }
void tearDown(void) { d2_tear_down(); }

static void assert_probe_report(bool success)
{
	grbl_test_command_expect("$#", "[PRB:");
	grbl_test_assert_wait_for(success ? ":1]" : ":0]");
}

static void run_probe_toward(const char *command)
{
	test_io_set_after(TEST_IO_PROBE, 200U, false, true);
	grbl_test_command_ok(command);
	d2_wait_idle_timeout(2*GRBL_TEST_TIMEOUT_MS);
	d2_sample_t p = d2_position();
	TEST_ASSERT_TRUE_MESSAGE(p.x >= 0 && p.x < 5, "probe-toward did not stop before target");
	assert_probe_report(true);
}

static void test_g38_2_stops_on_contact(void) { run_probe_toward("G38.2X5F120"); }
static void test_g38_3_stops_on_contact(void) { run_probe_toward("G38.3X5F120"); }

static void run_probe_away(const char *command)
{
	test_io_set_after(TEST_IO_PROBE, 200U, true, false);
	grbl_test_command_ok(command);
	d2_wait_idle_timeout(2*GRBL_TEST_TIMEOUT_MS);
	d2_sample_t p = d2_position();
	TEST_ASSERT_TRUE_MESSAGE(p.x >= 0 && p.x < 5, "probe-away did not stop before target");
	assert_probe_report(true);
}

static void test_g38_4_stops_on_release(void) { run_probe_away("G38.4X5F120"); }
static void test_g38_5_stops_on_release(void) { run_probe_away("G38.5X5F120"); }

static void test_non_error_probe_failures_reach_target(void)
{
	test_io_set(TEST_IO_PROBE, false);
	grbl_test_command_ok("G38.3X1F300"); d2_wait_idle_timeout(GRBL_TEST_TIMEOUT_MS);
	d2_sample_t p = d2_position(); TEST_ASSERT_FLOAT_WITHIN(D2_POSITION_TOLERANCE, 1, p.x);
	assert_probe_report(false);
	test_io_set(TEST_IO_PROBE, true);
	grbl_test_command_ok("G38.5X2F300"); d2_wait_idle_timeout(GRBL_TEST_TIMEOUT_MS);
	p = d2_position(); TEST_ASSERT_FLOAT_WITHIN(D2_POSITION_TOLERANCE, 2, p.x);
	assert_probe_report(false);
}

static void test_probe_initial_state_alarm(void)
{
	test_io_set(TEST_IO_PROBE, true);
	grbl_test_command_expect("G38.2X1F300", "ALARM:4");
}

D2_UNITY_MAIN(
	RUN_TEST(test_g38_2_stops_on_contact);
	RUN_TEST(test_g38_3_stops_on_contact);
	RUN_TEST(test_g38_4_stops_on_release);
	RUN_TEST(test_g38_5_stops_on_release);
	RUN_TEST(test_non_error_probe_failures_reach_target);
	RUN_TEST(test_probe_initial_state_alarm))
