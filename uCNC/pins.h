#ifndef PINS_H
#define PINS_H

#include "cpudefs.h"

#define BITVAL0 1UL
#define BITVAL1 2UL
#define BITVAL2 4UL
#define BITVAL3 8UL
#define BITVAL4 16UL
#define BITVAL5 32UL
#define BITVAL6 64UL
#define BITVAL7 128UL
#define BITVAL8 256UL
#define BITVAL9 512UL
#define BITVAL10 1024UL
#define BITVAL11 2048UL
#define BITVAL12 4096UL
#define BITVAL13 8192UL
#define BITVAL14 16384UL
#define BITVAL15 32768UL
#define BITVAL16 65536UL
#define BITVAL17 131072UL
#define BITVAL18 262144UL
#define BITVAL19 524288UL
#define BITVAL20 1048576UL
#define BITVAL21 2097152UL
#define BITVAL22 4194304UL
#define BITVAL23 8388608UL
#define BITVAL24 16777216UL
#define BITVAL25 33554432UL
#define BITVAL26 67108864UL
#define BITVAL27 134217728UL
#define BITVAL28 268435456UL
#define BITVAL29 536870912UL
#define BITVAL30 1073741824UL
#define BITVAL31 2147483648UL
#define BITVAL(X) BITVAL##X
#define BITVALPIN(X) BITVAL(X)

//Set default values for pins
//OUTPUTS
#ifndef DIR0
 #define DIR0 -1
 #define OUT0 0
 #define POUT0 0
#else
 #define OUT0 BITVAL(0)
 #define POUT0 BITVALPIN(DIR0)
#endif
#ifndef STEP0
 #define STEP0 -1
 #define OUT1 0
 #define POUT1 0
#else
 #define OUT1 BITVAL(1)
 #define POUT1 BITVALPIN(STEP0)
#endif
#ifndef DIR1
 #define DIR1 -1
 #define OUT2 0
 #define POUT2 0
#else
 #define OUT2 BITVAL(2)
 #define POUT2 BITVALPIN(DIR1)
#endif
#ifndef STEP1
 #define STEP1 -1
 #define OUT3 0
 #define POUT3 0
#else
 #define OUT3 BITVAL(3)
 #define POUT3 BITVALPIN(STEP1)
#endif
#ifndef DIR2
 #define DIR2 -1
 #define OUT4 0
 #define POUT4 0
#else
 #define OUT4 BITVAL(4)
 #define POUT4 BITVALPIN(DIR2)
#endif
#ifndef STEP2
 #define STEP2 -1
 #define OUT5 0
 #define POUT5 0
#else
 #define OUT5 BITVAL(5)
 #define POUT5 BITVALPIN(STEP2)
#endif
#ifndef DIR3
 #define DIR3 -1
 #define OUT6 0
 #define POUT6 0
#else
 #define OUT6 BITVAL(6)
 #define POUT6 BITVALPIN(DIR3)
#endif
#ifndef STEP3
 #define STEP3 -1
 #define OUT7 0
 #define POUT7 0
#else
 #define OUT7 BITVAL(7)
 #define POUT7 BITVALPIN(STEP3)
#endif
#ifndef DIR4
 #define DIR4 -1
 #define OUT8 0
 #define POUT8 0
#else
 #define OUT8 BITVAL(8)
 #define POUT8 BITVALPIN(DIR4)
#endif
#ifndef STEP4
 #define STEP4 -1
 #define OUT9 0
 #define POUT9 0
#else
 #define OUT9 BITVAL(9)
 #define POUT9 BITVALPIN(STEP4)
#endif
#ifndef TX
 #define TX -1
 #define OUT10 0
 #define POUT10 0
#else
 #define OUT10 BITVAL(10)
 #define POUT10 BITVALPIN(TX)
#endif
#ifndef PWM0
 #define PWM0 -1
 #define OUT11 0
 #define POUT11 0
#else
 #define OUT11 BITVAL(11)
 #define POUT11 BITVALPIN(PWM0)
