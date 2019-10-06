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


int main()
{
	char line[255];
	
	settings_load();
	board_setup();
	gcode_init();
	planner_init();
	kinematics_init();
			
	for(;;)
	{
		for(;;)
		{
			if(board_peek() != 0)
			{
				gcode_parse_nextline();
			}
		}
		
		
	}
	
	return 0;
}
