/*
	Name: interpolator.h
	Description: Function declarations for the stepper interpolator.

		TODO: Create an S-curve interpolator

	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 13/10/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <stdint.h>
#include <stdbool.h>
void interpolator_init();
bool interpolator_is_buffer_full();
void interpolator_run();
void interpolator_update();
void interpolator_step_isr();
void interpolator_step_reset_isr();
void interpolator_stop();
void interpolator_clear();
void interpolator_get_rt_position(float* axis);
void interpolator_reset_rt_position();
float interpolator_get_rt_feed();

#endif
