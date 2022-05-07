/*
    Name: spindle_relay.c
    Description: Defines a spindle tool using a brushless motor and a ESC controller

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 05/05/2022

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
#define SPINDLE_SERVO SERVO0
#define SPINDLE_POWER_RELAY DOUT0

// Uncomment below if you want the coolant pins enabled.
#define COOLANT_MIST_EN DOUT1
#define COOLANT_FLOOD_EN DOUT2

#define THROTTLE_DOWN 50
#define THROTTLE_NEUTRAL 127
#define THROTTLE_FULL 255
#define THROTTLE_RANGE (THROTTLE_FULL - THROTTLE_DOWN)

static uint8_t spindle_speed;

void spindle_besc_startup()
{

// do whatever routine you need to do here to arm the ESC
#if !(SPINDLE_POWER_RELAY < 0)
#if !(SPINDLE_SERVO < 0)
  mcu_set_servo(SPINDLE_SERVO, THROTTLE_NEUTRAL);
#endif
  mcu_set_output(SPINDLE_POWER_RELAY);
  cnc_delay_ms(1000);
#if !(SPINDLE_SERVO < 0)
  mcu_set_servo(SPINDLE_SERVO, THROTTLE_DOWN);
#endif
  cnc_delay_ms(2000);
#endif
}

void spindle_besc_shutdown()
{
#if !(SPINDLE_POWER_RELAY < 0)
  mcu_clear_output(SPINDLE_POWER_RELAY);
#endif
}

void spindle_besc_set_speed(uint8_t value, bool invert)
{

  if (!value || invert)
  {
#if !(SPINDLE_SERVO < 0)
    mcu_set_servo(SPINDLE_SERVO, THROTTLE_DOWN);
#endif
  }
  else
  {
#if !(SPINDLE_SERVO < 0)
    uint16_t scale = value * THROTTLE_RANGE;
    uint8_t new_val = (0xFF & (scale >> 8)) + THROTTLE_DOWN;
    mcu_set_servo(SPINDLE_SERVO, new_val);
#endif
  }

  spindle_speed = (invert) ? 0 : value;
}

void spindle_besc_set_coolant(uint8_t value)
{
  SET_COOLANT(COOLANT_FLOOD_EN, COOLANT_MIST_EN, value);
}

uint8_t spindle_besc_get_speed(void)
{
  return spindle_speed;
}

const tool_t __rom__ spindle_besc = {
    .startup_code = &spindle_besc_startup,
    .shutdown_code = &spindle_besc_shutdown,
    .set_speed = &spindle_besc_set_speed,
    .set_coolant = &spindle_besc_set_coolant,
#if PID_CONTROLLERS > 0
    .pid_update = NULL,
    .pid_error = NULL,
#endif
    .get_speed = &spindle_besc_get_speed};
