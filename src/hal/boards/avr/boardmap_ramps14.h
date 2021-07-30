/*
	Name: boardmap_ramps14.h
	Description: Contains all MCU and PIN definitions for Arduino UNO to run µCNC.
	
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 05/02/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUMAP_RAMPS14_H
#define MCUMAP_RAMPS14_H

#ifdef __cplusplus
extern "C"
{
#endif

#define PCINT0_PORT B
#define PCINT1_PORT J
#define PCINT2_PORT K

//SAME AS GRBL for test purposes
//Setup step pins
#define STEP2_BIT 3	 //assigns STEP2 pin
#define STEP2_PORT L //assigns STEP2 port
#define STEP1_BIT 6	 //assigns STEP1 pin
#define STEP1_PORT F //assigns STEP1 port
#define STEP0_BIT 0	 //assigns STEP0 pin
#define STEP0_PORT F //assigns STEP0 port

//Setup dir pins
#define DIR2_BIT 1	//assigns DIR2 pin
#define DIR2_PORT L //assigns DIR2 port
#define DIR1_BIT 7	//assigns DIR1 pin
#define DIR1_PORT F //assigns DIR1 port
#define DIR0_BIT 1	//assigns DIR0 pin
#define DIR0_PORT F //assigns DIR0 port

//Setup limit pins
#define LIMIT_Z_BIT 3  //assigns LIMIT_Z pin
#define LIMIT_Y_BIT 1  //assigns LIMIT_Y pin
#define LIMIT_X_BIT 5  //assigns LIMIT_X pin
#define LIMIT_Z_PORT D //assigns LIMIT_Z port
#define LIMIT_Y_PORT J //assigns LIMIT_Y port
#define LIMIT_X_PORT E //assigns LIMIT_X port
#define LIMIT_Z_ISR -4 //assigns LIMIT_Z ISR
#define LIMIT_Y_ISR 1  //assigns LIMIT_Y ISR
#define LIMIT_X_ISR -6 //assigns LIMIT_X ISR

//Active limits switch weak pull-ups
//#define LIMIT_X_PULLUP
//#define LIMIT_Y_PULLUP
//#define LIMIT_Z_PULLUP

//Setup probe pin
#define PROBE_BIT 2
#define PROBE_PORT D
#define PROBE_ISR -3

//Setup control input pins
/*#define ESTOP_BIT 0
#define FHOLD_BIT 1
#define CS_RES_BIT 2
#define ESTOP_PORT C
#define FHOLD_PORT C
#define CS_RES_PORT C
#define ESTOP_ISR 1
#define FHOLD_ISR 1
#define CS_RES_ISR 1
*/
//Active controls switch weak pull-ups
//#define ESTOP_PULLUP
//#define FHOLD_PULLUP

//Setup com pins
#define RX_BIT 0
#define TX_BIT 1
#define RX_PORT E
#define TX_PORT E
//only uncomment this if other port other then 0 is used
//#define COM_NUMBER 0

//Setup PWM
#define PWM0_BIT 5	//assigns PWM0 pin
#define PWM0_PORT H //assigns PWM0 pin
#define PWM0_OCR C
#define PWM0_TIMER 4

//Setup generic IO Pins
//Functionalities are set in config.h file
/*#define DOUT0_BIT 5
#define DOUT0_PORT B
#define DOUT1_BIT 0
#define DOUT1_PORT B
#define DOUT2_BIT 3
#define DOUT2_PORT C*/

//Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEPPER_ENABLE_BIT 7
#define STEPPER_ENABLE_PORT D
#define STEPPER1_ENABLE_BIT 2
#define STEPPER1_ENABLE_PORT F
#define STEPPER2_ENABLE_BIT 0
#define STEPPER2_ENABLE_PORT K

	//Setup the Step Timer used has the heartbeat for µCNC
	//Timer 1 is used by default
	//#define ITP_TIMER 1
	//Setup the RTC Timer used by µCNC to provide an (mostly) accurate time base for all time dependent functions
	//Timer 0 is set by default
	//#define RTC_TIMER 0

#ifdef __cplusplus
}
#endif

#endif
