/*
	Name: kinematic.h
	Description: Declares the functions needed to implement the machine kinematics and homing motion.
		This defines an opac interface that allows to adapt µCNC to different architectures/mechanics of
		different machines.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 26/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/
#ifndef KINEMATIC_H
#define KINEMATIC_H

#ifdef __cplusplus
extern "C"
{
#endif

// this should match the number of linear actuators on the machines (do not change unless you know what you are doing)
// laser PPI requires an additional stepper
#ifndef AXIS_TO_STEPPERS
#define AXIS_TO_STEPPERS AXIS_COUNT
#endif

#ifndef STEPPER_COUNT
#define STEPPER_COUNT AXIS_TO_STEPPERS
#endif

#define KINEMATIC_HOMING_ERROR_X 1
#define KINEMATIC_HOMING_ERROR_Y 2
#define KINEMATIC_HOMING_ERROR_Z 4
#define KINEMATIC_HOMING_ERROR_A 8
#define KINEMATIC_HOMING_ERROR_B 16
#define KINEMATIC_HOMING_ERROR_C 32

#include <stdint.h>

	void kinematics_init(void);

	/**
	 * @brief Converts from machine absolute coordinates to step position.
	 * This is done after computing position relative to the active coordinate system
	 *
	 * @param axis Position in world coordinates
	 * @param steps Position in steps
	 */

	void kinematics_apply_inverse(float *axis, int32_t *steps);

	/**
	 * @brief Converts from step position to machine absolute coordinates.
	 * This is done after computing position relative to the active coordinate system
	 *
	 * @param steps Position in steps
	 * @param axis Position in world coordinates
	 */
	void kinematics_apply_forward(int32_t *steps, float *axis);

	/**
	 * @brief Executes the homing motion for the machine.
	 * The homing motion for each axis is defined in the motion control.
	 * In the kinematics home function the axis order of the homing motion and other custom motions can be defined
	 *
	 * @return uint8_t Error status
	 */
	uint8_t kinematics_home(void);

	/**
	 * @brief Aplies a transformation to the position sent to planner.
	 * This is aplied only on normal and jog moves. Homing motions go directly to planner.
	 *
	 * @param axis Target in absolute coordinates
	 */
	void kinematics_apply_transform(float *axis);

	/**
	 * @brief Aplies a reverse transformation to the position returned from the planner.
	 * This is aplied only on normal and jog moves. Homing motions go directly to planner.
	 *
	 * @param axis Target in absolute coordinates
	 */
	void kinematics_apply_reverse_transform(float *axis);

		/**
	 * @brief Converts from machine absolute coordinates to step position.
	 * This calls kinematics_apply_inverse after applying any custom geometry transformation (like skew compensation)
	 *
	 * @param axis Position in world coordinates
	 * @param steps Position in steps
	 */

	void kinematics_coordinates_to_steps(float *axis, int32_t *steps);

	/**
	 * @brief Converts from step position to machine absolute coordinates.
	 * This calls kinematics_apply_forward and then recomputes any custom geometry transformation inversion (like skew compensation)
	 *
	 * @param steps Position in steps
	 * @param axis Position in world coordinates
	 */
	void kinematics_steps_to_coordinates(int32_t *steps, float *axis);

	/**
	 * @brief Checks if the desired target is inside sofware boundries
	 *
	 * @param axis Target in absolute coordinates
	 * @return true If inside boundries
	 * @return false If outside boundries
	 */
	bool kinematics_check_boundaries(float *axis);

	/**
	 * @brief Checks the motion software limits
	 * This internally calls kinematics_check_boundaries after applying machine transformations
	 * This ensure that in a transformation makes an axis travell past it's limits
	 *
	 * @param axis Target in absolute coordinates
	 * @return true If inside boundries
	 * @return false If outside boundries
	 */
	bool kinematics_check_softlimits(float *axis);

#ifdef __cplusplus
}
#endif

#endif
