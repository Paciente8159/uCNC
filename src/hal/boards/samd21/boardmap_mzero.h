/*
	Name: mcumap_mzero.h
	Description: Contains all MCU and PIN definitions for Arduino M0 to run µCNC.
	
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 09-08-2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_MZERO_H
#define BOARDMAP_MZERO_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SAMD21
#define SAMD21
#endif

#ifndef F_CPU
#define F_CPU 48000000UL
#endif

//Setup COM pins (must match selected COM port)
//Comment these to use USB virtual port
#ifndef USB_VCP
#define COM_PORT 1
#endif

//Setup step pins
#define STEP0_BIT 14 //assigns STEP0 pin
#define STEP0_PORT A //assigns STEP0 port
#define STEP1_BIT 9	 //assigns STEP1 pin
#define STEP1_PORT A //assigns STEP1 port
#define STEP2_BIT 8	 //assigns STEP2 pin
#define STEP2_PORT A //assigns STEP2 port

//Setup dir pins
#define DIR0_BIT 15 //assigns DIR0 pin
#define DIR0_PORT A //assigns DIR0 port
#define DIR1_BIT 20 //assigns DIR1 pin
#define DIR1_PORT A //assigns DIR1 port
#define DIR2_BIT 21 //assigns DIR2 pin
#define DIR2_PORT A //assigns DIR2 port

//Setup limit pins
#define LIMIT_X_BIT 7  //assigns LIMIT_X pin
#define LIMIT_X_PORT A //assigns LIMIT_X port
#define LIMIT_Y_BIT 18 //assigns LIMIT_Y pin
#define LIMIT_Y_PORT A //assigns LIMIT_Y port
#define LIMIT_Z_BIT 19 //assigns LIMIT_Z pin
#define LIMIT_Z_PORT A //assigns LIMIT_Z port

//Enable limits switch weak pull-ups
#define LIMIT_X_PULLUP
#define LIMIT_Y_PULLUP
#define LIMIT_Z_PULLUP

//Enable limits switch interrupt
#define LIMIT_X_ISR
#define LIMIT_Y_ISR
#define LIMIT_Z_ISR

//Setup control input pins
#define ESTOP_BIT 1
#define ESTOP_PORT A
#define FHOLD_BIT 8
#define FHOLD_PORT B
#define CS_RES_BIT 9
#define CS_RES_PORT B

//Setup probe pin
#define PROBE_BIT 2
#define PROBE_PORT B

//Enable controls switch weak pull-ups
#define ESTOP_PULLUP
#define FHOLD_PULLUP
#define CS_RES_PULLUP

//Enable controls switch interrupt
#define ESTOP_ISR
#define FHOLD_ISR
#define CS_RES_ISR

//On the STM32 always use sync TX UART (async doesn't work well)
#ifdef COM_PORT
#define TX_BIT 10
#define TX_PORT A
#define RX_BIT 11
#define RX_PORT A
#else
#define USB_DM_BIT 24
#define USB_DM_PORT A
#define USB_DM_MUX G
#define USB_DP_BIT 25
#define USB_DP_PORT A
#define USB_DP_MUX G
#endif

//Setup PWM
#define PWM0_BIT 16 //assigns PWM0 pin
#define PWM0_PORT A //assigns PWM0 pin
#define PWM0_CHANNEL 1
#define PWM0_TIMER 1

//Setup generic IO Pins
//Functionalities are set in cnc_hal_config.h file

//spindle dir
#define DOUT0_BIT 17
#define DOUT0_PORT A

//teste led pin
#define DOUT15_BIT 17
#define DOUT15_PORT A

//coolant
#define DOUT1_BIT 4
#define DOUT1_PORT A

//stepper enable
#define DOUT3_BIT 15
#define DOUT3_PORT A

//Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEPPER_ENABLE_BIT 6
#define STEPPER_ENABLE_PORT A

	//Setup the Step Timer used has the heartbeat for µCNC
	//#define ITP_TIMER 2

#ifdef __cplusplus
}
#endif

#endif
