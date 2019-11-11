#include "cnc.h"

int main()
{
	//initializes all systems
	cnc_init();
	
	for(;;)
	{
		cnc_run();
	}

	return 0;
}
