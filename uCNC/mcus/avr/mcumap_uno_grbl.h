/*
	Name: mcumap_uno_grbl.h
	Description: Contains all MCU and PIN definitions for Arduino UNO to run µCNC.
	
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUMAP_UNO_GRBL_H
#define MCUMAP_UNO_GRBL_H

//used by the parser
//this method is faster then normal multiplication (for 32 bit for 16 and 8 bits is slightly lower)
//if not defined utils.h will use standard math operation
#define fast_mult10(X) ((((X)<<2) + (X))<<1)

#define PCINT0_PORT B
#define PCINT1_PORT C
#define PCINT2_PORT D

//SAME AS GRBL for test purposes
//Setup step pins
#define STEP2_BIT 4 //assigns STEP2 pin
#define STEP2_PORT D //assigns STEP2 port
#define STEP1_BIT 3 //assigns STEP1 pin
#define STEP1_PORT D //assigns STEP1 port
#define STEP0_BIT 2 //assigns STEP0 pin
#define STEP0_PORT D //assigns STEP0 port

//Setup dir pins
#define DIR2_BIT 7 //assigns DIR2 pin
#define DIR2_PORT D //assigns DIR2 port
#define DIR1_BIT 6 //assigns DIR1 pin
#define DIR1_PORT D //assigns DIR1 port
#define DIR0_BIT 5 //assigns DIR0 pin
#define DIR0_PORT D //assigns DIR0 port

//Setup limit pins
#define LIMIT_Z_BIT 4 //assigns LIMIT_Z pin
#define LIMIT_Y_BIT 2 //assigns LIMIT_Y pin
#define LIMIT_X_BIT 1 //assigns LIMIT_X pin
#define LIMIT_Z_PORT B //assigns LIMIT_Z port
#define LIMIT_Y_PORT B //assigns LIMIT_Y port
#define LIMIT_X_PORT B //assigns LIMIT_X port
#define LIMIT_Z_ISR 0 //assigns LIMIT_Z ISR
#define LIMIT_Y_ISR 0 //assigns LIMIT_Y ISR
#define LIMIT_X_ISR 0 //assigns LIMIT_X ISR

//Active limits switch weak pull-ups
//#define LIMIT_X_PULLUP
//#define LIMIT_Y_PULLUP
//#define LIMIT_Z_PULLUP

//Setup probe pin
#define PROBE_BIT 5
#define PROBE_PORT C
#define PROBE_ISR 1

//Setup control input pins
#define ESTOP_BIT 0
#define FHOLD_BIT 1
#define CS_RES_BIT 2
#define ESTOP_PORT C
#define FHOLD_PORT C
#define CS_RES_PORT C
#define ESTOP_ISR 1
#define FHOLD_ISR 1
#define CS_RES_ISR 1

//Active controls switch weak pull-ups
//#define ESTOP_PULLUP
//#define FHOLD_PULLUP

//Setup com pins
#define RX_BIT 0
#define TX_BIT 1
#define RX_PORT D
#define TX_PORT D
//#define COM_ID 0 //if only one or default comment this

//Setup PWM
#define PWM0_BIT 3 //assigns PWM0 pin
#define PWM0_PORT B //assigns PWM0 pin
#define PWM0_OCR A
#define PWM0_TIMER 2

//Setup generic IO Pins
//Functions are set in config.h file
#define DOUT0_BIT 5
#define DOUT0_PORT B
#define DOUT1_BIT 0
#define DOUT1_PORT B
#define DOUT2_BIT 3
#define DOUT2_PORT C

//Stepper enable pin. For UNO a single pin is used
#define STEP0_EN_BIT 0
#define STEP0_EN_PORT B

//Setup the Step Timer used has the heartbeat for µCNC
#define TIMER_NUMBER 1

#endif
