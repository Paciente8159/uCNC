#include "../common/domain1_test.h"

void setUp(void) {}
void tearDown(void) {}

static void test_domain1_d_parameter_semantics(void)
{
	static const d1_case_t cases[] = {
		D1_CASE("D1-D-001", "G4", "error:28\r\n"), D1_CASE("D1-D-002", "G10L2P1", "error:26\r\n"),
		D1_CASE("D1-D-003", "G10X1", "error:28\r\n"), D1_CASE("D1-D-004", "G10L3P1X0", "error:20\r\n"),
		D1_CASE("D1-D-005", "G10L2P7X0", "error:29\r\n"), D1_CASE("D1-D-006", "G10L2P1X0R1", "error:20\r\n"),
		D1_CASE("D1-D-007", "G92", "error:26\r\n"), D1_CASE("D1-D-008", "G1X1", "error:22\r\n"),
		D1_CASE("D1-D-009", "G93G1X1", "error:22\r\n"), D1_CASE("D1-D-010", "G2I1J0F100", "error:26\r\n"),
		D1_CASE("D1-D-011", "G17G2Z1I1J0F100", "error:32\r\n"), D1_CASE("D1-D-012", "G2X0Y0R1F100", "error:33\r\n"),
		D1_CASE("D1-D-013", "G2X10Y0R1F100", "error:34\r\n"), D1_CASE("D1-D-014", "G17G2X1Y0K1F100", "error:35\r\n"),
		D1_CASE("D1-D-015", "G17G2X1Y0I0.1J0F100", "error:33\r\n"), D1_CASE("D1-D-016", "G38.2F100", "error:26\r\n"),
		D1_CASE("D1-D-017", "G38.2X0F100", "error:33\r\n"), D1_CASE("D1-D-018", "I1", "error:36\r\n"),
		D1_CASE("D1-D-019", "J1", "error:36\r\n"), D1_CASE("D1-D-020", "K1", "error:36\r\n"),
		D1_CASE("D1-D-021", "L2", "error:36\r\n"), D1_CASE("D1-D-022", "P1", "error:36\r\n"),
		D1_CASE("D1-D-023", "R1", "error:36\r\n")
	};
	d1_run_cases(cases, D1_COUNT(cases));
}

D1_FIXTURE(test_domain1_d_parameter_semantics)
