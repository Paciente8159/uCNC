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

MCU_IO_CALLBACK void mcu_limits_changed_cb(void)
{
#ifndef DISABLE_ALL_LIMITS

	if (g_settings.hard_limits_enabled)
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
#if (defined(DUAL_DRIVE0_ENABLE_SELFSQUARING) || defined(DUAL_DRIVE1_ENABLE_SELFSQUARING) || (KINEMATIC == KINEMATIC_DELTA))
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
#if (KINEMATIC == KINEMATIC_DELTA)
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

#if (defined(DUAL_DRIVE0_ENABLE_SELFSQUARING) || defined(DUAL_DRIVE1_ENABLE_SELFSQUARING) || (KINEMATIC == KINEMATIC_DELTA))
			itp_lock_stepper(0); // unlocks axis
#endif
			itp_stop();
			cnc_set_exec_state(EXEC_HALT);
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

#if !(ESTOP < 0)
	if (CHECKFLAG(controls, ESTOP_MASK))
	{
		cnc_set_exec_state(EXEC_KILL);
		cnc_stop();
		return; // forces exit
	}
#endif
#if !(SAFETY_DOOR < 0)
	if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
	{
		// safety door activates hold simultaneously to start the controlled stop
		cnc_set_exec_state(EXEC_DOOR | EXEC_HOLD);
	}
#endif
#if !(FHOLD < 0)
	if (CHECKFLAG(controls, FHOLD_MASK))
	{
		cnc_set_exec_state(EXEC_HOLD);
	}
#endif
#if !(CS_RES < 0)
	if (CHECKFLAG(controls, CS_RES_MASK))
	{
		cnc_call_rt_command(CMD_CODE_CYCLE_START);
	}
#endif
#endif
}

MCU_IO_CALLBACK void mcu_probe_changed_cb(void)
{
#ifdef DISABLE_PROBE
	return;
#endif

#if !(PROBE < 0)
	static bool prev_probe = false;
	bool probe = io_get_probe();

	if (prev_probe == probe)
	{
		return;
	}

	prev_probe = probe;

	// on hit enables hold (directly)
	cnc_set_exec_state(EXEC_HOLD);
	// stores rt position
	parser_sync_probe();
#endif
}

// overridable
// for now if encoders are enabled this will be override by the encoder call
MCU_IO_CALLBACK void __attribute__((weak)) mcu_inputs_changed_cb(void)
{
#ifdef ENABLE_IO_MODULES
	EVENT_INVOKE(input_change, NULL);
#endif
}

void io_lock_limits(uint8_t limitmask)
{
	io_lock_limits_mask = limitmask;
}

void io_invert_limits(uint8_t limitmask)
{
	io_invert_limits_mask = limitmask;
}

uint8_t io_get_limits(void)
{
#ifdef DISABLE_ALL_LIMITS
	return 0;
#endif
	uint8_t value = 0;

#if !(LIMIT_X < 0)
	value |= ((mcu_get_input(LIMIT_X)) ? LIMIT_X_MASK : 0);
#endif
#if !(LIMIT_Y < 0)
	value |= ((mcu_get_input(LIMIT_Y)) ? LIMIT_Y_MASK : 0);
#endif
#if !(LIMIT_Z < 0)
	value |= ((mcu_get_input(LIMIT_Z)) ? LIMIT_Z_MASK : 0);
#endif
#if !(LIMIT_A < 0)
	value |= ((mcu_get_input(LIMIT_A)) ? LIMIT_A_MASK : 0);
#endif
#if !(LIMIT_B < 0)
	value |= ((mcu_get_input(LIMIT_B)) ? LIMIT_B_MASK : 0);
#endif
#if !(LIMIT_C < 0)
	value |= ((mcu_get_input(LIMIT_C)) ? LIMIT_C_MASK : 0);
#endif

	uint8_t inv = g_settings.limits_invert_mask;
	uint8_t result = (value ^ (inv & LIMITS_INV_MASK));

#if LIMITS_DUAL_INV_MASK != 0
	uint8_t value2 = 0;

#if !(LIMIT_X2 < 0)
#if !(LIMITS_DUAL_MASK & LIMIT_X_MASK)
	value2 |= ((mcu_get_input(LIMIT_X2)) ? LIMIT_X_MASK : 0);
#endif
#endif
#if !(LIMIT_Y2 < 0)
#if !(LIMITS_DUAL_MASK & LIMIT_Y_MASK)
	value2 |= ((mcu_get_input(LIMIT_Y2)) ? LIMIT_Y_MASK : 0);
#endif
#endif
#if !(LIMIT_Z2 < 0)
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

	return result;
}

uint8_t io_get_limits_dual(void)
{
#if (defined(DISABLE_ALL_LIMITS) || (LIMITS_DUAL_MASK == 0))
	return 0;
#else
	uint8_t value = 0;
#if !(LIMIT_X2 < 0)
#if (LIMITS_DUAL_MASK & LIMIT_X_MASK)
	value |= ((mcu_get_input(LIMIT_X2)) ? LIMIT_X_MASK : 0);
#endif
#endif
#if !(LIMIT_Y2 < 0)
#if (LIMITS_DUAL_MASK & LIMIT_Y_MASK)
	value |= ((mcu_get_input(LIMIT_Y2)) ? LIMIT_Y_MASK : 0);
#endif
#endif
#if !(LIMIT_Z2 < 0)
#if (LIMITS_DUAL_MASK & LIMIT_Z_MASK)
	value |= ((mcu_get_input(LIMIT_Z2)) ? LIMIT_Z_MASK : 0);
#endif
#endif
	uint8_t inv = io_invert_limits_mask & LIMITS_DUAL_MASK;
	return (value ^ (g_settings.limits_invert_mask & LIMITS_DUAL_MASK & LIMITS_DUAL_INV_MASK) ^ inv);
#endif
}

