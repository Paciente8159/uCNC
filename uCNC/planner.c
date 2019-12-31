/*
	Name: planner.c - chain planner for linear motions and acceleration/deacceleration profiles
	Copyright: 2019 João Martins
	Author: João Martins
	Date: Oct/2019
	Description: uCNC is a free cnc controller software designed to be flexible and
	portable to several	microcontrollers/architectures.
	uCNC is a FREE SOFTWARE under the terms of the GPLv3 (see <http://www.gnu.org/licenses/>).
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
#include "ringbuffer.h"
#include "utils.h"
#include "trigger_control.h"
#include "cnc.h"

float g_planner_coord[AXIS_COUNT];
float g_planner_dir_vect[AXIS_COUNT];
static PLANNER_BLOCK g_planner_data[PLANNER_BUFFER_SIZE];
//ringbuffer_t g_planner_buffer_ring;
buffer_t g_planner_buffer;

void planner_init()
{
	memset(&g_planner_coord, 0, AXIS_COUNT*sizeof(float));
	memset(&g_planner_dir_vect, 0, AXIS_COUNT*sizeof(float));
	//resets buffer
	memset(&g_planner_data, 0, sizeof(PLANNER_BLOCK)*PLANNER_BUFFER_SIZE);
	g_planner_buffer = buffer_init(&g_planner_data, sizeof(PLANNER_BLOCK), PLANNER_BUFFER_SIZE);
}

void planner_clear()
{
	//clears all motions stored in the buffer
	buffer_clear(g_planner_buffer);
	//resets dir vector
	memset(&g_planner_dir_vect, 0, AXIS_COUNT*sizeof(float));
	//resyncs position with interpolator
	planner_resync_position();
}

bool planner_buffer_full()
{
	return is_buffer_full(g_planner_buffer);
}

bool planner_buffer_empty()
{
	return is_buffer_empty(g_planner_buffer);
}

PLANNER_BLOCK* planner_get_block()
{
	return buffer_get_first(g_planner_buffer);
}

float planner_get_block_exit_speed_sqr()
{
	//pointer to first buffer
	PLANNER_BLOCK *m = buffer_get_first(g_planner_buffer);
	
	//only one block in the buffer (exit speed is 0)
	if(m==buffer_get_last(g_planner_buffer))
		return 0;

	//exit speed = next block entry speed
	m = buffer_get_next(g_planner_buffer, m);
	
	return m->entry_speed_sqr;
}

float planner_get_block_top_speed(float exit_speed_sqr)
{
	//pointer to first buffer
	PLANNER_BLOCK *m = buffer_get_first(g_planner_buffer);
	
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

	float speed_delta = exit_speed_sqr - m->entry_speed_sqr;
	float speed_change = 2 * m->acceleration * m->distance;
	speed_change += speed_delta;
	speed_change *= m->accel_inv;
	float junction_speed_sqr = m->entry_speed_sqr + speed_change;

	//the average speed can't ever exceed the target speed
	float target_speed_sqr = m->target_speed*m->target_speed;

	return MIN(junction_speed_sqr, target_speed_sqr);
}
//
///*float planner_get_intersection_distance(float exit_speed_sqr)
//{
//	//pointer to first buffer
//	PLANNER_BLOCK *m = buffer_get_first(g_planner_buffer);
//	
//	/*
//	Computed the junction speed
//	
//	At full acceleration and deacceleration we have the following equations
//		v_max^2 = v_entry^2 + 2 * distance * acceleration
//		v_max^2 = v_exit^2 + 2 * distance * acceleration
//		
//	In this case v_max^2 for acceleration and deacceleration will be the same at
//	
//	v_max^2 = v_entry^2 + 2 * distance_entry * acceleration
//	v_max^2 = v_exit^2 + 2 * distance_exit * acceleration
//	
//	this translates to the equation
//	
//	distance_offset = distance_entry - distance_exit = (v_exit^2 - v_entry^2)/(2 * acceleration)
//	
//	the junction distance will be
//	distance_total / 2 + distance_offset
//	*/
//
//	float speed_delta = exit_speed_sqr - m->entry_speed_sqr;
//	float distance_offset = 0.5f * speed_delta * m->accel_inv;
//	return 0.5f * (m->distance + distance_offset);
//}

void planner_discard_block()
{
	buffer_read(g_planner_buffer, NULL);
}

void planner_recalculate()
{
	PLANNER_BLOCK *last = buffer_get_next_free(g_planner_buffer);
	PLANNER_BLOCK *first = buffer_get_first(g_planner_buffer);
	
	//starts from the last block
	PLANNER_BLOCK *block = last;
	
	
	//starts in the last added block
	//calculates the maximum entry speed of the block so that it can do a full stop in the end
	float entry_speed_sqr = 2 * block->distance * block->acceleration;
	block->entry_speed_sqr = MIN(block->entry_max_speed_sqr, entry_speed_sqr);
	//optimizes entry speeds given the current exit speed (backward pass)
	PLANNER_BLOCK *next = block;
	block = buffer_get_prev(g_planner_buffer, next);
	
	while(!block->optimal && block != first)
	{
		if(block->entry_speed_sqr != block->entry_max_speed_sqr)
		{
			entry_speed_sqr = next->entry_speed_sqr + 2 * block->distance * block->acceleration;
			block->entry_speed_sqr = MIN(block->entry_max_speed_sqr, entry_speed_sqr);
		}
		
		next = block;
		block = buffer_get_prev(g_planner_buffer, next);
	}

	//optimizes exit speeds (forward pass)
	while(block != last)
	{
		//next block is moving at a faster speed
		if(block->entry_speed_sqr < next->entry_speed_sqr)
		{
			//check if the next block entry speed can be achieved
			float exit_speed_sqr = block->entry_speed_sqr + (2 * block->distance * block->acceleration);
			if(exit_speed_sqr < next->entry_speed_sqr)
			{
				//lowers next entry speed (aka exit speed) to the maximum reachable speed from current block
				//optimization achieved for this movement
				next->entry_speed_sqr = exit_speed_sqr;
				next->optimal = true;
			}
		}
		
		//if the executing block was updated then update the interpolator limits
		if(block == first)
		{
			interpolator_update();
		}

		block = next;
		next = buffer_get_next(g_planner_buffer, block);
	}
}

