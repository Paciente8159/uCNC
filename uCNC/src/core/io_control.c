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

#if PID_CONTROLLERS > 0
static volatile uint8_t io_spindle_speed;
#endif

static uint8_t io_lock_limits_mask;
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

#if (LIMITS_DUAL_MASK != 0)
		static uint8_t prev_limits_dual = 0;
		uint8_t limits_dual = io_get_limits_dual();
		uint8_t limit_combined = limits | limits_dual;

		if (!(limits ^ prev_limits) && !(limits_dual ^ prev_limits_dual))
		{
			return;
		}
		prev_limits = limits;
		prev_limits_dual = limits_dual;
#else
		if (!(limits ^ prev_limits))
		{
			return;
		}
		prev_limits = limits;
		uint8_t limit_combined = limits;
#endif
		if (limit_combined)
		{
#if (defined(DUAL_DRIVE0_ENABLE_SELFSQUARING) || defined(DUAL_DRIVE1_ENABLE_SELFSQUARING) || defined(IS_DELTA_KINEMATICS))
			if (cnc_get_exec_state((EXEC_RUN | EXEC_HOMING)) == (EXEC_RUN | EXEC_HOMING) && (io_lock_limits_mask & limit_combined))
			{
				// if homing and dual drive axis are enabled
#ifdef DUAL_DRIVE0_ENABLE_SELFSQUARING
				if (limit_combined & LIMIT_DUAL0_MASK) // the limit triggered matches the first dual drive axis
				{
					// lock the stepper accodring to the blocked
					itp_lock_stepper((limits_dual & LIMIT_DUAL0_MASK) ? STEP_DUAL0_MASK : STEP_DUAL0);

					if (limits != limits_dual) // but not both
					{
						return; // exits and doesn't trip the alarm
					}
				}
#endif
#ifdef DUAL_DRIVE1_ENABLE_SELFSQUARING
				if (limit_combined & LIMIT_DUAL1_MASK) // the limit triggered matches the second dual drive axis
				{
					itp_lock_stepper((limits_dual & LIMIT_DUAL1_MASK) ? STEP_DUAL1_MASK : STEP_DUAL1);

					if (limits != limits_dual) // but not both
					{
						return; // exits and doesn't trip the alarm
					}
				}
#endif
#if (defined(IS_DELTA_KINEMATICS))
				if ((limit_combined & LIMITS_DELTA_MASK))
				{
					if (limit_combined != LIMITS_DELTA_MASK)
					{
						itp_lock_stepper(limit_combined);
						return;
					}
				}
				else
				{
					return;
				}
#endif
			}
#endif

#if (defined(DUAL_DRIVE0_ENABLE_SELFSQUARING) || defined(DUAL_DRIVE1_ENABLE_SELFSQUARING) || defined(IS_DELTA_KINEMATICS))
			itp_lock_stepper(0); // unlocks axis
#endif
			itp_stop();
			cnc_call_rt_state_command(RT_CMD_LIMITS_HIT);
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
		cnc_call_rt_state_command(RT_CMD_RESET);
		return; // forces exit
	}
#endif
#if ASSERT_PIN(SAFETY_DOOR)
	if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
	{
		// safety door activates hold simultaneously to start the controlled stop
		cnc_call_rt_state_command(RT_CMD_FEED_HOLD | RT_CMD_SAFETY_DOOR);
#ifdef ENABLE_IO_ALARM_DEBUG
		io_alarm_controls = controls;
#endif
	}
#endif
#if ASSERT_PIN(FHOLD)
	if (CHECKFLAG(controls, FHOLD_MASK))
	{
		cnc_call_rt_state_command(RT_CMD_FEED_HOLD);
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
	uint8_t inputs = io_get_onchange_inputs();
	uint8_t diff;

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

void io_lock_limits(uint8_t limitmask)
{
	io_lock_limits_mask = limitmask;
}

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
	value |= ((mcu_get_input(LIMIT_X)) ? LIMIT_X_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_Y)
	value |= ((mcu_get_input(LIMIT_Y)) ? LIMIT_Y_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_Z)
	value |= ((mcu_get_input(LIMIT_Z)) ? LIMIT_Z_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_A)
	value |= ((mcu_get_input(LIMIT_A)) ? LIMIT_A_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_B)
	value |= ((mcu_get_input(LIMIT_B)) ? LIMIT_B_MASK : 0);
#endif
#if ASSERT_PIN(LIMIT_C)
	value |= ((mcu_get_input(LIMIT_C)) ? LIMIT_C_MASK : 0);
#endif

	uint8_t inv = g_settings.limits_invert_mask;
	uint8_t result = (value ^ (inv & LIMITS_INV_MASK));

#if LIMITS_DUAL_INV_MASK != 0
	uint8_t value2 = 0;

#if ASSERT_PIN(LIMIT_X2)
#if !(LIMITS_DUAL_MASK & LIMIT_X_MASK)
	value2 |= ((mcu_get_input(LIMIT_X2)) ? LIMIT_X_MASK : 0);
#endif
#endif
#if ASSERT_PIN(LIMIT_Y2)
#if !(LIMITS_DUAL_MASK & LIMIT_Y_MASK)
	value2 |= ((mcu_get_input(LIMIT_Y2)) ? LIMIT_Y_MASK : 0);
#endif
#endif
#if ASSERT_PIN(LIMIT_Z2)
#if !(LIMITS_DUAL_MASK & LIMIT_Z_MASK)
	value2 |= ((mcu_get_input(LIMIT_Z2)) ? LIMIT_Z_MASK : 0);
#endif
#endif

	result |= (value2 ^ (inv & LIMITS_DUAL_INV_MASK & ~LIMITS_DUAL_MASK));

#endif

	if (cnc_get_exec_state(EXEC_HOMING))
	{
		result ^= io_invert_limits_mask;
	}
#if LIMITS_DUAL_INV_MASK
	else
	{
		result |= io_get_limits_dual();
	}
#endif

#ifdef ENABLE_MULTIBOARD
	result |= (g_slave_io.slave_io_bits.limits & ~(LIMITS_DUAL_MASK | LIMITS_MASK));
	g_slave_io.slave_io_bits.limits = result;
#endif

	return result;
}

