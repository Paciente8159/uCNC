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

#define INTERPOLATOR_BUFFER_SIZE 10

void interpolator_init();
void interpolator_execute();
void interpolator_update();
void interpolator_step();
void interpolator_stepReset();

#endif