uint8_t io_get_controls(void)
{
#ifdef DISABLE_ALL_CONTROLS
	return 0;
#endif
	uint8_t value = 0;
#if !(ESTOP < 0)
#ifndef INVERT_EMERGENCY_STOP
	value |= ((mcu_get_input(ESTOP)) ? ESTOP_MASK : 0);
#else
	value |= ((!mcu_get_input(ESTOP)) ? ESTOP_MASK : 0);
#endif
#endif
#if !(SAFETY_DOOR < 0)
	value |= ((mcu_get_input(SAFETY_DOOR)) ? SAFETY_DOOR_MASK : 0);
#endif
#if !(FHOLD < 0)
	value |= ((mcu_get_input(FHOLD)) ? FHOLD_MASK : 0);
#endif
#if !(CS_RES < 0)
	value |= ((mcu_get_input(CS_RES)) ? CS_RES_MASK : 0);
#endif

	return (value ^ (g_settings.control_invert_mask & CONTROLS_INV_MASK));
}

void io_enable_probe(void)
{
#ifdef ENABLE_MOTION_MODULES
	EVENT_INVOKE(probe_enable, NULL);
#endif
#ifndef FORCE_SOFT_POLLING
#if !(PROBE < 0)
	mcu_enable_probe_isr();
#endif
#endif
}

void io_disable_probe(void)
{
#ifndef FORCE_SOFT_POLLING
#if !(PROBE < 0)
	mcu_disable_probe_isr();
#endif
#endif
#ifdef ENABLE_MOTION_MODULES
	EVENT_INVOKE(probe_disable, NULL);
#endif
}

bool io_get_probe(void)
{
#ifdef DISABLE_PROBE
	return false;
#endif
#if !(PROBE < 0)
	bool probe = (mcu_get_input(PROBE) != 0);
	return (!g_settings.probe_invert_mask) ? probe : !probe;
#else
	return false;
#endif
}

// outputs
void io_set_steps(uint8_t mask)
{
#if !(STEP0 < 0)
	if (mask & STEP0_MASK)
	{
		mcu_set_output(STEP0);
	}
	else
	{
		mcu_clear_output(STEP0);
	}

#endif
#if !(STEP1 < 0)
	if (mask & STEP1_MASK)
	{
		mcu_set_output(STEP1);
	}
	else
	{
		mcu_clear_output(STEP1);
	}
#endif
#if !(STEP2 < 0)
	if (mask & STEP2_MASK)
	{
		mcu_set_output(STEP2);
	}
	else
	{
		mcu_clear_output(STEP2);
	}
#endif
#if !(STEP3 < 0)
	if (mask & STEP3_MASK)
	{
		mcu_set_output(STEP3);
	}
	else
	{
		mcu_clear_output(STEP3);
	}
#endif
#if !(STEP4 < 0)
	if (mask & STEP4_MASK)
	{
		mcu_set_output(STEP4);
	}
	else
	{
		mcu_clear_output(STEP4);
	}
#endif
#if !(STEP5 < 0)
	if (mask & STEP5_MASK)
	{
		mcu_set_output(STEP5);
	}
	else
	{
		mcu_clear_output(STEP5);
	}
#endif
#if !(STEP6 < 0)
	if (mask & STEP6_MASK)
	{
		mcu_set_output(STEP6);
	}
	else
	{
		mcu_clear_output(STEP6);
	}
#endif
#if !(STEP7 < 0)
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
#if !(STEP0 < 0)
	if (mask & STEP0_MASK)
	{
		mcu_toggle_output(STEP0);
	}
#endif
#if !(STEP1 < 0)
	if (mask & STEP1_MASK)
	{
		mcu_toggle_output(STEP1);
	}
#endif
#if !(STEP2 < 0)
	if (mask & STEP2_MASK)
	{
		mcu_toggle_output(STEP2);
	}
#endif
#if !(STEP3 < 0)
	if (mask & STEP3_MASK)
	{
		mcu_toggle_output(STEP3);
	}
#endif
#if !(STEP4 < 0)
	if (mask & STEP4_MASK)
	{
		mcu_toggle_output(STEP4);
	}
#endif
#if !(STEP5 < 0)
	if (mask & STEP5_MASK)
	{
		mcu_toggle_output(STEP5);
	}
#endif
#if !(STEP6 < 0)
	if (mask & STEP6_MASK)
	{
		mcu_toggle_output(STEP6);
	}
#endif
#if !(STEP7 < 0)
	if (mask & STEP7_MASK)
	{
		mcu_toggle_output(STEP7);
	}
#endif
}

