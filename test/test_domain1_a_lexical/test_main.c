#include "../common/domain1_test.h"

void setUp(void) { grbl_test_start(); }
void tearDown(void) { grbl_test_stop(); }

static const d1_case_t cases[] = {
	D1_CASE("D1-A-001", "", "ok\r\n", false),
	D1_CASE("D1-A-002", "   ", "ok\r\n", false),
	D1_CASE("D1-A-003", "g1x1f100", "ok\r\n", false),
	D1_CASE("D1-A-004", "G1 X1 Y2 F100", "ok\r\n", false),
	D1_CASE("D1-A-005", "G1X1(comment)F100", "ok\r\n", false),
	D1_CASE("D1-A-006", "G1X1F100;comment", "ok\r\n", false),
#ifdef PARSER_SUPPORTS_BLOCK_IGNORE
	D1_CASE("D1-A-007", "/G1X1F100", "ok\r\n", false), // ignore this one for now as block ignore is not implemented*/
#endif
	D1_CASE("D1-A-008", "G1X+1F100", "ok\r\n", false),
	D1_CASE("D1-A-009", "G1X.5F100", "ok\r\n", false),
	D1_CASE("D1-A-010", "G1X1.F100", "ok\r\n", false),
	D1_CASE("D1-A-011", "G01X0001F0100", "ok\r\n", false),
	D1_CASE("D1-A-012", "(comment only)", "ok\r\n", false),
	D1_CASE("D1-A-013", "1G0", "error:1\r\n", false),
	D1_CASE("D1-A-014", "G", "error:2\r\n", false),
	D1_CASE("D1-A-015", "G1X1.2.3F100", "error:", false),
	D1_CASE("D1-A-016", "G1X1X2F100", "error:25\r\n", false),
	D1_CASE("D1-A-017", "G1X1F-1", "error:4\r\n", false),
	D1_CASE("D1-A-018", "S-1", "error:4\r\n", false),
	D1_CASE("D1-A-019", "G4P-1", "error:4\r\n", false),
	D1_CASE("D1-A-020", "T-1", "error:4\r\n", false),
#if TOOL_COUNT > 1
	D1_CASE("D1-A-021", "T256", "error:38\r\n", false),
#else
	D1_CASE("D1-A-021", "T256", "error:38\r\n", true),
#endif
#if GCODE_COUNT_TEXT_LINES
	D1_CASE("D1-A-022", "N10000001G0", "error:27\r\n", false),
#else
	D1_CASE("D1-A-022", "N10000001G0", "error:27\r\n", true),
#endif
#if AXIS_COUNT < 4 && !defined(IGNORE_UNDEFINED_AXIS)
	D1_CASE("D1-A-023", "A1", "error:20\r\n", false),
#else
	D1_CASE("D1-A-023", "A1", "error:20\r\n", true),
#endif
	D1_CASE("D1-A-024", "G1X1F100AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "error:11\r\n", true) // not easy to induce in uCNC since the buffer is read live*/
};

D1_CASE_FIXTURE(cases)
