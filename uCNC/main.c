#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "settings.h"
#include "boarddefs.h"
#include "machinedefs.h"
#include "gcode.h"
#include "planner.h"
#include "kinematics.h"
#include "interpolator.h"


int main()
{
	//initializes all systems
	settings_load();
	board_init();
	gcode_init();
	planner_init();
	kinematics_init();
	interpolator_init();
	
	#ifdef DEBUGMODE
		board_printfp(PSTR("uCNC initialized\n"));
	#endif		
	for(;;)
	{
		#ifdef DEBUGMODE
		uint8_t inc = 1;
		#endif
		#ifdef DEBUGMODE
		board_loadDummyPayload(PSTR("G0X20\n"));
		#endif
		for(;;)
		{
			
			//#ifdef DEBUGMODE
			//board_loadDummyPayload(PSTR("G0X%d\n"), inc++);
			//#endif
			gcode_parse_nextline();
			#ifdef DEBUGMODE
				board_printfp(PSTR("planning motion\n"));
				fflush(stdout);
				board_startPerfCounter();
			#endif
			interpolator_exec_planner_block();
			#ifdef DEBUGMODE
				uint16_t count = board_stopPerfCounter();
				board_printfp(PSTR("planned in: in %u cycles\n"), count);
				fflush(stdout);
			#endif
			#ifdef DEBUGMODE
			board_printfp(PSTR("integrator max ticks %d\n"), board_performacecounters.integratorCounter);
			board_printfp(PSTR("pulse max ticks %d\n"), board_performacecounters.pulseCounter);
			//printf("integrator max ticks %d\n", board_performacecounters.integratorCounter);
			//printf("pulse max ticks %d\n", board_performacecounters.pulseCounter);
			#endif
		}
	}
	
	return 0;
}
