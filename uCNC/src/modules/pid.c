/*
	Name: pid.c
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

#include "../cnc.h"
#include "pid.h"
#include <stdint.h>
#include <math.h>

bool pid_compute(pid_data_t *pid, float *output, float setpoint, float input, uint32_t sample_rate_ms)
{
	if (mcu_millis() < pid->next_sample)                                             
	{
		return false;
	}

	uint32_t next_sample = mcu_millis() + sample_rate_ms;
	float delta_t = ((next_sample - pid->next_sample) * 0.001f); // convert to seconds     
	pid->next_sample = next_sample;                                              
	float pidkp = pid->k[0];
	float pidki = pid->k[1] * delta_t;
	float pidkd = pid->k[2] / delta_t;
	*output = 0;
	float error = setpoint - input;
	float input_delta = input - pid->last_input;
	pid->last_input = input;

	pidkp *= error;
	pidki *= error;
	pidki += pid->i_accum;
	pidkd *= input_delta;
	
	pidki = CLAMP(pid->min, pidki, pid->max);
	pid->i_accum = pidki;

	*output = pidkp + pidki + pidkd;
	*output = CLAMP(pid->min, *output, pid->max);

	return true;
}