void io_set_dirs(uint8_t mask)
{
	mask ^= g_settings.dir_invert_mask;
#if !(DIR0 < 0)
	if (mask & DIR0_MASK)
	{
		mcu_set_output(DIR0);
	}
	else
	{
		mcu_clear_output(DIR0);
	}
#endif
#if !(DIR1 < 0)
	if (mask & DIR1_MASK)
	{
		mcu_set_output(DIR1);
	}
	else
	{
		mcu_clear_output(DIR1);
	}
#endif
#if !(DIR2 < 0)
	if (mask & DIR2_MASK)
	{
		mcu_set_output(DIR2);
	}
	else
	{
		mcu_clear_output(DIR2);
	}
#endif
#if !(DIR3 < 0)
	if (mask & DIR3_MASK)
	{
		mcu_set_output(DIR3);
	}
	else
	{
		mcu_clear_output(DIR3);
	}
#endif
#if !(DIR4 < 0)
	if (mask & DIR4_MASK)
	{
		mcu_set_output(DIR4);
	}
	else
	{
		mcu_clear_output(DIR4);
	}
#endif
#if !(DIR5 < 0)
	if (mask & DIR5_MASK)
	{
		mcu_set_output(DIR5);
	}
	else
	{
		mcu_clear_output(DIR5);
	}
#endif
#if !(DIR6 < 0)
	if (mask & DIR6_MASK)
	{
		mcu_set_output(DIR6);
	}
	else
	{
		mcu_clear_output(DIR6);
	}
#endif
#if !(DIR7 < 0)
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
	switch (pin)
	{
#if !(PWM0 < 0)
	case PWM0:
		mcu_set_pwm(PWM0, value);
		break;
#endif
#if !(PWM1 < 0)
	case PWM1:
		mcu_set_pwm(PWM1, value);
		break;
#endif
#if !(PWM2 < 0)
	case PWM2:
		mcu_set_pwm(PWM2, value);
		break;
#endif
#if !(PWM3 < 0)
	case PWM3:
		mcu_set_pwm(PWM3, value);
		break;
#endif
#if !(PWM4 < 0)
	case PWM4:
		mcu_set_pwm(PWM4, value);
		break;
#endif
#if !(PWM5 < 0)
	case PWM5:
		mcu_set_pwm(PWM5, value);
		break;
#endif
#if !(PWM6 < 0)
	case PWM6:
		mcu_set_pwm(PWM6, value);
		break;
#endif
#if !(PWM7 < 0)
	case PWM7:
		mcu_set_pwm(PWM7, value);
		break;
#endif
#if !(PWM8 < 0)
	case PWM8:
		mcu_set_pwm(PWM8, value);
		break;
#endif
#if !(PWM9 < 0)
	case PWM9:
		mcu_set_pwm(PWM9, value);
		break;
#endif
#if !(PWM10 < 0)
	case PWM10:
		mcu_set_pwm(PWM10, value);
		break;
#endif
#if !(PWM11 < 0)
	case PWM11:
		mcu_set_pwm(PWM11, value);
		break;
#endif
#if !(PWM12 < 0)
	case PWM12:
		mcu_set_pwm(PWM12, value);
		break;
#endif
#if !(PWM13 < 0)
	case PWM13:
		mcu_set_pwm(PWM13, value);
		break;
#endif
#if !(PWM14 < 0)
	case PWM14:
		mcu_set_pwm(PWM14, value);
		break;
#endif
#if !(PWM15 < 0)
	case PWM15:
		mcu_set_pwm(PWM15, value);
		break;
#endif
#if !(SERVO0 < 0)
	case SERVO0:
		mcu_set_servo(SERVO0, value);
		break;
#endif
#if !(SERVO1 < 0)
	case SERVO1:
		mcu_set_servo(SERVO1, value);
		break;
#endif
#if !(SERVO2 < 0)
	case SERVO2:
		mcu_set_servo(SERVO2, value);
		break;
#endif
#if !(SERVO3 < 0)
	case SERVO3:
		mcu_set_servo(SERVO3, value);
		break;
#endif
#if !(SERVO4 < 0)
	case SERVO4:
		mcu_set_servo(SERVO4, value);
		break;
#endif
#if !(SERVO5 < 0)
	case SERVO5:
		mcu_set_servo(SERVO5, value);
		break;
#endif
	}
}

