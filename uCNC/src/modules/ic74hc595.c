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

#ifndef IC74HC595_DATA
#define IC74HC595_DATA DOUT8
#endif

#ifndef IC74HC595_CLK
#define IC74HC595_CLK DOUT9
#endif

#ifndef IC74HC595_LATCH
#define IC74HC595_LATCH DOUT10
#endif

#ifndef IC74HC595_DELAY_CYCLES
#define IC74HC595_DELAY_CYCLES 0
#endif

#define ic74hc595_delay() mcu_delay_cycles(IC74HC595_DELAY_CYCLES)
#if (IC74HC595_COUNT != 0)
volatile uint8_t ic74hc595_io_pins[IC74HC595_COUNT];
#ifndef IC74HC595_CUSTOM_SHIFT_IO
MCU_CALLBACK void __attribute__((weak)) ic74hc595_shift_io_pins(void)
{

	static volatile uint8_t ic74hc595_update_lock = 0;
	uint8_t pins[IC74HC595_COUNT];
	if (!ic74hc595_update_lock++)
	{
		do
		{
			memcpy(pins, (const void *)ic74hc595_io_pins, IC74HC595_COUNT);
			mcu_clear_output(IC74HC595_LATCH);
			for (uint8_t i = IC74HC595_COUNT; i != 0;)
			{
				i--;
#if (defined(IC74HC595_USE_HW_SPI) && defined(MCU_HAS_SPI))
				mcu_spi_xmit(pins[i]);
#else
				uint8_t pinbyte = pins[i];
				for (uint8_t j = 0x80; j != 0; j >>= 1)
				{
#if (IC74HC595_DELAY_CYCLES)
					ic74hc595_delay();
#endif
					mcu_clear_output(IC74HC595_CLK);
					if (pinbyte & j)
					{
						mcu_set_output(IC74HC595_DATA);
					}
					else
					{
						mcu_clear_output(IC74HC595_DATA);
					}
					mcu_set_output(IC74HC595_CLK);
				}
#endif
			}
#if (IC74HC595_DELAY_CYCLES)
			ic74hc595_delay();
#endif
			mcu_set_output(IC74HC595_LATCH);
		} while (--ic74hc595_update_lock);
	}
}
#else
MCU_CALLBACK void __attribute__((weak)) ic74hc595_shift_io_pins(void)
{
}
#endif
#endif
