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

#include <stdbool.h>
#include <stdint.h>

	// inputs
	void io_lock_limits(uint8_t limitmask);
	void io_invert_limits(uint8_t limitmask);
	uint8_t io_get_limits(void);
	uint8_t io_get_limits_dual(void);
	uint8_t io_get_controls(void);
	void io_enable_probe(void);
	void io_disable_probe(void);
	bool io_get_probe(void);

	uint8_t io_get_analog(uint8_t pin);

	// outputs
	void io_set_steps(uint8_t mask);
	void io_toggle_steps(uint8_t mask);
	void io_set_dirs(uint8_t mask);

	void io_set_pwm(uint8_t pin, uint8_t value);
	void io_set_output(uint8_t pin, bool state);

	void io_enable_steppers(uint8_t mask);

	int16_t io_get_pinvalue(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif
