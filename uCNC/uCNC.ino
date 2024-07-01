#include "src/cnc.h"
#include <src/modules/touch_screen/XPT2046.h>
void setup()
{
	// put your setup code here, to run once:
	ucnc_init();
	
}

void loop()
{
	// put your main code here, to run repeatedly:
	ucnc_run();
}
