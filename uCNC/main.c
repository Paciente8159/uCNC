#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "board.h"
#include "cnc.h"
//#include "boardcontroller.h"


/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main() {
	
	char str[255];
	cnc_setup();
	
	while(1)
	{
		cnc_execMainLoop();
		memset(str,0, 255);
		//sprintf(str, "cycles: (%d)\n\r", board_performacecounters.resetPulseCounter);
        board_comSendPacket(str,strlen(str));
	}
	
	
	
	/*char str[255];
	uint32_t a = 0;
	uint32_t b = 0;
	uint32_t c = 0;
	float res = 0.0;
	uint32_t ins;
	cnc_setup();
	uint16_t stepdirs = 0;
	
	while(1)
    {
    	stepdirs++;
    	
        board_startPerfCounter();
        cnc_execPulseReset();
        //board_setStepDirs(stepdirs);
        //ins = board_getCriticalInputs();
        a = board_stopPerfCounter();
        memset(str,0, 255);
		res = (float)a * 0.0625f;
		b = (uint32_t)res;
		res = res - b;
		c = (uint32_t)(res * 10000);
		sprintf(str, "us: (%ld)%ld.%ld - %ld\n\r", a,b,c,ins);
        board_comSendPacket(str,strlen(str));
          //uint8_t t = CNC._machine.getCriticalInputs();
          //Serial.println(CNC._machine._state.criticalInputs);
    }*/
	return 0;
}
