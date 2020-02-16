/*
	Name: motion_control.h
	Description: Contains the building blocks for performing motions/actions in �CNC.

	Copyright: Copyright (c) Jo�o Martins
	Author: Jo�o Martins
	Date: 19/11/2019

	�CNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	�CNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "planner.h"

void mc_init(void);
bool mc_get_checkmode(void);
bool mc_toogle_checkmode(void);
uint8_t mc_line(float *target, planner_block_data_t block_data);
uint8_t mc_arc(float *target, float center_offset_a, float center_offset_b, float radius, uint8_t axis_0, uint8_t axis_1, bool isclockwise, planner_block_data_t block_data);
uint8_t mc_dwell(planner_block_data_t block_data);
uint8_t mc_home_axis(uint8_t axis, uint8_t axis_limit);
uint8_t mc_spindle_coolant(planner_block_data_t block_data);
uint8_t mc_probe(float *target, bool invert_probe, planner_block_data_t block_data);

#endif
