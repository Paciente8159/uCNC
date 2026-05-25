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
	See the GNU General Public License for more details.
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

#define FLT_TO_Q15(x) ((x >= 0.999969f) ? INT16_MAX : ((x <= -1.0f) ? INT16_MIN : ((int16_t)(x * ((float)INT16_MAX)))))
#define INT_TO_Q15(x, maxref) ((int16_t)((((int32_t)x) * INT16_MAX) / maxref))
#define Q15_TO_INT(x, maxref) ((int16_t)((((int32_t)x) * maxref) / INT16_MAX))
#define INT_TO_Q31(x, maxref) ((int32_t)((((int64_t)x) * INT32_MAX) / maxref))
#define Q31_TO_INT(x, maxref) ((int32_t)((((int64_t)x) * maxref) / INT32_MAX))

#define HZ_TO_MS(hz) (1000 / (hz))

	typedef struct pid_data_
	{
		float k[3];
		float max;
		float min;
		float last_sample;
		float i_accum;
		float last_error;
	} pid_data_t;

	typedef struct pid_data_q15_
	{
		int16_t k[3];
		int16_t max;
		int16_t min;
		int32_t last_sample;
		int32_t i_accum;
		int32_t last_error;
	} pid_data_q15_t;

	typedef struct pid_data_q31_
	{
		int32_t k[3];
		int32_t max;
		int32_t min;
		int32_t last_sample;
		int64_t i_accum;
		int64_t last_error;
	} pid_data_q31_t;

	typedef struct
	{
		uint8_t active;
		uint8_t high;

		uint16_t setpoint;
		uint16_t output_max;
		uint16_t relay_amp;

		int16_t peak_max;
		int16_t peak_min;

		uint32_t last_cross;
		uint32_t period_sum;
		uint8_t period_count;

		int16_t Kp, Ki, Kd; // Q15 results
	} pid_autotune_q15_t;

	bool pid_compute(pid_data_t *pid, float *output, float setpoint, float input, uint32_t sample_rate_ms);
	bool pid_compute_q15(pid_data_q15_t *pid, int16_t *output, int16_t setpoint, int16_t input, uint32_t sample_rate_ms);
	bool pid_compute_q31(pid_data_q31_t *pid, int32_t *output, int32_t setpoint, int32_t input, uint32_t sample_rate_ms);

	// Auto tune
	void pid_autotune_q15_start(pid_autotune_q15_t *at, uint16_t setpoint, uint16_t output_max, uint16_t relay_amp);
	uint16_t pid_autotune_q15_step(pid_autotune_q15_t *at, uint16_t input, uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif
