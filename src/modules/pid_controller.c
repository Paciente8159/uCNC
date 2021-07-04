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
    static uint32_t last_ms[PID_CONTROLLERS];
    static int16_t cumulative_delta[PID_CONTROLLERS];
    static float last_error[PID_CONTROLLERS];
#endif

    static float pid_get_error(uint8_t i)
    {
        switch (i)
        {
#if (PID_CONTROLLERS > 0)
        case 0:
            return (float)PID0_DELTA();
#endif
#if (PID_CONTROLLERS > 1)
        case 1:
            return (float)PID1_DELTA();
#endif
#if (PID_CONTROLLERS > 2)
        case 2:
            return (float)PID2_DELTA();
#endif
#if (PID_CONTROLLERS > 3)
        case 3:
            return (float)PID3_DELTA();
#endif
#if (PID_CONTROLLERS > 4)
        case 4:
            return (float)PID4_DELTA();
#endif
#if (PID_CONTROLLERS > 5)
        case 5:
            return (float)PID5_DELTA();
#endif
#if (PID_CONTROLLERS > 6)
        case 6:
            return (float)PID6_DELTA();
#endif
#if (PID_CONTROLLERS > 7)
        case 7:
            return (float)PID7_DELTA();
#endif
        }

        return 0;
    }

    void pid_dotasks(void)
    {
#if (PID_CONTROLLERS > 0)
        static uint8_t current_pid = 0;
        uint32_t current_ms = mcu_millis();
        uint16_t elapsed = (uint16_t)(current_ms - last_ms[current_pid]);
        if (!elapsed)
        {
            return;
        }
        float error = pid_get_error(current_pid);
        cumulative_delta[current_pid] += (error * elapsed);
        float rateerror = (error - last_error[current_pid]) / elapsed;
        float output = settings.pids[current_pid][0] * error + settings.pids[current_pid][1] * cumulative_delta[current_pid] + settings.pids[current_pid][2] * rateerror;
        last_error[current_pid] = error;
        last_ms[current_pid] = current_ms;

        if (++current_pid >= PID_CONTROLLERS)
        {
            current_pid = 0;
        }
#endif
    }

#ifdef __cplusplus
}
#endif