uint8_t io_get_limits_dual(void)
{
#if (defined(DISABLE_ALL_LIMITS) || (LIMITS_DUAL_MASK == 0))
	return 0;
#else
	uint8_t value = 0;
#if ASSERT_PIN(LIMIT_X2)
#if (LIMITS_DUAL_MASK & LIMIT_X_MASK)
	value |= ((mcu_get_input(LIMIT_X2)) ? LIMIT_X_MASK : 0);
#endif
#endif
#if ASSERT_PIN(LIMIT_Y2)
#if (LIMITS_DUAL_MASK & LIMIT_Y_MASK)
	value |= ((mcu_get_input(LIMIT_Y2)) ? LIMIT_Y_MASK : 0);
#endif
#endif
#if ASSERT_PIN(LIMIT_Z2)
#if (LIMITS_DUAL_MASK & LIMIT_Z_MASK)
	value |= ((mcu_get_input(LIMIT_Z2)) ? LIMIT_Z_MASK : 0);
#endif
#endif
	uint8_t result = io_invert_limits_mask & LIMITS_DUAL_MASK;
	result ^= (value ^ (g_settings.limits_invert_mask & LIMITS_DUAL_MASK & LIMITS_DUAL_INV_MASK));

#ifdef ENABLE_MULTIBOARD
	result |= (g_slave_io.slave_io_bits.limits2 & ~LIMITS_DUAL_MASK);
	g_slave_io.slave_io_bits.limits2 = result;
#endif
	return result;
#endif
}

