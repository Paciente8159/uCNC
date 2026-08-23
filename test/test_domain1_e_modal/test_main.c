#include "../common/domain1_test.h"

void setUp(void) { grbl_test_start(); }
void tearDown(void) { grbl_test_stop(); }

typedef struct
{
	const char *id;
	const char *command;
	const char *modal;
} d1_modal_case_t;

static const d1_modal_case_t cases[] = {
	{"D1-E-001", "G20", "G20"}, {"D1-E-002", "G21", "G21"}, {"D1-E-003", "G90", "G90"}, {"D1-E-004", "G91", "G91"}, {"D1-E-005", "G17", "G17"}, {"D1-E-006", "G18", "G18"}, {"D1-E-007", "G19", "G19"}, {"D1-E-008", "G93", "G93"}, {"D1-E-009", "G94", "G94"}, {"D1-E-010", "G54", "G54"}, {"D1-E-011", "G59", "G59"}, {"D1-E-012", "M3S100", "M3"}, {"D1-E-013", "M4S100", "M4"}, {"D1-E-014", "M5", "M5"},
#ifdef ENABLE_COOLANT
	{"D1-E-015", "M8", "M8"},
	{"D1-E-016", "M9", "M9"}
#endif
};

static const d1_modal_case_t *d1_current_modal_case;

static void d1_run_current_modal_case(void)
{
	TEST_ASSERT_TRUE_MESSAGE(d1_enter_check(), "could not establish Check state");
	TEST_ASSERT_TRUE_MESSAGE(d1_send_line(d1_current_modal_case->command, "ok\r\n"), "modal command rejected");
	TEST_ASSERT_TRUE_MESSAGE(d1_get_modal(grbl_test_transcript, sizeof(grbl_test_transcript)), "could not read modal report");
	TEST_ASSERT_NOT_NULL_MESSAGE(strstr(grbl_test_transcript, d1_current_modal_case->modal), "expected modal value was not reported by $G");
	TEST_ASSERT_TRUE_MESSAGE(d1_still_in_check(), "controller left Check state");
}

int main(void)
{
	UNITY_BEGIN();
	for (size_t i = 0; i < D1_COUNT(cases); ++i)
	{
		d1_current_modal_case = &cases[i];
		UnityDefaultTestRun(d1_run_current_modal_case, cases[i].id, __LINE__);
	}
	return UNITY_END();
}
