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
        return false;

    uint32_t dt_ms = (now - last_sample);
    pid->last_sample = now;

    float kp = pid->k[0];
    float ki = pid->k[1];
    float kd = pid->k[2];

    float error = setpoint - input;

    float P = (kp * error);

    // Integral
    pid->i_accum += (ki * error);
    float I = pid->i_accum;

    float D = (kd * (error - pid->last_error));
    pid->last_error = error;

    // PID sum
    float out = P + I + D;

    // Clamp
    if (out < pid->min)
        out = pid->min;
    if (out > pid->max)
        out = pid->max;

    *output = out;
    return true;
}

bool pid_compute_q15(pid_data_q15_t *pid, int16_t *output, int16_t setpoint, int16_t input, uint32_t sample_rate_ms)
{
    uint32_t last_sample = pid->last_sample;
    uint32_t now = mcu_millis();

    if ((now - last_sample) < sample_rate_ms)
        return false;

    uint32_t dt_ms = (now - last_sample);
    pid->last_sample = now;

    int32_t kp = (int32_t)pid->k[0];
    int32_t ki = (int32_t)pid->k[1];
    int32_t kd = (int32_t)pid->k[2];

    int16_t error = setpoint - input;
    // Normalize error to Q32
    error = INT_TO_Q15(error, pid->max);

    int16_t P = (kp * error) >> 15;

    // Integral
    pid->i_accum += (ki * error);
    int32_t I = pid->i_accum >> 15;

    int32_t D = (kd * (error - pid->last_error)) >> 15;
    pid->last_error = error;

    // PID sum
    int32_t out = P + I + D;

    out = (out * pid->max) >> 15;

    // Clamp
    if (out < pid->min)
        out = pid->min;
    if (out > pid->max)
        out = pid->max;

    *output = (int16_t)out;
    return true;
}

bool pid_compute_q31(pid_data_q31_t *pid, int32_t *output, int32_t setpoint, int32_t input, uint32_t sample_rate_ms)
{
    uint32_t last_sample = pid->last_sample;
    uint32_t now = mcu_millis();

    if ((now - last_sample) < sample_rate_ms)
        return false;

    uint32_t dt_ms = (now - last_sample);
    pid->last_sample = now;

    int64_t kp = (int64_t)pid->k[0];
    int64_t ki = (int64_t)pid->k[1];
    int64_t kd = (int64_t)pid->k[2];

    int32_t error = setpoint - input;
    // Normalize error to Q31
    error = INT_TO_Q31(error, pid->max);

    int32_t P = (kp * error) >> 31;

    // Integral
    pid->i_accum += (ki * error);
    int64_t I = pid->i_accum >> 31;

    int64_t D = (kd * (error - pid->last_error)) >> 31;
    pid->last_error = error;

    // PID sum
    int64_t out = P + I + D;

    out = (out * pid->max) >> 31;

    // Clamp
    if (out < pid->min)
        out = pid->min;
    if (out > pid->max)
        out = pid->max;

    *output = (int32_t)out;
    return true;
}

void pid_autotune_q15_start(pid_autotune_q15_t *at, uint16_t setpoint, uint16_t output_max, uint16_t relay_amp)
{
    at->active = 1;
    at->high = 1;

    at->setpoint = setpoint;
    at->output_max = output_max;
    at->relay_amp = relay_amp;

    at->peak_max = -32768;
    at->peak_min = 32767;

    at->period_sum = 0;
    at->period_count = 0;
}

uint16_t pid_autotune_q15_step(pid_autotune_q15_t *at, uint16_t input, uint32_t now_ms)
{
    if (!at->active)
        return 0;

    int16_t error = at->setpoint - input;

    uint16_t mid = at->output_max / 2;
    uint16_t out = at->high ? (mid + at->relay_amp) : (mid - at->relay_amp);

    if (error > 0 && !at->high)
    {
        at->high = 1;
        at->peak_min = input;
        at->last_cross = now_ms;
    }
    else if (error < 0 && at->high)
    {
        at->high = 0;
        at->peak_max = input;

        uint32_t period = now_ms - at->last_cross;
        at->period_sum += period;
        at->period_count++;
    }

    if (at->period_count >= 6)
    {
        uint32_t Pu = at->period_sum / at->period_count;
        int16_t A = (at->peak_max - at->peak_min) / 2;

        // Ku = 4d / (πA)
        int32_t Ku = (4L * at->relay_amp * 32768L) / (102944L * A);

        at->Kp = (int16_t)(Ku * 0.6);
        at->Ki = (int16_t)(Ku * 1.2 / Pu);
        at->Kd = (int16_t)(Ku * 0.075 * Pu);

        at->active = 0;
    }

    return out;
}
