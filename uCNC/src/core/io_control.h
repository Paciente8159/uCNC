/*
	Name: io_control.h
	Description: The input control unit for µCNC.
		This is responsible to check all limit switches (both hardware and software), control switches,
		and probe.

		TODO:
			-implement generic inputs
			-implement outputs

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

#ifndef DIGITAL_IO_CONTROL_H
#define DIGITAL_IO_CONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../module.h"
#include "../modules/ic74hc595.h"
#include "../hal/io_hal.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * IO CONTROL<-> MCU HAL
 *
 **/
#define io_config_input(pin) io_hal_config_input(pin)
#define io_config_pullup(pin)  io_hal_config_pullup(pin)
#define io_get_input(pin)  io_hal_get_input(pin)
#define io_config_analog(pin)  io_hal_config_analog(pin)
#define io_get_analog(pin)  io_hal_get_analog(pin)

#define io_config_output(pin) io_hal_config_output(pin)
#define io_set_output(pin) io_hal_set_output(pin)
#define io_clear_output(pin) io_hal_clear_output(pin)
#define io_toggle_output(pin) io_hal_toggle_output(pin)
#define io_get_output(pin) io_hal_get_output(pin)
#define io_config_pwm(pin, freq) io_hal_config_pwm(pin, freq)
#define io_set_pwm(pin, value) io_hal_set_pwm(pin, value)
#define io_get_pwm(pin) io_hal_get_pwm(pin)

#if defined(MCU_HAS_SOFT_PWM_TIMER) || defined(IC74HC595_HAS_PWMS)
	extern uint8_t g_io_soft_pwm[16];
	extern uint8_t g_soft_pwm_res;
#define io_set_soft_pwm(pin, value) ({ g_io_soft_pwm[pin - PWM_PINS_OFFSET] = (0xFF & value); })
	void io_soft_pwm_update(void);
#endif

	// inputs
	void io_lock_limits(uint8_t limitmask);
	void io_invert_limits(uint8_t limitmask);
	uint8_t io_get_limits(void);
	uint8_t io_get_limits_dual(void);
	uint8_t io_get_controls(void);
	void io_enable_probe(void);
	void io_disable_probe(void);
	bool io_get_probe(void);

	// outputs
	void io_set_steps(uint8_t mask);
	void io_toggle_steps(uint8_t mask);
	void io_set_dirs(uint8_t mask);

	void io_enable_steppers(uint8_t mask);

	// all purpose functions
	void io_set_pinvalue(uint8_t pin, uint8_t value);
	int16_t io_get_pinvalue(uint8_t pin);

#ifdef ENABLE_IO_MODULES
	// event_input_change_handler
	DECL_EVENT_HANDLER(input_change);
	// event_probe_enable_handler
	DECL_EVENT_HANDLER(probe_enable);
	// event_probe_disable_handler
	DECL_EVENT_HANDLER(probe_disable);
// // event_set_steps_handler
// DECL_EVENT_HANDLER(set_steps);
// // event_toggle_steps_handler
// DECL_EVENT_HANDLER(toggle_steps);
// // event_set_dirs_handler
// DECL_EVENT_HANDLER(set_dirs);
// // event_enable_steppers_handler
// DECL_EVENT_HANDLER(enable_steppers);
// typedef struct set_output_args_
// {
// 	uint8_t pin;
// 	bool state;
// } set_output_args_t;
// // event_set_output_handler
// DECL_EVENT_HANDLER(set_output);
#endif

#ifdef ENABLE_IO_ALARM_DEBUG
	extern uint8_t io_alarm_limits;
	extern uint8_t io_alarm_controls;
#endif

#ifdef __cplusplus
}
#endif

#endif
