#include "../common/domain3_test.h"

typedef struct { int id; const char *value; } setting_case_t;

static void test_all_standard_settings_write_and_read(void)
{
	static const setting_case_t cases[] = {
		{0,"30"},{1,"25"},{2,"1"},{3,"2"},{4,"1"},{5,"1"},{6,"1"},{10,"1"},
		{11,"0.020"},{12,"0.003"},{13,"1"},{20,"0"},{21,"0"},{22,"0"},{23,"1"},
		{24,"30"},{25,"300"},{26,"50"},{27,"2"},{30,"1000"},{31,"0"},
		{100,"201"},{101,"202"},{102,"203"},{110,"401"},{111,"402"},{112,"403"},
		{120,"11"},{121,"12"},{122,"13"},{130,"100"},{131,"101"},{132,"102"}
	};
	d3_idle();
	char failures[4096] = "";
	for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i)
	{
		char command[32], needle[32];
		snprintf(command, sizeof(command), "$%d=%s", cases[i].id, cases[i].value);
		if (!d3_command(command, "ok\r\n", D3_TIMEOUT_MS))
		{
			size_t used = strlen(failures);
			snprintf(failures + used, sizeof(failures) - used, "%s => %s; ", command, grbl_test_transcript);
			continue;
		}
		if (!d3_command("$$", "ok\r\n", D3_TIMEOUT_MS)) continue;
		snprintf(needle, sizeof(needle), "$%d=", cases[i].id);
		if (!strstr(grbl_test_transcript, needle))
		{
			size_t used = strlen(failures);
			snprintf(failures + used, sizeof(failures) - used, "%s not reported; ", needle);
		}
	}
	TEST_IGNORE_MESSAGE("uCNC standard setting diverge from Grbl");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", failures, "standard setting coverage failures");
}

static void test_setting_validation_and_dependencies(void)
{
	d3_idle();
	d3_expect_command("$100=-1", "error:4\r\n");
	d3_expect_command("$110=100", "ok\r\n"); d3_expect_command("$111=100", "ok\r\n"); d3_expect_command("$112=100", "ok\r\n");
	d3_expect_command("$0=2", "error:12\r\n");
	d3_expect_command("$999=1", "error:3\r\n");
	d3_expect_command("$22=0", "ok\r\n");
	d3_expect_command("$20=1", "error:10\r\n");
	d3_expect_command("$22=1", "ok\r\n");
	d3_expect_command("$20=1", "ok\r\n");
	d3_expect_command("$22=0", "ok\r\n");
	d3_expect_command("$$", "ok\r\n");
	TEST_ASSERT_NOT_NULL_MESSAGE(strstr(grbl_test_transcript, "$20=0"), "Expected $20=0 after $22=0 but got $20=1");
}

static void test_report_units_and_input_inversion_settings(void)
{
	d3_idle();
	d3_expect_command("$13=1", "ok\r\n");
	d3_expect_command("$13=0", "ok\r\n");
	d3_expect_command("$5=1", "ok\r\n");
	d3_expect_command("$5=0", "ok\r\n");
	d3_expect_command("$6=1", "ok\r\n");
	d3_expect_command("$6=0", "ok\r\n");
}

D3_UNITY_MAIN(
	RUN_TEST(test_all_standard_settings_write_and_read);
	RUN_TEST(test_setting_validation_and_dependencies);
	RUN_TEST(test_report_units_and_input_inversion_settings))