#endif
#ifndef PWM1
 #define PWM1 -1
 #define OUT12 0
 #define POUT12 0
#else
 #define OUT12 BITVAL(12)
 #define POUT12 BITVALPIN(PWM1)
#endif
#ifndef PWM2
 #define PWM2 -1
 #define OUT13 0
 #define POUT13 0
#else
 #define OUT13 BITVAL(13)
 #define POUT13 BITVALPIN(PWM2)
#endif
#ifndef PWM3
 #define PWM3 -1
 #define OUT14 0
 #define POUT14 0
#else
 #define OUT14 BITVAL(14)
 #define POUT14 BITVALPIN(PWM3)
#endif
#ifndef PWM4
 #define PWM4 -1
 #define OUT15 0
 #define POUT15 0
#else
 #define OUT15 BITVAL(15)
 #define POUT15 BITVALPIN(PWM4)
#endif
#ifndef DOUT0
 #define DOUT0 -1
 #define OUT16 0
 #define POUT16 0
#else
 #define OUT16 BITVAL(16)
 #define POUT16 BITVALPIN(DOUT0)
#endif
#ifndef DOUT1
 #define DOUT1 -1
 #define OUT17 0
 #define POUT17 0
#else
 #define OUT17 BITVAL(17)
 #define POUT17 BITVALPIN(DOUT1)
#endif
#ifndef DOUT2
 #define DOUT2 -1
 #define OUT18 0
 #define POUT18 0
#else
 #define OUT18 BITVAL(18)
 #define POUT18 BITVALPIN(DOUT2)
#endif
#ifndef DOUT3
 #define DOUT3 -1
 #define OUT19 0
 #define POUT19 0
#else
 #define OUT19 BITVAL(19)
 #define POUT19 BITVALPIN(DOUT3)
#endif
#ifndef DOUT4
 #define DOUT4 -1
 #define OUT20 0
 #define POUT20 0
#else
 #define OUT20 BITVAL(20)
 #define POUT20 BITVALPIN(DOUT4)
#endif
#ifndef DOUT5
 #define DOUT5 -1
 #define OUT21 0
 #define POUT21 0
#else
 #define OUT21 BITVAL(21)
 #define POUT21 BITVALPIN(DOUT5)
#endif
#ifndef DOUT6
 #define DOUT6 -1
 #define OUT22 0
 #define POUT22 0
#else
 #define OUT22 BITVAL(22)
 #define POUT22 BITVALPIN(DOUT6)
#endif
#ifndef DOUT7
 #define DOUT7 -1
 #define OUT23 0
 #define POUT23 0
#else
 #define OUT23 BITVAL(23)
 #define POUT23 BITVALPIN(DOUT7)
#endif
#ifndef DOUT8
 #define DOUT8 -1
 #define OUT24 0
 #define POUT24 0
#else
 #define OUT24 BITVAL(24)
 #define POUT24 BITVALPIN(DOUT8)
#endif
#ifndef DOUT9
 #define DOUT9 -1
 #define OUT25 0
 #define POUT25 0
#else
 #define OUT25 BITVAL(25)
 #define POUT25 BITVALPIN(DOUT9)
#endif
#ifndef DOUT10
 #define DOUT10 -1
 #define OUT26 0
 #define POUT26 0
#else
 #define OUT26 BITVAL(26)
 #define POUT26 BITVALPIN(DOUT10)
#endif
#ifndef DOUT11
 #define DOUT11 -1
 #define OUT27 0
 #define POUT27 0
#else
 #define OUT27 BITVAL(27)
 #define POUT27 BITVALPIN(DOUT11)
#endif
#ifndef DOUT12
 #define DOUT12 -1
 #define OUT28 0
 #define POUT28 0
#else
 #define OUT28 BITVAL(28)
 #define POUT28 BITVALPIN(DOUT12)
#endif
#ifndef DOUT13
 #define DOUT13 -1
 #define OUT29 0
 #define POUT29 0
