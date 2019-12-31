#ifndef MCUMAP_ATMEGA328P_H
#define MCUMAP_ATMEGA328P_H

/*
	Setup the IO pins 
	The definition of the pins must match the PORT/REGISTER bit offset and not the IDE pin number
	with the formula:
	
	Ex:
	If DIR0 is bit 1 (<BIT_OFFSET>) of PORTC DIR0 will be 1
*/

//Setup output pins
//SAME AS GRBL for test purposes
#define STEP0 2
#define DIR0 5
#define STEP1 3
#define DIR1 6
#define STEP2 4
#define DIR2 7
#define STEPS_OUTREG PORTD
#define STEPS_DIRREG DDRD
#define DIRS_OUTREG PORTD
#define DIRS_DIRREG DDRD

//#define PROBE 5
#define LIMIT_X 1
#define LIMIT_Y 2
#define LIMIT_Z 4
#define LIMITS_INREG PINB
#define LIMITS_DIRREG DDRB
#define LIMITS_PULLUPREG PORTB
#define LIMITS_ISRREG PCMSK0
#define LIMITS_ISR_ID 0

//#define LIMIT_X_PULLUP
//#define LIMIT_Y_PULLUP
//#define LIMIT_Z_PULLUP

//Setup control input pins
#define ESTOP 0
#define FHOLD 1
#define CONTROLS_INREG PINC
#define CONTROLS_DIRREG DDRC
#define CONTROLS_PULLUPREG PORTC
#define CONTROLS_ISRREG PCMSK1
#define CONTROLS_ISR_ID 1

//#define ESTOP_PULLUP
//#define FHOLD_PULLUP

//Setup com pins
#define RX 0
#define TX 1
#define COM_INREG PIND
#define COM_OUTREG PORTD
#define COM_DIRREG DDRD

#define PWM0 3
#define PWM0_DIRREG DDRB
#define PWM0_REGINDEX 1
#define PWM0_TMRAREG TCCR2A
#define PWM0_TMRBREG TCCR2B
#define PWM0_CNTREG OCR2A

#endif
