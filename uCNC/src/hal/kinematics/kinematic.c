/*
        Name: kinematic.c
        Description: Implements all generic kinematics function calls.

        Copyright: Copyright (c) João Martins
        Author: João Martins
        Date: 09/08/2023

        µCNC is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version. Please see <http://www.gnu.org/licenses/>

        µCNC is distributed WITHOUT ANY WARRANTY;
        Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
        See the	GNU General Public License for more details.
*/

#include "../../cnc.h"

/**
 * @brief Aplies a transformation to the position sent to planner.
 * This is aplied only on normal and jog moves. Homing motions go directly to planner.
 *
 * @param axis Target in absolute coordinates
 */
void __attribute__((weak)) kinematics_apply_transform(float *axis)
{
}

/**
 * @brief Aplies a reverse transformation to the position returned from the planner.
 * This is aplied only on normal and jog moves. Homing motions go directly to planner.
 *
 * @param axis Target in absolute coordinates
 */
void __attribute__((weak)) kinematics_apply_reverse_transform(float *axis)
{
}

/**
 * @brief Converts from machine absolute coordinates to step position.
 * This calls kinematics_apply_inverse after applying any custom geometry transformation (like skew compensation)
 *
 * @param axis Position in world coordinates
 * @param steps Position in steps
 */

void kinematics_coordinates_to_steps(float *axis, int32_t *steps)
{
        // make an axis copy to preven unintended target modifications
        float axis_copy[AXIS_COUNT];
        memcpy(axis_copy, axis, sizeof(axis_copy));
        // In homing mode no kinematics modifications is applied to prevent unwanted axis movements
        if (!cnc_get_exec_state(EXEC_HOMING))
        {
                kinematics_apply_transform(axis_copy);

#ifdef ENABLE_SKEW_COMPENSATION
                // apply correction skew factors that compensate for machine axis alignemnt
                axis_copy[AXIS_X] -= axis_copy[AXIS_Y] * g_settings.skew_xy_factor;
#ifndef SKEW_COMPENSATION_XY_ONLY
                axis_copy[AXIS_X] -= axis_copy[AXIS_Z] * (g_settings.skew_xy_factor - g_settings.skew_xz_factor * g_settings.skew_yz_factor);
                axis_copy[AXIS_Y] -= axis_copy[AXIS_Z] * g_settings.skew_yz_factor;
#endif
#endif
        }

        kinematics_apply_inverse(axis_copy, steps);
}

/**
 * @brief Converts from step position to machine absolute coordinates.
 * This calls kinematics_apply_forward and then recomputes any custom geometry transformation inversion (like skew compensation)
 *
 * @param steps Position in steps
 * @param axis Position in world coordinates
 */
void kinematics_steps_to_coordinates(int32_t *steps, float *axis)
{
        kinematics_apply_forward(steps, axis);

        // In homing mode no kinematics modifications is applied to prevent unwanted axis movements
        if (!cnc_get_exec_state(EXEC_HOMING))
        {
                // perform unskew of the coordinates
#ifdef ENABLE_SKEW_COMPENSATION
                axis[AXIS_X] += axis[AXIS_Y] * g_settings.skew_xy_factor;
#ifndef SKEW_COMPENSATION_XY_ONLY
                axis[AXIS_X] += axis[AXIS_Z] * g_settings.skew_xz_factor;
                axis[AXIS_Y] += axis[AXIS_Z] * g_settings.skew_yz_factor;
#endif
#endif

                // reverse all transformation in order
                kinematics_apply_reverse_transform(axis);
        }
}

/**
 * @brief Checks the motion software limits
 * This internally calls kinematics_check_boundaries after applying machine transformations
 * This ensure that in a transformation makes an axis travell past it's limits
 *
 * @param axis Target in absolute coordinates
 * @return true If inside boundries
 * @return false If outside boundries
 */
bool kinematics_check_softlimits(float *axis)
{
        // make an axis copy to preven unintended target modifications
        float axis_copy[AXIS_COUNT];
        memcpy(axis_copy, axis, sizeof(axis_copy));
        // In homing mode no kinematics modifications is applied to prevent unwanted axis movements
        if (!cnc_get_exec_state(EXEC_HOMING))
        {
                kinematics_apply_transform(axis_copy);

#ifdef ENABLE_SKEW_COMPENSATION
                // apply correction skew factors that compensate for machine axis alignemnt
                axis_copy[AXIS_X] -= axis_copy[AXIS_Y] * g_settings.skew_xy_factor;
#ifndef SKEW_COMPENSATION_XY_ONLY
                axis_copy[AXIS_X] -= axis_copy[AXIS_Z] * (g_settings.skew_xy_factor - g_settings.skew_xz_factor * g_settings.skew_yz_factor);
                axis_copy[AXIS_Y] -= axis_copy[AXIS_Z] * g_settings.skew_yz_factor;
#endif
#endif
        }

        return kinematics_check_boundaries(axis_copy);
}