#else
 #define OUT29 BITVAL(29)
 #define POUT29 BITVALPIN(DOUT13)
#endif
#ifndef DOUT14
 #define DOUT14 -1
 #define OUT30 0
 #define POUT30 0
#else
 #define OUT30 BITVAL(30)
 #define POUT30 BITVALPIN(DOUT14)
#endif
#ifndef DOUT15
 #define DOUT15 -1
 #define OUT31 0
 #define POUT31 0
#else
 #define OUT31 BITVAL(31)
 #define POUT31 BITVALPIN(DOUT15)
#endif


//INPUTS
#ifndef ESTOP
 #define ESTOP -1
 #define IN0 0
#else
 #define IN0 BITVAL(0)
#endif
#ifndef FHOLD
 #define FHOLD -1
 #define IN1 0
#else
 #define IN1 BITVAL(1)
#endif
#ifndef PROBE
 #define PROBE -1
 #define IN2 0
#else
 #define IN2 BITVAL(2)
#endif
#ifndef LIMIT_X
 #define LIMIT_X -1
 #define IN3 0
#else
 #define IN3 BITVAL(3)
#endif
#ifndef LIMIT_Y
 #define LIMIT_Y -1
 #define IN4 0
#else
 #define IN4 BITVAL(4)
#endif
#ifndef LIMIT_Z
 #define LIMIT_Z -1
 #define IN5 0
#else
 #define IN5 BITVAL(5)
#endif
#ifndef LIMIT_A
 #define LIMIT_A -1
 #define IN6 0
#else
 #define IN6 BITVAL(6)
#endif
#ifndef LIMIT_B
 #define LIMIT_B -1
 #define IN7 0
#else
 #define IN7 BITVAL(7)
#endif
#ifndef UNUSED
 #define UNUSED -1
 #define IN8 0
#else
 #define IN8 BITVAL(8)
#endif
#ifndef CS_R
 #define CS_R -1
 #define IN9 0
#else
 #define IN9 BITVAL(9)
#endif
#ifndef RX
 #define RX -1
 #define IN10 0
#else
 #define IN10 BITVAL(10)
#endif
#ifndef ANA0
 #define ANA0 -1
 #define IN11 0
#else
 #define IN11 BITVAL(11)
#endif
#ifndef ANA1
 #define ANA1 -1
 #define IN12 0
#else
 #define IN12 BITVAL(12)
#endif
#ifndef ANA2
 #define ANA2 -1
 #define IN13 0
#else
 #define IN13 BITVAL(13)
#endif
#ifndef ANA3
 #define ANA3 -1
 #define IN14 0
#else
 #define IN14 BITVAL(14)
#endif
#ifndef ANA4
 #define ANA4 -1
 #define IN15 0
#else
 #define IN15 BITVAL(15)
#endif
#ifndef DIN0
 #define DIN0 -1
 #define IN16 0
#else
 #define IN16 BITVAL(16)
#endif
#ifndef DIN1
 #define DIN1 -1
 #define IN17 0
#else
 #define IN17 BITVAL(17)
#endif
#ifndef DIN2
 #define DIN2 -1
 #define IN18 0
#else
 #define IN18 BITVAL(18)
#endif
#ifndef DIN3
 #define DIN3 -1
 #define IN19 0
#else
 #define IN19 BITVAL(19)
#endif
#ifndef DIN4
 #define DIN4 -1
 #define IN20 0
#else
 #define IN20 BITVAL(20)
#endif
#ifndef DIN5
 #define DIN5 -1
 #define IN21 0
#else
 #define IN21 BITVAL(21)
#endif
#ifndef DIN6
 #define DIN6 -1
 #define IN22 0
#else
 #define IN22 BITVAL(22)
#endif
#ifndef DIN7
 #define DIN7 -1
 #define IN23 0
#else
 #define IN23 BITVAL(23)
#endif
#ifndef DIN8
 #define DIN8 -1
 #define IN24 0