/*
	Adds a new line to the trajectory planner
	The planner is responsible for calculating the entry and exit speeds of the transitions
	It also calculates the amount of necessary steps to perform the transitions
	The trajectory planner does the following actions:
		1. Calculates the direction change of the new movement
*/
void planner_add_line(float *axis, float feed)
{
	/*if(tc_get_controls(ESTOP_MASK))
	{
		cnc_kill();
		return;
	}
	*/
	PLANNER_BLOCK *m = buffer_get_next_free(g_planner_buffer);//buffer_write(g_planner_buffer, NULL);//BUFFER_WRITE_PTR(g_planner_buffer);
	m->dirbits = 0;
	m->target_speed = feed;
	m->optimal = false;
	m->acceleration = FLT_MAX;
	m->max_speed = FLT_MAX;
	m->entry_speed_sqr = 0;
	m->entry_max_speed_sqr = 0;

	m->distance = 0;
	memcpy(&(m->pos), axis, sizeof(m->pos));
	for(uint8_t i = AXIS_COUNT; i != 0; )
	{
		i--;
		m->dir_vect[i] = axis[i] - g_planner_coord[i];
		m->dirbits<<=1;
		if(m->dir_vect[i] != 0)
		{
			m->distance += m->dir_vect[i] * m->dir_vect[i];
			if(m->dir_vect[i]<0) //sets direction bits
			{
				m->dirbits |= 0x01;
			}
		}
	}

	//if no motion ignores planner block instruction
	if(m->distance == 0)
	{
		return;
	}
	
	//calculates the normalized direction vector
	//it also calculates the angle between previous direction and the current
	//this is given by the equation cos(theta) = dotprod(u,v)/(magnitude(u)*magnitude(v))
	//since normalized vector are being used (magnitude=1) this simplifies to cos(theta) = dotprod(u,v)
	//in the same loop the maximum linear speed and accel is calculated
	m->distance = sqrtf(m->distance);
	float inv_magn = 1.0f/m->distance;
	float cos_theta = 0;
	uint8_t dirs = m->dirbits;
	//uint8_t prev_index = BUFFER_WRITE_INDEX(g_planner_buffer);
	//prev_index = BUFFER_PREV_INDEX(g_planner_buffer,prev_index);
	PLANNER_BLOCK *prev = NULL;
	if(!is_buffer_empty(g_planner_buffer))
	{
		prev = buffer_get_prev(g_planner_buffer, m);//BUFFER_PTR(g_planner_buffer, prev_index);
	}
	
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		//if axis doesn't move skip computations
		if(m->dir_vect[i] != 0)
		{
			m->dir_vect[i] *= inv_magn;
			if(!is_buffer_empty(g_planner_buffer))
			{
				cos_theta += m->dir_vect[i] * prev->dir_vect[i];
			}
			
			float dir_axis_abs = (dirs & 0x01) ? -m->dir_vect[i] : m->dir_vect[i];
			
			//calcs maximum allowable speed for this diretion
			float axis_speed = g_settings.max_feed_rate[i] / dir_axis_abs;
			m->max_speed = MIN(m->max_speed, axis_speed);
			//calcs maximum allowable acceleration for this direction
			float axis_accel = g_settings.acceleration[i] / dir_axis_abs;
			m->acceleration = MIN(m->acceleration, axis_accel);
		}
		
		dirs>>=1;
	}

	m->accel_inv = 1.0f / m->acceleration;
	//reduces target speed if exceeds the maximum allowed speed in the current direction
	if(m->target_speed > m->max_speed)
	{
		m->target_speed = m->max_speed;
	}
	
	//sets entry and max junction speeds as if it would start and finish from a stoped state
	m->entry_speed_sqr = 0;
	m->entry_max_speed_sqr = (m->target_speed) * (m->target_speed);
	
	//if more than one move stored cals juntion speeds and recalculates speed profiles
	if(!is_buffer_empty(g_planner_buffer))
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
			m->angle_factor = 1.0f / (1.0f + cos_theta);
			m->angle_factor *= sqrt((1.0f-cos_theta*cos_theta));
		}
	
		//sets the maximum allowed speed at junction (if angle doesn't force a full stop)
		if(m->angle_factor < 1.0f)
		{
			float junc_speed_sqr = prev->target_speed*(1-m->angle_factor);
			junc_speed_sqr *= junc_speed_sqr; //sqr speed
			m->entry_max_speed_sqr = MIN(m->entry_max_speed_sqr, junc_speed_sqr);
		}
		
		
		planner_recalculate();
	}
	/*else
	{
		//first motion in buffer. Not optimizable
		m->optimal = true;
	}*/

	//advances the buffer
	buffer_write(g_planner_buffer, NULL);
	//updates the current planner coordinates
	memcpy(&g_planner_coord, axis, AXIS_COUNT*sizeof(float));
}


float* planner_get_position()
{
	return (float*)&g_planner_coord;
}

void planner_resync_position()
{
	//resyncs the position with the interpolator
	interpolator_get_rt_position((float*)&g_planner_coord);
}
