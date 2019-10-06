/*
	Name: kinematics.h
	Copyright: 
	Author: Jo�o Martins
	Date: 23/09/19 23:25
	Description: uCNC kinematic module
	Defines the kinematics module interface
*/
#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <stdint.h>

/*
	Initializes kinematics
*/
void kinematics_init();

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

#endif
