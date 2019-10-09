/*
	Name: 
	Copyright: 
	Author: 
	Date: 23/09/19 23:19
	Description: 
*/

#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "config.h"
#include "settings.h"
#include "planner.h"
#include "kinematics.h"
#include "utils.h"
#include "motion.h"

uint32_t g_planner_steppos[STEPPER_COUNT];
float g_planner_coord[AXIS_COUNT];
float g_planner_dir_vect[AXIS_COUNT];
/*float g_planner_dist;
float g_planner_speed;
float g_planner_exit_speed;
float g_planner_accel_dist;
float g_planner_speed;
float g_planner_accel;
uint8_t g_planner_dir_bits;
float planner_top_accel = 0;

PLANNER_MOTION planner_prev_motion;*/
PLANNER_MOTION_BLOCK g_planner_buffer[PLANNER_BUFFER_SIZE];
PLANNER_MOTION_BLOCK* g_planner_buffer_rd;
PLANNER_MOTION_BLOCK* g_planner_buffer_wt;


void planner_init()
{
	memset(&g_planner_steppos, 0, STEPPER_COUNT*sizeof(uint32_t));
	memset(&g_planner_coord, 0, AXIS_COUNT*sizeof(float));
	memset(&g_planner_dir_vect, 0, AXIS_COUNT*sizeof(float));
	//resets buffer
	memset(&g_planner_buffer, 0, PLANNER_BUFFER_SIZE * sizeof(PLANNER_MOTION_BLOCK));
	
	//creates the circular buffer (uses double linked list)
	for(uint8_t i = 1; i < PLANNER_BUFFER_SIZE; i++)
	{
		g_planner_buffer[i].prev = &g_planner_buffer[i-1];
		g_planner_buffer[i-1].next = &g_planner_buffer[i];
	}

	g_planner_buffer[0].prev = &g_planner_buffer[PLANNER_BUFFER_SIZE - 1];
	g_planner_buffer[PLANNER_BUFFER_SIZE - 1].next = &g_planner_buffer[0];

	//initializes read and write buffer
	g_planner_buffer_rd = (PLANNER_MOTION_BLOCK*)&g_planner_buffer;
	g_planner_buffer_wt = g_planner_buffer_rd;
}

bool planner_buffer_full()
{
	return (g_planner_buffer_wt->next==g_planner_buffer_rd);
}

bool planner_buffer_empty()
{
	return (g_planner_buffer_rd==g_planner_buffer_wt);
}

PLANNER_MOTION_BLOCK* planner_get_block()
{
	return 	g_planner_buffer_rd;	
}

float planner_get_block_exit_speed_sqr()
{
	if(g_planner_buffer_rd==g_planner_buffer_wt)
	{
		return 0;
	}
	
	return (g_planner_buffer_rd->next)->entry_speed_sqr;
}

void planner_recalculate()
{
	PLANNER_MOTION_BLOCK *block = g_planner_buffer_wt;

	//buffer only has one block or is fully optimized so no calculations to be done
	if(planner_buffer_empty() || g_planner_buffer_wt->optimal)
	{
		return;
	}

	//optimizes entry speeds given the current exit speed
	float entry_speed_sqr = 2 * block->distance * block->acceleration;
	block->entry_speed_sqr = MIN(block->entry_max_speed_sqr, entry_speed_sqr);
	PLANNER_MOTION_BLOCK *next;
	while(!block->optimal && block!=g_planner_buffer_rd)
	{
		next = block;
		block = next->prev;
		if(block->entry_speed_sqr != block->entry_max_speed_sqr)
		{
			entry_speed_sqr = next->entry_speed_sqr + 2 * block->distance * block->acceleration;
			block->entry_speed_sqr = MIN(block->entry_max_speed_sqr, entry_speed_sqr);
		}
	}
	
	while(block != g_planner_buffer_wt)
	{
		//next block is moving at a faster speed
		if(block->entry_speed_sqr < next->entry_speed_sqr)
		{
			//check if the next block entry speed can be achieved
			float exit_speed_sqr = block->entry_speed_sqr + (2 * block->distance * block->acceleration);
			if(exit_speed_sqr < next->entry_speed_sqr)
			{
				//lowers next entry speed
				//optimization achieved for this movement
				next->entry_speed_sqr = exit_speed_sqr;
				block->optimal = true;
			}
		}
		
		block = next;
		next = block->next;
	}
}