#else
 #define IN24 BITVAL(24)
#endif
#ifndef DIN9
 #define DIN9 -1
 #define IN25 0
#else
 #define IN25 BITVAL(25)
#endif
#ifndef DIN10
 #define DIN10 -1
 #define IN26 0
#else
 #define IN26 BITVAL(26)
#endif
#ifndef DIN11
 #define DIN11 -1
 #define IN27 0
#else
 #define IN27 BITVAL(27)
#endif
#ifndef DIN12
 #define DIN12 -1
 #define IN28 0
#else
 #define IN28 BITVAL(28)
#endif
#ifndef DIN13
 #define DIN13 -1
 #define IN29 0
#else
 #define IN29 BITVAL(29)
#endif
#ifndef DIN14
 #define DIN14 -1
 #define IN30 0
#else
 #define IN30 BITVAL(30)
#endif
#ifndef DIN15
 #define DIN15 -1
 #define IN31 0
#else
 #define IN31 BITVAL(31)
#endif

#define OUTPUT_BITS	( OUT0 \
					| OUT1 \
					| OUT2 \
					| OUT3 \
					| OUT4 \
					| OUT5 \
					| OUT6 \
					| OUT7 \
					| OUT8 \
					| OUT9 \
					| OUT10 \
					| OUT11 \
					| OUT12 \
					| OUT13 \
					| OUT14 \
					| OUT15 \
					| OUT16 \
					| OUT17 \
					| OUT18 \
					| OUT19 \
					| OUT20 \
					| OUT21 \
					| OUT22 \
					| OUT23 \
					| OUT24 \
					| OUT25 \
					| OUT26 \
					| OUT27 \
					| OUT28 \
					| OUT29 \
					| OUT30 \
					| OUT31 )

#define OUTPUT_PINS	( POUT0 \
					| POUT1 \
					| POUT2 \
					| POUT3 \
					| POUT4 \
					| POUT5 \
					| POUT6 \
					| POUT7 \
					| POUT8 \
					| POUT9 \
					| POUT10 \
					| POUT11 \
					| POUT12 \
					| POUT13 \
					| POUT14 \
					| POUT15 \
					| POUT16 \
					| POUT17 \
					| POUT18 \
					| POUT19 \
					| POUT20 \
					| POUT21 \
					| POUT22 \
					| POUT23 \
					| POUT24 \
					| POUT25 \
					| POUT26 \
					| POUT27 \
					| POUT28 \
					| POUT29 \
					| POUT30 \
					| POUT31 )
					
#define INPUT_BITS	( IN0 \
					| IN1 \
					| IN2 \
					| IN3 \
					| IN4 \
					| IN5 \
					| IN6 \
					| IN7 \
					| IN8 \
					| IN9 \
					| IN10 \
					| IN11 \
					| IN12 \
					| IN13 \
					| IN14 \
					| IN15 \
					| IN16 \
					| IN17 \
					| IN18 \
					| IN19 \
					| IN20 \
					| IN21 \
					| IN22 \
					| IN23 \
					| IN24 \
					| IN25 \
					| IN26 \
					| IN27 \
					| IN28 \
					| IN29 \
					| IN30 \
					| IN31 )
