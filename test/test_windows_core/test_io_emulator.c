/*
	Name: test_io_emulator.c
	Description: Test IO emulator for µCNC Windows unit tests.

	Implements the test_io_condition() function and backing state used to
	emulate control inputs, limits, and probe signals during unit tests.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 18-08-2026

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>
*/

#include "src/cnc.h"

#ifdef PIO_UNIT_TESTING

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

typedef bool (*test_io_callback_t)(void);

typedef struct
{
	bool static_value;

	bool timed_enabled;
	bool timed_initial;
	bool timed_final;
	uint32_t timed_start_ms;
	uint32_t timed_delay_ms;

	test_io_callback_t callback;
} test_io_signal_t;

static test_io_signal_t g_test_io[TEST_IO_COUNT];

bool test_io_condition(test_io_id_t input)
{
	if (input >= TEST_IO_COUNT)
	{
		return false;
	}

	test_io_signal_t *sig = &g_test_io[input];

	if (sig->callback)
	{
		return sig->callback();
	}

	if (sig->timed_enabled)
	{
		bool elapsed =
			(uint32_t)(mcu_millis() - sig->timed_start_ms) >=
			sig->timed_delay_ms;

		return elapsed ? sig->timed_final : sig->timed_initial;
	}

	return sig->static_value;
}

void test_io_reset(void)
{
	memset(g_test_io, 0, sizeof(g_test_io));
}

void test_io_set(test_io_id_t input, bool value)
{
	if (input < TEST_IO_COUNT)
	{
		g_test_io[input].static_value = value;
		g_test_io[input].timed_enabled = false;
		g_test_io[input].callback = NULL;
	}
}

void test_io_set_after(test_io_id_t input,
					   uint32_t delay_ms,
					   bool initial_value,
					   bool final_value)
{
	if (input < TEST_IO_COUNT)
	{
		g_test_io[input].timed_enabled = true;
		g_test_io[input].timed_initial = initial_value;
		g_test_io[input].timed_final = final_value;
		g_test_io[input].timed_start_ms = mcu_millis();
		g_test_io[input].timed_delay_ms = delay_ms;
		g_test_io[input].callback = NULL;
	}
}

void test_io_set_callback(test_io_id_t input,
						  test_io_callback_t cb)
{
	if (input < TEST_IO_COUNT)
	{
		g_test_io[input].callback = cb;
		g_test_io[input].timed_enabled = false;
	}
}

#endif