void io_set_output(uint8_t pin, bool state)
{
	if (state)
	{
		switch (pin)
		{
#if !(DOUT0 < 0)
		case DOUT0:
			mcu_set_output(DOUT0);
			break;
#endif
#if !(DOUT1 < 0)
		case DOUT1:
			mcu_set_output(DOUT1);
			break;
#endif
#if !(DOUT2 < 0)
		case DOUT2:
			mcu_set_output(DOUT2);
			break;
#endif
#if !(DOUT3 < 0)
		case DOUT3:
			mcu_set_output(DOUT3);
			break;
#endif
#if !(DOUT4 < 0)
		case DOUT4:
			mcu_set_output(DOUT4);
			break;
#endif
#if !(DOUT5 < 0)
		case DOUT5:
			mcu_set_output(DOUT5);
			break;
#endif
#if !(DOUT6 < 0)
		case DOUT6:
			mcu_set_output(DOUT6);
			break;
#endif
#if !(DOUT7 < 0)
		case DOUT7:
			mcu_set_output(DOUT7);
			break;
#endif
#if !(DOUT8 < 0)
		case DOUT8:
			mcu_set_output(DOUT8);
			break;
#endif
#if !(DOUT9 < 0)
		case DOUT9:
			mcu_set_output(DOUT9);
			break;
#endif
#if !(DOUT10 < 0)
		case DOUT10:
			mcu_set_output(DOUT10);
			break;
#endif
#if !(DOUT11 < 0)
		case DOUT11:
			mcu_set_output(DOUT11);
			break;
#endif
#if !(DOUT12 < 0)
		case DOUT12:
			mcu_set_output(DOUT12);
			break;
#endif
#if !(DOUT13 < 0)
		case DOUT13:
			mcu_set_output(DOUT13);
			break;
#endif
#if !(DOUT14 < 0)
		case DOUT14:
			mcu_set_output(DOUT14);
			break;
#endif
#if !(DOUT15 < 0)
		case DOUT15:
			mcu_set_output(DOUT15);
			break;
#endif
#if !(DOUT16 < 0)
		case DOUT16:
			mcu_set_output(DOUT16);
			break;
#endif
#if !(DOUT17 < 0)
		case DOUT17:
			mcu_set_output(DOUT17);
			break;
#endif
#if !(DOUT18 < 0)
		case DOUT18:
			mcu_set_output(DOUT18);
			break;
#endif
#if !(DOUT19 < 0)
		case DOUT19:
			mcu_set_output(DOUT19);
			break;
#endif
#if !(DOUT20 < 0)
		case DOUT20:
			mcu_set_output(DOUT20);
			break;
#endif
#if !(DOUT21 < 0)
		case DOUT21:
			mcu_set_output(DOUT21);
			break;
#endif
#if !(DOUT22 < 0)
		case DOUT22:
			mcu_set_output(DOUT22);
			break;
#endif
#if !(DOUT23 < 0)
		case DOUT23:
			mcu_set_output(DOUT23);
			break;
#endif
#if !(DOUT24 < 0)
		case DOUT24:
			mcu_set_output(DOUT24);
			break;
#endif
#if !(DOUT25 < 0)
		case DOUT25:
			mcu_set_output(DOUT25);
			break;
#endif
#if !(DOUT26 < 0)
		case DOUT26:
			mcu_set_output(DOUT26);
			break;
#endif
#if !(DOUT27 < 0)
		case DOUT27:
			mcu_set_output(DOUT27);
			break;
#endif
#if !(DOUT28 < 0)
		case DOUT28:
			mcu_set_output(DOUT28);
			break;
#endif
#if !(DOUT29 < 0)
		case DOUT29:
			mcu_set_output(DOUT29);
			break;
#endif
#if !(DOUT30 < 0)
		case DOUT30:
			mcu_set_output(DOUT30);
			break;
#endif
#if !(DOUT31 < 0)
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
#if !(DOUT0 < 0)
		case DOUT0:
			mcu_clear_output(DOUT0);
			break;
#endif
#if !(DOUT1 < 0)
		case DOUT1:
			mcu_clear_output(DOUT1);
			break;
#endif
#if !(DOUT2 < 0)
		case DOUT2:
			mcu_clear_output(DOUT2);
			break;
#endif
#if !(DOUT3 < 0)
		case DOUT3:
			mcu_clear_output(DOUT3);
			break;
#endif
#if !(DOUT4 < 0)
		case DOUT4:
			mcu_clear_output(DOUT4);
			break;
#endif
#if !(DOUT5 < 0)
		case DOUT5:
			mcu_clear_output(DOUT5);
			break;
#endif
#if !(DOUT6 < 0)
		case DOUT6:
			mcu_clear_output(DOUT6);
			break;
#endif
#if !(DOUT7 < 0)
		case DOUT7:
			mcu_clear_output(DOUT7);
			break;
#endif
#if !(DOUT8 < 0)
		case DOUT8:
			mcu_clear_output(DOUT8);
			break;
#endif
#if !(DOUT9 < 0)
		case DOUT9:
			mcu_clear_output(DOUT9);
			break;
#endif
#if !(DOUT10 < 0)
		case DOUT10:
			mcu_clear_output(DOUT10);
			break;
#endif
#if !(DOUT11 < 0)
		case DOUT11:
			mcu_clear_output(DOUT11);
			break;
#endif
#if !(DOUT12 < 0)
		case DOUT12:
			mcu_clear_output(DOUT12);
			break;
#endif
#if !(DOUT13 < 0)
		case DOUT13:
			mcu_clear_output(DOUT13);
			break;
#endif
#if !(DOUT14 < 0)
		case DOUT14:
			mcu_clear_output(DOUT14);
			break;
#endif
#if !(DOUT15 < 0)
		case DOUT15:
			mcu_clear_output(DOUT15);
			break;
#endif
#if !(DOUT16 < 0)
		case DOUT16:
			mcu_clear_output(DOUT16);
			break;
#endif
#if !(DOUT17 < 0)
		case DOUT17:
			mcu_clear_output(DOUT17);
			break;
#endif
#if !(DOUT18 < 0)
		case DOUT18:
			mcu_clear_output(DOUT18);
			break;
#endif
#if !(DOUT19 < 0)
		case DOUT19:
			mcu_clear_output(DOUT19);
			break;
#endif
#if !(DOUT20 < 0)
		case DOUT20:
			mcu_clear_output(DOUT20);
			break;
#endif
#if !(DOUT21 < 0)
		case DOUT21:
			mcu_clear_output(DOUT21);
			break;
#endif
#if !(DOUT22 < 0)
		case DOUT22:
			mcu_clear_output(DOUT22);
			break;
#endif
#if !(DOUT23 < 0)
		case DOUT23:
			mcu_clear_output(DOUT23);
			break;
#endif
#if !(DOUT24 < 0)
		case DOUT24:
			mcu_clear_output(DOUT24);
			break;
#endif
#if !(DOUT25 < 0)
		case DOUT25:
			mcu_clear_output(DOUT25);
			break;
#endif
#if !(DOUT26 < 0)
		case DOUT26:
			mcu_clear_output(DOUT26);
			break;
#endif
#if !(DOUT27 < 0)
		case DOUT27:
			mcu_clear_output(DOUT27);
			break;
#endif
#if !(DOUT28 < 0)
		case DOUT28:
			mcu_clear_output(DOUT28);
			break;
#endif
#if !(DOUT29 < 0)
		case DOUT29:
			mcu_clear_output(DOUT29);
			break;
#endif
#if !(DOUT30 < 0)
		case DOUT30:
			mcu_clear_output(DOUT30);
			break;
#endif
#if !(DOUT31 < 0)
		case DOUT31:
			mcu_clear_output(DOUT31);
			break;
#endif
		}
	}
}

