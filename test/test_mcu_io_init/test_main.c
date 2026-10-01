#include <unity.h>
#include <stdio.h>

#include "src/hal/mcus/virtual/mcumap_virtual.h"
#include "src/hal/mcus/mcu.h"

void setUp(void)
{
	mcu_unit_test_runtime_reset();
}

void tearDown(void) {}

static void reset_all_io_pins(void)
{
	for (uint16_t pin = 0; pin < IO_PIN_COUNT; pin++)
	{
		io_pins[pin].type = IO_PIN_UNDEF;
		io_pins[pin].value = 0;
	}
}

/*
 * Independent spec of what mcu_io_init() must configure for the virtual MCU
 * build. This is deliberately defined here rather than read back from
 * io_pin_info[] (or built with the same mcu_config_* calls as mcu.c), so a
 * missing configuration in mcu_outputs_init/mcu_inputs_init/mcu_coms_init is
 * caught as a test failure instead of silently leaving a pin unconfigured.
 */
static io_pin_type_t expected_mcu_io_init_type(uint16_t pin)
{
	/* step/dir/enable */
	if (pin >= 1 && pin <= 24)
		return IO_PIN_OUTPUT;
	/* pwm */
	if (pin >= 25 && pin <= 40)
		return IO_PIN_PWM;
	/* servo */
	if (pin >= 41 && pin <= 46)
		return IO_PIN_SERVO;
	/* generic outputs DOUT0..DOUT49 */
	if (pin >= 47 && pin <= 96)
		return IO_PIN_OUTPUT;
	/* unconfigured gap between DOUT49 (96) and LIMIT_X (100) */
	if (pin >= 97 && pin <= 99)
		return IO_PIN_UNDEF;
	/* control inputs */
	if (pin >= 100 && pin <= 113)
		return IO_PIN_INPUT;
	/* analog inputs ANALOG0..ANALOG15 */
	if (pin >= 114 && pin <= 129)
		return IO_PIN_ANALOG;
	/* generic inputs DIN0..DIN49 */
	if (pin >= 130 && pin <= 179)
		return IO_PIN_INPUT;
	/* unconfigured gap between DIN49 (179) and TX (200) */
	if (pin >= 180 && pin <= 199)
		return IO_PIN_UNDEF;
	/* hidden comms pins: only UART2 is enabled in the virtual build */
	if (pin == 210)
		return IO_PIN_OUTPUT; /* TX2 */
	if (pin == 211)
		return IO_PIN_INPUT; /* RX2 */
	return IO_PIN_UNDEF;      /* TX/RX/USB/SPI/I2C peripherals disabled */
}

static void test_mcu_io_init_configures_all_pins(void)
{
	reset_all_io_pins();
	mcu_io_init();

	for (uint16_t pin = 0; pin < IO_PIN_COUNT; pin++)
	{
		char msg[64];
		snprintf(msg, sizeof(msg), "pin %u type mismatch", (unsigned)pin);
		TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected_mcu_io_init_type(pin), io_pins[pin].type, msg);
	}
}

static void test_configured_types_after_init(void)
{
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_OUTPUT, io_pins[STEP0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_OUTPUT, io_pins[DIR0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_PWM, io_pins[PWM0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_SERVO, io_pins[SERVO0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_OUTPUT, io_pins[DOUT0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_OUTPUT, io_pins[DOUT49].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_INPUT, io_pins[LIMIT_X].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_INPUT, io_pins[PROBE].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_INPUT, io_pins[DIN0].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_INPUT, io_pins[DIN49].type);
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_ANALOG, io_pins[ANALOG0].type);
	/* unconfigured gap between DOUT49 (96) and LIMIT_X (100) */
	TEST_ASSERT_EQUAL_UINT8(IO_PIN_UNDEF, io_pins[97].type);
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
	RUN_TEST(test_mcu_io_init_configures_all_pins);
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