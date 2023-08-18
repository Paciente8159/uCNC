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
uint8_t ic74hc595_io_pins[IC74HC595_COUNT];
static volatile uint8_t ic74hc595_update_lock;
void __attribute__((weak)) ic74hc595_shift_io_pins(void)
{
	uint8_t pins[IC74HC595_COUNT];
	if (!ic74hc595_update_lock++)
	{
		do
		{
			memcpy(pins, ic74hc595_io_pins, IC74HC595_COUNT);
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

FORCEINLINE void ic74hc595_set_output(uint8_t pin, bool state)
{
	switch (pin - DOUT_PINS_OFFSET)
	{
#if ASSERT_IO_OFFSET(DOUT0_IO_OFFSET)
	case 0:
		if (state)
		{
			ic74hc595_io_pins[DOUT0_IO_BYTEOFFSET] |= DOUT0_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT0_IO_BYTEOFFSET] &= ~DOUT0_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT1_IO_OFFSET)
	case 1:
		if (state)
		{
			ic74hc595_io_pins[DOUT1_IO_BYTEOFFSET] |= DOUT1_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT1_IO_BYTEOFFSET] &= ~DOUT1_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT2_IO_OFFSET)
	case 2:
		if (state)
		{
			ic74hc595_io_pins[DOUT2_IO_BYTEOFFSET] |= DOUT2_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT2_IO_BYTEOFFSET] &= ~DOUT2_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT3_IO_OFFSET)
	case 3:
		if (state)
		{
			ic74hc595_io_pins[DOUT3_IO_BYTEOFFSET] |= DOUT3_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT3_IO_BYTEOFFSET] &= ~DOUT3_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT4_IO_OFFSET)
	case 4:
		if (state)
		{
			ic74hc595_io_pins[DOUT4_IO_BYTEOFFSET] |= DOUT4_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT4_IO_BYTEOFFSET] &= ~DOUT4_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT5_IO_OFFSET)
	case 5:
		if (state)
		{
			ic74hc595_io_pins[DOUT5_IO_BYTEOFFSET] |= DOUT5_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT5_IO_BYTEOFFSET] &= ~DOUT5_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT6_IO_OFFSET)
	case 6:
		if (state)
		{
			ic74hc595_io_pins[DOUT6_IO_BYTEOFFSET] |= DOUT6_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT6_IO_BYTEOFFSET] &= ~DOUT6_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT7_IO_OFFSET)
	case 7:
		if (state)
		{
			ic74hc595_io_pins[DOUT7_IO_BYTEOFFSET] |= DOUT7_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT7_IO_BYTEOFFSET] &= ~DOUT7_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT8_IO_OFFSET)
	case 8:
		if (state)
		{
			ic74hc595_io_pins[DOUT8_IO_BYTEOFFSET] |= DOUT8_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT8_IO_BYTEOFFSET] &= ~DOUT8_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT9_IO_OFFSET)
	case 9:
		if (state)
		{
			ic74hc595_io_pins[DOUT9_IO_BYTEOFFSET] |= DOUT9_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT9_IO_BYTEOFFSET] &= ~DOUT9_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT10_IO_OFFSET)
	case 10:
		if (state)
		{
			ic74hc595_io_pins[DOUT10_IO_BYTEOFFSET] |= DOUT10_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT10_IO_BYTEOFFSET] &= ~DOUT10_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT11_IO_OFFSET)
	case 11:
		if (state)
		{
			ic74hc595_io_pins[DOUT11_IO_BYTEOFFSET] |= DOUT11_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT11_IO_BYTEOFFSET] &= ~DOUT11_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT12_IO_OFFSET)
	case 12:
		if (state)
		{
			ic74hc595_io_pins[DOUT12_IO_BYTEOFFSET] |= DOUT12_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT12_IO_BYTEOFFSET] &= ~DOUT12_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT13_IO_OFFSET)
	case 13:
		if (state)
		{
			ic74hc595_io_pins[DOUT13_IO_BYTEOFFSET] |= DOUT13_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT13_IO_BYTEOFFSET] &= ~DOUT13_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT14_IO_OFFSET)
	case 14:
		if (state)
		{
			ic74hc595_io_pins[DOUT14_IO_BYTEOFFSET] |= DOUT14_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT14_IO_BYTEOFFSET] &= ~DOUT14_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT15_IO_OFFSET)
	case 15:
		if (state)
		{
			ic74hc595_io_pins[DOUT15_IO_BYTEOFFSET] |= DOUT15_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT15_IO_BYTEOFFSET] &= ~DOUT15_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT16_IO_OFFSET)
	case 16:
		if (state)
		{
			ic74hc595_io_pins[DOUT16_IO_BYTEOFFSET] |= DOUT16_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT16_IO_BYTEOFFSET] &= ~DOUT16_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT17_IO_OFFSET)
	case 17:
		if (state)
		{
			ic74hc595_io_pins[DOUT17_IO_BYTEOFFSET] |= DOUT17_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT17_IO_BYTEOFFSET] &= ~DOUT17_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT18_IO_OFFSET)
	case 18:
		if (state)
		{
			ic74hc595_io_pins[DOUT18_IO_BYTEOFFSET] |= DOUT18_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT18_IO_BYTEOFFSET] &= ~DOUT18_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT19_IO_OFFSET)
	case 19:
		if (state)
		{
			ic74hc595_io_pins[DOUT19_IO_BYTEOFFSET] |= DOUT19_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT19_IO_BYTEOFFSET] &= ~DOUT19_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT20_IO_OFFSET)
	case 20:
		if (state)
		{
			ic74hc595_io_pins[DOUT20_IO_BYTEOFFSET] |= DOUT20_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT20_IO_BYTEOFFSET] &= ~DOUT20_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT21_IO_OFFSET)
	case 21:
		if (state)
		{
			ic74hc595_io_pins[DOUT21_IO_BYTEOFFSET] |= DOUT21_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT21_IO_BYTEOFFSET] &= ~DOUT21_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT22_IO_OFFSET)
	case 22:
		if (state)
		{
			ic74hc595_io_pins[DOUT22_IO_BYTEOFFSET] |= DOUT22_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT22_IO_BYTEOFFSET] &= ~DOUT22_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT23_IO_OFFSET)
	case 23:
		if (state)
		{
			ic74hc595_io_pins[DOUT23_IO_BYTEOFFSET] |= DOUT23_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT23_IO_BYTEOFFSET] &= ~DOUT23_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT24_IO_OFFSET)
	case 24:
		if (state)
		{
			ic74hc595_io_pins[DOUT24_IO_BYTEOFFSET] |= DOUT24_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT24_IO_BYTEOFFSET] &= ~DOUT24_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT25_IO_OFFSET)
	case 25:
		if (state)
		{
			ic74hc595_io_pins[DOUT25_IO_BYTEOFFSET] |= DOUT25_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT25_IO_BYTEOFFSET] &= ~DOUT25_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT26_IO_OFFSET)
	case 26:
		if (state)
		{
			ic74hc595_io_pins[DOUT26_IO_BYTEOFFSET] |= DOUT26_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT26_IO_BYTEOFFSET] &= ~DOUT26_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT27_IO_OFFSET)
	case 27:
		if (state)
		{
			ic74hc595_io_pins[DOUT27_IO_BYTEOFFSET] |= DOUT27_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT27_IO_BYTEOFFSET] &= ~DOUT27_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT28_IO_OFFSET)
	case 28:
		if (state)
		{
			ic74hc595_io_pins[DOUT28_IO_BYTEOFFSET] |= DOUT28_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT28_IO_BYTEOFFSET] &= ~DOUT28_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT29_IO_OFFSET)
	case 29:
		if (state)
		{
			ic74hc595_io_pins[DOUT29_IO_BYTEOFFSET] |= DOUT29_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT29_IO_BYTEOFFSET] &= ~DOUT29_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT30_IO_OFFSET)
	case 30:
		if (state)
		{
			ic74hc595_io_pins[DOUT30_IO_BYTEOFFSET] |= DOUT30_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT30_IO_BYTEOFFSET] &= ~DOUT30_IO_BITMASK;
		}
		break;
#endif
#if ASSERT_IO_OFFSET(DOUT31_IO_OFFSET)
	case 31:
		if (state)
		{
			ic74hc595_io_pins[DOUT31_IO_BYTEOFFSET] |= DOUT31_IO_BITMASK;
		}
		else
		{
			ic74hc595_io_pins[DOUT31_IO_BYTEOFFSET] &= ~DOUT31_IO_BITMASK;
		}
		break;
#endif
	}

	ic74hc595_shift_io_pins();
}

#endif
