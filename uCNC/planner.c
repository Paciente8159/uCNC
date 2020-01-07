/*
	Name: planner.c
	Description: Chain planner for linear motions and acceleration/deacceleration profiles.
        It uses a similar algorithm to Grbl.
			
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 24/09/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "config.h"
#include "grbl_interface.h"
#include "mcumap.h"
#include "mcu.h"
#include "settings.h"
#include "planner.h"
#include "interpolator.h"
#include "utils.h"
#include "dio_control.h"
#include "cnc.h"

float planner_coord[AXIS_COUNT];
float planner_dir_vect[AXIS_COUNT];
static PLANNER_BLOCK planner_data[PLANNER_BUFFER_SIZE];
uint8_t planner_data_write;
uint8_t planner_data_read;
uint8_t planner_data_slots;

/*
	Planner buffer functions
*/
static inline void planner_buffer_read()
{
	planner_data_read++;
	planner_data_slots++;
	if(planner_data_read == PLANNER_BUFFER_SIZE)
	{
		planner_data_read = 0;
	}
}

static inline void planner_buffer_write()
{
	planner_data_write++;
	planner_data_slots--;
	if(planner_data_write == PLANNER_BUFFER_SIZE)
	{
		planner_data_write = 0;
	}
}

static inline uint8_t planner_buffer_next(uint8_t index)
{
	index++;
	if(index == PLANNER_BUFFER_SIZE)
	{
		index = 0;
	}
	
	return index;
}

static inline uint8_t planner_buffer_prev(uint8_t index)
{
	if(index == 0)
	{
		index = PLANNER_BUFFER_SIZE;
	}
	
	index--;
	return index;
}

bool planner_buffer_is_empty()
{
	return (planner_data_slots == PLANNER_BUFFER_SIZE);
}

bool planner_buffer_is_full()
{
	return (planner_data_slots == 0);
}

static inline void planner_buffer_clear()
{
	planner_data_write = 0;
	planner_data_read = 0;
	planner_data_slots = PLANNER_BUFFER_SIZE;
	memset(planner_data, 0, sizeof(planner_data));
}



void planner_init()
{
	memset(&planner_coord, 0, AXIS_COUNT*sizeof(float));
	memset(&planner_dir_vect, 0, AXIS_COUNT*sizeof(float));
	//resets buffer
	memset(&planner_data, 0, sizeof(planner_data));
	planner_buffer_clear();
}

void planner_clear()
{
	//clears all motions stored in the buffer
	planner_buffer_clear();
	//resets dir vector
	memset(&planner_dir_vect, 0, AXIS_COUNT*sizeof(float));
	//resyncs position with interpolator
	planner_resync_position();
}

PLANNER_BLOCK* planner_get_block()
{
	return &planner_data[planner_data_read];
}

float planner_get_block_exit_speed_sqr()
{
	//only one block in the buffer (exit speed is 0)
	if(planner_data_slots>=(PLANNER_BUFFER_SIZE-1))
		return 0;

	//exit speed = next block entry speed
	uint8_t next = planner_buffer_next(planner_data_read);

	return planner_data[next].entry_speed_sqr;
}

float planner_get_block_top_speed()
{
	/*
	Computed the junction speed
	
	At full acceleration and deacceleration we have the following equations
		v_max^2 = v_entry^2 + 2 * distance * acceleration
		v_max^2 = v_exit^2 + 2 * distance * acceleration
		
	In this case v_max^2 for acceleration and deacceleration will be the same at
	
	d_deaccel = d_total - d_start;
	
	this translates to the equation
	
	v_max = v_entry + (2 * acceleration * distance + v_exit - v_entry)/acceleration
	*/
	float exit_speed_sqr = planner_get_block_exit_speed_sqr();
	float speed_delta = exit_speed_sqr - planner_data[planner_data_read].entry_speed_sqr;
	float speed_change = 2 * planner_data[planner_data_read].acceleration * planner_data[planner_data_read].distance;
	speed_change += speed_delta;
	speed_change *= planner_data[planner_data_read].accel_inv;
	float junction_speed_sqr = planner_data[planner_data_read].entry_speed_sqr + speed_change;

	//the average speed can't ever exceed the target speed
	float target_speed_sqr = planner_data[planner_data_read].target_speed*planner_data[planner_data_read].target_speed;

	return MIN(junction_speed_sqr, target_speed_sqr);
}


