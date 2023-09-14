/*
	Name: io_control.c
	Description: The input control unit for µCNC.
		This is responsible to check all limit switches (both hardware and software), control switches,
		and probe.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/12/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"

#ifdef ENABLE_MULTI_STEP_HOMING
static uint8_t io_lock_limits_mask;
#endif
static uint8_t io_invert_limits_mask;

#if ASSERT_PIN(PROBE)
static volatile bool io_last_probe;
static bool io_probe_enabled;
#endif

#ifdef ENABLE_IO_ALARM_DEBUG
uint8_t io_alarm_limits;
uint8_t io_alarm_controls;
#endif

#ifdef ENABLE_IO_MODULES
// event_input_change_handler
WEAK_EVENT_HANDLER(input_change)
{
	// for now only encoder module uses this hook and overrides it
	// it actually overrides the mcu callback to be faster
	DEFAULT_EVENT_HANDLER(input_change);
}

// event_probe_enable_handler
WEAK_EVENT_HANDLER(probe_enable)
{
	DEFAULT_EVENT_HANDLER(probe_enable);
}

// event_probe_disable_handler
WEAK_EVENT_HANDLER(probe_disable)
{
	DEFAULT_EVENT_HANDLER(probe_disable);
}

// // event_set_steps_handler
// WEAK_EVENT_HANDLER(set_steps)
// {
// 	DEFAULT_EVENT_HANDLER(set_steps);
// }

// // event_toggle_steps_handler
// WEAK_EVENT_HANDLER(toggle_steps)
// {
// 	DEFAULT_EVENT_HANDLER(toggle_steps);
// }

// // event_set_dirs_handler
// WEAK_EVENT_HANDLER(set_dirs)
// {
// 	DEFAULT_EVENT_HANDLER(set_dirs);
// }

// // event_enable_steppers_handler
// WEAK_EVENT_HANDLER(enable_steppers)
// {
// 	DEFAULT_EVENT_HANDLER(enable_steppers);
// }

// // event_set_output_handler
// WEAK_EVENT_HANDLER(set_output)
// {
// 	DEFAULT_EVENT_HANDLER(set_output);
// }

#endif

MCU_IO_CALLBACK void mcu_limits_changed_cb(void)
{
#ifdef DISABLE_ALL_LIMITS
	return;
#else
	if (g_settings.hard_limits_enabled || cnc_get_exec_state(EXEC_HOMING))
	{
		static uint8_t prev_limits = 0;
		uint8_t limits = io_get_limits();
		uint8_t limits_diff = prev_limits;
		limits_diff ^= limits;
		if (!limits_diff)
		{
			return;
		}

		prev_limits = limits;

		if (limits)
		{
#ifdef ENABLE_MULTI_STEP_HOMING
			uint8_t limits_ref = io_lock_limits_mask;
			if (cnc_get_exec_state((EXEC_RUN | EXEC_HOMING)) == (EXEC_RUN | EXEC_HOMING) && (limits_ref & limits))
			{
				// changed limit is from the current mask
				if ((limits_diff & limits_ref))
				{
					// lock steps on the current limits
					itp_lock_stepper(limits);

					if (limits != limits_ref) // current limits are all the ones marked as locked
					{
						return; // exits and doesn't trip the alarm
					}
				}
			}

			itp_lock_stepper(0); // unlocks axis
#endif
			itp_stop();
			cnc_set_exec_state(EXEC_LIMITS);
#ifdef ENABLE_IO_ALARM_DEBUG
			io_alarm_limits = limits;
#endif
		}
	}

#endif
}

MCU_IO_CALLBACK void mcu_controls_changed_cb(void)
{
#ifdef DISABLE_ALL_CONTROLS
	return;
#else
	static uint8_t prev_controls = 0;
	uint8_t controls = io_get_controls();

	if (!(prev_controls ^ controls))
	{
		return;
	}

	prev_controls = controls;

#if ASSERT_PIN(ESTOP)
	if (CHECKFLAG(controls, ESTOP_MASK))
	{
#ifdef ENABLE_IO_ALARM_DEBUG
		io_alarm_controls = controls;
#endif
		cnc_alarm(EXEC_ALARM_EMERGENCY_STOP);
		return; // forces exit
	}
#endif
#if ASSERT_PIN(SAFETY_DOOR)
	if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
	{
		// safety door activates hold simultaneously to start the controlled stop
		cnc_set_exec_state(EXEC_DOOR | EXEC_HOLD);
#ifdef ENABLE_IO_ALARM_DEBUG
		io_alarm_controls = controls;
#endif
	}
#endif
#if ASSERT_PIN(FHOLD)
	if (CHECKFLAG(controls, FHOLD_MASK))
	{
		cnc_set_exec_state(EXEC_HOLD);
	}
#endif
#if ASSERT_PIN(CS_RES)
	if (CHECKFLAG(controls, CS_RES_MASK))
	{
		cnc_call_rt_command(CMD_CODE_CYCLE_START);
	}
#endif
#endif
}

MCU_IO_CALLBACK void mcu_probe_changed_cb(void)
{
#if !ASSERT_PIN(PROBE)
	return;
#else

	if (!io_probe_enabled)
	{
		return;
	}

	bool probe = io_get_probe();

	if (io_last_probe == probe)
	{
		return;
	}

	io_last_probe = probe;

	// stops the machine
	itp_stop();
	// stores rt position
	parser_sync_probe();
#endif
}

MCU_IO_CALLBACK void mcu_inputs_changed_cb(void)
{
	static uint8_t prev_inputs = 0;
	uint8_t inputs = 0;
	uint8_t diff;

#if (ASSERT_PIN(DIN0) && defined(DIN0_ISR))
	if (io_get_input(DIN0))
	{
		inputs |= DIN0_MASK;
	}
#endif
#if (ASSERT_PIN(DIN1) && defined(DIN1_ISR))
	if (io_get_input(DIN1))
	{
		inputs |= DIN1_MASK;
	}
#endif
#if (ASSERT_PIN(DIN2) && defined(DIN2_ISR))
	if (io_get_input(DIN2))
	{
		inputs |= DIN2_MASK;
	}
#endif
#if (ASSERT_PIN(DIN3) && defined(DIN3_ISR))
	if (io_get_input(DIN3))
	{
		inputs |= DIN3_MASK;
	}
#endif
#if (ASSERT_PIN(DIN4) && defined(DIN4_ISR))
	if (io_get_input(DIN4))
	{
		inputs |= DIN4_MASK;
	}
#endif
#if (ASSERT_PIN(DIN5) && defined(DIN5_ISR))
	if (io_get_input(DIN5))
	{
		inputs |= DIN5_MASK;
	}
#endif
#if (ASSERT_PIN(DIN6) && defined(DIN6_ISR))
	if (io_get_input(DIN6))
	{
		inputs |= DIN6_MASK;
	}
#endif
#if (ASSERT_PIN(DIN7) && defined(DIN7_ISR))
	if (io_get_input(DIN7))
	{
		inputs |= DIN7_MASK;
	}
#endif

#if (ENCODERS > 0)
	inputs ^= g_settings.encoders_pulse_invert_mask;
#endif
	diff = inputs ^ prev_inputs;

	if (diff)
	{
#if (ENCODERS > 0)
		encoders_update(inputs, diff);
#endif
#ifdef ENABLE_IO_MODULES
		uint8_t args[] = {inputs, diff};
		EVENT_INVOKE(input_change, args);
#endif

		prev_inputs = inputs;
	}
}

#ifdef ENABLE_MULTI_STEP_HOMING
void io_lock_limits(uint8_t limitmask)
{
	io_lock_limits_mask = limitmask;
}
#endif

void io_invert_limits(uint8_t limitmask)
{
	io_invert_limits_mask = limitmask;
	mcu_limits_changed_cb();
}

uint8_t io_get_limits(void)
{
#ifdef DISABLE_ALL_LIMITS
	return 0;
#endif
	uint8_t value = 0;

#if ASSERT_PIN(LIMIT_X)
	value |= ((io_get_input(LIMIT_X)) ? LIMIT_X_IO_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_Y)
	value |= ((io_get_input(LIMIT_Y)) ? LIMIT_Y_IO_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_Z)
	value |= ((io_get_input(LIMIT_Z)) ? LIMIT_Z_IO_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_X2)
	value |= ((io_get_input(LIMIT_X2)) ? LIMIT_X2_IO_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_Y2)
	value |= ((io_get_input(LIMIT_Y2)) ? LIMIT_Y2_IO_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_Z2)
	value |= ((io_get_input(LIMIT_Z2)) ? LIMIT_Z2_IO_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_A)
	value |= ((io_get_input(LIMIT_A)) ? LIMIT_A_IO_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_B)
	value |= ((io_get_input(LIMIT_B)) ? LIMIT_B_IO_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_C)
	value |= ((io_get_input(LIMIT_C)) ? LIMIT_C_IO_MASK : 0);
#endif

	uint8_t inv = g_settings.limits_invert_mask;
	uint8_t result = (value ^ (inv & LIMITS_INV_MASK));

	if (cnc_get_exec_state(EXEC_HOMING))
	{
		result ^= io_invert_limits_mask;
	}

	return result;
}

uint8_t io_get_controls(void)
{
#ifdef DISABLE_ALL_CONTROLS
	return 0;
#endif
	uint8_t value = 0;
#if ASSERT_PIN(ESTOP)
#ifndef INVERT_EMERGENCY_STOP
	value |= ((io_get_input(ESTOP)) ? ESTOP_MASK : 0);
#else
	value |= ((!io_get_input(ESTOP)) ? ESTOP_MASK : 0);
#endif
#endif
#if ASSERT_PIN(SAFETY_DOOR)
	value |= ((io_get_input(SAFETY_DOOR)) ? SAFETY_DOOR_MASK : 0);
#endif
#if ASSERT_PIN(FHOLD)
	value |= ((io_get_input(FHOLD)) ? FHOLD_MASK : 0);
#endif
#if ASSERT_PIN(CS_RES)
	value |= ((io_get_input(CS_RES)) ? CS_RES_MASK : 0);
#endif

	return (value ^ (g_settings.control_invert_mask & CONTROLS_INV_MASK));
}

void io_enable_probe(void)
{
#if ASSERT_PIN(PROBE)
	io_last_probe = io_get_probe();
#ifdef ENABLE_IO_MODULES
	EVENT_INVOKE(probe_enable, NULL);
#endif
#ifndef FORCE_SOFT_POLLING
	mcu_enable_probe_isr();
#endif
	io_probe_enabled = true;
#endif
}

void io_disable_probe(void)
{
#if ASSERT_PIN(PROBE)
	io_probe_enabled = false;
#ifndef FORCE_SOFT_POLLING
	mcu_disable_probe_isr();
#endif
#ifdef ENABLE_IO_MODULES
	EVENT_INVOKE(probe_disable, NULL);
#endif
#endif
}

bool io_get_probe(void)
{
#if !ASSERT_PIN(PROBE)
	return false;
#else
#if ASSERT_PIN(PROBE)
	bool probe = (io_get_input(PROBE) != 0);
	return (!g_settings.probe_invert_mask) ? probe : !probe;
#else
	return false;
#endif
#endif
}

// outputs
void io_set_steps(uint8_t mask)
{
	// #ifdef ENABLE_IO_MODULES
	// 	EVENT_INVOKE(set_steps, &mask);
	// #endif

#if ASSERT_PIN(STEP0)
	if (mask & STEP0_IO_MASK)
	{
		io_set_output(STEP0);
	}
	else
	{
		io_clear_output(STEP0);
	}

#endif
#if ASSERT_PIN(STEP1)
	if (mask & STEP1_IO_MASK)
	{
		io_set_output(STEP1);
	}
	else
	{
		io_clear_output(STEP1);
	}
#endif
#if ASSERT_PIN(STEP2)
	if (mask & STEP2_IO_MASK)
	{
		io_set_output(STEP2);
	}
	else
	{
		io_clear_output(STEP2);
	}
#endif
#if ASSERT_PIN(STEP3)
	if (mask & STEP3_IO_MASK)
	{
		io_set_output(STEP3);
	}
	else
	{
		io_clear_output(STEP3);
	}
#endif
#if ASSERT_PIN(STEP4)
	if (mask & STEP4_IO_MASK)
	{
		io_set_output(STEP4);
	}
	else
	{
		io_clear_output(STEP4);
	}
#endif
#if ASSERT_PIN(STEP5)
	if (mask & STEP5_IO_MASK)
	{
		io_set_output(STEP5);
	}
	else
	{
		io_clear_output(STEP5);
	}
#endif
#if ASSERT_PIN(STEP6)
	if (mask & STEP6_IO_MASK)
	{
		io_set_output(STEP6);
	}
	else
	{
		io_clear_output(STEP6);
	}
#endif
#if ASSERT_PIN(STEP7)
	if (mask & STEP7_IO_MASK)
	{
		io_set_output(STEP7);
	}
	else
	{
		io_clear_output(STEP7);
	}
#endif

#ifdef IC74HC595_HAS_STEPS
	ic74hc595_shift_io_pins();
#endif
}

void io_toggle_steps(uint8_t mask)
{
	// #ifdef ENABLE_IO_MODULES
	// 	EVENT_INVOKE(toggle_steps, &mask);
	// #endif
	if (!mask)
	{
		return;
	}

#if ASSERT_PIN(STEP0)
	if (mask & STEP0_IO_MASK)
	{
		io_toggle_output(STEP0);
	}
#endif
#if ASSERT_PIN(STEP1)
	if (mask & STEP1_IO_MASK)
	{
		io_toggle_output(STEP1);
	}
#endif
#if ASSERT_PIN(STEP2)
	if (mask & STEP2_IO_MASK)
	{
		io_toggle_output(STEP2);
	}
#endif
#if ASSERT_PIN(STEP3)
	if (mask & STEP3_IO_MASK)
	{
		io_toggle_output(STEP3);
	}
#endif
#if ASSERT_PIN(STEP4)
	if (mask & STEP4_IO_MASK)
	{
		io_toggle_output(STEP4);
	}
#endif
#if ASSERT_PIN(STEP5)
	if (mask & STEP5_IO_MASK)
	{
		io_toggle_output(STEP5);
	}
#endif
#if ASSERT_PIN(STEP6)
	if (mask & STEP6_IO_MASK)
	{
		io_toggle_output(STEP6);
	}
#endif
#if ASSERT_PIN(STEP7)
	if (mask & STEP7_IO_MASK)
	{
		io_toggle_output(STEP7);
	}
#endif

#ifdef IC74HC595_HAS_STEPS
	ic74hc595_shift_io_pins();
#endif
}

void io_set_dirs(uint8_t mask)
{
	mask ^= g_settings.dir_invert_mask;

	// #ifdef ENABLE_IO_MODULES
	// 	EVENT_INVOKE(set_dirs, &mask);
	// #endif

#if ASSERT_PIN(DIR0)
	if (mask & STEP0_IO_MASK)
	{
		io_set_output(DIR0);
	}
	else
	{
		io_clear_output(DIR0);
	}
#endif
#if ASSERT_PIN(DIR1)
	if (mask & STEP1_IO_MASK)
	{
		io_set_output(DIR1);
	}
	else
	{
		io_clear_output(DIR1);
	}
#endif
#if ASSERT_PIN(DIR2)
	if (mask & STEP2_IO_MASK)
	{
		io_set_output(DIR2);
	}
	else
	{
		io_clear_output(DIR2);
	}
#endif
#if ASSERT_PIN(DIR3)
	if (mask & STEP3_IO_MASK)
	{
		io_set_output(DIR3);
	}
	else
	{
		io_clear_output(DIR3);
	}
#endif
#if ASSERT_PIN(DIR4)
	if (mask & STEP4_IO_MASK)
	{
		io_set_output(DIR4);
	}
	else
	{
		io_clear_output(DIR4);
	}
#endif
#if ASSERT_PIN(DIR5)
	if (mask & STEP5_IO_MASK)
	{
		io_set_output(DIR5);
	}
	else
	{
		io_clear_output(DIR5);
	}
#endif
#if ASSERT_PIN(DIR6)
	if (mask & STEP6_IO_MASK)
	{
		io_set_output(DIR6);
	}
	else
	{
		io_clear_output(DIR6);
	}
#endif
#if ASSERT_PIN(DIR7)
	if (mask & STEP7_IO_MASK)
	{
		io_set_output(DIR7);
	}
	else
	{
		io_clear_output(DIR7);
	}
#endif

#ifdef IC74HC595_HAS_DIRS
	ic74hc595_shift_io_pins();
#endif
}

void io_enable_steppers(uint8_t mask)
{
	// #ifdef ENABLE_IO_MODULES
	// 	EVENT_INVOKE(enable_steppers, &mask);
	// #endif

#if ASSERT_PIN(STEP0_EN)
	if (mask & 0x01)
	{
		io_set_output(STEP0_EN);
	}
	else
	{
		io_clear_output(STEP0_EN);
	}
#endif
#if ASSERT_PIN(STEP1_EN)
	if (mask & 0x02)
	{
		io_set_output(STEP1_EN);
	}
	else
	{
		io_clear_output(STEP1_EN);
	}
#endif
#if ASSERT_PIN(STEP2_EN)
	if (mask & 0x04)
	{
		io_set_output(STEP2_EN);
	}
	else
	{
		io_clear_output(STEP2_EN);
	}
#endif
#if ASSERT_PIN(STEP3_EN)
	if (mask & 0x08)
	{
		io_set_output(STEP3_EN);
	}
	else
	{
		io_clear_output(STEP3_EN);
	}
#endif
#if ASSERT_PIN(STEP4_EN)
	if (mask & 0x10)
	{
		io_set_output(STEP4_EN);
	}
	else
	{
		io_clear_output(STEP4_EN);
	}
#endif
#if ASSERT_PIN(STEP5_EN)
	if (mask & 0x20)
	{
		io_set_output(STEP5_EN);
	}
	else
	{
		io_clear_output(STEP5_EN);
	}
#endif
#if ASSERT_PIN(STEP6_EN)
	if (mask & 0x40)
	{
		io_set_output(STEP6_EN);
	}
	else
	{
		io_clear_output(STEP6_EN);
	}
#endif
#if ASSERT_PIN(STEP7_EN)
	if (mask & 0x80)
	{
		io_set_output(STEP7_EN);
	}
	else
	{
		io_clear_output(STEP7_EN);
	}
#endif

#ifdef IC74HC595_HAS_STEPS_EN
	ic74hc595_shift_io_pins();
#endif
}

#if defined(MCU_HAS_SOFT_PWM_TIMER) || defined(IC74HC595_HAS_PWMS)
// software pwm counters
uint8_t g_io_soft_pwm[16];
// software pwm resolution reduction factor
// PWM resolution in bits will be equal to (8 - g_soft_pwm_res)
// this is determined by the mcu_softpwm_freq_config
uint8_t g_soft_pwm_res;

MCU_CALLBACK void io_soft_pwm_update(void)
{
	static uint8_t pwm_counter_last = 0;
	uint8_t pwm_counter = pwm_counter_last;
	uint8_t resolution = g_soft_pwm_res;
	// software PWM
	pwm_counter += (1 << resolution);
	pwm_counter_last = pwm_counter;
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM0))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM0)))
	if (pwm_counter > g_io_soft_pwm[0] || !g_io_soft_pwm[0])
	{
		io_clear_output(PWM0);
	}
	else
	{
		io_set_output(PWM0);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM1))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM1)))
	if (pwm_counter > g_io_soft_pwm[1] || !g_io_soft_pwm[1])
	{
		io_clear_output(PWM1);
	}
	else
	{
		io_set_output(PWM1);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM2))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM2)))

	if (pwm_counter > g_io_soft_pwm[2] || !g_io_soft_pwm[2])
	{
		io_clear_output(PWM2);
	}
	else
	{
		io_set_output(PWM2);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM3))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM3)))
	if (pwm_counter > g_io_soft_pwm[3] || !g_io_soft_pwm[3])
	{
		io_clear_output(PWM3);
	}
	else
	{
		io_set_output(PWM3);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM4))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM4)))
	if (pwm_counter > g_io_soft_pwm[4] || !g_io_soft_pwm[4])
	{
		io_clear_output(PWM4);
	}
	else
	{
		io_set_output(PWM4);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM5))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM5)))
	if (pwm_counter > g_io_soft_pwm[5] || !g_io_soft_pwm[5])
	{
		io_clear_output(PWM5);
	}
	else
	{
		io_set_output(PWM5);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM6))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM6)))
	if (pwm_counter > g_io_soft_pwm[6] || !g_io_soft_pwm[6])
	{
		io_clear_output(PWM6);
	}
	else
	{
		io_set_output(PWM6);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM7))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM7)))
	if (pwm_counter > g_io_soft_pwm[7] || !g_io_soft_pwm[7])
	{
		io_clear_output(PWM7);
	}
	else
	{
		io_set_output(PWM7);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM8))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM8)))
	if (pwm_counter > g_io_soft_pwm[8] || !g_io_soft_pwm[8])
	{
		io_clear_output(PWM8);
	}
	else
	{
		io_set_output(PWM8);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM9))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM9)))
	if (pwm_counter > g_io_soft_pwm[9] || !g_io_soft_pwm[9])
	{
		io_clear_output(PWM9);
	}
	else
	{
		io_set_output(PWM9);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM10))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM10)))
	if (pwm_counter > g_io_soft_pwm[10] || !g_io_soft_pwm[10])
	{
		io_clear_output(PWM10);
	}
	else
	{
		io_set_output(PWM10);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM11))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM11)))
	if (pwm_counter > g_io_soft_pwm[11] || !g_io_soft_pwm[11])
	{
		io_clear_output(PWM11);
	}
	else
	{
		io_set_output(PWM11);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM12))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM12)))
	if (pwm_counter > g_io_soft_pwm[12] || !g_io_soft_pwm[12])
	{
		io_clear_output(PWM12);
	}
	else
	{
		io_set_output(PWM12);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM13))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM13)))
	if (pwm_counter > g_io_soft_pwm[13] || !g_io_soft_pwm[13])
	{
		io_clear_output(PWM13);
	}
	else
	{
		io_set_output(PWM13);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM14))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM14)))
	if (pwm_counter > g_io_soft_pwm[14] || !g_io_soft_pwm[14])
	{
		io_clear_output(PWM14);
	}
	else
	{
		io_set_output(PWM14);
	}
#endif
#if ((defined(IC74HC595_HAS_PWMS) && ASSERT_PIN_EXTENDED(PWM15))||(defined(MCU_HAS_SOFT_PWM_TIMER) && ASSERT_PIN(PWM15)))
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
	ic74hc595_shift_io_pins();
#endif
}
#endif

void io_set_pinvalue(uint8_t pin, uint8_t value)
{
	// #ifdef ENABLE_IO_MODULES
	// 	set_output_args_t output_arg = {.pin = pin, .state = state};
	// 	EVENT_INVOKE(set_output, &output_arg);
	// #endif
	if (value)
	{
		switch (pin)
		{
#if ASSERT_PIN(PWM0)
		case PWM0:
			io_set_pwm(PWM0, value);
			break;
#endif
#if ASSERT_PIN(PWM1)
		case PWM1:
			io_set_pwm(PWM1, value);
			break;
#endif
#if ASSERT_PIN(PWM2)
		case PWM2:
			io_set_pwm(PWM2, value);
			break;
#endif
#if ASSERT_PIN(PWM3)
		case PWM3:
			io_set_pwm(PWM3, value);
			break;
#endif
#if ASSERT_PIN(PWM4)
		case PWM4:
			io_set_pwm(PWM4, value);
			break;
#endif
#if ASSERT_PIN(PWM5)
		case PWM5:
			io_set_pwm(PWM5, value);
			break;
#endif
#if ASSERT_PIN(PWM6)
		case PWM6:
			io_set_pwm(PWM6, value);
			break;
#endif
#if ASSERT_PIN(PWM7)
		case PWM7:
			io_set_pwm(PWM7, value);
			break;
#endif
#if ASSERT_PIN(PWM8)
		case PWM8:
			io_set_pwm(PWM8, value);
			break;
#endif
#if ASSERT_PIN(PWM9)
		case PWM9:
			io_set_pwm(PWM9, value);
			break;
#endif
#if ASSERT_PIN(PWM10)
		case PWM10:
			io_set_pwm(PWM10, value);
			break;
#endif
#if ASSERT_PIN(PWM11)
		case PWM11:
			io_set_pwm(PWM11, value);
			break;
#endif
#if ASSERT_PIN(PWM12)
		case PWM12:
			io_set_pwm(PWM12, value);
			break;
#endif
#if ASSERT_PIN(PWM13)
		case PWM13:
			io_set_pwm(PWM13, value);
			break;
#endif
#if ASSERT_PIN(PWM14)
		case PWM14:
			io_set_pwm(PWM14, value);
			break;
#endif
#if ASSERT_PIN(PWM15)
		case PWM15:
			io_set_pwm(PWM15, value);
			break;
#endif
#if ASSERT_PIN(SERVO0)
		case SERVO0:
			io_set_pwm(SERVO0, value);
			break;
#endif
#if ASSERT_PIN(SERVO1)
		case SERVO1:
			io_set_pwm(SERVO1, value);
			break;
#endif
#if ASSERT_PIN(SERVO2)
		case SERVO2:
			io_set_pwm(SERVO2, value);
			break;
#endif
#if ASSERT_PIN(SERVO3)
		case SERVO3:
			io_set_pwm(SERVO3, value);
			break;
#endif
#if ASSERT_PIN(SERVO4)
		case SERVO4:
			io_set_pwm(SERVO4, value);
			break;
#endif
#if ASSERT_PIN(SERVO5)
		case SERVO5:
			io_set_pwm(SERVO5, value);
			break;
#endif
#if ASSERT_PIN(DOUT0)
		case DOUT0:
			io_set_output(DOUT0);
			break;
#endif
#if ASSERT_PIN(DOUT1)
		case DOUT1:
			io_set_output(DOUT1);
			break;
#endif
#if ASSERT_PIN(DOUT2)
		case DOUT2:
			io_set_output(DOUT2);
			break;
#endif
#if ASSERT_PIN(DOUT3)
		case DOUT3:
			io_set_output(DOUT3);
			break;
#endif
#if ASSERT_PIN(DOUT4)
		case DOUT4:
			io_set_output(DOUT4);
			break;
#endif
#if ASSERT_PIN(DOUT5)
		case DOUT5:
			io_set_output(DOUT5);
			break;
#endif
#if ASSERT_PIN(DOUT6)
		case DOUT6:
			io_set_output(DOUT6);
			break;
#endif
#if ASSERT_PIN(DOUT7)
		case DOUT7:
			io_set_output(DOUT7);
			break;
#endif
#if ASSERT_PIN(DOUT8)
		case DOUT8:
			io_set_output(DOUT8);
			break;
#endif
#if ASSERT_PIN(DOUT9)
		case DOUT9:
			io_set_output(DOUT9);
			break;
#endif
#if ASSERT_PIN(DOUT10)
		case DOUT10:
			io_set_output(DOUT10);
			break;
#endif
#if ASSERT_PIN(DOUT11)
		case DOUT11:
			io_set_output(DOUT11);
			break;
#endif
#if ASSERT_PIN(DOUT12)
		case DOUT12:
			io_set_output(DOUT12);
			break;
#endif
#if ASSERT_PIN(DOUT13)
		case DOUT13:
			io_set_output(DOUT13);
			break;
#endif
#if ASSERT_PIN(DOUT14)
		case DOUT14:
			io_set_output(DOUT14);
			break;
#endif
#if ASSERT_PIN(DOUT15)
		case DOUT15:
			io_set_output(DOUT15);
			break;
#endif
#if ASSERT_PIN(DOUT16)
		case DOUT16:
			io_set_output(DOUT16);
			break;
#endif
#if ASSERT_PIN(DOUT17)
		case DOUT17:
			io_set_output(DOUT17);
			break;
#endif
#if ASSERT_PIN(DOUT18)
		case DOUT18:
			io_set_output(DOUT18);
			break;
#endif
#if ASSERT_PIN(DOUT19)
		case DOUT19:
			io_set_output(DOUT19);
			break;
#endif
#if ASSERT_PIN(DOUT20)
		case DOUT20:
			io_set_output(DOUT20);
			break;
#endif
#if ASSERT_PIN(DOUT21)
		case DOUT21:
			io_set_output(DOUT21);
			break;
#endif
#if ASSERT_PIN(DOUT22)
		case DOUT22:
			io_set_output(DOUT22);
			break;
#endif
#if ASSERT_PIN(DOUT23)
		case DOUT23:
			io_set_output(DOUT23);
			break;
#endif
#if ASSERT_PIN(DOUT24)
		case DOUT24:
			io_set_output(DOUT24);
			break;
#endif
#if ASSERT_PIN(DOUT25)
		case DOUT25:
			io_set_output(DOUT25);
			break;
#endif
#if ASSERT_PIN(DOUT26)
		case DOUT26:
			io_set_output(DOUT26);
			break;
#endif
#if ASSERT_PIN(DOUT27)
		case DOUT27:
			io_set_output(DOUT27);
			break;
#endif
#if ASSERT_PIN(DOUT28)
		case DOUT28:
			io_set_output(DOUT28);
			break;
#endif
#if ASSERT_PIN(DOUT29)
		case DOUT29:
			io_set_output(DOUT29);
			break;
#endif
#if ASSERT_PIN(DOUT30)
		case DOUT30:
			io_set_output(DOUT30);
			break;
#endif
#if ASSERT_PIN(DOUT31)
		case DOUT31:
			io_set_output(DOUT31);
			break;
#endif
		}
	}
	else
	{
		switch (pin)
		{
#if ASSERT_PIN(PWM0)
		case PWM0:
			io_set_pwm(PWM0, 0);
			break;
#endif
#if ASSERT_PIN(PWM1)
		case PWM1:
			io_set_pwm(PWM1, 0);
			break;
#endif
#if ASSERT_PIN(PWM2)
		case PWM2:
			io_set_pwm(PWM2, 0);
			break;
#endif
#if ASSERT_PIN(PWM3)
		case PWM3:
			io_set_pwm(PWM3, 0);
			break;
#endif
#if ASSERT_PIN(PWM4)
		case PWM4:
			io_set_pwm(PWM4, 0);
			break;
#endif
#if ASSERT_PIN(PWM5)
		case PWM5:
			io_set_pwm(PWM5, 0);
			break;
#endif
#if ASSERT_PIN(PWM6)
		case PWM6:
			io_set_pwm(PWM6, 0);
			break;
#endif
#if ASSERT_PIN(PWM7)
		case PWM7:
			io_set_pwm(PWM7, 0);
			break;
#endif
#if ASSERT_PIN(PWM8)
		case PWM8:
			io_set_pwm(PWM8, 0);
			break;
#endif
#if ASSERT_PIN(PWM9)
		case PWM9:
			io_set_pwm(PWM9, 0);
			break;
#endif
#if ASSERT_PIN(PWM10)
		case PWM10:
			io_set_pwm(PWM10, 0);
			break;
#endif
#if ASSERT_PIN(PWM11)
		case PWM11:
			io_set_pwm(PWM11, 0);
			break;
#endif
#if ASSERT_PIN(PWM12)
		case PWM12:
			io_set_pwm(PWM12, 0);
			break;
#endif
#if ASSERT_PIN(PWM13)
		case PWM13:
			io_set_pwm(PWM13, 0);
			break;
#endif
#if ASSERT_PIN(PWM14)
		case PWM14:
			io_set_pwm(PWM14, 0);
			break;
#endif
#if ASSERT_PIN(PWM15)
		case PWM15:
			io_set_pwm(PWM15, 0);
			break;
#endif
#if ASSERT_PIN(SERVO0)
		case SERVO0:
			io_set_pwm(SERVO0, 0);
			break;
#endif
#if ASSERT_PIN(SERVO1)
		case SERVO1:
			io_set_pwm(SERVO1, 0);
			break;
#endif
#if ASSERT_PIN(SERVO2)
		case SERVO2:
			io_set_pwm(SERVO2, 0);
			break;
#endif
#if ASSERT_PIN(SERVO3)
		case SERVO3:
			io_set_pwm(SERVO3, 0);
			break;
#endif
#if ASSERT_PIN(SERVO4)
		case SERVO4:
			io_set_pwm(SERVO4, 0);
			break;
#endif
#if ASSERT_PIN(SERVO5)
		case SERVO5:
			io_set_pwm(SERVO5, 0);
			break;
#endif

#if ASSERT_PIN(DOUT0)
		case DOUT0:
			io_clear_output(DOUT0);
			break;
#endif
#if ASSERT_PIN(DOUT1)
		case DOUT1:
			io_clear_output(DOUT1);
			break;
#endif
#if ASSERT_PIN(DOUT2)
		case DOUT2:
			io_clear_output(DOUT2);
			break;
#endif
#if ASSERT_PIN(DOUT3)
		case DOUT3:
			io_clear_output(DOUT3);
			break;
#endif
#if ASSERT_PIN(DOUT4)
		case DOUT4:
			io_clear_output(DOUT4);
			break;
#endif
#if ASSERT_PIN(DOUT5)
		case DOUT5:
			io_clear_output(DOUT5);
			break;
#endif
#if ASSERT_PIN(DOUT6)
		case DOUT6:
			io_clear_output(DOUT6);
			break;
#endif
#if ASSERT_PIN(DOUT7)
		case DOUT7:
			io_clear_output(DOUT7);
			break;
#endif
#if ASSERT_PIN(DOUT8)
		case DOUT8:
			io_clear_output(DOUT8);
			break;
#endif
#if ASSERT_PIN(DOUT9)
		case DOUT9:
			io_clear_output(DOUT9);
			break;
#endif
#if ASSERT_PIN(DOUT10)
		case DOUT10:
			io_clear_output(DOUT10);
			break;
#endif
#if ASSERT_PIN(DOUT11)
		case DOUT11:
			io_clear_output(DOUT11);
			break;
#endif
#if ASSERT_PIN(DOUT12)
		case DOUT12:
			io_clear_output(DOUT12);
			break;
#endif
#if ASSERT_PIN(DOUT13)
		case DOUT13:
			io_clear_output(DOUT13);
			break;
#endif
#if ASSERT_PIN(DOUT14)
		case DOUT14:
			io_clear_output(DOUT14);
			break;
#endif
#if ASSERT_PIN(DOUT15)
		case DOUT15:
			io_clear_output(DOUT15);
			break;
#endif
#if ASSERT_PIN(DOUT16)
		case DOUT16:
			io_clear_output(DOUT16);
			break;
#endif
#if ASSERT_PIN(DOUT17)
		case DOUT17:
			io_clear_output(DOUT17);
			break;
#endif
#if ASSERT_PIN(DOUT18)
		case DOUT18:
			io_clear_output(DOUT18);
			break;
#endif
#if ASSERT_PIN(DOUT19)
		case DOUT19:
			io_clear_output(DOUT19);
			break;
#endif
#if ASSERT_PIN(DOUT20)
		case DOUT20:
			io_clear_output(DOUT20);
			break;
#endif
#if ASSERT_PIN(DOUT21)
		case DOUT21:
			io_clear_output(DOUT21);
			break;
#endif
#if ASSERT_PIN(DOUT22)
		case DOUT22:
			io_clear_output(DOUT22);
			break;
#endif
#if ASSERT_PIN(DOUT23)
		case DOUT23:
			io_clear_output(DOUT23);
			break;
#endif
#if ASSERT_PIN(DOUT24)
		case DOUT24:
			io_clear_output(DOUT24);
			break;
#endif
#if ASSERT_PIN(DOUT25)
		case DOUT25:
			io_clear_output(DOUT25);
			break;
#endif
#if ASSERT_PIN(DOUT26)
		case DOUT26:
			io_clear_output(DOUT26);
			break;
#endif
#if ASSERT_PIN(DOUT27)
		case DOUT27:
			io_clear_output(DOUT27);
			break;
#endif
#if ASSERT_PIN(DOUT28)
		case DOUT28:
			io_clear_output(DOUT28);
			break;
#endif
#if ASSERT_PIN(DOUT29)
		case DOUT29:
			io_clear_output(DOUT29);
			break;
#endif
#if ASSERT_PIN(DOUT30)
		case DOUT30:
			io_clear_output(DOUT30);
			break;
#endif
#if ASSERT_PIN(DOUT31)
		case DOUT31:
			io_clear_output(DOUT31);
			break;
#endif
		}
	}

#ifdef IC74HC595_HAS_DOUTS
	ic74hc595_shift_io_pins();
#endif
}

int16_t io_get_pinvalue(uint8_t pin)
{
	switch (pin)
	{
#if ASSERT_PIN(STEP0)
	case STEP0:
		return (io_get_output(STEP0) != 0);
#endif
#if ASSERT_PIN(STEP1)
	case STEP1:
		return (io_get_output(STEP1) != 0);
#endif
#if ASSERT_PIN(STEP2)
	case STEP2:
		return (io_get_output(STEP2) != 0);
#endif
#if ASSERT_PIN(STEP3)
	case STEP3:
		return (io_get_output(STEP3) != 0);
#endif
#if ASSERT_PIN(STEP4)
	case STEP4:
		return (io_get_output(STEP4) != 0);
#endif
#if ASSERT_PIN(STEP5)
	case STEP5:
		return (io_get_output(STEP5) != 0);
#endif
#if ASSERT_PIN(STEP6)
	case STEP6:
		return (io_get_output(STEP6) != 0);
#endif
#if ASSERT_PIN(STEP7)
	case STEP7:
		return (io_get_output(STEP7) != 0);
#endif
#if ASSERT_PIN(DIR0)
	case DIR0:
		return (io_get_output(DIR0) != 0);
#endif
#if ASSERT_PIN(DIR1)
	case DIR1:
		return (io_get_output(DIR1) != 0);
#endif
#if ASSERT_PIN(DIR2)
	case DIR2:
		return (io_get_output(DIR2) != 0);
#endif
#if ASSERT_PIN(DIR3)
	case DIR3:
		return (io_get_output(DIR3) != 0);
#endif
#if ASSERT_PIN(DIR4)
	case DIR4:
		return (io_get_output(DIR4) != 0);
#endif
#if ASSERT_PIN(DIR5)
	case DIR5:
		return (io_get_output(DIR5) != 0);
#endif
#if ASSERT_PIN(DIR6)
	case DIR6:
		return (io_get_output(DIR6) != 0);
#endif
#if ASSERT_PIN(DIR7)
	case DIR7:
		return (io_get_output(DIR7) != 0);
#endif
#if ASSERT_PIN(STEP0_EN)
	case STEP0_EN:
		return (io_get_output(STEP0_EN) != 0);
#endif
#if ASSERT_PIN(STEP1_EN)
	case STEP1_EN:
		return (io_get_output(STEP1_EN) != 0);
#endif
#if ASSERT_PIN(STEP2_EN)
	case STEP2_EN:
		return (io_get_output(STEP2_EN) != 0);
#endif
#if ASSERT_PIN(STEP3_EN)
	case STEP3_EN:
		return (io_get_output(STEP3_EN) != 0);
#endif
#if ASSERT_PIN(STEP4_EN)
	case STEP4_EN:
		return (io_get_output(STEP4_EN) != 0);
#endif
#if ASSERT_PIN(STEP5_EN)
	case STEP5_EN:
		return (io_get_output(STEP5_EN) != 0);
#endif
#if ASSERT_PIN(STEP6_EN)
	case STEP6_EN:
		return (io_get_output(STEP6_EN) != 0);
#endif
#if ASSERT_PIN(STEP7_EN)
	case STEP7_EN:
		return (io_get_output(STEP7_EN) != 0);
#endif
#if ASSERT_PIN(PWM0)
	case PWM0:
		return io_get_pwm(PWM0);
#endif
#if ASSERT_PIN(PWM1)
	case PWM1:
		return io_get_pwm(PWM1);
#endif
#if ASSERT_PIN(PWM2)
	case PWM2:
		return io_get_pwm(PWM2);
#endif
#if ASSERT_PIN(PWM3)
	case PWM3:
		return io_get_pwm(PWM3);
#endif
#if ASSERT_PIN(PWM4)
	case PWM4:
		return io_get_pwm(PWM4);
#endif
#if ASSERT_PIN(PWM5)
	case PWM5:
		return io_get_pwm(PWM5);
#endif
#if ASSERT_PIN(PWM6)
	case PWM6:
		return io_get_pwm(PWM6);
#endif
#if ASSERT_PIN(PWM7)
	case PWM7:
		return io_get_pwm(PWM7);
#endif
#if ASSERT_PIN(PWM8)
	case PWM8:
		return io_get_pwm(PWM8);
#endif
#if ASSERT_PIN(PWM9)
	case PWM9:
		return io_get_pwm(PWM9);
#endif
#if ASSERT_PIN(PWM10)
	case PWM10:
		return io_get_pwm(PWM10);
#endif
#if ASSERT_PIN(PWM11)
	case PWM11:
		return io_get_pwm(PWM11);
#endif
#if ASSERT_PIN(PWM12)
	case PWM12:
		return io_get_pwm(PWM12);
#endif
#if ASSERT_PIN(PWM13)
	case PWM13:
		return io_get_pwm(PWM13);
#endif
#if ASSERT_PIN(PWM14)
	case PWM14:
		return io_get_pwm(PWM14);
#endif
#if ASSERT_PIN(PWM15)
	case PWM15:
		return io_get_pwm(PWM15);
#endif
#if ASSERT_PIN(DOUT0)
	case DOUT0:
		return (io_get_output(DOUT0) != 0);
#endif
#if ASSERT_PIN(DOUT1)
	case DOUT1:
		return (io_get_output(DOUT1) != 0);
#endif
#if ASSERT_PIN(DOUT2)
	case DOUT2:
		return (io_get_output(DOUT2) != 0);
#endif
#if ASSERT_PIN(DOUT3)
	case DOUT3:
		return (io_get_output(DOUT3) != 0);
#endif
#if ASSERT_PIN(DOUT4)
	case DOUT4:
		return (io_get_output(DOUT4) != 0);
#endif
#if ASSERT_PIN(DOUT5)
	case DOUT5:
		return (io_get_output(DOUT5) != 0);
#endif
#if ASSERT_PIN(DOUT6)
	case DOUT6:
		return (io_get_output(DOUT6) != 0);
#endif
#if ASSERT_PIN(DOUT7)
	case DOUT7:
		return (io_get_output(DOUT7) != 0);
#endif
#if ASSERT_PIN(DOUT8)
	case DOUT8:
		return (io_get_output(DOUT8) != 0);
#endif
#if ASSERT_PIN(DOUT9)
	case DOUT9:
		return (io_get_output(DOUT9) != 0);
#endif
#if ASSERT_PIN(DOUT10)
	case DOUT10:
		return (io_get_output(DOUT10) != 0);
#endif
#if ASSERT_PIN(DOUT11)
	case DOUT11:
		return (io_get_output(DOUT11) != 0);
#endif
#if ASSERT_PIN(DOUT12)
	case DOUT12:
		return (io_get_output(DOUT12) != 0);
#endif
#if ASSERT_PIN(DOUT13)
	case DOUT13:
		return (io_get_output(DOUT13) != 0);
#endif
#if ASSERT_PIN(DOUT14)
	case DOUT14:
		return (io_get_output(DOUT14) != 0);
#endif
#if ASSERT_PIN(DOUT15)
	case DOUT15:
		return (io_get_output(DOUT15) != 0);
#endif
#if ASSERT_PIN(DOUT16)
	case DOUT16:
		return (io_get_output(DOUT16) != 0);
#endif
#if ASSERT_PIN(DOUT17)
	case DOUT17:
		return (io_get_output(DOUT17) != 0);
#endif
#if ASSERT_PIN(DOUT18)
	case DOUT18:
		return (io_get_output(DOUT18) != 0);
#endif
#if ASSERT_PIN(DOUT19)
	case DOUT19:
		return (io_get_output(DOUT19) != 0);
#endif
#if ASSERT_PIN(DOUT20)
	case DOUT20:
		return (io_get_output(DOUT20) != 0);
#endif
#if ASSERT_PIN(DOUT21)
	case DOUT21:
		return (io_get_output(DOUT21) != 0);
#endif
#if ASSERT_PIN(DOUT22)
	case DOUT22:
		return (io_get_output(DOUT22) != 0);
#endif
#if ASSERT_PIN(DOUT23)
	case DOUT23:
		return (io_get_output(DOUT23) != 0);
#endif
#if ASSERT_PIN(DOUT24)
	case DOUT24:
		return (io_get_output(DOUT24) != 0);
#endif
#if ASSERT_PIN(DOUT25)
	case DOUT25:
		return (io_get_output(DOUT25) != 0);
#endif
#if ASSERT_PIN(DOUT26)
	case DOUT26:
		return (io_get_output(DOUT26) != 0);
#endif
#if ASSERT_PIN(DOUT27)
	case DOUT27:
		return (io_get_output(DOUT27) != 0);
#endif
#if ASSERT_PIN(DOUT28)
	case DOUT28:
		return (io_get_output(DOUT28) != 0);
#endif
#if ASSERT_PIN(DOUT29)
	case DOUT29:
		return (io_get_output(DOUT29) != 0);
#endif
#if ASSERT_PIN(DOUT30)
	case DOUT30:
		return (io_get_output(DOUT30) != 0);
#endif
#if ASSERT_PIN(DOUT31)
	case DOUT31:
		return (io_get_output(DOUT31) != 0);
#endif
#if ASSERT_PIN(LIMIT_X)
	case LIMIT_X:
		return (io_get_input(LIMIT_X) != 0);
#endif
#if ASSERT_PIN(LIMIT_Y)
	case LIMIT_Y:
		return (io_get_input(LIMIT_Y) != 0);
#endif
#if ASSERT_PIN(LIMIT_Z)
	case LIMIT_Z:
		return (io_get_input(LIMIT_Z) != 0);
#endif
#if ASSERT_PIN(LIMIT_X2)
	case LIMIT_X2:
		return (io_get_input(LIMIT_X2) != 0);
#endif
#if ASSERT_PIN(LIMIT_Y2)
	case LIMIT_Y2:
		return (io_get_input(LIMIT_Y2) != 0);
#endif
#if ASSERT_PIN(LIMIT_Z2)
	case LIMIT_Z2:
		return (io_get_input(LIMIT_Z2) != 0);
#endif
#if ASSERT_PIN(LIMIT_A)
	case LIMIT_A:
		return (io_get_input(LIMIT_A) != 0);
#endif
#if ASSERT_PIN(LIMIT_B)
	case LIMIT_B:
		return (io_get_input(LIMIT_B) != 0);
#endif
#if ASSERT_PIN(LIMIT_C)
	case LIMIT_C:
		return (io_get_input(LIMIT_C) != 0);
#endif
#if ASSERT_PIN(PROBE)
	case PROBE:
		return (io_get_input(PROBE) != 0);
#endif
#if ASSERT_PIN(ESTOP)
	case ESTOP:
#ifndef INVERT_EMERGENCY_STOP
		return (io_get_input(ESTOP) != 0);
#else
		return (io_get_input(ESTOP) == 0);
#endif
#endif
#if ASSERT_PIN(SAFETY_DOOR)
	case SAFETY_DOOR:
		return (io_get_input(SAFETY_DOOR) != 0);
#endif
#if ASSERT_PIN(FHOLD)
	case FHOLD:
		return (io_get_input(FHOLD) != 0);
#endif
#if ASSERT_PIN(CS_RES)
	case CS_RES:
		return (io_get_input(CS_RES) != 0);
#endif
#if ASSERT_PIN(ANALOG0)
	case ANALOG0:
		return mcu_get_analog(ANALOG0);
#endif
#if ASSERT_PIN(ANALOG1)
	case ANALOG1:
		return mcu_get_analog(ANALOG1);
#endif
#if ASSERT_PIN(ANALOG2)
	case ANALOG2:
		return mcu_get_analog(ANALOG2);
#endif
#if ASSERT_PIN(ANALOG3)
	case ANALOG3:
		return mcu_get_analog(ANALOG3);
#endif
#if ASSERT_PIN(ANALOG4)
	case ANALOG4:
		return mcu_get_analog(ANALOG4);
#endif
#if ASSERT_PIN(ANALOG5)
	case ANALOG5:
		return mcu_get_analog(ANALOG5);
#endif
#if ASSERT_PIN(ANALOG6)
	case ANALOG6:
		return mcu_get_analog(ANALOG6);
#endif
#if ASSERT_PIN(ANALOG7)
	case ANALOG7:
		return mcu_get_analog(ANALOG7);
#endif
#if ASSERT_PIN(ANALOG8)
	case ANALOG8:
		return mcu_get_analog(ANALOG8);
#endif
#if ASSERT_PIN(ANALOG9)
	case ANALOG9:
		return mcu_get_analog(ANALOG9);
#endif
#if ASSERT_PIN(ANALOG10)
	case ANALOG10:
		return mcu_get_analog(ANALOG10);
#endif
#if ASSERT_PIN(ANALOG11)
	case ANALOG11:
		return mcu_get_analog(ANALOG11);
#endif
#if ASSERT_PIN(ANALOG12)
	case ANALOG12:
		return mcu_get_analog(ANALOG12);
#endif
#if ASSERT_PIN(ANALOG13)
	case ANALOG13:
		return mcu_get_analog(ANALOG13);
#endif
#if ASSERT_PIN(ANALOG14)
	case ANALOG14:
		return mcu_get_analog(ANALOG14);
#endif
#if ASSERT_PIN(ANALOG15)
	case ANALOG15:
		return mcu_get_analog(ANALOG15);
#endif
#if ASSERT_PIN(DIN0)
	case DIN0:
		return (io_get_input(DIN0) != 0);
#endif
#if ASSERT_PIN(DIN1)
	case DIN1:
		return (io_get_input(DIN1) != 0);
#endif
#if ASSERT_PIN(DIN2)
	case DIN2:
		return (io_get_input(DIN2) != 0);
#endif
#if ASSERT_PIN(DIN3)
	case DIN3:
		return (io_get_input(DIN3) != 0);
#endif
#if ASSERT_PIN(DIN4)
	case DIN4:
		return (io_get_input(DIN4) != 0);
#endif
#if ASSERT_PIN(DIN5)
	case DIN5:
		return (io_get_input(DIN5) != 0);
#endif
#if ASSERT_PIN(DIN6)
	case DIN6:
		return (io_get_input(DIN6) != 0);
#endif
#if ASSERT_PIN(DIN7)
	case DIN7:
		return (io_get_input(DIN7) != 0);
#endif
#if ASSERT_PIN(DIN8)
	case DIN8:
		return (io_get_input(DIN8) != 0);
#endif
#if ASSERT_PIN(DIN9)
	case DIN9:
		return (io_get_input(DIN9) != 0);
#endif
#if ASSERT_PIN(DIN10)
	case DIN10:
		return (io_get_input(DIN10) != 0);
#endif
#if ASSERT_PIN(DIN11)
	case DIN11:
		return (io_get_input(DIN11) != 0);
#endif
#if ASSERT_PIN(DIN12)
	case DIN12:
		return (io_get_input(DIN12) != 0);
#endif
#if ASSERT_PIN(DIN13)
	case DIN13:
		return (io_get_input(DIN13) != 0);
#endif
#if ASSERT_PIN(DIN14)
	case DIN14:
		return (io_get_input(DIN14) != 0);
#endif
#if ASSERT_PIN(DIN15)
	case DIN15:
		return (io_get_input(DIN15) != 0);
#endif
#if ASSERT_PIN(DIN16)
	case DIN16:
		return (io_get_input(DIN16) != 0);
#endif
#if ASSERT_PIN(DIN17)
	case DIN17:
		return (io_get_input(DIN17) != 0);
#endif
#if ASSERT_PIN(DIN18)
	case DIN18:
		return (io_get_input(DIN18) != 0);
#endif
#if ASSERT_PIN(DIN19)
	case DIN19:
		return (io_get_input(DIN19) != 0);
#endif
#if ASSERT_PIN(DIN20)
	case DIN20:
		return (io_get_input(DIN20) != 0);
#endif
#if ASSERT_PIN(DIN21)
	case DIN21:
		return (io_get_input(DIN21) != 0);
#endif
#if ASSERT_PIN(DIN22)
	case DIN22:
		return (io_get_input(DIN22) != 0);
#endif
#if ASSERT_PIN(DIN23)
	case DIN23:
		return (io_get_input(DIN23) != 0);
#endif
#if ASSERT_PIN(DIN24)
	case DIN24:
		return (io_get_input(DIN24) != 0);
#endif
#if ASSERT_PIN(DIN25)
	case DIN25:
		return (io_get_input(DIN25) != 0);
#endif
#if ASSERT_PIN(DIN26)
	case DIN26:
		return (io_get_input(DIN26) != 0);
#endif
#if ASSERT_PIN(DIN27)
	case DIN27:
		return (io_get_input(DIN27) != 0);
#endif
#if ASSERT_PIN(DIN28)
	case DIN28:
		return (io_get_input(DIN28) != 0);
#endif
#if ASSERT_PIN(DIN29)
	case DIN29:
		return (io_get_input(DIN29) != 0);
#endif
#if ASSERT_PIN(DIN30)
	case DIN30:
		return (io_get_input(DIN30) != 0);
#endif
#if ASSERT_PIN(DIN31)
	case DIN31:
		return (io_get_input(DIN31) != 0);
#endif
#if ASSERT_PIN(SERVO0)
	case SERVO0:
		return (uint8_t)mcu_get_servo(SERVO0);
#endif
#if ASSERT_PIN(SERVO1)
	case SERVO1:
		return (uint8_t)mcu_get_servo(SERVO1);
#endif
#if ASSERT_PIN(SERVO2)
	case SERVO2:
		return (uint8_t)mcu_get_servo(SERVO2);
#endif
#if ASSERT_PIN(SERVO3)
	case SERVO3:
		return (uint8_t)mcu_get_servo(SERVO3);
#endif
#if ASSERT_PIN(SERVO4)
	case SERVO4:
		return (uint8_t)mcu_get_servo(SERVO4);
#endif
#if ASSERT_PIN(SERVO5)
	case SERVO5:
		return (uint8_t)mcu_get_servo(SERVO5);
#endif
	}
	return -1;
}
