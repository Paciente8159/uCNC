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
#include "ic74hc595.h"
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

FORCEINLINE void ic74hc595_set_steps(uint8_t mask)
{
#if ASSERT_PIN_EXTENDER(STEP0_IO_OFFSET)
	if (mask & (1 << 0))
	{
		ic74hc595_io_pins[STEP0_IO_BYTEOFFSET] |= STEP0_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP0_IO_BYTEOFFSET] &= ~STEP0_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP1_IO_OFFSET)
	if (mask & (1 << 1))
	{
		ic74hc595_io_pins[STEP1_IO_BYTEOFFSET] |= STEP1_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP1_IO_BYTEOFFSET] &= ~STEP1_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP2_IO_OFFSET)
	if (mask & (1 << 2))
	{
		ic74hc595_io_pins[STEP2_IO_BYTEOFFSET] |= STEP2_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP2_IO_BYTEOFFSET] &= ~STEP2_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP3_IO_OFFSET)
	if (mask & (1 << 3))
	{
		ic74hc595_io_pins[STEP3_IO_BYTEOFFSET] |= STEP3_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP3_IO_BYTEOFFSET] &= ~STEP3_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP4_IO_OFFSET)
	if (mask & (1 << 4))
	{
		ic74hc595_io_pins[STEP4_IO_BYTEOFFSET] |= STEP4_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP4_IO_BYTEOFFSET] &= ~STEP4_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP5_IO_OFFSET)
	if (mask & (1 << 5))
	{
		ic74hc595_io_pins[STEP5_IO_BYTEOFFSET] |= STEP5_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP5_IO_BYTEOFFSET] &= ~STEP5_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP6_IO_OFFSET)
	if (mask & (1 << 6))
	{
		ic74hc595_io_pins[STEP6_IO_BYTEOFFSET] |= STEP6_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP6_IO_BYTEOFFSET] &= ~STEP6_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP7_IO_OFFSET)
	if (mask & (1 << 7))
	{
		ic74hc595_io_pins[STEP7_IO_BYTEOFFSET] |= STEP7_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP7_IO_BYTEOFFSET] &= ~STEP7_IO_BITMASK;
	}
#endif

	ic74hc595_shift_io_pins();
}

FORCEINLINE void ic74hc595_toggle_steps(uint8_t mask)
{
#if ASSERT_PIN_EXTENDER(STEP0_IO_OFFSET)
	if (mask & (1 << 0))
	{
		ic74hc595_io_pins[STEP0_IO_BYTEOFFSET] ^= STEP0_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP1_IO_OFFSET)
	if (mask & (1 << 1))
	{
		ic74hc595_io_pins[STEP1_IO_BYTEOFFSET] ^= STEP1_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP2_IO_OFFSET)
	if (mask & (1 << 2))
	{
		ic74hc595_io_pins[STEP2_IO_BYTEOFFSET] ^= STEP2_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP3_IO_OFFSET)
	if (mask & (1 << 3))
	{
		ic74hc595_io_pins[STEP3_IO_BYTEOFFSET] ^= STEP3_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP4_IO_OFFSET)
	if (mask & (1 << 4))
	{
		ic74hc595_io_pins[STEP4_IO_BYTEOFFSET] ^= STEP4_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP5_IO_OFFSET)
	if (mask & (1 << 5))
	{
		ic74hc595_io_pins[STEP5_IO_BYTEOFFSET] ^= STEP5_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP6_IO_OFFSET)
	if (mask & (1 << 6))
	{
		ic74hc595_io_pins[STEP6_IO_BYTEOFFSET] ^= STEP6_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP7_IO_OFFSET)
	if (mask & (1 << 7))
	{
		ic74hc595_io_pins[STEP7_IO_BYTEOFFSET] ^= STEP7_IO_BITMASK;
	}
#endif

	ic74hc595_shift_io_pins();
}

