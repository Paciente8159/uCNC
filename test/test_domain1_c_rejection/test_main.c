#include "../common/domain1_test.h"

void setUp(void) { grbl_test_start(); }
void tearDown(void) { grbl_test_stop(); }

static const d1_case_t cases[] = {
	D1_CASE("D1-C-001", "G33X1K1", "error:20\r\n", false),
	D1_CASE("D1-C-002", "G41", "error:20\r\n", true), D1_CASE("D1-C-003", "G42", "error:20\r\n", true), // G40/41/42 are ignored and not implemented
	D1_CASE("D1-C-004", "G43Z1", "error:22\r\n", true),													// supported as well as G43.1Zx
	D1_CASE("D1-C-005", "G59.1", "error:20\r\n", true),													// supported
#ifndef DISABLE_PATH_MODES
	D1_CASE("D1-C-006", "G64", "ok\r\n", false), // supported
#else
	D1_CASE("D1-C-006", "G64", "error:20\r\n", false),
#endif
	D1_CASE("D1-C-007", "G90.1", "error:20\r\n", false), D1_CASE("D1-C-008", "G95", "error:20\r\n", false),
	D1_CASE("D1-C-009", "G38.1X1F100", "error:20\r\n", false), D1_CASE("D1-C-010", "G38.6X1F100", "error:20\r\n", false),
	D1_CASE("D1-C-011", "M6", "error:20\r\n", false),
	D1_CASE("D1-C-012", "M48", "error:20\r\n", true), // supported
	D1_CASE("D1-C-013", "M3.1", "error:23\r\n", false),
	D1_CASE("D1-C-014", "G1.5X1F100", "error:23\r\n", false),
	D1_CASE("D1-C-015", "G0G1X1F100", "error:21\r\n", false), D1_CASE("D1-C-016", "G17G18", "error:21\r\n", false),
	D1_CASE("D1-C-017", "G90G91", "error:21\r\n", false), D1_CASE("D1-C-018", "G20G21", "error:21\r\n", false),
	D1_CASE("D1-C-019", "M3M5", "error:21\r\n", false), D1_CASE("D1-C-020", "M0M2", "error:21\r\n", false),
	D1_CASE("D1-C-021", "G10L2P1X0G1", "error:24\r\n", false), D1_CASE("D1-C-022", "G92X0G0", "error:24\r\n", false),
	D1_CASE("D1-C-023", "G80X1", "error:31\r\n", false), D1_CASE("D1-C-024", "G2X1Y0I0.5J0F100G53", "error:30\r\n", false),
	D1_CASE("D1-C-025", "G43.1X1", "error:37\r\n", false), D1_CASE("D1-C-026", "G43.1Z1X1", "error:37\r\n", false)};

D1_CASE_FIXTURE(cases)
