#ifndef CNC_PIO_TESTING_H
#define CNC_PIO_TESTING_H

#ifdef PIO_UNIT_TESTING

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
	TEST_IO_ESTOP = 0,
	TEST_IO_SAFETY_DOOR,
	TEST_IO_FHOLD,
	TEST_IO_CS_RES,
	TEST_IO_PROBE,

	TEST_IO_LIMIT_X,
	TEST_IO_LIMIT_X2,
	TEST_IO_LIMIT_Y,
	TEST_IO_LIMIT_Y2,
	TEST_IO_LIMIT_Z,
	TEST_IO_LIMIT_Z2,
	TEST_IO_LIMIT_A,
	TEST_IO_LIMIT_B,
	TEST_IO_LIMIT_C,

	TEST_IO_COUNT
} test_io_id_t;

bool test_io_condition(uint8_t input);
void test_io_reset(void);
void test_io_set(uint8_t input, bool value);
void test_io_set_after(uint8_t input, uint32_t delay_ms, bool initial_value, bool final_value);

typedef bool (*test_io_callback_t)(void);
void test_io_set_callback(uint8_t input, test_io_callback_t cb);

#define IO_CONDITION_ESTOP	   test_io_condition(TEST_IO_ESTOP)
#define IO_CONDITION_SAFETY_DOOR test_io_condition(TEST_IO_SAFETY_DOOR)
#define IO_CONDITION_FHOLD	   test_io_condition(TEST_IO_FHOLD)
#define IO_CONDITION_CS_RES	   test_io_condition(TEST_IO_CS_RES)
#define IO_CONDITION_PROBE	   test_io_condition(TEST_IO_PROBE)

#define IO_CONDITION_LIMIT_X	 test_io_condition(TEST_IO_LIMIT_X)
#define IO_CONDITION_LIMIT_X2	test_io_condition(TEST_IO_LIMIT_X2)
#define IO_CONDITION_LIMIT_Y	 test_io_condition(TEST_IO_LIMIT_Y)
#define IO_CONDITION_LIMIT_Y2	test_io_condition(TEST_IO_LIMIT_Y2)
#define IO_CONDITION_LIMIT_Z	 test_io_condition(TEST_IO_LIMIT_Z)
#define IO_CONDITION_LIMIT_Z2	test_io_condition(TEST_IO_LIMIT_Z2)
#define IO_CONDITION_LIMIT_A	 test_io_condition(TEST_IO_LIMIT_A)
#define IO_CONDITION_LIMIT_B	 test_io_condition(TEST_IO_LIMIT_B)
#define IO_CONDITION_LIMIT_C	 test_io_condition(TEST_IO_LIMIT_C)

#endif
#endif