/*
	Name: kinematics.h
	Description: Declares the functions needed to implement the machine kinematics and homing motion.
		This defines an opac interface that allows to adapt uCNC to different architectures/mechanics of
		different machines.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 26/09/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/
#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <stdint.h>

/*
	Converts from machine absolute coordinates to step position.
	Note: Machine absolute coordinates go from 0 to max_axis.
	This is done after computing position relative to the active coordinate system
*/
void kinematics_apply_inverse(float* axis, uint32_t* steps);


/*
	Converts from step position to machine absolute coordinates
	Note: Machine absolute coordinates go from 0 to max_axis.
	This is done after computing position relative to the active coordinate system
*/
void kinematics_apply_forward(uint32_t* steps, float* axis);

/*
	Executes the homing motion for the given kinematic
*/
uint8_t kinematics_home();

#endif
