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
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef DIGITAL_IO_CONTROL_H
#define DIGITAL_IO_CONTROL_H

#include <stdbool.h>

//ISR
void io_limits_isr(uint8_t limits);
void io_controls_isr(uint8_t controls);
void io_probe_isr(uint8_t probe);

//inputs
bool io_check_boundaries(float* axis);
uint8_t io_get_limits(uint8_t limitmask);
uint8_t io_get_controls(uint8_t controlmask);
void io_enable_probe();
void io_disable_probe();
bool io_get_probe();
uint32_t io_get_inputs();

//outputs
void io_set_outputs(uint32_t mask);
void io_clear_outputs(uint32_t mask);
void io_toogle_outputs(uint32_t mask);
uint32_t io_get_outputs();

//analogs
uint8_t io_get_analog(uint8_t channel);
void io_set_pwm(uint8_t channel, uint8_t value);
uint8_t io_get_pwm(uint8_t channel);

#endif