void planner_discard_block()
{
	planner_buffer_read();
}

void planner_recalculate()
{
	uint8_t last = planner_data_write;
	uint8_t first = planner_data_read;
	uint8_t block = planner_data_write;
	//starts in the last added block
	//calculates the maximum entry speed of the block so that it can do a full stop in the end
	float entry_speed_sqr = 2 * planner_data[block].distance * planner_data[block].acceleration;
	planner_data[block].entry_speed_sqr = MIN(planner_data[block].entry_max_speed_sqr, entry_speed_sqr);
	//optimizes entry speeds given the current exit speed (backward pass)
	uint8_t next = block;
	block = planner_buffer_prev(block);
	
	while(!planner_data[block].optimal && block != first)
	{
		if(planner_data[block].entry_speed_sqr != planner_data[block].entry_max_speed_sqr)
		{
			entry_speed_sqr = planner_data[next].entry_speed_sqr + 2 * planner_data[block].distance * planner_data[block].acceleration;
			planner_data[block].entry_speed_sqr = MIN(planner_data[block].entry_max_speed_sqr, entry_speed_sqr);
		}
		
		next = block;
		block = planner_buffer_prev(block);
	}

	//optimizes exit speeds (forward pass)
	while(block != last)
	{
		//next block is moving at a faster speed
		if(planner_data[block].entry_speed_sqr < planner_data[next].entry_speed_sqr)
		{
			//check if the next block entry speed can be achieved
			float exit_speed_sqr = planner_data[block].entry_speed_sqr + (2 * planner_data[block].distance * planner_data[block].acceleration);
			if(exit_speed_sqr < planner_data[next].entry_speed_sqr)
			{
				//lowers next entry speed (aka exit speed) to the maximum reachable speed from current block
				//optimization achieved for this movement
				planner_data[next].entry_speed_sqr = exit_speed_sqr;
				planner_data[next].optimal = true;
			}
		}
		
		//if the executing block was updated then update the interpolator limits
		if(block == first)
		{
			interpolator_update();
		}

		block = next;
		next = planner_buffer_next(block);
	}
}

