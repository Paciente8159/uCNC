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
#include <stdbool.h>
void interpolator_init();
bool interpolator_is_buffer_full();
void interpolator_run();
void interpolator_update();
void interpolator_step_isr();
void interpolator_step_reset_isr();
void interpolator_stop();
void interpolator_clear();
void interpolator_get_rt_position(float* axis);
void interpolator_reset_rt_position();
float interpolator_get_rt_feed();

#endif