FORCEINLINE void ic74hc595_set_dirs(uint8_t mask)
{
#if ASSERT_PIN_EXTENDER(DIR0_IO_OFFSET)
	if (mask & (1 << 0))
	{
		ic74hc595_io_pins[DIR0_IO_BYTEOFFSET] |= DIR0_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[DIR0_IO_BYTEOFFSET] &= ~DIR0_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(DIR1_IO_OFFSET)
	if (mask & (1 << 1))
	{
		ic74hc595_io_pins[DIR1_IO_BYTEOFFSET] |= DIR1_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[DIR1_IO_BYTEOFFSET] &= ~DIR1_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(DIR2_IO_OFFSET)
	if (mask & (1 << 2))
	{
		ic74hc595_io_pins[DIR2_IO_BYTEOFFSET] |= DIR2_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[DIR2_IO_BYTEOFFSET] &= ~DIR2_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(DIR3_IO_OFFSET)
	if (mask & (1 << 3))
	{
		ic74hc595_io_pins[DIR3_IO_BYTEOFFSET] |= DIR3_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[DIR3_IO_BYTEOFFSET] &= ~DIR3_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(DIR4_IO_OFFSET)
	if (mask & (1 << 4))
	{
		ic74hc595_io_pins[DIR4_IO_BYTEOFFSET] |= DIR4_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[DIR4_IO_BYTEOFFSET] &= ~DIR4_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(DIR5_IO_OFFSET)
	if (mask & (1 << 5))
	{
		ic74hc595_io_pins[DIR5_IO_BYTEOFFSET] |= DIR5_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[DIR5_IO_BYTEOFFSET] &= ~DIR5_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(DIR6_IO_OFFSET)
	if (mask & (1 << 6))
	{
		ic74hc595_io_pins[DIR6_IO_BYTEOFFSET] |= DIR6_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[DIR6_IO_BYTEOFFSET] &= ~DIR6_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(DIR7_IO_OFFSET)
	if (mask & (1 << 7))
	{
		ic74hc595_io_pins[DIR7_IO_BYTEOFFSET] |= DIR7_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[DIR7_IO_BYTEOFFSET] &= ~DIR7_IO_BITMASK;
	}
#endif

	ic74hc595_shift_io_pins();
}

FORCEINLINE void ic74hc595_enable_steppers(uint8_t mask)
{
#if ASSERT_PIN_EXTENDER(STEP0_EN_IO_OFFSET)
	if (mask & (1 << 0))
	{
		ic74hc595_io_pins[STEP0_EN_IO_BYTEOFFSET] |= STEP0_EN_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP0_EN_IO_BYTEOFFSET] &= ~STEP0_EN_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP1_EN_IO_OFFSET)
	if (mask & (1 << 1))
	{
		ic74hc595_io_pins[STEP1_EN_IO_BYTEOFFSET] |= STEP1_EN_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP1_EN_IO_BYTEOFFSET] &= ~STEP1_EN_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP2_EN_IO_OFFSET)
	if (mask & (1 << 2))
	{
		ic74hc595_io_pins[STEP2_EN_IO_BYTEOFFSET] |= STEP2_EN_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP2_EN_IO_BYTEOFFSET] &= ~STEP2_EN_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP3_EN_IO_OFFSET)
	if (mask & (1 << 3))
	{
		ic74hc595_io_pins[STEP3_EN_IO_BYTEOFFSET] |= STEP3_EN_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP3_EN_IO_BYTEOFFSET] &= ~STEP3_EN_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP4_EN_IO_OFFSET)
	if (mask & (1 << 4))
	{
		ic74hc595_io_pins[STEP4_EN_IO_BYTEOFFSET] |= STEP4_EN_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP4_EN_IO_BYTEOFFSET] &= ~STEP4_EN_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP5_EN_IO_OFFSET)
	if (mask & (1 << 5))
	{
		ic74hc595_io_pins[STEP5_EN_IO_BYTEOFFSET] |= STEP5_EN_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP5_EN_IO_BYTEOFFSET] &= ~STEP5_EN_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP6_EN_IO_OFFSET)
	if (mask & (1 << 6))
	{
		ic74hc595_io_pins[STEP6_EN_IO_BYTEOFFSET] |= STEP6_EN_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP6_EN_IO_BYTEOFFSET] &= ~STEP6_EN_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(STEP7_EN_IO_OFFSET)
	if (mask & (1 << 7))
	{
		ic74hc595_io_pins[STEP7_EN_IO_BYTEOFFSET] |= STEP7_EN_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[STEP7_EN_IO_BYTEOFFSET] &= ~STEP7_EN_IO_BITMASK;
	}
#endif

	ic74hc595_shift_io_pins();
}

FORCEINLINE void ic74hc595_set_pwms(uint16_t mask)
{
#if ASSERT_PIN_EXTENDER(PWM0_IO_OFFSET)
	if (mask & (1 << 0))
	{
		ic74hc595_io_pins[PWM0_IO_BYTEOFFSET] |= PWM0_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM0_IO_BYTEOFFSET] &= ~PWM0_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM1_IO_OFFSET)
	if (mask & (1 << 1))
	{
		ic74hc595_io_pins[PWM1_IO_BYTEOFFSET] |= PWM1_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM1_IO_BYTEOFFSET] &= ~PWM1_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM2_IO_OFFSET)
	if (mask & (1 << 2))
	{
		ic74hc595_io_pins[PWM2_IO_BYTEOFFSET] |= PWM2_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM2_IO_BYTEOFFSET] &= ~PWM2_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM3_IO_OFFSET)
	if (mask & (1 << 3))
	{
		ic74hc595_io_pins[PWM3_IO_BYTEOFFSET] |= PWM3_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM3_IO_BYTEOFFSET] &= ~PWM3_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM4_IO_OFFSET)
	if (mask & (1 << 4))
	{
		ic74hc595_io_pins[PWM4_IO_BYTEOFFSET] |= PWM4_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM4_IO_BYTEOFFSET] &= ~PWM4_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM5_IO_OFFSET)
	if (mask & (1 << 5))
	{
		ic74hc595_io_pins[PWM5_IO_BYTEOFFSET] |= PWM5_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM5_IO_BYTEOFFSET] &= ~PWM5_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM6_IO_OFFSET)
	if (mask & (1 << 6))
	{
		ic74hc595_io_pins[PWM6_IO_BYTEOFFSET] |= PWM6_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM6_IO_BYTEOFFSET] &= ~PWM6_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM7_IO_OFFSET)
	if (mask & (1 << 7))
	{
		ic74hc595_io_pins[PWM7_IO_BYTEOFFSET] |= PWM7_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM7_IO_BYTEOFFSET] &= ~PWM7_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM8_IO_OFFSET)
	if (mask & (1 << 8))
	{
		ic74hc595_io_pins[PWM8_IO_BYTEOFFSET] |= PWM8_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM8_IO_BYTEOFFSET] &= ~PWM8_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM9_IO_OFFSET)
	if (mask & (1 << 9))
	{
		ic74hc595_io_pins[PWM9_IO_BYTEOFFSET] |= PWM9_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM9_IO_BYTEOFFSET] &= ~PWM9_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM10_IO_OFFSET)
	if (mask & (1 << 10))
	{
		ic74hc595_io_pins[PWM10_IO_BYTEOFFSET] |= PWM10_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM10_IO_BYTEOFFSET] &= ~PWM10_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM11_IO_OFFSET)
	if (mask & (1 << 11))
	{
		ic74hc595_io_pins[PWM11_IO_BYTEOFFSET] |= PWM11_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM11_IO_BYTEOFFSET] &= ~PWM11_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM12_IO_OFFSET)
	if (mask & (1 << 12))
	{
		ic74hc595_io_pins[PWM12_IO_BYTEOFFSET] |= PWM12_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM12_IO_BYTEOFFSET] &= ~PWM12_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM13_IO_OFFSET)
	if (mask & (1 << 13))
	{
		ic74hc595_io_pins[PWM13_IO_BYTEOFFSET] |= PWM13_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM13_IO_BYTEOFFSET] &= ~PWM13_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM14_IO_OFFSET)
	if (mask & (1 << 14))
	{
		ic74hc595_io_pins[PWM14_IO_BYTEOFFSET] |= PWM14_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM14_IO_BYTEOFFSET] &= ~PWM14_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(PWM15_IO_OFFSET)
	if (mask & (1 << 15))
	{
		ic74hc595_io_pins[PWM15_IO_BYTEOFFSET] |= PWM15_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[PWM15_IO_BYTEOFFSET] &= ~PWM15_IO_BITMASK;
	}
