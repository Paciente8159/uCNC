#include "../common/domain3_test.h"

static void test_homing_disabled_error(void)
{
	d3_idle();
	d3_expect_command("$X", "ok\r\n");
	d3_expect_command("$22=0", "ok\r\n");
	d3_expect_command("$H", "error:5\r\n");
}

static void test_reset_during_homing_alarm_6(void)
{
	d3_idle();
	d3_expect_command("$X", "ok\r\n");
	d3_expect_command("$21=1", "ok\r\n");
	d3_expect_command("$22=1", "ok\r\n");
	grbl_test_clear_output();
	TEST_ASSERT_TRUE(mcu_unit_test_inject("$H\n"));
	d3_realtime(0x18);
	grbl_test_assert_wait_for("ALARM:6");
	grbl_test_assert_wait_for("Grbl ");
	d3_expect_state("<Alarm", D3_TIMEOUT_MS);
}

static void test_door_during_homing_alarm_7(void)
{
	d3_idle();
	d3_expect_command("$X", "ok\r\n");
	d3_expect_command("$21=1", "ok\r\n");
	d3_expect_command("$22=1", "ok\r\n");
	grbl_test_clear_output();
	TEST_ASSERT_TRUE(mcu_unit_test_inject("$H\n"));
	test_io_set_after(TEST_IO_SAFETY_DOOR, 10U, false, true);
	grbl_test_assert_wait_for("ALARM:7");
	grbl_test_assert_wait_for("Grbl ");
	d3_expect_state("<Alarm", D3_TIMEOUT_MS);
}

static void test_homing_search_failure_alarm_9(void)
{
	d3_idle();
	d3_expect_command("$X", "ok\r\n");
	d3_expect_command("$21=1", "ok\r\n");
	d3_expect_command("$22=1", "ok\r\n");
	d3_expect_command("$130=1", "ok\r\n");
	d3_expect_command("$131=1", "ok\r\n");
	d3_expect_command("$132=1", "ok\r\n");
	d3_expect_command("$25=500", "ok\r\n");
	grbl_test_clear_output();
	TEST_ASSERT_TRUE(mcu_unit_test_inject("$H\n"));
	grbl_test_assert_wait_for("ALARM:9");
	d3_expect_state("<Alarm", D3_MOTION_TIMEOUT_MS);
}

D3_UNITY_MAIN(
	RUN_TEST(test_homing_disabled_error);
	RUN_TEST(test_reset_during_homing_alarm_6);
	RUN_TEST(test_door_during_homing_alarm_7);
	RUN_TEST(test_homing_search_failure_alarm_9))
