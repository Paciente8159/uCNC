/*
	Name: trigger_control.h
	Description: The input control unit for uCNC.
        This is responsible to check all limit switches (both hardware and software), control switches,
        and probe.

		TODO:
			-implement generic inputs

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

#ifndef TRIGGER_CONTROL_H
#define TRIGGER_CONTROL_H

#include <stdbool.h>

void tc_init();
void tc_limits_isr(uint8_t limits);
void tc_controls_isr(uint8_t controls);
bool tc_check_boundaries(float* axis);
uint8_t tc_get_limits(uint8_t limitmask);
uint8_t tc_get_controls(uint8_t controlmask);
bool tc_get_probe();

#endif

