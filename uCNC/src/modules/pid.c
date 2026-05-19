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
	See the GNU General Public License for more details.
*/

#include "../cnc.h"
#include "pid.h"
#include <stdint.h>
#include <math.h>

bool pid_compute(pid_data_t *pid, float *output, float setpoint, float input, uint32_t sample_rate_ms)
{
	uint32_t last_sample = pid->last_sample;
	uint32_t now = mcu_millis();

	if ((now - last_sample) < sample_rate_ms)
	{
		return false;
	}

	float delta_t = ((now - last_sample) * 0.001f); // convert to seconds
	pid->last_sample = now;
	float pidkp = pid->k[0];
	float pidki = pid->k[1] * delta_t;
	float pidkd = pid->k[2] / delta_t;

	float error = setpoint - input;
	float input_delta = pid->last_input - input;
	pid->last_input = input;

	pidkp *= error;
	pidki = pid->i_accum + (pidki * error);
	pidkd *= input_delta;
	
	float out = (pidkp + pidki + pidkd);
	float clamped_out = CLAMP(pid->min, out, pid->max);

	if (out == clamped_out || 
        (error > 0 && out < pid->min) || 
        (error < 0 && out > pid->max))
    {
        pid->i_accum = pidki;
    }

	*output = clamped_out;

	return true;
}

static inline int16_t q15_mul(int16_t a, int16_t b)
{
    return (int16_t)(((int32_t)a * (int32_t)b) >> 15);
}

bool pid_compute_q15(pid_data_q15_t *pid, int16_t *output, int16_t setpoint, int16_t input, uint32_t sample_rate_ms)
{
    uint32_t last_sample = pid->last_sample;
    uint32_t now = mcu_millis();

    if ((now - last_sample) < sample_rate_ms)
        return false;

    uint32_t dt_ms = (now - last_sample);
    pid->last_sample = now;

    int16_t kp = pid->k[0];
    int16_t ki = pid->k[1];
    int16_t kd = pid->k[2];

    int16_t error = setpoint - input;

    int16_t input_delta = pid->last_input - input;
    pid->last_input = input;

    int16_t P = q15_mul(kp, error);

    int32_t I_new = pid->i_accum + ((int32_t)q15_mul(ki, error) * (int32_t)dt_ms) / 1000;

    int32_t D = 0;
    if (dt_ms > 0)
    {
        int32_t deriv = ((int32_t)input_delta * 32768) / (int32_t)dt_ms;
        D = q15_mul(kd, (int16_t)deriv);
    }

    int32_t out = (int32_t)P + (int32_t)I_new + (int32_t)D;

    int16_t clamped_out = CLAMP(pid->min, out, pid->max);

    if (out == clamped_out ||
        (error > 0 && out < pid->min) ||
        (error < 0 && out > pid->max))
    {
        pid->i_accum = I_new;
    }

    *output = clamped_out;
    return true;
}

