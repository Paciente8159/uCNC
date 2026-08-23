#include "../common/domain3_test.h"

static int status_override(unsigned index)
{
	for (uint8_t attempt = 0; attempt < 20U; ++attempt)
	{
		char status[512];
		if (!d3_status(status, sizeof(status))) continue;
		char *ov = strstr(status, "|Ov:");
		unsigned feed = 0, rapid = 0, spindle = 0;
		if (ov && sscanf(ov + 4, "%u,%u,%u", &feed, &rapid, &spindle) == 3)
			return (int)(index == 0 ? feed : index == 1 ? rapid : spindle);
	}
	return -1;
}

static void send_override(uint8_t byte)
{
	d3_expect_no_serial_response(byte, 20U);
}

static void test_feed_override_bytes_and_clamps(void)
{
	d3_idle();
	send_override(0x90); TEST_ASSERT_EQUAL(100, status_override(0));
	send_override(0x91); TEST_ASSERT_EQUAL(110, status_override(0));
	send_override(0x92); TEST_ASSERT_EQUAL(100, status_override(0));
	send_override(0x93); TEST_ASSERT_EQUAL(101, status_override(0));
	send_override(0x94); TEST_ASSERT_EQUAL(100, status_override(0));
	for (int i = 0; i < 20; ++i) send_override(0x91);
	TEST_ASSERT_EQUAL(200, status_override(0));
	for (int i = 0; i < 25; ++i) send_override(0x92);
	TEST_ASSERT_EQUAL(10, status_override(0));
}

static void test_rapid_override_bytes(void)
{
	d3_idle();
	send_override(0x95); TEST_ASSERT_EQUAL(100, status_override(1));
	send_override(0x96); TEST_ASSERT_EQUAL(50, status_override(1));
	send_override(0x97); TEST_ASSERT_EQUAL(25, status_override(1));
	send_override(0x95); TEST_ASSERT_EQUAL(100, status_override(1));
}

static void test_spindle_override_bytes(void)
{
	d3_idle(); d3_expect_command("M3S500", "ok\r\n");
	send_override(0x99); TEST_ASSERT_EQUAL(100, status_override(2));
	send_override(0x9A); TEST_ASSERT_EQUAL(110, status_override(2));
	send_override(0x9B); TEST_ASSERT_EQUAL(100, status_override(2));
	send_override(0x9C); TEST_ASSERT_EQUAL(101, status_override(2));
	send_override(0x9D); TEST_ASSERT_EQUAL(100, status_override(2));
}

static void test_coolant_realtime_toggles(void)
{
	d3_idle();
	uint32_t before = virtualmap.outputs;
	send_override(0xA0);
#ifdef ENABLE_COOLANT
	TEST_ASSERT_NOT_EQUAL(before, virtualmap.outputs);
	send_override(0xA0); TEST_ASSERT_EQUAL(before, virtualmap.outputs);
#else
	TEST_ASSERT_EQUAL(before, virtualmap.outputs);
#endif
}

D3_UNITY_MAIN(
	RUN_TEST(test_feed_override_bytes_and_clamps);
	RUN_TEST(test_rapid_override_bytes);
	RUN_TEST(test_spindle_override_bytes);
	RUN_TEST(test_coolant_realtime_toggles))
