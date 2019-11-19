#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "settings.h"
#include "mcu.h"
#include "protocol.h"
#include "gcode.h"
#include "kinematics.h"
#include "motion_control.h"
#include "planner.h"
#include "interpolator.h"

void cnc_init()
{
	mcu_init();
	protocol_init();
	protocol_send(PSTR("uCNC starting up\r"));
	settings_load();
	gcode_init();
	gcode_print_states();
	kinematics_init();
	mc_init();
	planner_init();
	interpolator_init();
}

void cnc_run()
{
	/*
	#ifdef __SIMUL__
	static uint8_t a = 0;
	
	if(!planner_buffer_full())
	{
		a=4;
		#ifdef __SIMUL__
		mcu_printfp(PSTR("Load dummy G0 X%u\n"), a);
		mcu_loadDummyPayload(PSTR("G0X%u\n"), a);
		#endif
	}
	#endif
	gcode_parse_nextline();
	interpolator_execute();*/
	
	float target[AXIS_COUNT];
	float offset[2];
	
	//X = 4
	target[0] = 4;
	target[1] = 0;
	target[2] = 0;
	mc_line(target, 5);
	
	//arc from X = 4 to origin
	offset[0] = -2;
	offset[1] = 0;
	target[0] = 0;
	mc_arc(target, offset, 2, 0, false, 5);
}

