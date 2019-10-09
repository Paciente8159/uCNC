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
	char line[255];
	
	//initializes all systems
	settings_load();
	board_init();
	gcode_init();
	planner_init();
	kinematics_init();
	interpolator_init();
			
	for(;;)
	{
		for(;;)
		{
			gcode_parse_nextline();
			interpolator_exec_planner_block();
		}
	}
	
	return 0;
}
