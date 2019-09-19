#ifndef PINS_UNO_H
#define PINS_UNO_H

#include <avr/io.h>
//Regiter Mapping

//Maps the UNO internal 8-bit registers to the 32-bit machine register
//<PORT3>-<PORT2>-<PORT1>-<PORT0>

//PORT0
//#define PORTWR0 PORTB
//#define PORTRD0 PINB
//#define PORTDIR0 DDRB
//#define PORTISR0 PCMSK0

//port c as only outputs
//PORT1
#define PORTWR1 PORTC
//#define PORTRD1 PINC
#define PORTDIR1 DDRC
//#define PORTISR1 PCMSK1

//PORT2
#define PORTWR2 PORTD
#define PORTDIR2 DDRD
#define PORTRD2 PIND
#define PORTISR2 PCMSK2

//PORT3
//doesn't have

//SERIAL PORT INDEX (protects port changes during output update
//#define SERIALPORTINDEX 5
//#define SERIALRESET 0xFC
//#define SERIALRESTORE (PORTD&0x03)

//setup the IO pins
//the definition of the pins must match the REGISTER bit offset and not the IDE pin number

//Setup input pins
#define RX 16
#define TX 17
#define ESTOP 18
#define FHOLD 19
#define PROBE 20 //probe
#define LIMIT_X 21
#define LIMIT_Y 22
#define LIMIT_Z 23

//Setup output pins
#define STEP0 8
#define STEP1 10
#define STEP2 12
#define DIR0 9
#define DIR1 11
#define DIR2 13
/*#define DIR0 5
#define STEP1 3
#define DIR1 6
#define STEP2 4
#define DIR2 7
#define DOUT0 16 //stepper enable
#define DOUT1 17 //coolant enable
#define DOUT2 18 //spindle dir
#define PWM0 19 //spindle speed control
*/
/*
//maps the inputs and outputs to the internal 32bit register
typedef union
{
	uint32_t reg32in;
	struct
	{
		unsigned : 16;
		unsigned : 2;
		unsigned emergencystop : 1;
		unsigned feedhold : 1;
		unsigned probe : 1;
		unsigned limit_x : 1;
		unsigned limit_y : 1;
		unsigned limit_z : 1;
		unsigned : 8;
	}; //digital inputs pins
	
	struct
	{
		unsigned : 16;
		unsigned : 2;
		unsigned critical_inputs : 6;
		unsigned : 8;
	};
} INPUT_REGISTER;

typedef union
{
	uint32_t reg32out;
	struct
	{
		unsigned : 8;
		unsigned step0 : 1;
		unsigned dir0 : 1;
		unsigned step1 : 1;
		unsigned dir1 : 1;
		unsigned step2 : 1;
		unsigned dir2 : 1;
		unsigned : 2;
		unsigned : 16;
	};
} OUTPUT_REGISTER;
*/
#endif
