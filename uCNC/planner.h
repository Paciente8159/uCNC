/*
	Name: 
	Copyright: 
	Author: 
	Date: 23/09/19 23:14
	Description: 
*/

#ifndef PLANNER_H
#define PLANNER_H

void planner_init();
void planner_add_line(float* axis, float feed);
void planner_add_curve(float* axis, float* center, float feed);

#endif

