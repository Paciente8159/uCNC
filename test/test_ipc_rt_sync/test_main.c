/*
 * NonRT/RT startup handshake.
 *
 * This fixture is built by EMULATOR_WINDOWS_IPC_TEST, which enables the split
 * (IPC_MODE=IPC_MULTICORE_NONRT) so ipc_init() actually performs the handshake.
 * It replaces the weak transport stubs of ipc.c with a host controlled one, so
 * both outcomes can be produced: an RT side that answers, and an RT side that
 * never does (a missing or unbooted RT board).
 */
#include <stddef.h>

#include <unity.h>

#include "src/cnc.h"
#include "src/hal/mcus/virtual/mcumap_virtual.h"

static bool test_rt_answers;

/* overrides the weak ipc_send() of ipc.c */
void ipc_send(ipc_packet_type_t type, const void *data, size_t len)
{
	(void)type;
	(void)data;
	(void)len;
}

/* overrides the weak ipc_receive() of ipc.c. A reply always starts with the
 * echoed request type, so one byte is a valid ACK for any request. */
size_t ipc_receive(ipc_packet_type_t type, void *data, size_t len)
{
	(void)type;
	(void)data;
	(void)len;
	return test_rt_answers ? 1 : 0;
}

void setUp(void)
{
#if !(IPC_MODE & IPC_MULTICORE_NONRT)
	/* the handshake only exists in the split build. Everywhere else ipc_init()
	 * is a no-op and no fault can be raised, so there is nothing to assert.
	 * Build this fixture with EMULATOR_WINDOWS_IPC_TEST. */
	TEST_IGNORE_MESSAGE("requires the NonRT/RT split (IPC_MODE=IPC_MULTICORE_NONRT)");
#endif
	mcu_unit_test_runtime_reset();
}

void tearDown(void) {}

/*
 * Runs first: it is the only test that brings the controller up, so the global
 * state is initialized before the direct ipc_init() calls below.
 */
static void test_failed_handshake_locks_the_machine_in_alarm(void)
{
	test_rt_answers = false;
	cnc_init();

	TEST_ASSERT_TRUE(cnc_has_alarm());
	TEST_ASSERT_EQUAL_UINT8(EXEC_STATUS_ALARM, cnc_get_status());

	/* neither a user unlock nor the automatic unlock performed by cnc_run()
	 * may clear it: only a reboot does */
	TEST_ASSERT_EQUAL_UINT8(UNLOCK_ERROR, cnc_unlock(true));
	TEST_ASSERT_TRUE(cnc_has_alarm());
	TEST_ASSERT_EQUAL_UINT8(EXEC_STATUS_ALARM, cnc_get_status());
}

static void test_ipc_init_fails_when_the_rt_side_never_answers(void)
{
	test_rt_answers = false;
	TEST_ASSERT_FALSE(ipc_init());
}

static void test_ipc_init_succeeds_when_the_rt_side_answers(void)
{
	test_rt_answers = true;
	TEST_ASSERT_TRUE(ipc_init());
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_failed_handshake_locks_the_machine_in_alarm);
	RUN_TEST(test_ipc_init_fails_when_the_rt_side_never_answers);
	RUN_TEST(test_ipc_init_succeeds_when_the_rt_side_answers);
	return UNITY_END();
}
