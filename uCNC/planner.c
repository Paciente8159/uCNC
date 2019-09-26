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


uint32_t planner_prev_pos[STEPPER_COUNT];
float planner_prev_coord[AXIS_COUNT];
float planner_prev_dir_vect[AXIS_COUNT];
float planner_prev_speed;
float planner_prev_accel;
uint8_t planner_prev_dirs;
float planner_max_speed[AXIS_COUNT];
uint32_t planner_max_accel[AXIS_COUNT];

void planner_init()
{
	memset(&planner_prev_pos, 0, STEPPER_COUNT*sizeof(uint32_t));
	memset(&planner_prev_coord, 0, AXIS_COUNT*sizeof(float));
	memset(&planner_max_speed, 0, AXIS_COUNT*sizeof(float));
	memset(&planner_prev_dir_vect, 0, AXIS_COUNT*sizeof(float));
	memset(&planner_max_accel, 0, AXIS_COUNT*sizeof(float));
	//kinematics_apply_inverse((float*)&(g_settings.max_speed), (uint32_t*)&planner_max_speed);
	kinematics_apply_inverse((float*)&(g_settings.max_accel), (uint32_t*)&planner_max_accel);
	planner_prev_speed = 0;
	
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
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
	uint8_t dirs = 0;
	uint32_t steps[STEPPER_COUNT];
	uint32_t stepspeeds[STEPPER_COUNT];
	uint32_t stepaccels[STEPPER_COUNT];
	uint32_t totalsteps = 0;
	float feedspeed = FLT_MAX;
	uint8_t conv_index = 0;
	float stepspeed = FLT_MAX;
	float stepaccel = FLT_MAX;
	float axis_speeds[AXIS_COUNT];
	float axis_accels[AXIS_COUNT];
	float dir_vect[AXIS_COUNT];
	float target_speed = 0;
	float target_accel = 0;
	
	//when adding a line the first thing to do is to calculate the direction of the movement
	//and compare it to the previous movement
	//this will allow to compute the previous exit speed/next entry speed profiles
	float magnitude = 0;
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		dir_vect[i] = axis[i] - planner_prev_coord[i];
		magnitude += dir_vect[i] * dir_vect[i];
	}
	
	//calculates the normalized direction vector
	//it also calculates the angle between previous direction and the current
	//this is given by the equation cos(theta) = dotprod(u,v)/(magnitude(u)*magnitude(v))
	//since normalized vector are being used (magnitude=1) this simplifies to cos(theta) = dotprod(u,v)
	//in the same loop the maximum linear speed and accel is calculated
	magnitude = sqrt(magnitude);
	float cos_theta = 0;
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		dir_vect[i] /= magnitude;
		cos_theta += dir_vect[i] * planner_prev_dir_vect[i];
		float dir_axis_abs = (dir_vect[i] < 0) ? -dir_vect[i] : dir_vect[i];
		float axis_speed = dir_axis_abs * feed;
		
		if(axis_speed > g_settings.max_speed[i])//if target speed exceeds the speed allowed for the axis lowers the feed rate
		{
			feed = g_settings.max_speed[i] / dir_axis_abs;
		}
		
		float axis_accel = g_settings.max_accel[i] / dir_axis_abs;
		if(stepaccel > axis_accel) //determines maximum accel rate for 
		{
			stepaccel = axis_accel;
		}
		//stepaccel = (axis_accel < 0) ? -dir_vect[i] : dir_vect[i];
		//target_speed += axis_speed;
		//target_accel += axis_accel * g_settings.max_accel[i];
	}
	
	//is is converted into speed and accel vectors
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		axis_speeds[i] = feed * dir_vect[i];
		axis_accels[i] = stepaccel * dir_vect[i];
	}

	float dir_angle = 1.0f + cos_theta;
	
	//if cos is negative the angle is greater then 90º so the previous movement must end with speed = 0
	//and there is no point in calculating the angle
	if(cos_theta > 0)
	{
		//uses the half angle identity conversion to convert from cos(theta) to tan(theta/2)
		//instead of sqrt((1-cos(theta)/(1+cos(theta))
		//to simplify the calculations it multiplies by sqrt((1+cos(theta)/(1+cos(theta))
		//transforming the equation to sqrt((1^2-cos(theta)^2))/(1+cos(theta))
		//this way the output will be between 0<tan(theta/2)<inf
		//but if theta is 0º<theta<90º the tan(theta/2) will be 0<tan(theta/2)<1
		//all angles greater than that can be excluded
		dir_angle = 1.0f + cos_theta;
		
		//if angle is between 0º<theta<90º calculates tan(theta/2) by using trig half angle identities
		//a direct aproximation can be used to calc the arctan(theta) by considering arctan(theta) = theta * Pi / 4 with an aprox. error of +-5º;
		//since will output a tan values between 0<tan(theta/2)<1 can be used directly
		//more accurate methods can be used but at the expense of math operations
		dir_angle = sqrt((1.0f-cos_theta*cos_theta))/dir_angle;
	}
	
	float prev_exit_speed = planner_prev_speed * dir_angle;
	prev_exit_speed = (prev_exit_speed>target_speed) ?  target_speed : prev_exit_speed;
	
	
	
	
	
	
	
	
	//previous exit speed exceeds current motion transition speed so the previous movement
	//must be deaccelerated to the transitional speed
	if(prev_exit_speed>planner_prev_speed)
	{
		float deaccel_time = (planner_prev_speed - prev_exit_speed) / planner_prev_accel;
	}

	
	

	//resets steps
	memset(&steps, 0, STEPPER_COUNT*sizeof(uint32_t));
	
	//reset feeds
	
	feedspeed = FLT_MAX;
				
	//calculates the inverse kinematics for the position and for the speeds
	kinematics_apply_inverse(axis, (uint32_t*)&steps);
	kinematics_apply_inverse((float*)&axis_speeds, (uint32_t*)&stepspeeds);

	//calculates the number of required steps to execute
	for(uint8_t i = 0; i < STEPPER_COUNT; i++)
	{
		if(steps[i] >= planner_prev_pos[i])
		{
			steps[i] -= planner_prev_pos[i];
			totalsteps = (totalsteps >= steps[i]) ? totalsteps : steps[i];
		}
		else
		{
			steps[i] = planner_prev_pos[i] - steps[i];
			dirs |= (1<<i);
			totalsteps = (totalsteps >= steps[i]) ? totalsteps : steps[i];
		}
		
	}
	
	//due to trunc/round errors/limitations of integer math the profile must be computed with real/float math

	//calculates the feedrate in steps per minutes
	//the feedrate is the smallest of the maximum allowed speed to each axis that has movement (given the amount of movement)
	/*for(uint8_t i = 0; i < STEPPER_COUNT; i++)
	{
		if(steps[i] != 0)
		{
			float max_speed = (float)stepspeeds[i];
			max_speed *= totalsteps;
			max_speed /= steps[i];
			max_speed = (stepspeeds[i] <= max_speed) ? stepspeeds[i] : max_speed;
			//stepspeeds[i] = (stepspeeds[i] <= max_speed) ? stepspeeds[i] : (uint32_t)round(max_speed);
			stepspeed = (stepspeed < max_speed) ? stepspeed : round(max_speed);
		}	
	}*/
	
	//calculates the accel in steps per minute ^2
	//the accel is the smallest of the maximum allowed speed to each axis that has movement (given the amount of movement)
	/*for(uint8_t i = 0; i < STEPPER_COUNT; i++)
	{
		if(steps[i] != 0)
		{
			float max_accel = (float)planner_max_accel[i] * dir_vect[i];
			stepaccel = (stepaccel < max_accel) ? stepaccel : max_accel;
		}	
	}*/
	
	stepspeed = stepspeeds[0];
	//to avoid 32bit trunc errors the operation calculation is broken in to steps
	//instead of 0.5*delta_speed^2/accel
	//its done like (deltaspeed*0.5)^2/(accel*0.5)
	//multiply by 0.5 is a simple bitshift right
	
	float accel_inv = 0.5f/stepaccel;
	//calculates as if the movement will deaccelerate to 0
	float steps_to_deaccel = round(stepspeed*stepspeed*accel_inv);
	//steps_to_deaccel *= steps_to_deaccel;
	//steps_to_deaccel /= (stepaccel>>1);
	
	//calculates de acceleretion given the previous exit
	float steps_to_accel = (stepspeed>planner_prev_speed) ? (stepspeed - planner_prev_speed) : (planner_prev_speed - stepspeed);
	steps_to_accel = round(steps_to_accel * steps_to_accel * accel_inv);
	
	//if full speed can't be achieved calcs the final speed to be reached
	if(steps_to_accel + steps_to_deaccel > totalsteps)
	{
		//accellerate half way
		steps_to_accel = (float)(totalsteps >> 1);
		//calculates new topspeed
		stepspeed = sqrt(stepaccel * steps_to_accel*2.0f);
	}
	
	//a
	//if(planner_prev_dirs != dirs)
	
	//uint32_t steps_to_speed = ((stepspeed * stepspeed) >> 1) / stepaccel;
}
	

/*
	Name: 
	Copyright: 
	Author: 
	Date: 25/09/19 11:42
	Description: 
*/
void planner_add_curve(float* axis, float* center, float feed)
{
	
}
