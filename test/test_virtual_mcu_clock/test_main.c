#include <unity.h>

#include "src/hal/mcus/virtual/mcumap_virtual.h"

static unsigned event_count;

static void count_event(void *argument)
{
	(void)argument;
	event_count++;
}

void setUp(void)
{
	event_count = 0;
	mcu_unit_test_clock_reset();
}

void tearDown(void) {}

static void test_clock_advances_exactly_and_runs_due_events(void)
{
	mcu_add_event(2500U, count_event, NULL);

	mcu_unit_test_advance_time(2499U);
	TEST_ASSERT_EQUAL_UINT32(2499U, mcu_micros());
	TEST_ASSERT_EQUAL_UINT(0U, event_count);

	mcu_unit_test_advance_time(1U);
	TEST_ASSERT_EQUAL_UINT32(2500U, mcu_micros());
	TEST_ASSERT_EQUAL_UINT(1U, event_count);
}

static void test_clock_reset_discards_pending_events(void)
{
	mcu_add_event(1U, count_event, NULL);
	mcu_unit_test_clock_reset();
	mcu_unit_test_advance_time(1U);

	TEST_ASSERT_EQUAL_UINT32(1U, mcu_micros());
	TEST_ASSERT_EQUAL_UINT(0U, event_count);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_clock_advances_exactly_and_runs_due_events);
	RUN_TEST(test_clock_reset_discards_pending_events);
	return UNITY_END();
}
