#include "../common/domain1_test.h"

void setUp(void) {}
void tearDown(void) {}

typedef struct
{
	const char *id;
	const char *command;
	const char *modal;
} d1_modal_case_t;

static void test_domain1_e_modal_reports(void)
{
	static const d1_modal_case_t cases[] = {
		{"D1-E-001", "G20", "G20"}, {"D1-E-002", "G21", "G21"},
		{"D1-E-003", "G90", "G90"}, {"D1-E-004", "G91", "G91"},
		{"D1-E-005", "G17", "G17"}, {"D1-E-006", "G18", "G18"}, {"D1-E-007", "G19", "G19"},
		{"D1-E-008", "G93", "G93"}, {"D1-E-009", "G94", "G94"},
		{"D1-E-010", "G54", "G54"}, {"D1-E-011", "G59", "G59"},
		{"D1-E-012", "M3S100", "M3"}, {"D1-E-013", "M4S100", "M4"}, {"D1-E-014", "M5", "M5"},
		{"D1-E-015", "M8", "M8"}, {"D1-E-016", "M9", "M9"}
	};
	char failures[4096] = "";
	for (size_t i = 0; i < D1_COUNT(cases); ++i)
	{
		if (!d1_reset_and_enter_check() || !d1_send_line(cases[i].command, "ok\r\n"))
		{
			d1_append_failure(failures, sizeof(failures), cases[i].id, "modal command rejected");
			continue;
		}
		if (!d1_get_modal(grbl_test_transcript, sizeof(grbl_test_transcript)) || !strstr(grbl_test_transcript, cases[i].modal))
		{
			char reason[384];
			snprintf(reason, sizeof(reason), "expected `%s` in `$G`, received `%s`", cases[i].modal, grbl_test_transcript);
			d1_append_failure(failures, sizeof(failures), cases[i].id, reason);
		}
		if (!d1_still_in_check())
		{
			d1_append_failure(failures, sizeof(failures), cases[i].id, "controller left Check state");
		}
	}
	d1_suite_stop();
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", failures, "Domain 1 modal-report failures");
}

D1_FIXTURE(test_domain1_e_modal_reports)
