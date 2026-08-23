#include "../common/domain1_test.h"

void setUp(void) {}
void tearDown(void) {}

static void test_domain1_a_lexical_and_preprocessing(void)
{
	static const d1_case_t cases[] = {
		D1_CASE("D1-A-001", "", "ok\r\n"),
		D1_CASE("D1-A-002", "   ", "ok\r\n"),
		D1_CASE("D1-A-003", "g1x1f100", "ok\r\n"),
		D1_CASE("D1-A-004", "G1 X1 Y2 F100", "ok\r\n"),
		D1_CASE("D1-A-005", "G1X1(comment)F100", "ok\r\n"),
		D1_CASE("D1-A-006", "G1X1F100;comment", "ok\r\n"),
		// D1_CASE("D1-A-007", "/G1X1F100", "ok\r\n"), // ignore this one for now as block ignore is not implemented
		D1_CASE("D1-A-008", "G1X+1F100", "ok\r\n"),
		D1_CASE("D1-A-009", "G1X.5F100", "ok\r\n"),
		D1_CASE("D1-A-010", "G1X1.F100", "ok\r\n"),
		D1_CASE("D1-A-011", "G01X0001F0100", "ok\r\n"),
		D1_CASE("D1-A-012", "(comment only)", "ok\r\n"),
		D1_CASE("D1-A-013", "1G0", "error:1\r\n"),
		D1_CASE("D1-A-014", "G", "error:2\r\n"),
		D1_CASE("D1-A-015", "G1X1.2.3F100", "error:"),
		D1_CASE("D1-A-016", "G1X1X2F100", "error:25\r\n"),
		D1_CASE("D1-A-017", "G1X1F-1", "error:4\r\n"),
		D1_CASE("D1-A-018", "S-1", "error:4\r\n"),
		D1_CASE("D1-A-019", "G4P-1", "error:4\r\n"),
		D1_CASE("D1-A-020", "T-1", "error:4\r\n"),
		D1_CASE("D1-A-021", "T256", "error:38\r\n"),
		D1_CASE("D1-A-022", "N10000001G0", "error:27\r\n"),
		D1_CASE("D1-A-023", "A1", "error:20\r\n"),
		D1_CASE("D1-A-024", "G1X1F100AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "error:11\r\n")
	};
	d1_run_cases(cases, D1_COUNT(cases));
}

D1_FIXTURE(test_domain1_a_lexical_and_preprocessing)
