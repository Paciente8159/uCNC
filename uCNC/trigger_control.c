#include "config.h"
#include "grbl_interface.h"
#include "settings.h"
#include "mcudefs.h"
#include "mcu.h"
#include "mcumap.h"
#include "trigger_control.h"
#include "cnc.h"

static volatile bool tc_homing;

void tc_init()
{
	tc_homing = false;
}

void tc_home()
{
	tc_homing = true;
}

void tc_limits_isr(uint8_t limits)
{	
	limits ^= g_settings.limits_invert_mask;
	if(g_settings.hard_limits_enabled)
	{
		g_cnc_state.limits = limits;
		//limits &= tc_limitsmask;
		if(limits)
		{
			g_cnc_state.halt = true; //kills motion imediatly
			if(!tc_homing)
			{
				cnc_alarm(EXEC_ALARM_HARD_LIMIT);
			}
		}
	}
}

void tc_controls_isr(uint8_t controls)
{
	g_cnc_state.controls = controls;
	if(controls & ESTOP_MASK)
	{
		//flags all isr to stop
		g_cnc_state.halt = true; 
		cnc_alarm(EXEC_ALARM_ABORT_CYCLE);
		return;
	}
	
	if(controls & (FHOLD_MASK | DOOR_OPEN_MASK))
	{
		//flags HOLD
		g_cnc_state.exec_state |= EXEC_HOLD; 
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
