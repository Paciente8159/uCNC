#include "src/cnc.h"

void setup()
{
	// put your setup code here, to run once:
	ucnc_init();
	mcu_start_itp_isr(1, 1);
}

void loop()
{
	// put your main code here, to run repeatedly:
	// ucnc_run();
}
