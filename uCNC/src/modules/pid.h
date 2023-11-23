/*
	Name: pid.h
	Description: PID controller for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/03/2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef PID_H
#define PID_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../module.h"
#include <stdint.h>
#include <stdbool.h>
#include <float.h>
#include <stdio.h>

#define HZ_TO_MS(hz) ((uint32_t)(0.001f*hz))

	typedef struct pid_data_
	{
		float k[3];
		float last_input;
		float i_accum;
		float max;
		float min;
		uint32_t next_sample;
	} pid_data_t;

	bool pid_compute(pid_data_t *pid, float *output, float setpoint, float input, uint32_t sample_rate_ms);

#ifdef __cplusplus
}
#endif

#endif