/*
	Adds a new line to the trajectory planner
	The planner is responsible for calculating the entry and exit speeds of the transitions
	It also calculates the amount of necessary steps to perform the transitions
	The trajectory planner does the following actions:
		1. Calculates the direction change of the new movement
*/
void planner_add_line(float* axis, float feed)
{
	g_planner_buffer_wt->dirbits = 0;
	g_planner_buffer_wt->target_speed = feed;
	g_planner_buffer_wt->optimal = false;
	g_planner_buffer_wt->acceleration = FLT_MAX;
	g_planner_buffer_wt->max_speed = FLT_MAX;
	g_planner_buffer_wt->entry_speed_sqr = 0;
	g_planner_buffer_wt->entry_max_speed_sqr = 0;

	g_planner_buffer_wt->distance = 0;
	for(uint8_t i = AXIS_COUNT; i != 0; )
	{
		i--;
		g_planner_buffer_wt->dir_vect[i] = axis[i] - g_planner_coord[i];
		g_planner_buffer_wt->dirbits<<=1;
		if(g_planner_buffer_wt->dir_vect[i] != 0)
		{
			g_planner_buffer_wt->distance += g_planner_buffer_wt->dir_vect[i] * g_planner_buffer_wt->dir_vect[i];
			if(g_planner_buffer_wt->dir_vect[i]<0) //sets direction bits
			{
				g_planner_buffer_wt->dirbits |= 0x01;
			}
		}
	}

	//if no motion ignores planner block instruction
	if(g_planner_buffer_wt->distance == 0)
	{
		return;
	}
	
	uint32_t steps_pos[STEPPER_COUNT];
	kinematics_apply_inverse(axis, (uint32_t*)&steps_pos);
	uint8_t dirs = g_planner_buffer_wt->dirbits;
	for(uint8_t i = 0; i < STEPPER_COUNT; i++)
	{
		g_planner_buffer_wt->steps[i] = (dirs & 0x01) ? (g_planner_steppos[i]-steps_pos[i]) : (steps_pos[i]-g_planner_steppos[i]);
		dirs>>=1;
		if(g_planner_buffer_wt->totalsteps < g_planner_buffer_wt->steps[i])
		{
			g_planner_buffer_wt->totalsteps = g_planner_buffer_wt->steps[i];			
		}
	}

	//calculates the normalized direction vector
	//it also calculates the angle between previous direction and the current
	//this is given by the equation cos(theta) = dotprod(u,v)/(magnitude(u)*magnitude(v))
	//since normalized vector are being used (magnitude=1) this simplifies to cos(theta) = dotprod(u,v)
	//in the same loop the maximum linear speed and accel is calculated
	g_planner_buffer_wt->distance = sqrtf(g_planner_buffer_wt->distance);
	float inv_magn = 1.0f/g_planner_buffer_wt->distance;
	float cos_theta = 0;
	dirs = g_planner_buffer_wt->dirbits;
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		//if axis doesn't move skip computations
		if(g_planner_buffer_wt->dir_vect[i] != 0)
		{
			g_planner_buffer_wt->dir_vect[i] *= inv_magn;
			if(g_planner_buffer_wt != g_planner_buffer_rd)
			{
				cos_theta += g_planner_buffer_wt->dir_vect[i] * (g_planner_buffer_wt->prev)->dir_vect[i];
			}
			
			float dir_axis_abs = (dirs & 0x01) ? -g_planner_buffer_wt->dir_vect[i] : g_planner_buffer_wt->dir_vect[i];
			
			//calcs maximum allowable speed for this diretion
			float axis_speed = g_settings.max_speed[i] / dir_axis_abs;
			g_planner_buffer_wt->max_speed = MIN(g_planner_buffer_wt->max_speed, axis_speed);
			//calcs maximum allowable acceleration for this direction
			float axis_accel = g_settings.max_accel[i] / dir_axis_abs;
			g_planner_buffer_wt->acceleration = MIN(g_planner_buffer_wt->acceleration, axis_accel);
		}
		
		dirs>>=1;
	}

	//reduces target speed if exceeds the maximum allowed speed in the current direction
	if(g_planner_buffer_wt->target_speed > g_planner_buffer_wt->max_speed)
	{
		g_planner_buffer_wt->target_speed = g_planner_buffer_wt->max_speed;
	}
	
	//sets entry and max junction speeds as if it would start and finish from a stoped state
	g_planner_buffer_wt->entry_speed_sqr = 0;
	g_planner_buffer_wt->entry_max_speed_sqr = 0;
	
	//if more than one move stored cals juntion speeds and recalculates speed profiles
	if(g_planner_buffer_wt != g_planner_buffer_rd)
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
			g_planner_buffer_wt->angle_factor = 1.0f / (1.0f + cos_theta);
			g_planner_buffer_wt->angle_factor *= sqrt((1.0f-cos_theta*cos_theta));
		}
	
		//sets the maximum allowed speed at junction (if angle doesn't force a full stop)
		if(g_planner_buffer_wt->angle_factor < 1.0f)
		{
			g_planner_buffer_wt->entry_max_speed_sqr = g_planner_buffer_wt->target_speed*g_planner_buffer_wt->target_speed;
			float junc_speed_sqr = (g_planner_buffer_wt->prev)->target_speed*(1-g_planner_buffer_wt->angle_factor);
			junc_speed_sqr *= junc_speed_sqr; //sqr speed
			g_planner_buffer_wt->entry_max_speed_sqr = MIN(g_planner_buffer_wt->entry_max_speed_sqr, junc_speed_sqr);
		}
		
		
		planner_recalculate();
	}
	else
	{
		//first motion in buffer. Not optimizable
		g_planner_buffer_wt->optimal = true;
	}

	//advances the buffer
	g_planner_buffer_wt = g_planner_buffer_wt->next;
	//updates the current planner coordinates
	memcpy(&g_planner_coord, axis, AXIS_COUNT*sizeof(float));
}

