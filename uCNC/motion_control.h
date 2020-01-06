/*
	Name: motion_control.h
	Description: Contains the building blocks for performing motions/actions in uCNC
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 19/11/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

void mc_init();
bool mc_toogle_checkmode();
uint8_t mc_line(float* target, float feed);
uint8_t mc_arc(float* target, float center_offset_a, float center_offset_b, float radius, uint8_t plane, bool isclockwise, float feed);
uint8_t mc_dwell(uint16_t milliseconds);
uint8_t mc_home_axis(uint8_t axis, uint8_t axis_limit);

#endif