/*
//define input pins
//critical inputs
#define IN0 ESTOP
#define IN1 FHOLD		//reserved
#define IN2 PROBE		//reserved
#define IN3 LIMIT_X		//reserved
#define IN4 LIMIT_Y		//reserved
#define IN5 LIMIT_Z		//reserved
#define IN6 LIMIT_A		//reserved
#define IN7 LIMIT_B		//reserved

//non critical inputs
#define IN8 CS_R		//reserved
#define IN9	UNUSED		//reserved
#define IN10 SERIAL_RX	//reserved
#define IN11 ANA0	    //reserved
#define IN12 ANA1		//reserved
#define IN13 ANA2		//reserved
#define IN14 ANA3		//reserved
#define IN15 ANA4		//reserved
#define IN16 DIN0
#define IN17 DIN1
#define IN18 DIN2
#define IN19 DIN3
#define IN20 DIN4
#define IN21 DIN5
#define IN22 DIN6
#define IN23 DIN7
#define IN24 DIN8
#define IN25 DIN9
#define IN26 DIN10
#define IN27 DIN11
#define IN28 DIN12
#define IN29 DIN13
#define IN30 DIN14
#define IN31 DIN15

//define output pins
#define OUT0 STEP0			//reserved
#define OUT1 DIR0			//reserved
#define OUT2 STEP1			//reserved
#define OUT3 DIR1			//reserved
#define OUT4 STEP2			//reserved
#define OUT5 DIR2			//reserved
#define OUT6 STEP3			//reserved
#define OUT7 DIR3			//reserved
#define OUT8 STEP4			//reserved
#define OUT9 DIR4			//reserved
#define OUT10 SERIAL_TX		//reserved
#define OUT11 PWM0			//reserved
#define OUT12 PWM1			//reserved
#define OUT13 PWM2			//reserved
#define OUT14 PWM3			//reserved
#define OUT15 PWM4			//reserved
#define OUT16 DOUT0
#define OUT17 DOUT1
#define OUT18 DOUT2
#define OUT19 DOUT3
#define OUT20 DOUT4
#define OUT21 DOUT5
#define OUT22 DOUT6
#define OUT23 DOUT7
#define OUT24 DOUT8
#define OUT25 DOUT9
#define OUT26 DOUT10
#define OUT27 DOUT11
#define OUT28 DOUT12
#define OUT29 DOUT13
#define OUT30 DOUT14
#define OUT31 DOUT15
*/

typedef union
{
	uint32_t reg32in;
	struct
	{
		unsigned emergencystop : 1;
		unsigned feedhold : 1;
		unsigned probe : 1;
		unsigned limit_x : 1;
		unsigned limit_y : 1;
		unsigned limit_z : 1;
		unsigned limit_a : 1;
		unsigned limit_b : 1;
		unsigned cyclestart_resume : 1;
		unsigned : 7;
		unsigned input0 : 1;
		unsigned input1 : 1;
		unsigned input2 : 1;
		unsigned input3 : 1;
		unsigned input4 : 1;
		unsigned input5 : 1;
		unsigned input6 : 1;
		unsigned input7 : 1;
		unsigned input8 : 1;
		unsigned input9 : 1;
		unsigned input10 : 1;
		unsigned input11 : 1;
		unsigned input12 : 1;
		unsigned input13 : 1;
		unsigned input14 : 1;
		unsigned input15 : 1;
	}; //digital inputs pins
	struct
	{
		uint8_t critical_inputs;
		uint8_t unused;
		uint16_t inputs;
	};
} INPUT_REGISTER;

typedef union
{
	uint32_t reg32out;
	struct
	{
		unsigned dir0 : 1;
		unsigned step0 : 1;
		unsigned dir1 : 1;
		unsigned step1 : 1;
		unsigned dir2 : 1;
		unsigned step2 : 1;
		unsigned dir3 : 1;
		unsigned step3 : 1;
		unsigned dir4 : 1;
		unsigned step4 : 1;
		unsigned : 6;
		unsigned output0 : 1;
		unsigned output1 : 1;
		unsigned output2 : 1;
		unsigned output3 : 1;
		unsigned output4 : 1;
		unsigned output5 : 1;
		unsigned output6 : 1;
		unsigned output7 : 1;
		unsigned output8 : 1;
		unsigned output9 : 1;
		unsigned output10 : 1;
		unsigned output11 : 1;
		unsigned output12 : 1;
		unsigned output13 : 1;
		unsigned output14 : 1;
		unsigned output15 : 1;
	};
	
	struct
	{
		uint8_t dirstep_0_3;
		uint8_t dirstep_4;
		uint16_t outputs;
	};
	
} OUTPUT_REGISTER;

#endif