/*
void planner_add_line(float* axis, float feed)
{
	#ifdef DEBUGMODE
		board_startPerfCounter();
	#endif
	
	uint32_t steps_pos[STEPPER_COUNT];
	uint32_t stepspeeds[STEPPER_COUNT];
	uint32_t stepaccels[STEPPER_COUNT];
	float axis_speeds[AXIS_COUNT];
	float axis_accels[AXIS_COUNT];
	float dir_vect[AXIS_COUNT];
	float axis_max_accel = planner_top_accel;
	float axis_max_speed = feed;
	float axis_max_accel_fact = 0;
	PLANNER_MOTION new_motion;
	#ifdef DEBUGMODE
		uint16_t count = 0;
	#endif
	
	uint8_t isfullstop = 1;
	uint8_t dirs;
	
	//initialize new_motion
	memset(&new_motion, 0, sizeof(PLANNER_MOTION));
	
	//when adding a line the first thing to do is to calculate the direction of the movement
	//and compare it to the previous movement
	//this will allow to compute the previous exit speed/next entry speed profiles
	float magnitude = 0;
	for(uint8_t i = AXIS_COUNT; i != 0; )
	{
		i--;
		dir_vect[i] = axis[i] - g_planner_coord[i];
		new_motion.dirs<<=1;
		if(dir_vect[i] != 0)
		{
			magnitude += dir_vect[i] * dir_vect[i];
			if(dir_vect[i]<0) //sets direction bits
			{
				new_motion.dirs |= 0x01;
			}
		}
	}
	
	if(magnitude==0) //if no movement exit
	{
		#ifdef DEBUGMODE
			count = board_stopPerfCounter();
			printf("planner no motion: ");
			printf(" in %u cycles\n", count);
		#endif
		return;
	}
	
	//calculates the normalized direction vector
	//it also calculates the angle between previous direction and the current
	//this is given by the equation cos(theta) = dotprod(u,v)/(magnitude(u)*magnitude(v))
	//since normalized vector are being used (magnitude=1) this simplifies to cos(theta) = dotprod(u,v)
	//in the same loop the maximum linear speed and accel is calculated
	magnitude = sqrtf(magnitude);
	float inv_magn = 1.0f/magnitude;
	float cos_theta = 0;
	dirs = new_motion.dirs;
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		//if axis doesn't move skip computations
		if(dir_vect[i] != 0)
		{
			dir_vect[i] *= inv_magn;
			cos_theta += dir_vect[i] * g_planner_dir_vect[i];
			float dir_axis_abs = (dirs & 0x01) ? -dir_vect[i] : dir_vect[i];
			
			float axis_speed = dir_axis_abs * axis_max_speed;
			if(axis_speed > g_settings.max_speed[i])//if target speed exceeds the speed allowed for the axis lowers the feed rate
			{
				axis_max_speed = g_settings.max_speed[i] / dir_axis_abs;
			}
			
			float axis_accel = axis_max_accel * dir_axis_abs;
			if(axis_accel > g_settings.max_accel[i]) //determines maximum accel rate for all axis
			{
				axis_max_accel = g_settings.max_accel[i] / dir_axis_abs;
			}
		}
		
		dirs>>=1;
	}
	
	
	float dir_angle = 0;
	//if cos is negative the angle is greater then 90� so the previous movement must end with speed = 0
	//and there is no point in calculating the angle
	if(cos_theta > 0)
	{
		//is not a +90� angle corner
		isfullstop = 0;
		//uses the half angle identity conversion to convert from cos(theta) to tan(theta/2)
		//instead of sqrt((1-cos(theta)/(1+cos(theta))
		//to simplify the calculations it multiplies by sqrt((1+cos(theta)/(1+cos(theta))
		//transforming the equation to sqrt((1^2-cos(theta)^2))/(1+cos(theta))
		//this way the output will be between 0<tan(theta/2)<inf
		//but if theta is 0�<theta<90� the tan(theta/2) will be 0<tan(theta/2)<1
		//all angles greater than that can be excluded
		dir_angle = 1.0f + cos_theta;
		
		//if angle is between 0�<theta<90� calculates tan(theta/2) by using trig half angle identities
		//a direct aproximation can be used to calc the arctan(theta) by considering arctan(theta) = theta * Pi / 4 with an aprox. error of +-5�;
		//since will output a tan values between 0<tan(theta/2)<1 can be used directly
		//more accurate methods can be used but at the expense of math operations
		dir_angle = sqrt((1.0f-cos_theta*cos_theta))/dir_angle;
	}
	
	//finds new junction speed for previous motion
	float prev_exit_speed_sq;
	
	if(!isfullstop)
	{
		float prev_exit_speed = axis_max_speed * (1.0f - dir_angle);
		prev_exit_speed = MIN(prev_exit_speed,g_planner_speed);
		//the new movement is faster then the previous even with the juntion and there is room to accel more
		if(g_planner_exit_speed < prev_exit_speed)
		{
			//no deaccel
			planner_prev_motion.deaccel_from = 0;
			//calc new max speed of previous move
			float prev_inv_acc = 1.0f / g_planner_accel;
			//calcs distance to accel to fullspeed
			float t_accel = (prev_exit_speed - g_planner_exit_speed)*prev_inv_acc;
			float new_accel_dist = g_planner_accel_dist + t_accel*(g_planner_exit_speed+0.5f*t_accel*g_planner_accel);
			//can't exceed the full distance minus the previous acceleration distance
			if(new_accel_dist>g_planner_dist)
			{
				planner_prev_motion.accel_until = planner_prev_motion.totalsteps;
				new_accel_dist = g_planner_dist - g_planner_accel_dist;
				g_planner_exit_speed = sqrtf(2.0f*new_accel_dist*g_planner_accel + g_planner_exit_speed*g_planner_exit_speed);
			}
			else
			{
				//find ratio for new distance
				new_accel_dist = new_accel_dist/g_planner_dist;
				//new accel
				planner_prev_motion.accel_until = (uint32_t)roundf(planner_prev_motion.totalsteps*new_accel_dist);
				g_planner_exit_speed = prev_exit_speed;
			}
		}
		else if(prev_exit_speed > 0)
		{
			isfullstop = 0;
			prev_exit_speed_sq = prev_exit_speed * prev_exit_speed;
			//must update previous exit speed
			float prev_deacc = g_planner_speed*g_planner_speed - prev_exit_speed_sq;
			prev_deacc *= 0.5f / (g_planner_accel * g_planner_dist);//relative percentage of deaccl from the hole move
			planner_prev_motion.deaccel_from = (uint32_t)roundf(planner_prev_motion.totalsteps * (1.0f - prev_deacc));
		}
		else
		{
			isfullstop = 1;
		}
	}
	
	//the previous motion is optimized and goes to the integrator

	//stores the current movement target speed
	g_planner_speed = axis_max_speed;
	
	//checks if it can accelerate to target speed and do a full stop.
	//if not reduces top target speed
	//float dt = 1.0f/magnitude;
	float inv_acc = 0.5f / axis_max_accel;
	float vf_sqr = axis_max_speed * axis_max_speed;
	//deaccels from full speed to complete stop
	float d_deacc = inv_acc * vf_sqr;
	//same for accel
	float d_acc = d_deacc;
	//if previous move doesn't end in a full stop recalcs the accel
	if(!isfullstop)
	{
		d_acc = inv_acc * (vf_sqr - prev_exit_speed_sq);
	}

	//if it can't reach full speed recalcs the maximum achievable speed
	if(d_acc + d_deacc > magnitude)
	{
		axis_max_speed = axis_max_accel * sqrt(2 * magnitude * inv_acc);
		d_acc = d_deacc = 0.5f * magnitude;
	}
	
	//reconverts linear speed and accel to a vector
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		axis_speeds[i] = axis_max_speed * dir_vect[i];
		axis_accels[i] = axis_max_accel * dir_vect[i];
	}

	//calculates the inverse kinematics for the position, speeds and accel
	kinematics_apply_inverse(axis, (uint32_t*)&steps_pos);
	kinematics_apply_inverse((float*)&axis_speeds, (uint32_t*)&stepspeeds);
	kinematics_apply_inverse((float*)&axis_accels, (uint32_t*)&stepaccels);

	//calculates the number of required steps to execute step rate and step rate accel
	dirs = new_motion.dirs;
	for(uint8_t i = 0; i < STEPPER_COUNT; i++)
	{
		new_motion.steps[i] = (dirs & 0x01) ? (g_planner_pos[i]-steps_pos[i]) : (steps_pos[i]-g_planner_pos[i]);
		dirs>>=1;
		if(new_motion.totalsteps < new_motion.steps[i])
		{
			new_motion.totalsteps = new_motion.steps[i];
			new_motion.speed_rate = (uint16_t)stepspeeds[i];
			new_motion.accel_rate = (uint16_t)stepaccels[i];			
		}
	}

	//calculates the steps needded for accel and deaccel
	new_motion.accel_until = (uint32_t)round(new_motion.totalsteps*d_acc*inv_magn);
	new_motion.deaccel_from = (uint32_t)round(new_motion.totalsteps*d_deacc*inv_magn);
	
	//updates all variables for next pass
	memcpy(&planner_prev_motion, &new_motion, sizeof(PLANNER_MOTION));
	memcpy(&g_planner_pos, &steps_pos, STEPPER_COUNT*sizeof(uint32_t));
	memcpy(&g_planner_coord, axis, AXIS_COUNT*sizeof(float));
	memcpy(&g_planner_dir_vect, &dir_vect, AXIS_COUNT*sizeof(float));
	//stores the exit speed
	//this speed is the actual top speed reached by the motion after acceleration
	g_planner_exit_speed = axis_max_speed;
	g_planner_accel = axis_max_accel;
	g_planner_dist = magnitude;
	g_planner_accel_dist = magnitude - d_acc;
	
	#ifdef DEBUGMODE
		count = board_stopPerfCounter();
		printf("planner motion: ");
		printf("in %u cycles\n", count);
	#endif
}
*/	

/*
	Idea for the planner arc function
	find arc initial direction given the previous move
	break the arc in quadrants
	accel and deaccel will be the same as in linemovemnt
	for axis of arc the contribution will be the same
	the magnitude of the each sub arc will be r
	for other axis must be determined as it wold for a line
	at this point the gcode must have checked if plane is supported
	to find normal use the 2d rule were normal of [dx,dy] = [dy,-dx](clockwise) or [-dy,dx](counterclockwise)
	
	
	PROBLEM:CAN ONLY EXECUTE BRESENHAM ARC WITH CARTESIAN
*/
void planner_add_arc(float* axis, float* center, uint8_t clockwise, uint8_t plane, float feed)
{
	float dir_vect[AXIS_COUNT];

	//calculate radius start vector
	float magnitude = 0;
	memset(&dir_vect, 0, AXIS_COUNT*sizeof(float));
	switch(plane)
	{
		case 0:
			dir_vect[AXIS_X] = center[AXIS_X] - axis[AXIS_X];
			dir_vect[AXIS_Y] = center[AXIS_Y] - axis[AXIS_Y];
			break;
		case 1:
		case 2:
			break;
	}
	
}
