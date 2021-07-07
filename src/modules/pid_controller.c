/*
	Name: pid_controller.c
	Description: µCNC main unit.

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

#ifdef __cplusplus
extern "C"
{
#endif

#include "cnc.h"
#include <stdint.h>

#if (PID_CONTROLLERS > 0)
    static int32_t cumulative_delta[PID_CONTROLLERS];
    static int32_t last_error[PID_CONTROLLERS];
#endif

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

    //PID ISR should run once every millisecond
    //this is equal to a 125Hz sampling rate (for 8 PID controllers)
    //to reduce computation cycles the value of 124.12Hz (the error/2^7 + error/2^12) is used to compute the integral part of the error
    //to reduce computation cycles the value of 124Hz (the error*2^7 - error*2^2) is used to compute the derivative part of the error
    void pid_update_isr(void)
    {
#if (PID_CONTROLLERS > 0)
        static uint8_t current_pid = 0;
        if (current_pid < PID_CONTROLLERS)
        {
            int16_t error = MIN(pid_get_error(current_pid), 0xffff);
            int64_t output = settings.pids[current_pid][0] * error;

            if (settings.pids[current_pid][1])
            {
                cumulative_delta[current_pid] += (error >> 7) + (error >> 12);
                output += settings.pids[current_pid][1] * cumulative_delta[current_pid];
            }

            if (settings.pids[current_pid][2])
            {
                int32_t rateerror = (error - last_error[current_pid]);
                rateerror = (rateerror << 7) - (rateerror >> 2);
                output += settings.pids[current_pid][1] * cumulative_delta[current_pid];
            }

            last_error[current_pid] = error;
            last_ms[current_pid] = current_ms;

            int16_t pid_result = MIN((output >> PID_BITSHIFT_FACTOR), 255);
            pid_result = MAX(pid_result, -255);
            pid_set_output(current_pid, pid_result);
        }

        if (++current_pid >= 8)
        {
            current_pid = 0;
        }
#endif
    }

#ifdef __cplusplus
}
#endif
