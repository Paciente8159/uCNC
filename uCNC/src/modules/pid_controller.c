/*
    Name: pid_controller.c
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
#include <stdint.h>
#include <math.h>

#if (PID_CONTROLLERS > 0)

// These parameters adjust the PID to use integer math only and output limiting (usually to be used with PWM)
#define PID_BITSHIFT_FACTOR 8
#define PID_OUTPUT_MAX ((1 << PID_BITSHIFT_FACTOR) - 1)
#define PID_OUTPUT_MIN (-((1 << PID_BITSHIFT_FACTOR) - 1))

static float cumulative_delta[PID_CONTROLLERS];
static float last_error[PID_CONTROLLERS];
static float ki[PID_CONTROLLERS];
static float kd[PID_CONTROLLERS];
static uint8_t pid_freqdiv[PID_CONTROLLERS];

// #define GAIN_FLT_TO_INT (1 << PID_BITSHIFT_FACTOR)
// #define ERROR_MAX (GAIN_FLT_TO_INT - 1)
// #define KP_MAX (GAIN_FLT_TO_INT - 1)
// #define KI_MAX (((GAIN_FLT_TO_INT << 1) >> PID_DIVISIONS) - 1)
// #define KD_MAX ((GAIN_FLT_TO_INT << 10) - 1)
// #define ERROR_SUM_MAX (1 << (31 - PID_BITSHIFT_FACTOR - PID_DIVISIONS))

static int32_t pid_get_freqdiv(uint8_t i)
{
    switch (i)
    {
#if (PID_CONTROLLERS > 0)
    case 0:
        return PID0_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 1)
    case 1:
        return PID1_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 2)
    case 2:
        return PID2_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 3)
    case 3:
        return PID3_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 4)
    case 4:
        return PID4_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 5)
    case 5:
        return PID5_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 6)
    case 6:
        return PID6_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 7)
    case 7:
        return PID7_FREQ_DIV;
#endif
    }

    return 0;
}

static int32_t pid_get_error(uint8_t i)
{
    switch (i)
    {
#if (PID_CONTROLLERS > 0)
    case 0:
        return (int32_t)PID0_DELTA();
#endif
#if (PID_CONTROLLERS > 1)
    case 1:
        return (int32_t)PID1_DELTA();
#endif
#if (PID_CONTROLLERS > 2)
    case 2:
        return (int32_t)PID2_DELTA();
#endif
#if (PID_CONTROLLERS > 3)
    case 3:
        return (int32_t)PID3_DELTA();
#endif
#if (PID_CONTROLLERS > 4)
    case 4:
        return (int32_t)PID4_DELTA();
#endif
#if (PID_CONTROLLERS > 5)
    case 5:
        return (int32_t)PID5_DELTA();
#endif
#if (PID_CONTROLLERS > 6)
    case 6:
        return (int32_t)PID6_DELTA();
#endif
#if (PID_CONTROLLERS > 7)
    case 7:
        return (int32_t)PID7_DELTA();
#endif
    }

    return 0;
}

static int32_t pid_set_output(uint8_t i, int16_t val)
{
    switch (i)
    {
#if (PID_CONTROLLERS > 0)
    case 0:
        PID0_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 1)
    case 1:
        PID1_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 2)
    case 2:
        PID2_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 3)
    case 3:
        PID3_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 4)
    case 4:
        PID4_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 5)
    case 5:
        PID5_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 6)
    case 6:
        PID6_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 7)
    case 7:
        PID7_OUTPUT(val);
        return;
#endif
    }
}

void FORCEINLINE pid_stop()
{

#if (PID_CONTROLLERS > 0)
    PID0_STOP();
#endif
#if (PID_CONTROLLERS > 1)
    PID1_STOP();
#endif
#if (PID_CONTROLLERS > 2)
    PID2_STOP(val);
#endif
#if (PID_CONTROLLERS > 3)
    PID3_STOP(val);
#endif
#if (PID_CONTROLLERS > 4)
    PID4_STOP(val);
#endif
#if (PID_CONTROLLERS > 5)
    PID5_STOP(val);
#endif
#if (PID_CONTROLLERS > 6)
    PID6_STOP(val);
#endif
#if (PID_CONTROLLERS > 7)
    PID7_STOP(val);
#endif
}

// this overrides the module handler since no other module is using it
// may be modified in the future
void mod_cnc_stop_hook(void)
{
    pid_stop();
}

// PID ISR should run once every millisecond
// sampling rate is 1000/(log2*(Nº of PID controllers))
// a single PID controller can run at 1000Hz
// all 8 PID will run at a max freq of 125Hz
// this precomputes the PID factors to save computation cycles
void pid_init(void)
{
    for (uint8_t i = 0; i < PID_CONTROLLERS; i++)
    {
        // error gains must be between 0% and 100% (0 and 1)
        ki[i] = (g_settings.pid_gain[i][1] / (PID_SAMP_FREQ / (float)pid_get_freqdiv(i)));
        kd[i] = (g_settings.pid_gain[i][2] * (PID_SAMP_FREQ / (float)pid_get_freqdiv(i)));
    }
}

void FORCEINLINE pid_update(void)
{
    static uint8_t current_pid = 0;
    if (current_pid < PID_CONTROLLERS)
    {
        if (!pid_freqdiv[current_pid])
        {
            float error = (float)pid_get_error(current_pid);
            float output = 0;
            float pidkp = g_settings.pid_gain[current_pid][0];
            float pidki = ki[current_pid];
            float pidkd = kd[current_pid];
            if (pidkp)
            {
                output = pidkp * error;
            }

            if (pidki)
            {
                float sum = cumulative_delta[current_pid] + error;
                cumulative_delta[current_pid] = sum;
                output += (pidki * sum);
            }

            if (pidkd)
            {
                float rateerror = (error - last_error[current_pid]) * pidkd;
                last_error[current_pid] = error;
                output += rateerror;
            }

            pid_set_output(current_pid, (int16_t)roundf(output));
        }

        if (++pid_freqdiv[current_pid] >= pid_get_freqdiv(current_pid))
        {
            pid_freqdiv[current_pid] = 0;
        }
    }

    // restart
    if (++current_pid >= PID_CONTROLLERS)
    {
        current_pid = 0;
    }
}

// overrides the default mod_rtc_tick_hook
// may be modified in the future
void mod_rtc_tick_hook(void)
{
    pid_update();
}

// overrides the default mod_rtc_tick_hook
// may be modified in the future
void mod_settings_change_hook(void)
{
    pid_init();
}

#endif
