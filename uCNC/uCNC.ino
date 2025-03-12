#include "src/cnc.h"

void setup()
{
	// put your setup code here, to run once:
	// ucnc_init();
	mcu_io_init();
}

void loop()
{
	// put your main code here, to run repeatedly:
	// ucnc_run();
	mcu_toggle_output(DOUT31);
	
}
