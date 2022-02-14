/*
	Name: pid_controller.h
	Description: PID controller for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 03-07-2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#ifdef __cplusplus
extern "C"
{
#endif

// These parameters adjust the PID to use integer math only and output limiting (usually to be used with PWM)
#define PID_BITSHIFT_FACTOR 8
#define PID_OUTPUT_MAX ((1 << PID_BITSHIFT_FACTOR) - 1)
#define PID_OUTPUT_MIN (-((1 << PID_BITSHIFT_FACTOR) - 1))

	void pid_init(void);
	void pid_update(void);
	void pid_stop(void);

#ifdef __cplusplus
}
#endif

#endif