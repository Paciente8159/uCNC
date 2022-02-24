/*
    Name: bltouch.c
    Description: BLTouch probe module for µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 23-02-2022

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#include "../cnc.h"

#if ENABLE_BLTOUCH_PROBE

#define BLTOUCH_DELAY 500
// tunned values with scope
#define BLTOUCH_DEPLOY (23)             // 10º
#define BLTOUCH_ALARM_REL_TOUCH_SW (85) // 60º
#define BLTOUCH_STOW (125)              // 90º
#define BLTOUCH_SELF_TEST (165)         // 120º
#define BLTOUCH_ALARM_REL_PUSH_UP (216) // 160º

static uint16_t blsignal;
static bool blenable;

void mod_probe_enable_hook()
{
    mcu_set_servo(BLTOUCH_PROBE_SERVO, BLTOUCH_DEPLOY);
    cnc_delay_ms(BLTOUCH_DELAY);
}

void mod_probe_disable_hook()
{
    mcu_set_servo(BLTOUCH_PROBE_SERVO, BLTOUCH_STOW);
    cnc_delay_ms(BLTOUCH_DELAY);
}

#endif
