#include <unity.h>

#include "src/hal/mcus/virtual/mcumap_virtual.h"
#include "src/hal/mcus/mcu.h"

void setUp(void)
{
	mcu_unit_test_runtime_reset();
}

void tearDown(void) {}

static void test_configured_types_after_init(void)
{
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_OUTPUT, io_pins[STEP0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_OUTPUT, io_pins[DIR0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_PWM, io_pins[PWM0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_SERVO, io_pins[SERVO0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_OUTPUT, io_pins[DOUT0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_INPUT, io_pins[LIMIT_X].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_INPUT, io_pins[PROBE].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_INPUT, io_pins[DIN0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_ANALOG, io_pins[ANALOG0].type);
	/* unconfigured gap between DOUT30 (77) and LIMIT_X (100) */
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_UNDEF, io_pins[78].type);
}

static void test_configured_values_start_zeroed(void)
{
	TEST_ASSERT_EQUAL_UINT16(0, io_pins[STEP0].value);
	TEST_ASSERT_EQUAL_UINT16(0, io_pins[DOUT0].value);
	TEST_ASSERT_EQUAL_UINT16(0, io_pins[PWM0].value);
	TEST_ASSERT_EQUAL_UINT16(0, io_pins[ANALOG0].value);
}

static void test_output_value_round_trips(void)
{
	mcu_set_output(DOUT0);
	TEST_ASSERT_EQUAL_UINT8(1, mcu_get_output(DOUT0));
	mcu_clear_output(DOUT0);
	TEST_ASSERT_EQUAL_UINT8(0, mcu_get_output(DOUT0));
	mcu_toggle_output(DOUT0);
	TEST_ASSERT_EQUAL_UINT8(1, mcu_get_output(DOUT0));
	mcu_toggle_output(DOUT0);
	TEST_ASSERT_EQUAL_UINT8(0, mcu_get_output(DOUT0));
}

static void test_pwm_value_round_trip(void)
{
	mcu_set_pwm(PWM0, 123);
	TEST_ASSERT_EQUAL_UINT8(123, mcu_get_pwm(PWM0));
	TEST_ASSERT_EQUAL_UINT16(123, io_pins[PWM0].value);
}

static void test_servo_value_round_trip(void)
{
	mcu_set_servo(SERVO0, 200);
	TEST_ASSERT_EQUAL_UINT8(200, mcu_get_servo(SERVO0));
}

static void test_input_value_reads(void)
{
	io_pins[LIMIT_X].value = 1;
	TEST_ASSERT_EQUAL_UINT8(1, mcu_get_input(LIMIT_X));
	io_pins[LIMIT_X].value = 0;
	TEST_ASSERT_EQUAL_UINT8(0, mcu_get_input(LIMIT_X));
}

static void test_analog_value_reads(void)
{
	io_pins[ANALOG0].value = 512;
	TEST_ASSERT_EQUAL_UINT16(512, mcu_get_analog(ANALOG0));
}

static void test_config_functions_change_type(void)
{
	mcu_config_input(DOUT0);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_INPUT, io_pins[DOUT0].type);
	mcu_config_output(DOUT0);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_OUTPUT, io_pins[DOUT0].type);
	mcu_config_pwm(PWM0, 1000);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_PWM, io_pins[PWM0].type);
	mcu_config_analog(ANALOG0);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_ANALOG, io_pins[ANALOG0].type);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_configured_types_after_init);
	RUN_TEST(test_configured_values_start_zeroed);
	RUN_TEST(test_output_value_round_trips);
	RUN_TEST(test_pwm_value_round_trip);
	RUN_TEST(test_servo_value_round_trip);
	RUN_TEST(test_input_value_reads);
	RUN_TEST(test_analog_value_reads);
	RUN_TEST(test_config_functions_change_type);
	return UNITY_END();
}