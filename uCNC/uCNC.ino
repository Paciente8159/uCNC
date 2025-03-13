#include "src/cnc.h"

void setup()
{
	// put your setup code here, to run once:
	ucnc_init();
	// mcu_io_init();
	// pinMode(GPIO_NUM_48, OUTPUT);
}

void loop()
{
	// put your main code here, to run repeatedly:
	ucnc_run();
	// // mcu_toggle_output(DOUT31);
	// digitalWrite(GPIO_NUM_48, 1);
	// delay(1000);
	// digitalWrite(GPIO_NUM_48, 0);
	// delay(1000);
	// // digitalWrite(GPIO_NUM_38, 1);
	// // digitalWrite(GPIO_NUM_38, 0);
	// // digitalWrite(GPIO_NUM_38, 1);
	// // digitalWrite(GPIO_NUM_38, 0);	
}
