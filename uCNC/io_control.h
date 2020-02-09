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
#include "config.h"

//ISR
void io_limits_isr();
void io_controls_isr();
void io_probe_isr();

//inputs
bool io_check_boundaries(float* axis);
uint8_t io_get_limits();
uint8_t io_get_controls();
void io_enable_probe();
void io_disable_probe();
bool io_get_probe();
void io_set_homing_limits_filter(uint8_t filter_mask);

//outputs
void io_set_steps(uint8_t mask);
void io_toggle_steps(uint8_t mask);
void io_set_dirs(uint8_t mask);

void io_enable_steps();

#endif

