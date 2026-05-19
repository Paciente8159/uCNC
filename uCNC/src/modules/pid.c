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

// Call this once to start auto-tune
void pid_autotune_start(pid_autotune_t *at, float setpoint_rpm, uint8_t relay_center_pwm, uint8_t relay_amp_pwm)
{
    memset(at, 0, sizeof(*at));
    at->state = PID_AT_WAIT_STABLE;
    at->setpoint_rpm = setpoint_rpm;
    at->relay_center_pwm = relay_center_pwm;
    at->relay_amp_pwm = relay_amp_pwm;
    at->start_time = mcu_millis();
    at->max_rpm = -1e9f;
    at->min_rpm = 1e9f;
}

// Returns true when done, false while running
bool pid_autotune_step(pid_autotune_t *at, float current_rpm, uint8_t *pwm_out, uint8_t cycles)
{
    uint32_t now = mcu_millis();

    switch (at->state)
    {
    case PID_AT_IDLE:
    case PID_AT_DONE:
    case PID_AT_ABORT:
        return (at->state == PID_AT_DONE);

    case PID_AT_WAIT_STABLE:
        // Simple: wait some time for spindle to reach near setpoint
        if ((now - at->start_time) > 2000)
        {
            at->state = PID_AT_RELAY_HIGH;
            at->last_cross_time = now;
            at->max_rpm = -1e9f;
            at->min_rpm = 1e9f;
        }
        *pwm_out = at->relay_center_pwm;
        break;

    case PID_AT_RELAY_HIGH:
        *pwm_out = at->relay_center_pwm + at->relay_amp_pwm;
        if (current_rpm > at->max_rpm)
            at->max_rpm = current_rpm;

        if (current_rpm > at->setpoint_rpm)
        {
            // crossed above setpoint → go low
            uint32_t now_cross = now;
            if (at->period_count > 0)
            {
                at->period_accum += (now_cross - at->last_cross_time);
            }
            at->last_cross_time = now_cross;
            at->period_count++;
            at->state = PID_AT_RELAY_LOW;
        }
        break;

    case PID_AT_RELAY_LOW:
        *pwm_out = at->relay_center_pwm - at->relay_amp_pwm;
        if (current_rpm < at->min_rpm)
            at->min_rpm = current_rpm;

        if (current_rpm < at->setpoint_rpm)
        {
            // crossed below setpoint → go high
            uint32_t now_cross = now;
            if (at->period_count > 0)
            {
                at->period_accum += (now_cross - at->last_cross_time);
            }
            at->last_cross_time = now_cross;
            at->period_count++;

            // after enough cycles, compute Ku, Tu
            if (at->period_count >= cycles)
            {
                float A = (at->max_rpm - at->min_rpm) * 0.5f; // amplitude
                if (A <= 0.0f)
                {
                    at->state = PID_AT_ABORT;
                    *pwm_out = 0;
                    return false;
                }

                float d = (float)at->relay_amp_pwm; // PWM amplitude
                at->Ku = (4.0f * d) / (3.1415926f * A);

                float avg_period_ms = (float)at->period_accum / (float)(at->period_count - 1);
                at->Tu = avg_period_ms * 0.001f; // seconds

                // Ziegler–Nichols PID
                at->kp = 0.6f * at->Ku;
                at->ki = 1.2f * at->Ku / at->Tu;
                at->kd = 0.075f * at->Ku * at->Tu;

                at->state = PID_AT_DONE;
                *pwm_out = at->relay_center_pwm;
                return true;
            }

            // prepare for next high phase
            at->max_rpm = -1e9f;
            at->min_rpm = 1e9f;
            at->state = PID_AT_RELAY_HIGH;
        }
        break;
    }

    return false;
}

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
