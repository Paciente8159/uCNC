/*
	Name: esp32_signal.c
	Description: Signal generation functions. Calculates several signals in a time based way.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 15-10-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3)

signal_timer_t signal_timer;

MCU_CALLBACK void mcu_gen_step(void)
{
	static volatile bool step_reset = true;
	static volatile int32_t elapsed_us;

	// generate steps
	if (signal_timer.step_alarm_en)
	{
		// stream mode tick
		int32_t t = elapsed_us;
		bool reset = step_reset;
		t -= signal_timer.us_step;
		if (t < 0)
		{
			if (!reset)
			{
				mcu_step_cb();
			}
			else
			{
				mcu_step_reset_cb();
			}
			step_reset = !reset;
			elapsed_us = MAX(0, signal_timer.itp_reload + t);
		}
		else
		{
			elapsed_us = t;
		}
	}
}

// software pwm counters
uint8_t g_io_soft_pwm[16];

MCU_CALLBACK void mcu_gen_pwm(void)
{
#if defined(MCU_HAS_SOFT_PWM_TIMER) || defined(IC74HC595_HAS_PWMS)
	static int32_t elapsed_us;
	int32_t t = elapsed_us;
	t -= signal_timer.us_step;
	uint8_t pwm_counter = 0;
	if (t < 0)
	{
		elapsed_us = signal_timer.pwm_reload + t;
	}
	else
	{
		uint32_t pwmcycle = (((signal_timer.pwm_reload - t)) << 8);
		pwmcycle /= signal_timer.pwm_reload;
		pwm_counter = (uint8_t)CLAMP(0, pwmcycle, 255);
		elapsed_us = t;
	}

#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM0)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM0)))
	if (pwm_counter > g_io_soft_pwm[0] || !g_io_soft_pwm[0])
	{
		io_clear_output(PWM0);
	}
	else
	{
		io_set_output(PWM0);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM1)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM1)))
	if (pwm_counter > g_io_soft_pwm[1] || !g_io_soft_pwm[1])
	{
		io_clear_output(PWM1);
	}
	else
	{
		io_set_output(PWM1);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM2)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM2)))

	if (pwm_counter > g_io_soft_pwm[2] || !g_io_soft_pwm[2])
	{
		io_clear_output(PWM2);
	}
	else
	{
		io_set_output(PWM2);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM3)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM3)))
	if (pwm_counter > g_io_soft_pwm[3] || !g_io_soft_pwm[3])
	{
		io_clear_output(PWM3);
	}
	else
	{
		io_set_output(PWM3);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM4)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM4)))
	if (pwm_counter > g_io_soft_pwm[4] || !g_io_soft_pwm[4])
	{
		io_clear_output(PWM4);
	}
	else
	{
		io_set_output(PWM4);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM5)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM5)))
	if (pwm_counter > g_io_soft_pwm[5] || !g_io_soft_pwm[5])
	{
		io_clear_output(PWM5);
	}
	else
	{
		io_set_output(PWM5);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM6)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM6)))
	if (pwm_counter > g_io_soft_pwm[6] || !g_io_soft_pwm[6])
	{
		io_clear_output(PWM6);
	}
	else
	{
		io_set_output(PWM6);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM7)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM7)))
	if (pwm_counter > g_io_soft_pwm[7] || !g_io_soft_pwm[7])
	{
		io_clear_output(PWM7);
	}
	else
	{
		io_set_output(PWM7);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM8)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM8)))
	if (pwm_counter > g_io_soft_pwm[8] || !g_io_soft_pwm[8])
	{
		io_clear_output(PWM8);
	}
	else
	{
		io_set_output(PWM8);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM9)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM9)))
	if (pwm_counter > g_io_soft_pwm[9] || !g_io_soft_pwm[9])
	{
		io_clear_output(PWM9);
	}
	else
	{
		io_set_output(PWM9);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM10)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM10)))
	if (pwm_counter > g_io_soft_pwm[10] || !g_io_soft_pwm[10])
	{
		io_clear_output(PWM10);
	}
	else
	{
		io_set_output(PWM10);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM11)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM11)))
	if (pwm_counter > g_io_soft_pwm[11] || !g_io_soft_pwm[11])
	{
		io_clear_output(PWM11);
	}
	else
	{
		io_set_output(PWM11);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM12)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM12)))
	if (pwm_counter > g_io_soft_pwm[12] || !g_io_soft_pwm[12])
	{
		io_clear_output(PWM12);
	}
	else
	{
		io_set_output(PWM12);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM13)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM13)))
	if (pwm_counter > g_io_soft_pwm[13] || !g_io_soft_pwm[13])
	{
		io_clear_output(PWM13);
	}
	else
	{
		io_set_output(PWM13);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM14)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM14)))
	if (pwm_counter > g_io_soft_pwm[14] || !g_io_soft_pwm[14])
	{
		io_clear_output(PWM14);
	}
	else
	{
		io_set_output(PWM14);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM15)) || (defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM15)))
	if (pwm_counter > g_io_soft_pwm[15] || !g_io_soft_pwm[15])
	{
		io_clear_output(PWM15);
	}
	else
	{
		io_set_output(PWM15);
	}
#endif

#ifdef IC74HC595_HAS_PWMS
	io_extended_pins_update();
#endif
#endif
}

#if SERVOS_MASK > 0
// also run servo pin signals
static uint32_t servo_tick_counter = 0;
static uint32_t servo_tick_alarm = 0;
uint8_t mcu_servos[6];
static FORCEINLINE void servo_reset(void)
{
#if ASSERT_PIN(SERVO0)
	io_clear_output(SERVO0);
#endif
#if ASSERT_PIN(SERVO1)
	io_clear_output(SERVO1);
#endif
#if ASSERT_PIN(SERVO2)
	io_clear_output(SERVO2);
#endif
#if ASSERT_PIN(SERVO3)
	io_clear_output(SERVO3);
#endif
#if ASSERT_PIN(SERVO4)
	io_clear_output(SERVO4);
#endif
#if ASSERT_PIN(SERVO5)
	io_clear_output(SERVO5);
#endif
}

#define start_servo_timeout(timeout)                      \
	{                                                       \
		servo_tick_alarm = servo_tick_counter + timeout + 64; \
	}

static FORCEINLINE void servo_update(void)
{
	static uint8_t servo_counter = 0;

	switch (servo_counter)
	{
#if ASSERT_PIN(SERVO0)
	case SERVO0_FRAME:
		io_set_output(SERVO0);
		start_servo_timeout(mcu_servos[0]);
		break;
#endif
#if ASSERT_PIN(SERVO1)
	case SERVO1_FRAME:
		io_set_output(SERVO1);
		start_servo_timeout(mcu_servos[1]);
		break;
#endif
#if ASSERT_PIN(SERVO2)
	case SERVO2_FRAME:
		io_set_output(SERVO2);
		start_servo_timeout(mcu_servos[2]);
		break;
#endif
#if ASSERT_PIN(SERVO3)
	case SERVO3_FRAME:
		io_set_output(SERVO3);
		start_servo_timeout(mcu_servos[3]);
		break;
#endif
#if ASSERT_PIN(SERVO4)
	case SERVO4_FRAME:
		io_set_output(SERVO4);
		start_servo_timeout(mcu_servos[4]);
		break;
#endif
#if ASSERT_PIN(SERVO5)
	case SERVO5_FRAME:
		io_set_output(SERVO5);
		start_servo_timeout(mcu_servos[5]);
		break;
#endif
	}

	servo_counter++;
	servo_counter = (servo_counter != 20) ? servo_counter : 0;
}
#endif

MCU_CALLBACK void mcu_gen_servo(void)
{
#if SERVOS_MASK > 0
	static int32_t elapsed_us;
	int32_t t = elapsed_us;
	t -= signal_timer.us_step;
	if (t < 0)
	{
		// update servo pins

		static uint32_t servo_tick_counter;
		// also run servo pin signals
		uint32_t counter = servo_tick_counter;

		// updated next servo output
		if (!(counter & 0x7F))
		{
			servo_update();
		}

		// reached set tick alarm and resets all servo outputs
		if (counter == servo_tick_alarm)
		{
			servo_reset();
		}

		// resets every 3ms
		servo_tick_counter = ++counter;

		elapsed_us = /*(1000 / 128) =~ 8*/ 8 + t;
	}
	else
	{
		elapsed_us = t;
	}
#endif
}

#endif