/*
	Name: 
	Copyright: 
	Author: 
	Date: 23/09/19 23:14
	Description: 
*/

#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <stdint.h>

#define INTERPOLATOR_BUFFER_SIZE 2

void interpolator_init();
void interpolator_exec_planner_block();
void interpolator_update_block();
void interpolator_rt_pulse();
void interpolator_rt_pulsereset();
void interpolator_rt_integrator();

#endif
