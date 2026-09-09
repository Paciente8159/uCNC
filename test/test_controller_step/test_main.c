#include <unity.h>
#include <string.h>
#include "src/cnc.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_controller_step_processes_one_stream_command_and_returns(void)
{
	cnc_init();
	cnc_unit_test_start();
	mcu_unit_test_buffer_clear();
	TEST_ASSERT_TRUE(mcu_unit_test_inject("$I\n"));

	TEST_ASSERT_TRUE(cnc_unit_test_run_once());

	TEST_ASSERT_NOT_NULL(strstr(mcu_unit_test_buffer(), "[VER:"));
	TEST_ASSERT_NOT_NULL(strstr(mcu_unit_test_buffer(), "ok\r\n"));
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_controller_step_processes_one_stream_command_and_returns);
	return UNITY_END();
}
