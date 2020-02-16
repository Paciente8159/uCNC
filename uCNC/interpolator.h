/*
	Name: interpolator.h
	Description: Function declarations for the stepper interpolator.

		TODO: Create an S-curve interpolator

	Copyright: Copyright (c) Jo�o Martins
	Author: Jo�o Martins
	Date: 13/10/2019

	�CNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	�CNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <stdint.h>
#include <stdbool.h>
void itp_init(void);
void itp_run(void);
void itp_update(void);
void itp_step_isr(void);
void itp_step_reset_isr(void);
void itp_stop(void);
void itp_clear(void);
void itp_get_rt_position(float* axis);
void itp_reset_rt_position(void);
float itp_get_rt_feed(void);
#ifdef USE_SPINDLE
uint16_t itp_get_rt_spindle(void);
#endif
#ifdef ENABLE_DUAL_DRIVE_AXIS
void itp_lock_stepper(uint8_t lockmask);
#endif
#ifdef GCODE_PROCESS_LINE_NUMBERS
uint32_t itp_get_rt_line_number(void);
#endif
void itp_delay(uint16_t delay);

#endif
