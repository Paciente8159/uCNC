/*
	Name: interpolator.h
	Description: Function declarations for the stepper interpolator.

		TODO: Create an S-curve interpolator

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 13/10/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <stdint.h>
#include <stdbool.h>
void itp_init();
void itp_run();
void itp_update();
void itp_step_isr();
void itp_step_reset_isr();
void itp_stop();
void itp_clear();
void itp_get_rt_position(float* axis);
void itp_reset_rt_position();
float itp_get_rt_feed();
#ifdef USE_SPINDLE
uint16_t itp_get_rt_spindle();
#endif
#ifdef AXIS_DUAL_DRIVE
void itp_lock_stepper(uint8_t stepindex);
#endif
#ifdef GCODE_PROCESS_LINE_NUMBERS
uint32_t itp_get_rt_line_number();
#endif
void itp_delay(uint16_t delay);

#endif
