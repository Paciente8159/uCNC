#include "motion.h"
#include "structures.h"
#include "pins.h"

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define MOTION_RT_BUFFER_SIZE 20
MOTION_COMMAND g_motion_buffer[MOTION_RT_BUFFER_SIZE];
uint8_t g_motion_buffer_head;
uint8_t g_motion_buffer_tail;

uint16_t g_motion_exitspeed;

//formula de aceleracao
//a cada step a velocidade vai ser
//sqrt(2*accel+vel_inicial*vel_inicial)
//o tempo de pulso vai ser (vel-vel_inicial)/accel

//metodo direto
//em aceleracao constante o tempo de acele 0 t_acel = (vel_final-vel_ini)/accel
//o numero de passos para atingir a aceler e accel_steps = 0.5*accel*t_accel^2

//cada pulso e dado em sqrt(2*step_count/accel), sendo que o step_count e increm ate accel_steps

void motion_rt_linear(uint16_t *steps, uint16_t *totalsteps, uint16_t initial_speed, int16_t accel)
{
	uint16_t step_counter = 0;
	uint16_t stepcount = *totalsteps;
    uint16_t error = (stepcount >> 1);
    #if(STEP0>=0)
        uint16_t step_error0 = error;
	#endif
	#if(STEP1>=0)
        uint16_t step_error1 = error;
	#endif
	#if(STEP2>=0)
        uint16_t step_error2 = error;
	#endif
	#if(STEP3>=0)
        uint16_t step_error3 = error;
	#endif
	#if(STEP4>=0)
        uint16_t step_error4 = error;
	#endif
	
	do {
		//if the realtime motion buffer is full exit to other task and returns later
		if(motion_rt_buffer_full())
		{
			return;
		}
		
	#if(STEP0>=0)
        step_error0 += steps[0];
        if (step_error0 > stepcount)
        {
            step_error0 -= stepcount;
            g_motion_buffer[g_motion_buffer_tail].stepdirs |= 2;
        }
    #endif
    
    #if(STEP1>=0)
        step_error1 += steps[1];
        if (step_error1 > stepcount)
        {
            step_error1 -= stepcount;
            g_motion_buffer[g_motion_buffer_tail].stepdirs |= 8;
        }
    #endif
    
    #if(STEP2>=0)
        step_error2 += steps[2];
        if (step_error2 > stepcount)
        {
            step_error2 -= stepcount;
            g_motion_buffer[g_motion_buffer_tail].stepdirs |= 16;
        }
    #endif
    
    #if(STEP3>=0)
        step_error3 += steps[3];
        if (step_error3 > stepcount)
        {
            step_error3 -= stepcount;
            g_motion_buffer[g_motion_buffer_tail].stepdirs |= 64;
        }
    #endif
    
    #if(STEP4>=0)
        step_error4 += steps[4];
        if (step_error4 > stepcount)
        {
            step_error4 -= stepcount;
            g_motion_buffer[g_motion_buffer_tail].stepdirs |= 256;
        }
    #endif
    
    if(accel==0)
	{
		
	}
	else
	{
		step_counter++;
		uint16_t freq = (uint16_t)sqrt((step_counter<<1) / accel);
	}
	
	} while(--*totalsteps);
}

void motion_rt_buffer_dequeue()
{
	if(++g_motion_buffer_head==MOTION_RT_BUFFER_SIZE)
	{
		g_motion_buffer_head = 0;
	}
}

void motion_rt_buffer_enqueue()
{
	if(++g_motion_buffer_tail==MOTION_RT_BUFFER_SIZE)
	{
		g_motion_buffer_tail = 0;
	}
}

uint8_t motion_rt_buffer_empty()
{
	return (g_motion_buffer_head==g_motion_buffer_tail);
}


uint8_t motion_rt_buffer_full()
{
	if(g_motion_buffer_tail + 1 == MOTION_RT_BUFFER_SIZE && g_motion_buffer_head == 0)
	{
		return 1;
	}
	
	return (g_motion_buffer_head==(g_motion_buffer_tail + 1));
}
