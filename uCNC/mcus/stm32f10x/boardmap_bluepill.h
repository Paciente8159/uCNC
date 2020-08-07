/*
	Name: mcumap_grbl.h
	Description: Contains all MCU and PIN definitions for Arduino UNO to run µCNC.
	
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 04/02/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_BLUEPILL_H
#define BOARDMAP_BLUEPILL_H

//Setup external interrupt enabled ports
//#define EXTINT1_PORT B
//#define EXTINT2_PORT B
//#define EXTINT3_PORT C
//#define EXTINT4_PORT D

//Setup step pins
#define STEP2_BIT 2	 //assigns STEP2 pin
#define STEP2_PORT A //assigns STEP2 port
#define STEP1_BIT 1	 //assigns STEP1 pin
#define STEP1_PORT A //assigns STEP1 port
#define STEP0_BIT 0	 //assigns STEP0 pin
#define STEP0_PORT A //assigns STEP0 port

//Setup dir pins
#define DIR2_BIT 5	//assigns DIR2 pin
#define DIR2_PORT A //assigns DIR2 port
#define DIR1_BIT 4	//assigns DIR1 pin
#define DIR1_PORT A //assigns DIR1 port
#define DIR0_BIT 3	//assigns DIR0 pin
#define DIR0_PORT A //assigns DIR0 port

//Setup limit pins
#define LIMIT_Z_BIT 14 //assigns LIMIT_Z pin
#define LIMIT_Z_PORT B //assigns LIMIT_Z port
#define LIMIT_Y_BIT 13 //assigns LIMIT_Y pin
#define LIMIT_Y_PORT B //assigns LIMIT_Y port
#define LIMIT_X_BIT 12 //assigns LIMIT_X pin
#define LIMIT_X_PORT B //assigns LIMIT_X port

//Active limits switch weak pull-ups
#define LIMIT_X_PULLUP
#define LIMIT_Y_PULLUP
#define LIMIT_Z_PULLUP

//Setup probe pin
#define PROBE_BIT 9
#define PROBE_PORT B
//#define PROBE_ISR 1

//Setup control input pins
/*#define ESTOP_BIT 5
#define ESTOP_PORT B
//#define ESTOP_ISR
#define FHOLD_BIT 6
#define FHOLD_PORT B
#define CS_RES_BIT 7
#define CS_RES_PORT B
/*#define ESTOP_ISR 1
#define FHOLD_ISR 1
#define CS_RES_ISR 1*/

//Active controls switch weak pull-ups
//#define ESTOP_PULLUP
//#define FHOLD_PULLUP

//Setup COM pins (must match selected COM port)
//#define RX_BIT 10
//#define TX_BIT 9
//#define RX_PORT A
//#define TX_PORT A
//only uncomment this if not using USB VCP
//#define COM_PORT 1
//#define TX_BIT 9
//#define TX_PORT A
//#define RX_BIT 10
//#define RX_PORT A

//Setup PWM
#define PWM0_BIT 8	//assigns PWM0 pin
#define PWM0_PORT A //assigns PWM0 pin
#define PWM0_CHANNEL 1
#define PWM0_TIMER 1

//Setup generic IO Pins
//Functionalities are set in config.h file
#define DOUT0_BIT 0
#define DOUT0_PORT B
/*#define DOUT1_BIT 0
#define DOUT1_PORT B
#define DOUT2_BIT 3
#define DOUT2_PORT C
*/
#define DOUT15_BIT 13
#define DOUT15_PORT C

//Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 15
#define STEP0_EN_PORT A

//Setup the Step Timer used has the heartbeat for µCNC
#define TIMER_NUMBER 2

//in this case include de mcumap file to generate the definition do DOUT15 and assign to LED
#define LED DOUT15

#include "mcumap_stm32f10x.h"

#endif
