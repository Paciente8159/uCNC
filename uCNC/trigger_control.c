#include "config.h"
#include "grbl_interface.h"
#include "settings.h"
#include "mcudefs.h"
#include "mcu.h"
#include "mcumap.h"
#include "trigger_control.h"
#include "cnc.h"

static uint8_t tc_limitmask;

void tc_init()
{
	tc_limitmask = 0;
}

void tc_set_limit_mask(uint8_t mask)
{
	tc_limitmask = mask;
}

void tc_limits_isr(uint8_t limits)
{
	g_cnc_state.limits = limits;
	if(tc_limitmask & limits)
	{
		g_cnc_state.halt = true; //kills motion imediatly
	}
	
	cnc_alarm(EXEC_ALARM_HARD_LIMIT);
}

void tc_controls_isr(uint8_t controls)
{
	g_cnc_state.controls = controls;
	if(g_cnc_state.controls & ESTOP_MASK)
	{
		//flags all isr to stop
		g_cnc_state.halt = true; 
		g_cnc_state.rt_cmd |= RT_CMD_RESET;
		return;
	}
}

bool tc_check_boundaries(float* axis)
{
	#ifdef AXIS_X
		if(axis[AXIS_X]<0 || axis[AXIS_X]>g_settings.max_distance[AXIS_X])
		{
			g_cnc_state.limits |= LIMIT_X_MASK;
			return false;
		}
	#endif
	#ifdef AXIS_Y
		if(axis[AXIS_Y]<0 || axis[AXIS_Y]>g_settings.max_distance[AXIS_Y])
		{
			g_cnc_state.limits |= LIMIT_Y_MASK;
			return false;
		}
	#endif
	#ifdef AXIS_Z
		if(axis[AXIS_Z]<0 || axis[AXIS_Z]>g_settings.max_distance[AXIS_Z])
		{
			g_cnc_state.limits |= LIMIT_Z_MASK;
			return false;
		}
	#endif
	#ifdef AXIS_A
		if(axis[AXIS_A]<0 || axis[AXIS_A]>g_settings.max_distance[AXIS_A])
		{
			g_cnc_state.limits |= LIMIT_A_MASK;
			return false;
		}
	#endif
	#ifdef AXIS_B
		if(axis[AXIS_B]<0 || axis[AXIS_B]>g_settings.max_distance[AXIS_B])
		{
			g_cnc_state.limits |= LIMIT_B_MASK;
			return false;
		}
	#endif
	#ifdef AXIS_C
		if(axis[AXIS_C]<0 || axis[AXIS_C]>g_settings.max_distance[AXIS_C])
		{
			g_cnc_state.limits |= LIMIT_C_MASK;
			return false;
		}
	#endif

	return true;
}
