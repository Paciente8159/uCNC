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

#define HZ_TO_MS(hz) (1000 / (hz))

	typedef struct pid_data_
	{
		float k[3];
		float last_input;
		float i_accum;
		float max;
		float min;
		uint32_t last_sample;
	} pid_data_t;

	typedef struct pid_data_q15_
	{
		int16_t k[3];
		int16_t last_input;
		int16_t i_accum;
		int16_t max;
		int16_t min;
		uint32_t last_sample;
	} pid_data_q15_t;

	typedef enum
	{
		PID_AT_IDLE = 0,
		PID_AT_WAIT_STABLE,
		PID_AT_RELAY_LOW,
		PID_AT_RELAY_HIGH,
		PID_AT_DONE,
		PID_AT_ABORT
	} pid_at_state_t;

	typedef struct
	{
		pid_at_state_t state;
		uint32_t start_time;
		uint32_t last_cross_time;
		uint32_t period_accum;
		uint8_t period_count;

		float setpoint_rpm;
		uint8_t relay_center_pwm; // e.g. 128
		uint8_t relay_amp_pwm;	  // e.g. 40

		float max_rpm;
		float min_rpm;

		float Ku;
		float Tu;

		// results
		float kp, ki, kd;
	} pid_autotune_t;

	bool pid_compute(pid_data_t *pid, float *output, float setpoint, float input, uint32_t sample_rate_ms);
	bool pid_compute_q15(pid_data_q15_t *pid, int16_t *output, int16_t setpoint, int16_t input, uint32_t sample_rate_ms);
	void pid_autotune_start(pid_autotune_t *at, float setpoint_rpm, uint8_t relay_center_pwm, uint8_t relay_amp_pwm);
	bool pid_autotune_step(pid_autotune_t *at, float current_rpm, uint8_t *pwm_out, uint8_t cycles);

#ifdef __cplusplus
}
#endif

#endif
