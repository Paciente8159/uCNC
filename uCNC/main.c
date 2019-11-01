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


int main()
{
	//initializes all systems
	mcu_init();
	mcu_printfp(PSTR("uCNC starting up\n"));
	settings_load();
	gcode_init();
	gcode_print_states();
	kinematics_init();
	planner_init();
	interpolator_init();
		
	for(;;)
	{
		
		for(uint8_t a = 1; a < 20; a+=4)
		{
			#ifdef __SIMUL__
			mcu_printfp(PSTR("Load dummy G0 X20.0\n"));
			mcu_loadDummyPayload(PSTR("G0X%u\n"), a);
		#endif
			
			gcode_parse_nextline();
			interpolator_exec_planner_block();
		}
	}
	
	return 0;
}
