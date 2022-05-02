/*
    Name: spindle_relay.c
    Description: Defines a spindle tool using DOUT to enable a digital pin for µCNC.
                 Defines a coolant output using DOUT1.

    Copyright: Copyright (c) João Martins
    Author: James Harton
    Date: 1/5/2022

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

/**
 * Configures a simple spindle control with digital pins for tool forward (`M3`) and reverse (`M4`).
 * There is also provision for digital pins for coolant mist (`M7`) and flood (`M8`).
 *
 * */

// Spindle enable pins.  You can set these to the same pin if required.
#define SPINDLE_FWD_EN DOUT0
#define SPINDLE_REV_EN DOUT1

// Uncomment below if you want the coolant pins enabled.
// #define ENABLE_COOLANT
#ifdef ENABLE_COOLANT
#define COOLANT_MIST_EN DOUT2
#define COOLANT_FLOOD_EN DOUT3
#endif

static uint8_t spindle_speed;

void spindle_relay_set_speed(uint8_t value, bool invert)
{

  if (value == 0)
  {
    mcu_clear_output(SPINDLE_FWD_EN);
#if SPINDLE_FWD_EN != SPINDLE_REV_EN
    mcu_clear_output(SPINDLE_REV_EN);
#endif
  }
  else if (invert)
  {
    mcu_set_output(SPINDLE_REV_EN);
  }
  else
  {
    mcu_set_output(SPINDLE_FWD_EN);
  }

  spindle_speed = value;
}

void spindle_relay_set_coolant(uint8_t value)
{
#ifdef ENABLE_COOLANT
  SET_COOLANT(COOLANT_FLOOD_EN, COOLANT_MIST_EN, value);
#endif
}

uint8_t spindle_relay_get_speed(void)
{
  return spindle_speed;
}

const tool_t __rom__ spindle_relay = {
    .startup_code = NULL,
    .shutdown_code = NULL,
    .set_speed = &spindle_relay_set_speed,
    .set_coolant = &spindle_relay_set_coolant,
    .get_speed = &spindle_relay_get_speed};
