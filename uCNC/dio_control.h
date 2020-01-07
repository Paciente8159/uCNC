/*
	Name: dio_control.h
	Description: The input control unit for uCNC.
        This is responsible to check all limit switches (both hardware and software), control switches,
        and probe.

		TODO:
			-implement generic inputs
			-implement outputs

	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 07/12/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef DIGITAL_IO_CONTROL_H
#define DIGITAL_IO_CONTROL_H

#include <stdbool.h>

void dio_init();
void dio_limits_isr(uint8_t limits);
void dio_controls_isr(uint8_t controls);
bool dio_check_boundaries(float* axis);
uint8_t dio_get_limits(uint8_t limitmask);
uint8_t dio_get_controls(uint8_t controlmask);
bool dio_get_probe();

uint16_t dio_get_inputs();
void dio_set_outputs(uint16_t mask);
void dio_clear_outputs(uint16_t mask);

uint8_t dio_get_analog(uint8_t channel);
void dio_set_pwm(uint8_t channel, uint8_t value);

#endif

