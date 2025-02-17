/*
	Name: ic74hc595.c
	Description: This module adds the ability to control the IC74HC595 shift register controller (used for example in the MKS-DLC32 board) to µCNC.
				 Up to 56 output pins can be assigned.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/09/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef IC74HC595_COUNT
#define IC74HC595_COUNT 0
#endif

#ifndef IC74HC165_COUNT
#define IC74HC165_COUNT 0
#endif

#ifndef SHIFT_REGISTER_SDO
#define SHIFT_REGISTER_SDO DOUT8
#endif

#ifndef SHIFT_REGISTER_SDI
#define SHIFT_REGISTER_SDI DIN8
#endif

#ifndef SHIFT_REGISTER_CLK
#define SHIFT_REGISTER_CLK DOUT9
#endif

#ifndef IC74HC595_LATCH
#define IC74HC595_LATCH DOUT10
#endif

#ifndef IC74HC165_LOAD
#define IC74HC165_LOAD DOUT11
#endif

#ifndef SHIFT_REGISTER_DELAY_CYCLES
#define SHIFT_REGISTER_DELAY_CYCLES 0
#endif

#if (IC74HC595_COUNT >= IC74HC165_COUNT)
#define SHIFT_REGISTER_BYTES IC74HC595_COUNT
#else
#define SHIFT_REGISTER_BYTES IC74HC165_COUNT
#endif

#define shift_register_delay() mcu_delay_cycles(SHIFT_REGISTER_DELAY_CYCLES)
#if (IC74HC595_COUNT != 0) || (IC74HC165_COUNT != 0)
#if (IC74HC595_COUNT != 0)
volatile uint8_t ic74hc595_io_pins[IC74HC595_COUNT];
#endif
#if (IC74HC165_COUNT != 0)
volatile uint8_t ic74hc165_io_pins[IC74HC165_COUNT];
#endif
#ifndef SHIFT_REGISTER_CUSTOM_CALLBACK

DECL_MUTEX(shifter_running);

MCU_CALLBACK void /*__attribute__((weak))*/ shift_register_io_pins(void)
{
	MUTEX_INIT(shifter_running);

	MUTEX_TAKE(shifter_running)
	{
		uint8_t pins[SHIFT_REGISTER_BYTES];

#if (IC74HC165_COUNT > 0)
		mcu_clear_output(IC74HC165_LOAD);
		memset(pins, 0, IC74HC165_COUNT);
#endif
		__ATOMIC__
		{
#if (IC74HC595_COUNT > 0)
			memcpy(pins, (const void *)ic74hc595_io_pins, IC74HC595_COUNT);
#endif
		}
#if (IC74HC165_COUNT > 0)
		mcu_delay_us(5);
		mcu_set_output(IC74HC165_LOAD);
#endif
		mcu_clear_output(IC74HC595_LATCH);

		/**
		 * shift bytes
		 */
		for (uint8_t i = SHIFT_REGISTER_BYTES; i != 0;)
		{
			i--;
#if (defined(SHIFT_REGISTER_USE_HW_SPI) && defined(MCU_HAS_SPI))
			mcu_spi_xmit(pins[i]);
#else
			uint8_t pinbyte = pins[i];
			for (uint8_t j = 0x80; j != 0; j >>= 1)
			{
#if (SHIFT_REGISTER_DELAY_CYCLES)
				shift_register_delay();
#endif
				mcu_clear_output(SHIFT_REGISTER_CLK);
#if (IC74HC595_COUNT > 0)
				// write
				if (pinbyte & j)
				{
					mcu_set_output(SHIFT_REGISTER_SDO);
				}
				else
				{
					mcu_clear_output(SHIFT_REGISTER_SDO);
				}
#endif
// read
#if (IC74HC165_COUNT > 0)
				if (mcu_get_input(SHIFT_REGISTER_SDI))
				{
					pinbyte |= j;
				}
				else
				{
					pinbyte &= ~j;
				}
#endif
				mcu_set_output(SHIFT_REGISTER_CLK);
			}

#if (IC74HC165_COUNT > 0)
			pins[i] = pinbyte;
#endif

#endif
		}
#if (SHIFT_REGISTER_DELAY_CYCLES)
		shift_register_delay();
#endif
#if (IC74HC165_COUNT > 0)
		memcpy((void *)ic74hc165_io_pins, (const void *)pins, IC74HC165_COUNT);
#endif
		mcu_set_output(IC74HC595_LATCH);
		mcu_set_output(SHIFT_REGISTER_CLK);
	}
}
#else
MCU_CALLBACK void __attribute__((weak)) shift_register_io_pins(void)
{
}
#endif
#endif