uint8_t io_get_controls(void)
{
#ifdef DISABLE_ALL_CONTROLS
	return 0;
#endif
	uint8_t value = 0;
#if ASSERT_PIN(ESTOP)
#ifndef INVERT_EMERGENCY_STOP
	value |= ((mcu_get_input(ESTOP)) ? ESTOP_MASK : 0);
#else
	value |= ((!mcu_get_input(ESTOP)) ? ESTOP_MASK : 0);
#endif
#endif
#if ASSERT_PIN(SAFETY_DOOR)
	value |= ((mcu_get_input(SAFETY_DOOR)) ? SAFETY_DOOR_MASK : 0);
#endif
#if ASSERT_PIN(FHOLD)
	value |= ((mcu_get_input(FHOLD)) ? FHOLD_MASK : 0);
#endif
#if ASSERT_PIN(CS_RES)
	value |= ((mcu_get_input(CS_RES)) ? CS_RES_MASK : 0);
#endif

	uint8_t result = (value ^ (g_settings.control_invert_mask & CONTROLS_INV_MASK));

#ifdef ENABLE_MULTIBOARD
	result |= (g_slave_io.slave_io_bits.controls & ~CONTROLS_MASK);
	g_slave_io.slave_io_bits.controls = result;
#endif

	return result;
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

uint8_t io_get_onchange_inputs(void)
{
	uint8_t inputs = 0;
#if (DIN_ONCHANGE_MASK != 0)
#if (DIN0_MASK != 0)
	if (mcu_get_input(DIN0))
	{
		inputs |= DIN0_MASK;
	}
#endif
#if (DIN1_MASK != 0)
	if (mcu_get_input(DIN1))
	{
		inputs |= DIN1_MASK;
	}
#endif
#if (DIN2_MASK != 0)
	if (mcu_get_input(DIN2))
	{
		inputs |= DIN2_MASK;
	}
#endif
#if (DIN3_MASK != 0)
	if (mcu_get_input(DIN3))
	{
		inputs |= DIN3_MASK;
	}
#endif
#if (DIN4_MASK != 0)
	if (mcu_get_input(DIN4))
	{
		inputs |= DIN4_MASK;
	}
#endif
#if (DIN5_MASK != 0)
	if (mcu_get_input(DIN5))
	{
		inputs |= DIN5_MASK;
	}
#endif
#if (DIN6_MASK != 0)
	if (mcu_get_input(DIN6))
	{
		inputs |= DIN6_MASK;
	}
#endif
#if (DIN7_MASK != 0)
	if (mcu_get_input(DIN7))
	{
		inputs |= DIN7_MASK;
	}
#endif
#endif

#ifdef ENABLE_MULTIBOARD
	inputs |= (g_slave_io.slave_io_bits.onchange_inputs & ~DIN_ONCHANGE_MASK);
	g_slave_io.slave_io_bits.onchange_inputs = inputs;
#endif

	return inputs;
}

bool io_get_probe(void)
{
#if ASSERT_PIN(PROBE)
	bool probe = (mcu_get_input(PROBE) != 0);
	probe = (!g_settings.probe_invert_mask) ? probe : !probe;

#ifdef ENABLE_MULTIBOARD
#if !ASSERT_PIN(PROBE)
	probe |= g_slave_io.slave_io_bits.probe;
#else
	g_slave_io.slave_io_bits.probe = probe;
#endif
#endif

	return probe;
#else
	return false;
#endif
}

// outputs
void io_set_steps(uint8_t mask)
{
	// #ifdef ENABLE_IO_MODULES
	// 	EVENT_INVOKE(set_steps, &mask);
	// #endif

#ifdef IC74HC595_HAS_STEPS
	ic74hc595_set_steps(mask);
#endif

#if ASSERT_PIN(STEP0)
	if (mask & STEP0_MASK)
	{
		mcu_set_output(STEP0);
	}
	else
	{
		mcu_clear_output(STEP0);
	}

#endif
#if ASSERT_PIN(STEP1)
	if (mask & STEP1_MASK)
	{
		mcu_set_output(STEP1);
	}
	else
	{
		mcu_clear_output(STEP1);
	}
#endif
#if ASSERT_PIN(STEP2)
	if (mask & STEP2_MASK)
	{
		mcu_set_output(STEP2);
	}
	else
	{
		mcu_clear_output(STEP2);
	}
#endif
#if ASSERT_PIN(STEP3)
	if (mask & STEP3_MASK)
	{
		mcu_set_output(STEP3);
	}
	else
	{
		mcu_clear_output(STEP3);
	}
#endif
#if ASSERT_PIN(STEP4)
	if (mask & STEP4_MASK)
	{
		mcu_set_output(STEP4);
	}
	else
	{
		mcu_clear_output(STEP4);
	}
#endif
#if ASSERT_PIN(STEP5)
	if (mask & STEP5_MASK)
	{
		mcu_set_output(STEP5);
	}
	else
	{
		mcu_clear_output(STEP5);
	}
#endif
#if ASSERT_PIN(STEP6)
	if (mask & STEP6_MASK)
	{
		mcu_set_output(STEP6);
	}
	else
	{
		mcu_clear_output(STEP6);
	}
#endif
#if ASSERT_PIN(STEP7)
	if (mask & STEP7_MASK)
	{
		mcu_set_output(STEP7);
	}
	else
	{
		mcu_clear_output(STEP7);
	}
#endif
}

void io_toggle_steps(uint8_t mask)
{
	// #ifdef ENABLE_IO_MODULES
	// 	EVENT_INVOKE(toggle_steps, &mask);
	// #endif

#ifdef IC74HC595_HAS_STEPS
	ic74hc595_toggle_steps(mask);
#endif

#if ASSERT_PIN(STEP0)
	if (mask & STEP0_MASK)
	{
		mcu_toggle_output(STEP0);
	}
#endif
#if ASSERT_PIN(STEP1)
	if (mask & STEP1_MASK)
	{
		mcu_toggle_output(STEP1);
	}
#endif
#if ASSERT_PIN(STEP2)
	if (mask & STEP2_MASK)
	{
		mcu_toggle_output(STEP2);
	}
#endif
#if ASSERT_PIN(STEP3)
	if (mask & STEP3_MASK)
	{
		mcu_toggle_output(STEP3);
	}
#endif
#if ASSERT_PIN(STEP4)
	if (mask & STEP4_MASK)
	{
		mcu_toggle_output(STEP4);
	}
#endif
#if ASSERT_PIN(STEP5)
	if (mask & STEP5_MASK)
	{
		mcu_toggle_output(STEP5);
	}
#endif
#if ASSERT_PIN(STEP6)
	if (mask & STEP6_MASK)
	{
		mcu_toggle_output(STEP6);
	}
#endif
#if ASSERT_PIN(STEP7)
	if (mask & STEP7_MASK)
	{
		mcu_toggle_output(STEP7);
	}
#endif
}

void io_set_dirs(uint8_t mask)
{
	mask ^= g_settings.dir_invert_mask;

	// #ifdef ENABLE_IO_MODULES
	// 	EVENT_INVOKE(set_dirs, &mask);
	// #endif

#ifdef IC74HC595_HAS_DIRS
	ic74hc595_set_dirs(mask);
#endif

#if ASSERT_PIN(DIR0)
	if (mask & DIR0_MASK)
	{
		mcu_set_output(DIR0);
	}
	else
	{
		mcu_clear_output(DIR0);
	}
#endif
#if ASSERT_PIN(DIR1)
	if (mask & DIR1_MASK)
	{
		mcu_set_output(DIR1);
	}
	else
	{
		mcu_clear_output(DIR1);
	}
#endif
#if ASSERT_PIN(DIR2)
	if (mask & DIR2_MASK)
	{
		mcu_set_output(DIR2);
	}
	else
	{
		mcu_clear_output(DIR2);
	}
#endif
#if ASSERT_PIN(DIR3)
	if (mask & DIR3_MASK)
	{
		mcu_set_output(DIR3);
	}
	else
	{
		mcu_clear_output(DIR3);
	}
#endif
#if ASSERT_PIN(DIR4)
	if (mask & DIR4_MASK)
	{
		mcu_set_output(DIR4);
	}
	else
	{
		mcu_clear_output(DIR4);
	}
#endif
#if ASSERT_PIN(DIR5)
	if (mask & DIR5_MASK)
	{
		mcu_set_output(DIR5);
	}
	else
	{
		mcu_clear_output(DIR5);
	}
#endif
#if ASSERT_PIN(DIR6)
	if (mask & DIR6_MASK)
	{
		mcu_set_output(DIR6);
	}
	else
	{
		mcu_clear_output(DIR6);
	}
#endif
#if ASSERT_PIN(DIR7)
	if (mask & DIR7_MASK)
	{
		mcu_set_output(DIR7);
	}
	else
	{
		mcu_clear_output(DIR7);
	}
#endif
}

void io_set_pwm(uint8_t pin, uint8_t value)
{
#if (defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS))
	if (pin > 127)
	{
		pin = ~pin;
		mcu_set_pwm(pin, value);
		return;
	}
#endif
	switch (pin)
	{
#if ASSERT_PIN(PWM0)
	case PWM0:
		mcu_set_pwm(PWM0, value);
		break;
#endif
#if ASSERT_PIN(PWM1)
	case PWM1:
		mcu_set_pwm(PWM1, value);
		break;
#endif
#if ASSERT_PIN(PWM2)
	case PWM2:
		mcu_set_pwm(PWM2, value);
		break;
#endif
#if ASSERT_PIN(PWM3)
	case PWM3:
		mcu_set_pwm(PWM3, value);
		break;
#endif
#if ASSERT_PIN(PWM4)
	case PWM4:
		mcu_set_pwm(PWM4, value);
		break;
#endif
#if ASSERT_PIN(PWM5)
	case PWM5:
		mcu_set_pwm(PWM5, value);
		break;
#endif
#if ASSERT_PIN(PWM6)
	case PWM6:
		mcu_set_pwm(PWM6, value);
		break;
#endif
#if ASSERT_PIN(PWM7)
	case PWM7:
		mcu_set_pwm(PWM7, value);
		break;
#endif
#if ASSERT_PIN(PWM8)
	case PWM8:
		mcu_set_pwm(PWM8, value);
		break;
#endif
#if ASSERT_PIN(PWM9)
	case PWM9:
		mcu_set_pwm(PWM9, value);
		break;
#endif
#if ASSERT_PIN(PWM10)
	case PWM10:
		mcu_set_pwm(PWM10, value);
		break;
#endif
#if ASSERT_PIN(PWM11)
	case PWM11:
		mcu_set_pwm(PWM11, value);
		break;
#endif
#if ASSERT_PIN(PWM12)
	case PWM12:
		mcu_set_pwm(PWM12, value);
		break;
#endif
#if ASSERT_PIN(PWM13)
	case PWM13:
		mcu_set_pwm(PWM13, value);
		break;
#endif
#if ASSERT_PIN(PWM14)
	case PWM14:
		mcu_set_pwm(PWM14, value);
		break;
#endif
#if ASSERT_PIN(PWM15)
	case PWM15:
		mcu_set_pwm(PWM15, value);
		break;
#endif
#if ASSERT_PIN(SERVO0)
	case SERVO0:
		mcu_set_servo(SERVO0, value);
		break;
#endif
#if ASSERT_PIN(SERVO1)
	case SERVO1:
		mcu_set_servo(SERVO1, value);
		break;
#endif
#if ASSERT_PIN(SERVO2)
	case SERVO2:
		mcu_set_servo(SERVO2, value);
		break;
#endif
#if ASSERT_PIN(SERVO3)
	case SERVO3:
		mcu_set_servo(SERVO3, value);
		break;
#endif
#if ASSERT_PIN(SERVO4)
	case SERVO4:
		mcu_set_servo(SERVO4, value);
		break;
#endif
#if ASSERT_PIN(SERVO5)
	case SERVO5:
		mcu_set_servo(SERVO5, value);
		break;
#endif
	}
}

void io_set_output(uint8_t pin, bool state)
{
	// #ifdef ENABLE_IO_MODULES
	// 	set_output_args_t output_arg = {.pin = pin, .state = state};
	// 	EVENT_INVOKE(set_output, &output_arg);
	// #endif
#ifdef IC74HC595_HAS_DOUTS
	if (((int8_t)pin) < 0)
	{
		pin = -(((int8_t)pin) + 1);
		ic74hc595_set_output(pin, state);
		return;
	}
#endif
	if (state)
	{
		switch (pin)
		{
#if ASSERT_PIN(DOUT0)
		case DOUT0:
			mcu_set_output(DOUT0);
			break;
#endif
#if ASSERT_PIN(DOUT1)
		case DOUT1:
			mcu_set_output(DOUT1);
			break;
#endif
#if ASSERT_PIN(DOUT2)
		case DOUT2:
			mcu_set_output(DOUT2);
			break;
#endif
#if ASSERT_PIN(DOUT3)
		case DOUT3:
			mcu_set_output(DOUT3);
			break;
#endif
#if ASSERT_PIN(DOUT4)
		case DOUT4:
			mcu_set_output(DOUT4);
			break;
#endif
#if ASSERT_PIN(DOUT5)
		case DOUT5:
			mcu_set_output(DOUT5);
			break;
#endif
#if ASSERT_PIN(DOUT6)
		case DOUT6:
			mcu_set_output(DOUT6);
			break;
#endif
#if ASSERT_PIN(DOUT7)
		case DOUT7:
			mcu_set_output(DOUT7);
			break;
#endif
#if ASSERT_PIN(DOUT8)
		case DOUT8:
			mcu_set_output(DOUT8);
			break;
#endif
#if ASSERT_PIN(DOUT9)
		case DOUT9:
			mcu_set_output(DOUT9);
			break;
#endif
#if ASSERT_PIN(DOUT10)
		case DOUT10:
			mcu_set_output(DOUT10);
			break;
#endif
#if ASSERT_PIN(DOUT11)
		case DOUT11:
			mcu_set_output(DOUT11);
			break;
#endif
#if ASSERT_PIN(DOUT12)
		case DOUT12:
			mcu_set_output(DOUT12);
			break;
#endif
#if ASSERT_PIN(DOUT13)
		case DOUT13:
			mcu_set_output(DOUT13);
			break;
#endif
#if ASSERT_PIN(DOUT14)
		case DOUT14:
			mcu_set_output(DOUT14);
			break;
#endif
#if ASSERT_PIN(DOUT15)
		case DOUT15:
			mcu_set_output(DOUT15);
			break;
#endif
#if ASSERT_PIN(DOUT16)
		case DOUT16:
			mcu_set_output(DOUT16);
			break;
#endif
#if ASSERT_PIN(DOUT17)
		case DOUT17:
			mcu_set_output(DOUT17);
			break;
#endif
#if ASSERT_PIN(DOUT18)
		case DOUT18:
			mcu_set_output(DOUT18);
			break;
#endif
#if ASSERT_PIN(DOUT19)
		case DOUT19:
			mcu_set_output(DOUT19);
			break;
#endif
#if ASSERT_PIN(DOUT20)
		case DOUT20:
			mcu_set_output(DOUT20);
			break;
#endif
#if ASSERT_PIN(DOUT21)
		case DOUT21:
			mcu_set_output(DOUT21);
			break;
#endif
#if ASSERT_PIN(DOUT22)
		case DOUT22:
			mcu_set_output(DOUT22);
			break;
#endif
#if ASSERT_PIN(DOUT23)
		case DOUT23:
			mcu_set_output(DOUT23);
			break;
#endif
#if ASSERT_PIN(DOUT24)
		case DOUT24:
			mcu_set_output(DOUT24);
			break;
#endif
#if ASSERT_PIN(DOUT25)
		case DOUT25:
			mcu_set_output(DOUT25);
			break;
#endif
#if ASSERT_PIN(DOUT26)
		case DOUT26:
			mcu_set_output(DOUT26);
			break;
#endif
#if ASSERT_PIN(DOUT27)
		case DOUT27:
			mcu_set_output(DOUT27);
			break;
#endif
#if ASSERT_PIN(DOUT28)
		case DOUT28:
			mcu_set_output(DOUT28);
			break;
#endif
#if ASSERT_PIN(DOUT29)
		case DOUT29:
			mcu_set_output(DOUT29);
			break;
#endif
#if ASSERT_PIN(DOUT30)
		case DOUT30:
			mcu_set_output(DOUT30);
			break;
#endif
#if ASSERT_PIN(DOUT31)
		case DOUT31:
			mcu_set_output(DOUT31);
			break;
#endif
		}
	}
	else
	{
		switch (pin)
		{
#if ASSERT_PIN(DOUT0)
		case DOUT0:
			mcu_clear_output(DOUT0);
			break;
#endif
#if ASSERT_PIN(DOUT1)
		case DOUT1:
			mcu_clear_output(DOUT1);
			break;
#endif
#if ASSERT_PIN(DOUT2)
		case DOUT2:
			mcu_clear_output(DOUT2);
			break;
#endif
#if ASSERT_PIN(DOUT3)
		case DOUT3:
			mcu_clear_output(DOUT3);
			break;
#endif
#if ASSERT_PIN(DOUT4)
		case DOUT4:
			mcu_clear_output(DOUT4);
			break;
#endif
#if ASSERT_PIN(DOUT5)
		case DOUT5:
			mcu_clear_output(DOUT5);
			break;
#endif
#if ASSERT_PIN(DOUT6)
		case DOUT6:
			mcu_clear_output(DOUT6);
			break;
#endif
#if ASSERT_PIN(DOUT7)
		case DOUT7:
			mcu_clear_output(DOUT7);
			break;
#endif
#if ASSERT_PIN(DOUT8)
		case DOUT8:
			mcu_clear_output(DOUT8);
			break;
#endif
#if ASSERT_PIN(DOUT9)
		case DOUT9:
			mcu_clear_output(DOUT9);
			break;
#endif
#if ASSERT_PIN(DOUT10)
		case DOUT10:
			mcu_clear_output(DOUT10);
			break;
#endif
#if ASSERT_PIN(DOUT11)
		case DOUT11:
			mcu_clear_output(DOUT11);
			break;
#endif
#if ASSERT_PIN(DOUT12)
		case DOUT12:
			mcu_clear_output(DOUT12);
			break;
#endif
#if ASSERT_PIN(DOUT13)
		case DOUT13:
			mcu_clear_output(DOUT13);
			break;
#endif
#if ASSERT_PIN(DOUT14)
		case DOUT14:
			mcu_clear_output(DOUT14);
			break;
#endif
#if ASSERT_PIN(DOUT15)
		case DOUT15:
			mcu_clear_output(DOUT15);
			break;
#endif
#if ASSERT_PIN(DOUT16)
		case DOUT16:
			mcu_clear_output(DOUT16);
			break;
#endif
#if ASSERT_PIN(DOUT17)
		case DOUT17:
			mcu_clear_output(DOUT17);
			break;
#endif
#if ASSERT_PIN(DOUT18)
		case DOUT18:
			mcu_clear_output(DOUT18);
			break;
#endif
#if ASSERT_PIN(DOUT19)
		case DOUT19:
			mcu_clear_output(DOUT19);
			break;
#endif
#if ASSERT_PIN(DOUT20)
		case DOUT20:
			mcu_clear_output(DOUT20);
			break;
#endif
#if ASSERT_PIN(DOUT21)
		case DOUT21:
			mcu_clear_output(DOUT21);
			break;
#endif
#if ASSERT_PIN(DOUT22)
		case DOUT22:
			mcu_clear_output(DOUT22);
			break;
#endif
#if ASSERT_PIN(DOUT23)
		case DOUT23:
			mcu_clear_output(DOUT23);
			break;
#endif
#if ASSERT_PIN(DOUT24)
		case DOUT24:
			mcu_clear_output(DOUT24);
			break;
#endif
#if ASSERT_PIN(DOUT25)
		case DOUT25:
			mcu_clear_output(DOUT25);
			break;
#endif
#if ASSERT_PIN(DOUT26)
		case DOUT26:
			mcu_clear_output(DOUT26);
			break;
#endif
#if ASSERT_PIN(DOUT27)
		case DOUT27:
			mcu_clear_output(DOUT27);
			break;
#endif
#if ASSERT_PIN(DOUT28)
		case DOUT28:
			mcu_clear_output(DOUT28);
			break;
#endif
#if ASSERT_PIN(DOUT29)
		case DOUT29:
			mcu_clear_output(DOUT29);
			break;
#endif
#if ASSERT_PIN(DOUT30)
		case DOUT30:
			mcu_clear_output(DOUT30);
			break;
#endif
#if ASSERT_PIN(DOUT31)
		case DOUT31:
			mcu_clear_output(DOUT31);
			break;
#endif
		}
	}
}

void io_enable_steppers(uint8_t mask)
{
	// #ifdef ENABLE_IO_MODULES
	// 	EVENT_INVOKE(enable_steppers, &mask);
	// #endif

#ifdef IC74HC595_HAS_STEPS_EN
	ic74hc595_enable_steppers(mask);
#endif

#if ASSERT_PIN(STEP0_EN)
	if (mask & 0x01)
	{
		mcu_set_output(STEP0_EN);
	}
	else
	{
		mcu_clear_output(STEP0_EN);
	}
#endif
#if ASSERT_PIN(STEP1_EN)
	if (mask & 0x02)
	{
		mcu_set_output(STEP1_EN);
	}
	else
	{
		mcu_clear_output(STEP1_EN);
	}
#endif
#if ASSERT_PIN(STEP2_EN)
	if (mask & 0x04)
	{
		mcu_set_output(STEP2_EN);
	}
	else
	{
		mcu_clear_output(STEP2_EN);
	}
#endif
#if ASSERT_PIN(STEP3_EN)
	if (mask & 0x08)
	{
		mcu_set_output(STEP3_EN);
	}
	else
	{
		mcu_clear_output(STEP3_EN);
	}
#endif
#if ASSERT_PIN(STEP4_EN)
	if (mask & 0x10)
	{
		mcu_set_output(STEP4_EN);
	}
	else
	{
		mcu_clear_output(STEP4_EN);
	}
#endif
#if ASSERT_PIN(STEP5_EN)
	if (mask & 0x20)
	{
		mcu_set_output(STEP5_EN);
	}
	else
	{
		mcu_clear_output(STEP5_EN);
	}
#endif
#if ASSERT_PIN(STEP6_EN)
	if (mask & 0x40)
	{
		mcu_set_output(STEP6_EN);
	}
	else
	{
		mcu_clear_output(STEP6_EN);
	}
#endif
#if ASSERT_PIN(STEP7_EN)
	if (mask & 0x80)
	{
		mcu_set_output(STEP7_EN);
	}
	else
	{
		mcu_clear_output(STEP7_EN);
	}
#endif
}

uint8_t io_get_analog(uint8_t pin)
{
	switch (pin)
	{
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
	}

	return 0;
}

int16_t io_get_pinvalue(uint8_t pin)
{
	switch (pin)
	{
#if ASSERT_PIN(STEP0)
	case STEP0:
		return (mcu_get_output(STEP0) != 0);
#endif
#if ASSERT_PIN(STEP1)
	case STEP1:
		return (mcu_get_output(STEP1) != 0);
#endif
#if ASSERT_PIN(STEP2)
	case STEP2:
		return (mcu_get_output(STEP2) != 0);
#endif
#if ASSERT_PIN(STEP3)
	case STEP3:
		return (mcu_get_output(STEP3) != 0);
#endif
#if ASSERT_PIN(STEP4)
	case STEP4:
		return (mcu_get_output(STEP4) != 0);
#endif
#if ASSERT_PIN(STEP5)
	case STEP5:
		return (mcu_get_output(STEP5) != 0);
#endif
#if ASSERT_PIN(STEP6)
	case STEP6:
		return (mcu_get_output(STEP6) != 0);
#endif
#if ASSERT_PIN(STEP7)
	case STEP7:
		return (mcu_get_output(STEP7) != 0);
#endif
#if ASSERT_PIN(DIR0)
	case DIR0:
		return (mcu_get_output(DIR0) != 0);
#endif
#if ASSERT_PIN(DIR1)
	case DIR1:
		return (mcu_get_output(DIR1) != 0);
#endif
#if ASSERT_PIN(DIR2)
	case DIR2:
		return (mcu_get_output(DIR2) != 0);
#endif
#if ASSERT_PIN(DIR3)
	case DIR3:
		return (mcu_get_output(DIR3) != 0);
#endif
#if ASSERT_PIN(DIR4)
	case DIR4:
		return (mcu_get_output(DIR4) != 0);
#endif
#if ASSERT_PIN(DIR5)
	case DIR5:
		return (mcu_get_output(DIR5) != 0);
#endif
#if ASSERT_PIN(DIR6)
	case DIR6:
		return (mcu_get_output(DIR6) != 0);
#endif
#if ASSERT_PIN(DIR7)
	case DIR7:
		return (mcu_get_output(DIR7) != 0);
#endif
#if ASSERT_PIN(STEP0_EN)
	case STEP0_EN:
		return (mcu_get_output(STEP0_EN) != 0);
#endif
#if ASSERT_PIN(STEP1_EN)
	case STEP1_EN:
		return (mcu_get_output(STEP1_EN) != 0);
#endif
#if ASSERT_PIN(STEP2_EN)
	case STEP2_EN:
		return (mcu_get_output(STEP2_EN) != 0);
#endif
#if ASSERT_PIN(STEP3_EN)
	case STEP3_EN:
		return (mcu_get_output(STEP3_EN) != 0);
#endif
#if ASSERT_PIN(STEP4_EN)
	case STEP4_EN:
		return (mcu_get_output(STEP4_EN) != 0);
#endif
#if ASSERT_PIN(STEP5_EN)
	case STEP5_EN:
		return (mcu_get_output(STEP5_EN) != 0);
#endif
#if ASSERT_PIN(STEP6_EN)
	case STEP6_EN:
		return (mcu_get_output(STEP6_EN) != 0);
#endif
#if ASSERT_PIN(STEP7_EN)
	case STEP7_EN:
		return (mcu_get_output(STEP7_EN) != 0);
#endif
#if ASSERT_PIN(PWM0)
	case PWM0:
		return mcu_get_pwm(PWM0);
#endif
#if ASSERT_PIN(PWM1)
	case PWM1:
		return mcu_get_pwm(PWM1);
#endif
#if ASSERT_PIN(PWM2)
	case PWM2:
		return mcu_get_pwm(PWM2);
#endif
#if ASSERT_PIN(PWM3)
	case PWM3:
		return mcu_get_pwm(PWM3);
#endif
#if ASSERT_PIN(PWM4)
	case PWM4:
		return mcu_get_pwm(PWM4);
#endif
#if ASSERT_PIN(PWM5)
	case PWM5:
		return mcu_get_pwm(PWM5);
#endif
#if ASSERT_PIN(PWM6)
	case PWM6:
		return mcu_get_pwm(PWM6);
#endif
#if ASSERT_PIN(PWM7)
	case PWM7:
		return mcu_get_pwm(PWM7);
#endif
#if ASSERT_PIN(PWM8)
	case PWM8:
		return mcu_get_pwm(PWM8);
#endif
#if ASSERT_PIN(PWM9)
	case PWM9:
		return mcu_get_pwm(PWM9);
#endif
#if ASSERT_PIN(PWM10)
	case PWM10:
		return mcu_get_pwm(PWM10);
#endif
#if ASSERT_PIN(PWM11)
	case PWM11:
		return mcu_get_pwm(PWM11);
#endif
#if ASSERT_PIN(PWM12)
	case PWM12:
		return mcu_get_pwm(PWM12);
#endif
#if ASSERT_PIN(PWM13)
	case PWM13:
		return mcu_get_pwm(PWM13);
#endif
#if ASSERT_PIN(PWM14)
	case PWM14:
		return mcu_get_pwm(PWM14);
#endif
#if ASSERT_PIN(PWM15)
	case PWM15:
		return mcu_get_pwm(PWM15);
#endif
#if ASSERT_PIN(DOUT0)
	case DOUT0:
		return (mcu_get_output(DOUT0) != 0);
#endif
#if ASSERT_PIN(DOUT1)
	case DOUT1:
		return (mcu_get_output(DOUT1) != 0);
#endif
#if ASSERT_PIN(DOUT2)
	case DOUT2:
		return (mcu_get_output(DOUT2) != 0);
#endif
#if ASSERT_PIN(DOUT3)
	case DOUT3:
		return (mcu_get_output(DOUT3) != 0);
#endif
#if ASSERT_PIN(DOUT4)
	case DOUT4:
		return (mcu_get_output(DOUT4) != 0);
#endif
#if ASSERT_PIN(DOUT5)
	case DOUT5:
		return (mcu_get_output(DOUT5) != 0);
#endif
#if ASSERT_PIN(DOUT6)
	case DOUT6:
		return (mcu_get_output(DOUT6) != 0);
#endif
#if ASSERT_PIN(DOUT7)
	case DOUT7:
		return (mcu_get_output(DOUT7) != 0);
#endif
#if ASSERT_PIN(DOUT8)
	case DOUT8:
		return (mcu_get_output(DOUT8) != 0);
#endif
#if ASSERT_PIN(DOUT9)
	case DOUT9:
		return (mcu_get_output(DOUT9) != 0);
#endif
#if ASSERT_PIN(DOUT10)
	case DOUT10:
		return (mcu_get_output(DOUT10) != 0);
#endif
#if ASSERT_PIN(DOUT11)
	case DOUT11:
		return (mcu_get_output(DOUT11) != 0);
#endif
#if ASSERT_PIN(DOUT12)
	case DOUT12:
		return (mcu_get_output(DOUT12) != 0);
#endif
#if ASSERT_PIN(DOUT13)
	case DOUT13:
		return (mcu_get_output(DOUT13) != 0);
#endif
#if ASSERT_PIN(DOUT14)
	case DOUT14:
		return (mcu_get_output(DOUT14) != 0);
#endif
#if ASSERT_PIN(DOUT15)
	case DOUT15:
		return (mcu_get_output(DOUT15) != 0);
#endif
#if ASSERT_PIN(DOUT16)
	case DOUT16:
		return (mcu_get_output(DOUT16) != 0);
#endif
#if ASSERT_PIN(DOUT17)
	case DOUT17:
		return (mcu_get_output(DOUT17) != 0);
#endif
#if ASSERT_PIN(DOUT18)
	case DOUT18:
		return (mcu_get_output(DOUT18) != 0);
#endif
#if ASSERT_PIN(DOUT19)
	case DOUT19:
		return (mcu_get_output(DOUT19) != 0);
#endif
#if ASSERT_PIN(DOUT20)
	case DOUT20:
		return (mcu_get_output(DOUT20) != 0);
#endif
#if ASSERT_PIN(DOUT21)
	case DOUT21:
		return (mcu_get_output(DOUT21) != 0);
#endif
#if ASSERT_PIN(DOUT22)
	case DOUT22:
		return (mcu_get_output(DOUT22) != 0);
#endif
#if ASSERT_PIN(DOUT23)
	case DOUT23:
		return (mcu_get_output(DOUT23) != 0);
#endif
#if ASSERT_PIN(DOUT24)
	case DOUT24:
		return (mcu_get_output(DOUT24) != 0);
#endif
#if ASSERT_PIN(DOUT25)
	case DOUT25:
		return (mcu_get_output(DOUT25) != 0);
#endif
#if ASSERT_PIN(DOUT26)
	case DOUT26:
		return (mcu_get_output(DOUT26) != 0);
#endif
#if ASSERT_PIN(DOUT27)
	case DOUT27:
		return (mcu_get_output(DOUT27) != 0);
#endif
#if ASSERT_PIN(DOUT28)
	case DOUT28:
		return (mcu_get_output(DOUT28) != 0);
#endif
#if ASSERT_PIN(DOUT29)
	case DOUT29:
		return (mcu_get_output(DOUT29) != 0);
#endif
#if ASSERT_PIN(DOUT30)
	case DOUT30:
		return (mcu_get_output(DOUT30) != 0);
#endif
#if ASSERT_PIN(DOUT31)
	case DOUT31:
		return (mcu_get_output(DOUT31) != 0);
#endif
#if ASSERT_PIN(LIMIT_X)
	case LIMIT_X:
		return (mcu_get_input(LIMIT_X) != 0);
#endif
#if ASSERT_PIN(LIMIT_Y)
	case LIMIT_Y:
		return (mcu_get_input(LIMIT_Y) != 0);
#endif
#if ASSERT_PIN(LIMIT_Z)
	case LIMIT_Z:
		return (mcu_get_input(LIMIT_Z) != 0);
#endif
#if ASSERT_PIN(LIMIT_X2)
	case LIMIT_X2:
		return (mcu_get_input(LIMIT_X2) != 0);
#endif
#if ASSERT_PIN(LIMIT_Y2)
	case LIMIT_Y2:
		return (mcu_get_input(LIMIT_Y2) != 0);
#endif
#if ASSERT_PIN(LIMIT_Z2)
	case LIMIT_Z2:
		return (mcu_get_input(LIMIT_Z2) != 0);
#endif
#if ASSERT_PIN(LIMIT_A)
	case LIMIT_A:
		return (mcu_get_input(LIMIT_A) != 0);
#endif
#if ASSERT_PIN(LIMIT_B)
	case LIMIT_B:
		return (mcu_get_input(LIMIT_B) != 0);
#endif
#if ASSERT_PIN(LIMIT_C)
	case LIMIT_C:
		return (mcu_get_input(LIMIT_C) != 0);
#endif
#if ASSERT_PIN(PROBE)
	case PROBE:
		return (mcu_get_input(PROBE) != 0);
#endif
#if ASSERT_PIN(ESTOP)
	case ESTOP:
#ifndef INVERT_EMERGENCY_STOP
		return (mcu_get_input(ESTOP) != 0);
#else
		return (mcu_get_input(ESTOP) == 0);
#endif
#endif
#if ASSERT_PIN(SAFETY_DOOR)
	case SAFETY_DOOR:
		return (mcu_get_input(SAFETY_DOOR) != 0);
#endif
#if ASSERT_PIN(FHOLD)
	case FHOLD:
		return (mcu_get_input(FHOLD) != 0);
#endif
#if ASSERT_PIN(CS_RES)
	case CS_RES:
		return (mcu_get_input(CS_RES) != 0);
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
		return (mcu_get_input(DIN0) != 0);
#endif
#if ASSERT_PIN(DIN1)
	case DIN1:
		return (mcu_get_input(DIN1) != 0);
#endif
#if ASSERT_PIN(DIN2)
	case DIN2:
		return (mcu_get_input(DIN2) != 0);
#endif
#if ASSERT_PIN(DIN3)
	case DIN3:
		return (mcu_get_input(DIN3) != 0);
#endif
#if ASSERT_PIN(DIN4)
	case DIN4:
		return (mcu_get_input(DIN4) != 0);
#endif
#if ASSERT_PIN(DIN5)
	case DIN5:
		return (mcu_get_input(DIN5) != 0);
#endif
#if ASSERT_PIN(DIN6)
	case DIN6:
		return (mcu_get_input(DIN6) != 0);
#endif
#if ASSERT_PIN(DIN7)
	case DIN7:
		return (mcu_get_input(DIN7) != 0);
#endif
#if ASSERT_PIN(DIN8)
	case DIN8:
		return (mcu_get_input(DIN8) != 0);
#endif
#if ASSERT_PIN(DIN9)
	case DIN9:
		return (mcu_get_input(DIN9) != 0);
#endif
#if ASSERT_PIN(DIN10)
	case DIN10:
		return (mcu_get_input(DIN10) != 0);
#endif
#if ASSERT_PIN(DIN11)
	case DIN11:
		return (mcu_get_input(DIN11) != 0);
#endif
#if ASSERT_PIN(DIN12)
	case DIN12:
		return (mcu_get_input(DIN12) != 0);
#endif
#if ASSERT_PIN(DIN13)
	case DIN13:
		return (mcu_get_input(DIN13) != 0);
#endif
#if ASSERT_PIN(DIN14)
	case DIN14:
		return (mcu_get_input(DIN14) != 0);
#endif
#if ASSERT_PIN(DIN15)
	case DIN15:
		return (mcu_get_input(DIN15) != 0);
#endif
#if ASSERT_PIN(DIN16)
	case DIN16:
		return (mcu_get_input(DIN16) != 0);
#endif
#if ASSERT_PIN(DIN17)
	case DIN17:
		return (mcu_get_input(DIN17) != 0);
#endif
#if ASSERT_PIN(DIN18)
	case DIN18:
		return (mcu_get_input(DIN18) != 0);
#endif
#if ASSERT_PIN(DIN19)
	case DIN19:
		return (mcu_get_input(DIN19) != 0);
#endif
#if ASSERT_PIN(DIN20)
	case DIN20:
		return (mcu_get_input(DIN20) != 0);
#endif
#if ASSERT_PIN(DIN21)
	case DIN21:
		return (mcu_get_input(DIN21) != 0);
#endif
#if ASSERT_PIN(DIN22)
	case DIN22:
		return (mcu_get_input(DIN22) != 0);
#endif
#if ASSERT_PIN(DIN23)
	case DIN23:
		return (mcu_get_input(DIN23) != 0);
#endif
#if ASSERT_PIN(DIN24)
	case DIN24:
		return (mcu_get_input(DIN24) != 0);
#endif
#if ASSERT_PIN(DIN25)
	case DIN25:
		return (mcu_get_input(DIN25) != 0);
#endif
#if ASSERT_PIN(DIN26)
	case DIN26:
		return (mcu_get_input(DIN26) != 0);
#endif
#if ASSERT_PIN(DIN27)
	case DIN27:
		return (mcu_get_input(DIN27) != 0);
#endif
#if ASSERT_PIN(DIN28)
	case DIN28:
		return (mcu_get_input(DIN28) != 0);
#endif
#if ASSERT_PIN(DIN29)
	case DIN29:
		return (mcu_get_input(DIN29) != 0);
#endif
#if ASSERT_PIN(DIN30)
	case DIN30:
		return (mcu_get_input(DIN30) != 0);
#endif
#if ASSERT_PIN(DIN31)
	case DIN31:
		return (mcu_get_input(DIN31) != 0);
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
