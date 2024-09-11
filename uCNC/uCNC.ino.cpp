# 1 "C:\\Users\\JCMART~2\\AppData\\Local\\Temp\\tmp0wlvbuqc"
#include <Arduino.h>
# 1 "C:/Users/jcmartins/Downloads/GitHub/uCNC/uCNC/uCNC.ino"
#include "src/cnc.h"
void setup();
void loop();
#line 3 "C:/Users/jcmartins/Downloads/GitHub/uCNC/uCNC/uCNC.ino"
void setup()
{

 ucnc_init();
}

void loop()
{

 ucnc_run();
}