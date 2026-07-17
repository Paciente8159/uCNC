# 1 "C:\\Users\\JCMART~1\\AppData\\Local\\Temp\\tmpu5lpm35i"
#include <Arduino.h>
# 1 "C:/Users/jcmartins/Downloads/Github/uCNC/uCNC/uCNC.ino"
#include "src/cnc.h"
void setup();
void loop();
#line 3 "C:/Users/jcmartins/Downloads/Github/uCNC/uCNC/uCNC.ino"
void setup()
{

 ucnc_init();
}

void loop()
{

 ucnc_run();
}