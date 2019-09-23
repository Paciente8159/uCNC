#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "board.h"
#include "gcode.h"

int main()
{
	char line[255];
	int count = 0;
	
	board_setup();
		
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