void io_enable_steppers(uint8_t mask)
{
#if !(STEP0_EN < 0)
	if (mask & 0x01)
	{
		mcu_set_output(STEP0_EN);
	}
	else
	{
		mcu_clear_output(STEP0_EN);
	}
#endif
#if !(STEP1_EN < 0)
	if (mask & 0x02)
	{
		mcu_set_output(STEP1_EN);
	}
	else
	{
		mcu_clear_output(STEP1_EN);
	}
#endif
#if !(STEP2_EN < 0)
	if (mask & 0x04)
	{
		mcu_set_output(STEP2_EN);
	}
	else
	{
		mcu_clear_output(STEP2_EN);
	}
#endif
#if !(STEP3_EN < 0)
	if (mask & 0x08)
	{
		mcu_set_output(STEP3_EN);
	}
	else
	{
		mcu_clear_output(STEP3_EN);
	}
#endif
#if !(STEP4_EN < 0)
	if (mask & 0x10)
	{
		mcu_set_output(STEP4_EN);
	}
	else
	{
		mcu_clear_output(STEP4_EN);
	}
#endif
#if !(STEP5_EN < 0)
	if (mask & 0x20)
	{
		mcu_set_output(STEP5_EN);
	}
	else
	{
		mcu_clear_output(STEP5_EN);
	}
#endif
#if !(STEP6_EN < 0)
	if (mask & 0x40)
	{
		mcu_set_output(STEP6_EN);
	}
	else
	{
		mcu_clear_output(STEP6_EN);
	}
#endif
#if !(STEP7_EN < 0)
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
#if !(ANALOG0 < 0)
	case ANALOG0:
		return mcu_get_analog(ANALOG0);
#endif
#if !(ANALOG1 < 0)
	case ANALOG1:
		return mcu_get_analog(ANALOG1);
#endif
#if !(ANALOG2 < 0)
	case ANALOG2:
		return mcu_get_analog(ANALOG2);
#endif
#if !(ANALOG3 < 0)
	case ANALOG3:
		return mcu_get_analog(ANALOG3);
#endif
#if !(ANALOG4 < 0)
	case ANALOG4:
		return mcu_get_analog(ANALOG4);
#endif
#if !(ANALOG5 < 0)
	case ANALOG5:
		return mcu_get_analog(ANALOG5);
#endif
#if !(ANALOG6 < 0)
	case ANALOG6:
		return mcu_get_analog(ANALOG6);
#endif
#if !(ANALOG7 < 0)
	case ANALOG7:
		return mcu_get_analog(ANALOG7);
#endif
#if !(ANALOG8 < 0)
	case ANALOG8:
		return mcu_get_analog(ANALOG8);
#endif
#if !(ANALOG9 < 0)
	case ANALOG9:
		return mcu_get_analog(ANALOG9);
#endif
#if !(ANALOG10 < 0)
	case ANALOG10:
		return mcu_get_analog(ANALOG10);
#endif
#if !(ANALOG11 < 0)
	case ANALOG11:
		return mcu_get_analog(ANALOG11);
#endif
#if !(ANALOG12 < 0)
	case ANALOG12:
		return mcu_get_analog(ANALOG12);
#endif
#if !(ANALOG13 < 0)
	case ANALOG13:
		return mcu_get_analog(ANALOG13);
#endif
#if !(ANALOG14 < 0)
	case ANALOG14:
		return mcu_get_analog(ANALOG14);
#endif
#if !(ANALOG15 < 0)
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
#if !(STEP0 < 0)
	case STEP0:
		return (mcu_get_output(STEP0) != 0);
#endif
#if !(STEP1 < 0)
	case STEP1:
		return (mcu_get_output(STEP1) != 0);
#endif
#if !(STEP2 < 0)
	case STEP2:
		return (mcu_get_output(STEP2) != 0);
#endif
#if !(STEP3 < 0)
	case STEP3:
		return (mcu_get_output(STEP3) != 0);
#endif
#if !(STEP4 < 0)
	case STEP4:
		return (mcu_get_output(STEP4) != 0);
#endif
#if !(STEP5 < 0)
	case STEP5:
		return (mcu_get_output(STEP5) != 0);
#endif
#if !(STEP6 < 0)
	case STEP6:
		return (mcu_get_output(STEP6) != 0);
#endif
#if !(STEP7 < 0)
	case STEP7:
		return (mcu_get_output(STEP7) != 0);
#endif
#if !(DIR0 < 0)
	case DIR0:
		return (mcu_get_output(DIR0) != 0);
#endif
#if !(DIR1 < 0)
	case DIR1:
		return (mcu_get_output(DIR1) != 0);
#endif
#if !(DIR2 < 0)
	case DIR2:
		return (mcu_get_output(DIR2) != 0);
#endif
#if !(DIR3 < 0)
	case DIR3:
		return (mcu_get_output(DIR3) != 0);
#endif
#if !(DIR4 < 0)
	case DIR4:
		return (mcu_get_output(DIR4) != 0);
#endif
#if !(DIR5 < 0)
	case DIR5:
		return (mcu_get_output(DIR5) != 0);
#endif
#if !(DIR6 < 0)
	case DIR6:
		return (mcu_get_output(DIR6) != 0);
#endif
#if !(DIR7 < 0)
	case DIR7:
		return (mcu_get_output(DIR7) != 0);
#endif
#if !(STEP0_EN < 0)
	case STEP0_EN:
		return (mcu_get_output(STEP0_EN) != 0);
#endif
#if !(STEP1_EN < 0)
	case STEP1_EN:
		return (mcu_get_output(STEP1_EN) != 0);
#endif
#if !(STEP2_EN < 0)
	case STEP2_EN:
		return (mcu_get_output(STEP2_EN) != 0);
#endif
#if !(STEP3_EN < 0)
	case STEP3_EN:
		return (mcu_get_output(STEP3_EN) != 0);
#endif
#if !(STEP4_EN < 0)
	case STEP4_EN:
		return (mcu_get_output(STEP4_EN) != 0);
#endif
#if !(STEP5_EN < 0)
	case STEP5_EN:
		return (mcu_get_output(STEP5_EN) != 0);
#endif
#if !(STEP6_EN < 0)
	case STEP6_EN:
		return (mcu_get_output(STEP6_EN) != 0);
#endif
#if !(STEP7_EN < 0)
	case STEP7_EN:
		return (mcu_get_output(STEP7_EN) != 0);
#endif
#if !(PWM0 < 0)
	case PWM0:
		return mcu_get_pwm(PWM0);
#endif
#if !(PWM1 < 0)
	case PWM1:
		return mcu_get_pwm(PWM1);
#endif
#if !(PWM2 < 0)
	case PWM2:
		return mcu_get_pwm(PWM2);
#endif
#if !(PWM3 < 0)
	case PWM3:
		return mcu_get_pwm(PWM3);
#endif
#if !(PWM4 < 0)
	case PWM4:
		return mcu_get_pwm(PWM4);
#endif
#if !(PWM5 < 0)
	case PWM5:
		return mcu_get_pwm(PWM5);
#endif
#if !(PWM6 < 0)
	case PWM6:
		return mcu_get_pwm(PWM6);
#endif
#if !(PWM7 < 0)
	case PWM7:
		return mcu_get_pwm(PWM7);
#endif
#if !(PWM8 < 0)
	case PWM8:
		return mcu_get_pwm(PWM8);
#endif
#if !(PWM9 < 0)
	case PWM9:
		return mcu_get_pwm(PWM9);
#endif
#if !(PWM10 < 0)
	case PWM10:
		return mcu_get_pwm(PWM10);
#endif
#if !(PWM11 < 0)
	case PWM11:
		return mcu_get_pwm(PWM11);
#endif
#if !(PWM12 < 0)
	case PWM12:
		return mcu_get_pwm(PWM12);
#endif
#if !(PWM13 < 0)
	case PWM13:
		return mcu_get_pwm(PWM13);
#endif
#if !(PWM14 < 0)
	case PWM14:
		return mcu_get_pwm(PWM14);
#endif
#if !(PWM15 < 0)
	case PWM15:
		return mcu_get_pwm(PWM15);
#endif
#if !(DOUT0 < 0)
	case DOUT0:
		return (mcu_get_output(DOUT0) != 0);
#endif
#if !(DOUT1 < 0)
	case DOUT1:
		return (mcu_get_output(DOUT1) != 0);
#endif
#if !(DOUT2 < 0)
	case DOUT2:
		return (mcu_get_output(DOUT2) != 0);
#endif
#if !(DOUT3 < 0)
	case DOUT3:
		return (mcu_get_output(DOUT3) != 0);
#endif
#if !(DOUT4 < 0)
	case DOUT4:
		return (mcu_get_output(DOUT4) != 0);
#endif
#if !(DOUT5 < 0)
	case DOUT5:
		return (mcu_get_output(DOUT5) != 0);
#endif
#if !(DOUT6 < 0)
	case DOUT6:
		return (mcu_get_output(DOUT6) != 0);
#endif
#if !(DOUT7 < 0)
	case DOUT7:
		return (mcu_get_output(DOUT7) != 0);
#endif
#if !(DOUT8 < 0)
	case DOUT8:
		return (mcu_get_output(DOUT8) != 0);
#endif
#if !(DOUT9 < 0)
	case DOUT9:
		return (mcu_get_output(DOUT9) != 0);
#endif
#if !(DOUT10 < 0)
	case DOUT10:
		return (mcu_get_output(DOUT10) != 0);
#endif
#if !(DOUT11 < 0)
	case DOUT11:
		return (mcu_get_output(DOUT11) != 0);
#endif
#if !(DOUT12 < 0)
	case DOUT12:
		return (mcu_get_output(DOUT12) != 0);
#endif
#if !(DOUT13 < 0)
	case DOUT13:
		return (mcu_get_output(DOUT13) != 0);
#endif
#if !(DOUT14 < 0)
	case DOUT14:
		return (mcu_get_output(DOUT14) != 0);
#endif
#if !(DOUT15 < 0)
	case DOUT15:
		return (mcu_get_output(DOUT15) != 0);
#endif
#if !(DOUT16 < 0)
	case DOUT16:
		return (mcu_get_output(DOUT16) != 0);
#endif
#if !(DOUT17 < 0)
	case DOUT17:
		return (mcu_get_output(DOUT17) != 0);
#endif
#if !(DOUT18 < 0)
	case DOUT18:
		return (mcu_get_output(DOUT18) != 0);
#endif
#if !(DOUT19 < 0)
	case DOUT19:
		return (mcu_get_output(DOUT19) != 0);
#endif
#if !(DOUT20 < 0)
	case DOUT20:
		return (mcu_get_output(DOUT20) != 0);
#endif
#if !(DOUT21 < 0)
	case DOUT21:
		return (mcu_get_output(DOUT21) != 0);
#endif
#if !(DOUT22 < 0)
	case DOUT22:
		return (mcu_get_output(DOUT22) != 0);
#endif
#if !(DOUT23 < 0)
	case DOUT23:
		return (mcu_get_output(DOUT23) != 0);
#endif
#if !(DOUT24 < 0)
	case DOUT24:
		return (mcu_get_output(DOUT24) != 0);
#endif
#if !(DOUT25 < 0)
	case DOUT25:
		return (mcu_get_output(DOUT25) != 0);
#endif
#if !(DOUT26 < 0)
	case DOUT26:
		return (mcu_get_output(DOUT26) != 0);
#endif
#if !(DOUT27 < 0)
	case DOUT27:
		return (mcu_get_output(DOUT27) != 0);
#endif
#if !(DOUT28 < 0)
	case DOUT28:
		return (mcu_get_output(DOUT28) != 0);
#endif
#if !(DOUT29 < 0)
	case DOUT29:
		return (mcu_get_output(DOUT29) != 0);
#endif
#if !(DOUT30 < 0)
	case DOUT30:
		return (mcu_get_output(DOUT30) != 0);
#endif
#if !(DOUT31 < 0)
	case DOUT31:
		return (mcu_get_output(DOUT31) != 0);
#endif
#if !(LIMIT_X < 0)
	case LIMIT_X:
		return (mcu_get_input(LIMIT_X) != 0);
#endif
#if !(LIMIT_Y < 0)
	case LIMIT_Y:
		return (mcu_get_input(LIMIT_Y) != 0);
#endif
#if !(LIMIT_Z < 0)
	case LIMIT_Z:
		return (mcu_get_input(LIMIT_Z) != 0);
#endif
#if !(LIMIT_X2 < 0)
	case LIMIT_X2:
		return (mcu_get_input(LIMIT_X2) != 0);
#endif
#if !(LIMIT_Y2 < 0)
	case LIMIT_Y2:
		return (mcu_get_input(LIMIT_Y2) != 0);
#endif
#if !(LIMIT_Z2 < 0)
	case LIMIT_Z2:
		return (mcu_get_input(LIMIT_Z2) != 0);
#endif
#if !(LIMIT_A < 0)
	case LIMIT_A:
		return (mcu_get_input(LIMIT_A) != 0);
#endif
#if !(LIMIT_B < 0)
	case LIMIT_B:
		return (mcu_get_input(LIMIT_B) != 0);
#endif
#if !(LIMIT_C < 0)
	case LIMIT_C:
		return (mcu_get_input(LIMIT_C) != 0);
#endif
#if !(PROBE < 0)
	case PROBE:
		return (mcu_get_input(PROBE) != 0);
#endif
#if !(ESTOP < 0)
	case ESTOP:
#ifndef INVERT_EMERGENCY_STOP
		return (mcu_get_input(ESTOP) != 0);
#else
		return (mcu_get_input(ESTOP) == 0);
#endif
#endif
#if !(SAFETY_DOOR < 0)
	case SAFETY_DOOR:
		return (mcu_get_input(SAFETY_DOOR) != 0);
#endif
#if !(FHOLD < 0)
	case FHOLD:
		return (mcu_get_input(FHOLD) != 0);
#endif
#if !(CS_RES < 0)
	case CS_RES:
		return (mcu_get_input(CS_RES) != 0);
#endif
#if !(ANALOG0 < 0)
	case ANALOG0:
		return mcu_get_analog(ANALOG0);
#endif
#if !(ANALOG1 < 0)
	case ANALOG1:
		return mcu_get_analog(ANALOG1);
#endif
#if !(ANALOG2 < 0)
	case ANALOG2:
		return mcu_get_analog(ANALOG2);
#endif
#if !(ANALOG3 < 0)
	case ANALOG3:
		return mcu_get_analog(ANALOG3);
#endif
#if !(ANALOG4 < 0)
	case ANALOG4:
		return mcu_get_analog(ANALOG4);
#endif
#if !(ANALOG5 < 0)
	case ANALOG5:
		return mcu_get_analog(ANALOG5);
#endif
#if !(ANALOG6 < 0)
	case ANALOG6:
		return mcu_get_analog(ANALOG6);
#endif
#if !(ANALOG7 < 0)
	case ANALOG7:
		return mcu_get_analog(ANALOG7);
#endif
#if !(ANALOG8 < 0)
	case ANALOG8:
		return mcu_get_analog(ANALOG8);
#endif
#if !(ANALOG9 < 0)
	case ANALOG9:
		return mcu_get_analog(ANALOG9);
#endif
#if !(ANALOG10 < 0)
	case ANALOG10:
		return mcu_get_analog(ANALOG10);
#endif
#if !(ANALOG11 < 0)
	case ANALOG11:
		return mcu_get_analog(ANALOG11);
#endif
#if !(ANALOG12 < 0)
	case ANALOG12:
		return mcu_get_analog(ANALOG12);
#endif
#if !(ANALOG13 < 0)
	case ANALOG13:
		return mcu_get_analog(ANALOG13);
#endif
#if !(ANALOG14 < 0)
	case ANALOG14:
		return mcu_get_analog(ANALOG14);
#endif
#if !(ANALOG15 < 0)
	case ANALOG15:
		return mcu_get_analog(ANALOG15);
#endif
#if !(DIN0 < 0)
	case DIN0:
		return (mcu_get_input(DIN0) != 0);
#endif
#if !(DIN1 < 0)
	case DIN1:
		return (mcu_get_input(DIN1) != 0);
#endif
#if !(DIN2 < 0)
	case DIN2:
		return (mcu_get_input(DIN2) != 0);
#endif
#if !(DIN3 < 0)
	case DIN3:
		return (mcu_get_input(DIN3) != 0);
#endif
#if !(DIN4 < 0)
	case DIN4:
		return (mcu_get_input(DIN4) != 0);
#endif
#if !(DIN5 < 0)
	case DIN5:
		return (mcu_get_input(DIN5) != 0);
#endif
#if !(DIN6 < 0)
	case DIN6:
		return (mcu_get_input(DIN6) != 0);
#endif
#if !(DIN7 < 0)
	case DIN7:
		return (mcu_get_input(DIN7) != 0);
#endif
#if !(DIN8 < 0)
	case DIN8:
		return (mcu_get_input(DIN8) != 0);
#endif
#if !(DIN9 < 0)
	case DIN9:
		return (mcu_get_input(DIN9) != 0);
#endif
#if !(DIN10 < 0)
	case DIN10:
		return (mcu_get_input(DIN10) != 0);
#endif
#if !(DIN11 < 0)
	case DIN11:
		return (mcu_get_input(DIN11) != 0);
#endif
#if !(DIN12 < 0)
	case DIN12:
		return (mcu_get_input(DIN12) != 0);
#endif
#if !(DIN13 < 0)
	case DIN13:
		return (mcu_get_input(DIN13) != 0);
#endif
#if !(DIN14 < 0)
	case DIN14:
		return (mcu_get_input(DIN14) != 0);
#endif
#if !(DIN15 < 0)
	case DIN15:
		return (mcu_get_input(DIN15) != 0);
#endif
#if !(DIN16 < 0)
	case DIN16:
		return (mcu_get_input(DIN16) != 0);
#endif
#if !(DIN17 < 0)
	case DIN17:
		return (mcu_get_input(DIN17) != 0);
#endif
#if !(DIN18 < 0)
	case DIN18:
		return (mcu_get_input(DIN18) != 0);
#endif
#if !(DIN19 < 0)
	case DIN19:
		return (mcu_get_input(DIN19) != 0);
#endif
#if !(DIN20 < 0)
	case DIN20:
		return (mcu_get_input(DIN20) != 0);
#endif
#if !(DIN21 < 0)
	case DIN21:
		return (mcu_get_input(DIN21) != 0);
#endif
#if !(DIN22 < 0)
	case DIN22:
		return (mcu_get_input(DIN22) != 0);
#endif
#if !(DIN23 < 0)
	case DIN23:
		return (mcu_get_input(DIN23) != 0);
#endif
#if !(DIN24 < 0)
	case DIN24:
		return (mcu_get_input(DIN24) != 0);
#endif
#if !(DIN25 < 0)
	case DIN25:
		return (mcu_get_input(DIN25) != 0);
#endif
#if !(DIN26 < 0)
	case DIN26:
		return (mcu_get_input(DIN26) != 0);
#endif
#if !(DIN27 < 0)
	case DIN27:
		return (mcu_get_input(DIN27) != 0);
#endif
#if !(DIN28 < 0)
	case DIN28:
		return (mcu_get_input(DIN28) != 0);
#endif
#if !(DIN29 < 0)
	case DIN29:
		return (mcu_get_input(DIN29) != 0);
#endif
#if !(DIN30 < 0)
	case DIN30:
		return (mcu_get_input(DIN30) != 0);
#endif
#if !(DIN31 < 0)
	case DIN31:
		return (mcu_get_input(DIN31) != 0);
#endif
#if !(SERVO0 < 0)
	case SERVO0:
		return (uint8_t)mcu_get_servo(SERVO0);
#endif
#if !(SERVO1 < 0)
	case SERVO1:
		return (uint8_t)mcu_get_servo(SERVO1);
#endif
#if !(SERVO2 < 0)
	case SERVO2:
		return (uint8_t)mcu_get_servo(SERVO2);
#endif
#if !(SERVO3 < 0)
	case SERVO3:
		return (uint8_t)mcu_get_servo(SERVO3);
#endif
#if !(SERVO4 < 0)
	case SERVO4:
		return (uint8_t)mcu_get_servo(SERVO4);
#endif
#if !(SERVO5 < 0)
	case SERVO5:
		return (uint8_t)mcu_get_servo(SERVO5);
#endif
	}
	return -1;
}