#endif

	ic74hc595_shift_io_pins();
}

FORCEINLINE void ic74hc595_set_servos(uint8_t mask)
{
#if ASSERT_PIN_EXTENDER(SERVO0_IO_OFFSET)
	if (mask & (1 << 0))
	{
		ic74hc595_io_pins[SERVO0_IO_BYTEOFFSET] |= SERVO0_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[SERVO0_IO_BYTEOFFSET] &= ~SERVO0_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(SERVO1_IO_OFFSET)
	if (mask & (1 << 1))
	{
		ic74hc595_io_pins[SERVO1_IO_BYTEOFFSET] |= SERVO1_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[SERVO1_IO_BYTEOFFSET] &= ~SERVO1_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(SERVO2_IO_OFFSET)
	if (mask & (1 << 2))
	{
		ic74hc595_io_pins[SERVO2_IO_BYTEOFFSET] |= SERVO2_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[SERVO2_IO_BYTEOFFSET] &= ~SERVO2_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(SERVO3_IO_OFFSET)
	if (mask & (1 << 3))
	{
		ic74hc595_io_pins[SERVO3_IO_BYTEOFFSET] |= SERVO3_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[SERVO3_IO_BYTEOFFSET] &= ~SERVO3_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(SERVO4_IO_OFFSET)
	if (mask & (1 << 4))
	{
		ic74hc595_io_pins[SERVO4_IO_BYTEOFFSET] |= SERVO4_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[SERVO4_IO_BYTEOFFSET] &= ~SERVO4_IO_BITMASK;
	}
#endif
#if ASSERT_PIN_EXTENDER(SERVO5_IO_OFFSET)
	if (mask & (1 << 5))
	{
		ic74hc595_io_pins[SERVO5_IO_BYTEOFFSET] |= SERVO5_IO_BITMASK;
	}
	else
	{
		ic74hc595_io_pins[SERVO5_IO_BYTEOFFSET] &= ~SERVO5_IO_BITMASK;
	}
#endif

	ic74hc595_shift_io_pins();
}

FORCEINLINE void ic74hc595_set_output(uint8_t pin, bool state)
{
	switch (pin - DOUT_PINS_OFFSET)
	{
#if ASSERT_PIN_EXTENDER(DOUT0_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT1_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT2_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT3_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT4_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT5_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT6_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT7_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT8_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT9_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT10_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT11_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT12_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT13_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT14_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT15_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT16_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT17_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT18_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT19_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT20_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT21_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT22_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT23_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT24_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT25_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT26_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT27_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT28_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT29_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT30_IO_OFFSET)
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
#if ASSERT_PIN_EXTENDER(DOUT31_IO_OFFSET)
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