/*
	Adds a new line to the trajectory planner
	The planner is responsible for calculating the entry and exit speeds of the transitions
	It also calculates the amount of necessary steps to perform the transitions
	The trajectory planner does the following actions:
		1. Calculates the direction change of the new movement
		2. Adjusts entry speed according to the angle of the junction point
		3. Recalculates all chained segments 
*/
void planner_add_line(float *axis, float feed)
{
	planner_data[planner_data_write].dirbits = 0;
	planner_data[planner_data_write].target_speed = feed;
	planner_data[planner_data_write].optimal = false;
	planner_data[planner_data_write].acceleration = FLT_MAX;
	planner_data[planner_data_write].max_speed = FLT_MAX;
	planner_data[planner_data_write].entry_speed_sqr = 0;
	planner_data[planner_data_write].entry_max_speed_sqr = 0;

	planner_data[planner_data_write].distance = 0;
	memcpy(&(planner_data[planner_data_write].pos), axis, sizeof(planner_data[planner_data_write].pos));
	for(uint8_t i = AXIS_COUNT; i != 0; )
	{
		i--;
		planner_data[planner_data_write].dir_vect[i] = axis[i] - planner_coord[i];
		planner_data[planner_data_write].dirbits<<=1;
		if(planner_data[planner_data_write].dir_vect[i] != 0)
		{
			planner_data[planner_data_write].distance += planner_data[planner_data_write].dir_vect[i] * planner_data[planner_data_write].dir_vect[i];
			if(planner_data[planner_data_write].dir_vect[i]<0) //sets direction bits
			{
				planner_data[planner_data_write].dirbits |= 0x01;
			}
		}
	}

	//if no motion ignores planner block instruction
	if(planner_data[planner_data_write].distance == 0)
	{
		return;
	}
	
	//calculates the normalized direction vector
	//it also calculates the angle between previous direction and the current
	//this is given by the equation cos(theta) = dotprod(u,v)/(magnitude(u)*magnitude(v))
	//since normalized vector are being used (magnitude=1) this simplifies to cos(theta) = dotprod(u,v)
	//in the same loop the maximum linear speed and accel is calculated
	planner_data[planner_data_write].distance = sqrtf(planner_data[planner_data_write].distance);
	float inv_magn = 1.0f/planner_data[planner_data_write].distance;
	float cos_theta = 0;
	uint8_t dirs = planner_data[planner_data_write].dirbits;
	//uint8_t prev_index = BUFFER_WRITE_INDEX(planner_buffer);
	//prev_index = BUFFER_PREV_INDEX(planner_buffer,prev_index);
	uint8_t prev = 0;
	if(!planner_buffer_is_empty())
	{
		prev = planner_buffer_prev(planner_data_write);//BUFFER_PTR(planner_buffer, prev_index);
	}
	
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		//if axis doesn't move skip computations
		if(planner_data[planner_data_write].dir_vect[i] != 0)
		{
			planner_data[planner_data_write].dir_vect[i] *= inv_magn;
			if(!planner_buffer_is_empty())
			{
				cos_theta += planner_data[planner_data_write].dir_vect[i] * planner_data[prev].dir_vect[i];
			}
			
			float dir_axis_abs = (dirs & 0x01) ? -planner_data[planner_data_write].dir_vect[i] : planner_data[planner_data_write].dir_vect[i];
			
			//calcs maximum allowable speed for this diretion
			float axis_speed = g_settings.max_feed_rate[i] / dir_axis_abs;
			planner_data[planner_data_write].max_speed = MIN(planner_data[planner_data_write].max_speed, axis_speed);
			//calcs maximum allowable acceleration for this direction
			float axis_accel = g_settings.acceleration[i] / dir_axis_abs;
			planner_data[planner_data_write].acceleration = MIN(planner_data[planner_data_write].acceleration, axis_accel);
		}
		
		dirs>>=1;
	}

	planner_data[planner_data_write].accel_inv = 1.0f / planner_data[planner_data_write].acceleration;
	//reduces target speed if exceeds the maximum allowed speed in the current direction
	if(planner_data[planner_data_write].target_speed > planner_data[planner_data_write].max_speed)
	{
		planner_data[planner_data_write].target_speed = planner_data[planner_data_write].max_speed;
	}
	
	//sets entry and max junction speeds as if it would start and finish from a stoped state
	planner_data[planner_data_write].entry_speed_sqr = 0;
	planner_data[planner_data_write].entry_max_speed_sqr = (planner_data[planner_data_write].target_speed) * (planner_data[planner_data_write].target_speed);
	
	//if more than one move stored cals juntion speeds and recalculates speed profiles
	if(!planner_buffer_is_empty())
	{
		//calculates the junction angle with previous
		if(cos_theta > 0)
		{
			//uses the half angle identity conversion to convert from cos(theta) to tan(theta/2) where:
			//	tan(theta/2) = sqrt((1-cos(theta)/(1+cos(theta))
			//to simplify the calculations it multiplies by sqrt((1+cos(theta)/(1+cos(theta))
			//transforming the equation to sqrt((1^2-cos(theta)^2))/(1+cos(theta))
			//this way the output will be between 0<tan(theta/2)<inf
			//but if theta is 0<theta<90 the tan(theta/2) will be 0<tan(theta/2)<1
			//all angles greater than 1 that can be excluded
			planner_data[planner_data_write].angle_factor = 1.0f / (1.0f + cos_theta);
			planner_data[planner_data_write].angle_factor *= sqrt((1.0f-cos_theta*cos_theta));
		}
	
		//sets the maximum allowed speed at junction (if angle doesn't force a full stop)
		if(planner_data[planner_data_write].angle_factor < 1.0f)
		{
			float junc_speed_sqr = planner_data[prev].target_speed*(1-planner_data[planner_data_write].angle_factor);
			junc_speed_sqr *= junc_speed_sqr; //sqr speed
			planner_data[planner_data_write].entry_max_speed_sqr = MIN(planner_data[planner_data_write].entry_max_speed_sqr, junc_speed_sqr);
		}
		
		
		planner_recalculate();
	}

	//advances the buffer
	planner_buffer_write();
	//updates the current planner coordinates
	memcpy(&planner_coord, axis, sizeof(planner_coord));
}


void planner_get_position(float* axis)
{
	memcpy(axis, planner_coord, sizeof(planner_coord));
}

void planner_resync_position()
{
	//resyncs the position with the interpolator
	interpolator_get_rt_position((float*)&planner_coord);
}
