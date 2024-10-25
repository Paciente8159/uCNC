# 1 "C:\\Users\\JCEM\\AppData\\Local\\Temp\\tmpb881qvmj"
#include <Arduino.h>
# 1 "C:/Users/JCEM/Documents/GitHub/uCNC/uCNC/uCNC.ino"
#include "src/cnc.h"
void setup();
void loop();
#line 3 "C:/Users/JCEM/Documents/GitHub/uCNC/uCNC/uCNC.ino"
void setup()
{

 ucnc_init();
}

void loop()
{

 ucnc_run();
}