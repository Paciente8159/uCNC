#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "settings.h"
#include "mcu.h"
#include "gcode.h"
#include "kinematics.h"
#include "planner.h"
#include "interpolator.h"

void cnc_init()
{
	mcu_init();
	mcu_printfp(PSTR("uCNC starting up\n"));
	settings_load();
	gcode_init();
	gcode_print_states();
	kinematics_init();
	planner_init();
	interpolator_init();
}

void cnc_run()
{
	
	#ifdef __SIMUL__
	static uint8_t a = 0;
	
	if(!planner_buffer_full())
	{
		a+=4;
		#ifdef __SIMUL__
		mcu_printfp(PSTR("Load dummy G0 X%u\n"), a);
		mcu_loadDummyPayload(PSTR("G0X%u\n"), a);
		#endif
	}
	#endif
	gcode_parse_nextline();
	interpolator_execute();
}

