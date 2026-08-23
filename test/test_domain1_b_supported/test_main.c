#include "../common/domain1_test.h"

void setUp(void) {}
void tearDown(void) {}

static void test_domain1_b_supported_commands(void)
{
	static const d1_case_t cases[] = {
		D1_CASE("D1-B-001", "G4P0.01", "ok\r\n"), D1_CASE("D1-B-002", "G10L2P1X0Y0Z0", "ok\r\n"),
		D1_CASE("D1-B-003", "G10L20P1X0Y0Z0", "ok\r\n"), D1_CASE("D1-B-004", "G28", "ok\r\n"),
		D1_CASE("D1-B-005", "G30", "ok\r\n"),
		// D1_CASE("D1-B-006", "G28.1", "ok\r\n"), D1_CASE("D1-B-007", "G30.1", "ok\r\n"), // supported via extension
		D1_CASE("D1-B-008", "G53G0X0", "ok\r\n"),
		D1_CASE("D1-B-009", "G92X0", "ok\r\n"), D1_CASE("D1-B-010", "G92.1", "ok\r\n"),
		D1_CASE("D1-B-011", "G0X1", "ok\r\n"), D1_CASE("D1-B-012", "G1X1F100", "ok\r\n"),
		D1_CASE("D1-B-013", "G17G2X1Y0I0.5J0F100", "ok\r\n"), D1_CASE("D1-B-014", "G17G3X1Y0I0.5J0F100", "ok\r\n"),
		// D1_CASE("D1-B-015", "G38.2X1F100", "ok\r\n"), D1_CASE("D1-B-016", "G38.3X1F100", "ok\r\n"), D1_CASE("D1-B-017", "G38.4X1F100", "ok\r\n"), D1_CASE("D1-B-018", "G38.5X1F100", "ok\r\n"), // no point in testing in check mode
		D1_CASE("D1-B-019", "G80", "ok\r\n"), D1_CASE("D1-B-020", "G93", "ok\r\n"),
		D1_CASE("D1-B-021", "G94", "ok\r\n"), D1_CASE("D1-B-022", "G20", "ok\r\n"),
		D1_CASE("D1-B-023", "G21", "ok\r\n"), D1_CASE("D1-B-024", "G90", "ok\r\n"),
		D1_CASE("D1-B-025", "G91", "ok\r\n"), // D1_CASE("D1-B-026", "G91.1", "ok\r\n"), //no supported
		D1_CASE("D1-B-027", "G17", "ok\r\n"), D1_CASE("D1-B-028", "G18", "ok\r\n"),
		D1_CASE("D1-B-029", "G19", "ok\r\n"), D1_CASE("D1-B-030", "G43.1Z1", "ok\r\n"),
		D1_CASE("D1-B-031", "G49", "ok\r\n"), D1_CASE("D1-B-032", "G40", "ok\r\n"),
		D1_CASE("D1-B-033", "G54", "ok\r\n"), D1_CASE("D1-B-034", "G55", "ok\r\n"),
		D1_CASE("D1-B-035", "G56", "ok\r\n"), D1_CASE("D1-B-036", "G57", "ok\r\n"),
		D1_CASE("D1-B-037", "G58", "ok\r\n"), D1_CASE("D1-B-038", "G59", "ok\r\n"),
		D1_CASE("D1-B-039", "G61", "ok\r\n"), D1_CASE("D1-B-040", "M0", "ok\r\n"),
		D1_CASE("D1-B-041", "M1", "ok\r\n"), D1_CASE("D1-B-042", "M2", "ok\r\n"),
		D1_CASE("D1-B-043", "M30", "ok\r\n"), D1_CASE("D1-B-044", "M3S100", "ok\r\n"),
		D1_CASE("D1-B-045", "M4S100", "ok\r\n"), D1_CASE("D1-B-046", "M5", "ok\r\n"),
		D1_CASE("D1-B-047", "M8", "ok\r\n"), D1_CASE("D1-B-048", "M9", "ok\r\n"),
#ifdef ENABLE_COOLANT
		D1_CASE("D1-B-049", "M7", "ok\r\n"),
#else
		D1_CASE("CAP-O-001", "M7", "error:20\r\n"),
#endif
		D1_CASE("D1-B-050", "F100", "ok\r\n"), D1_CASE("D1-B-051", "S100", "ok\r\n"),
		D1_CASE("D1-B-052", "T1", "ok\r\n"), D1_CASE("D1-B-053", "N1", "ok\r\n")
	};
	d1_run_cases(cases, D1_COUNT(cases));
}

D1_FIXTURE(test_domain1_b_supported_commands)
