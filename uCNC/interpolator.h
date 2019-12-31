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

//the amount of motion precomputed and stored for the step generator is never less then
//the size of the buffer x time window size
//in this case the buffer never holds less then 100ms of motions

#define F_INTEGRATOR 100 //integrator calculates 10ms time windows
#define INTERPOLATOR_BUFFER_SIZE 10 //number of windows in the buffer


void interpolator_init();
void interpolator_run();
void interpolator_update();
bool interpolator_buffer_is_full();
void interpolator_step_isr();
void interpolator_step_reset_isr();
void interpolator_stop();
void interpolator_clear();
void interpolator_get_rt_position(float* axis);
void interpolator_reset_rt_position();

#endif
