#include "../common/domain3_test.h"

static void expect_alarm_lock_and_unlock(const char *alarm)
{
	grbl_test_assert_wait_for(alarm);
	d3_expect_state("<Alarm", D3_TIMEOUT_MS);
	d3_expect_command("G0X0", "error:9\r\n");
}

static void test_hard_limits_idle_each_axis(void)
{
	const test_io_id_t limits[] = {TEST_IO_LIMIT_X, TEST_IO_LIMIT_Y, TEST_IO_LIMIT_Z};
	for (size_t i = 0; i < 3; ++i)
	{
		d3_idle(); d3_expect_command("$21=1", "ok\r\n");
		grbl_test_clear_output(); test_io_set(limits[i], true);
		expect_alarm_lock_and_unlock("ALARM:1");
		test_io_set(limits[i], false);
	}
}

static void test_hard_limit_during_run_stops_motion(void)
{
	d3_idle(); d3_expect_command("$21=1", "ok\r\n"); d3_start_long_motion();
	grbl_test_clear_output(); test_io_set(TEST_IO_LIMIT_X, true);
	grbl_test_assert_wait_for("ALARM:1"); d3_expect_state("<Alarm", D3_TIMEOUT_MS);
	d3_expect_command("G0X0", "error:9\r\n");
	test_io_set(TEST_IO_LIMIT_X, false); d3_reset(); d3_expect_command("$X", "ok\r\n");
}

static void test_disabled_hard_limit_is_ignored(void)
{
	d3_idle(); d3_expect_command("$21=0", "ok\r\n");
	test_io_set(TEST_IO_LIMIT_X, true);
	TEST_ASSERT_FALSE(d3_wait_state("<Alarm", 300U));
	test_io_set(TEST_IO_LIMIT_X, false); d3_expect_state("<Idle", D3_TIMEOUT_MS);
}

static void test_soft_limit_alarm_and_lock(void)
{
	d3_idle();
	d3_expect_command("$130=10", "ok\r\n"); d3_expect_command("$22=1", "ok\r\n"); d3_expect_command("$20=1", "ok\r\n");
	grbl_test_clear_output();
	char line[] = "G53G0X-11\n"; TEST_ASSERT_TRUE(mcu_unit_test_inject(line));
	expect_alarm_lock_and_unlock("ALARM:2");
	d3_reset(); d3_expect_command("$X", "ok\r\n");
}

static void test_probe_alarm_4_and_5(void)
{
	d3_idle(); test_io_set(TEST_IO_PROBE, true);
	grbl_test_clear_output(); TEST_ASSERT_TRUE(mcu_unit_test_inject("G38.2X1F300\n"));
	expect_alarm_lock_and_unlock("ALARM:4");
	test_io_set(TEST_IO_PROBE, false); d3_idle();
	grbl_test_clear_output(); TEST_ASSERT_TRUE(mcu_unit_test_inject("G38.2X1F300\n"));
	expect_alarm_lock_and_unlock("ALARM:5");
}

static void test_reset_during_run_alarm_3(void)
{
	d3_idle(); d3_start_long_motion();
	grbl_test_clear_output(); d3_realtime(0x18);
	grbl_test_assert_wait_for("Grbl "); grbl_test_assert_wait_for("ALARM:3");
	d3_expect_state("<Alarm", D3_TIMEOUT_MS);
}

D3_UNITY_MAIN(
	RUN_TEST(test_hard_limits_idle_each_axis);
	RUN_TEST(test_hard_limit_during_run_stops_motion);
	RUN_TEST(test_disabled_hard_limit_is_ignored);
	RUN_TEST(test_soft_limit_alarm_and_lock);
	RUN_TEST(test_probe_alarm_4_and_5);
	RUN_TEST(test_reset_during_run_alarm_3))
