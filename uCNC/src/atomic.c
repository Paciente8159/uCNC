/*
    Name: atomic.c
    Description: Some useful macros for thread syncronzation and atomic operations.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 16/12/2025

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#include "cnc.h"

bool mutex_safe_lock(void *lock, uint32_t timeout)
{
    // converts to us
    if (timeout > (UINT32_MAX / 1000))
    {
        timeout *= 1000;
    }

    uint32_t now = mcu_free_micros();
    for (;;)
    {
        ATOMIC_TYPE expected = MUTEX_UNLOCKED;
        if (ATOMIC_COMPARE_EXCHANGE_N((ATOMIC_TYPE *)lock, &expected, MUTEX_LOCKED, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        {
            return true;
        }
        if (!timeout)
        {
            break;
        }
        cnc_yield();
        uint32_t tstamp = mcu_free_micros();
        uint32_t elapsed = (tstamp > now) ? (tstamp - now) : (1000 - now + tstamp);
        if (elapsed >= timeout)
            break;
        timeout -= elapsed;
        now = tstamp;
    }

    return false;
}