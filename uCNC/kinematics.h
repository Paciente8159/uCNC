/*
	Name: 
	Copyright: 
	Author: 
	Date: 23/09/19 23:25
	Description: 
*/
#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <stdint.h>

void kinematics_apply_inverse(float* axis, uint32_t* steps);
void kinematics_apply_forward(uint32_t* steps, float* axis);

#endif
