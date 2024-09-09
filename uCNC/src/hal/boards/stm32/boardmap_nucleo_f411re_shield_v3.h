/*
	Name: boardmap_nucleo_f411re_shield_v3.h
	Description: Contains all MCU and PIN definitions for Arduino Nucleo F411 with CNC Shield V3 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 15-02-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_NUCLEO_F411RE_SHIELD_V3_H
#define BOARDMAP_NUCLEO_F411RE_SHIELD_V3_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_STM32F4X
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "STM32 Nucleo F411RE"
#endif

// Setup step pins
#define STEP0_BIT 10 // assigns STEP0 pin
#define STEP0_PORT A // assigns STEP0 port
#define STEP1_BIT 3	 // assigns STEP1 pin
#define STEP1_PORT B // assigns STEP1 port
#define STEP2_BIT 5	 // assigns STEP2 pin
#define STEP2_PORT B // assigns STEP2 port
// #define STEP3_BIT 6	 // assigns STEP3 pin but must comment Spindle PWM and Dir
// #define STEP3_PORT A // assigns STEP3 port but must comment Spindle PWM and Dir

// Setup dir pins
#define DIR0_BIT 4	// assigns DIR0 pin
#define DIR0_PORT B // assigns DIR0 port
#define DIR1_BIT 10 // assigns DIR1 pin
#define DIR1_PORT B // assigns DIR1 port
#define DIR2_BIT 8	// assigns DIR2 pin
#define DIR2_PORT A // assigns DIR2 port
// #define DIR3_BIT 5	// assigns DIR3 pin but must comment Spindle PWM and Dir
// #define DIR3_PORT A // assigns DIR3 port but must comment Spindle PWM and Dir

// Setup limit pins
#define LIMIT_X_BIT 7	 // assigns LIMIT_X pin
#define LIMIT_X_PORT C // assigns LIMIT_X port
#define LIMIT_Y_BIT 6	 // assigns LIMIT_Y pin
#define LIMIT_Y_PORT B // assigns LIMIT_Y port
#define LIMIT_Z_BIT 7	 // assigns LIMIT_Z pin
#define LIMIT_Z_PORT A // assigns LIMIT_Z port

// Enable limits switch interrupt
#define LIMIT_X_ISR
#define LIMIT_Y_ISR
#define LIMIT_Z_ISR
#define LIMIT_A_ISR

// Setup control input pins
#define ESTOP_BIT 0
#define ESTOP_PORT A
#define FHOLD_BIT 1
#define FHOLD_PORT A
#define CS_RES_BIT 4
#define CS_RES_PORT A

// Setup probe pin
#define PROBE_BIT 0
#define PROBE_PORT C

// Enable controls switch interrupt
#define ESTOP_ISR
#define FHOLD_ISR
#define CS_RES_ISR
#define SAFETY_DOOR_ISR

#define UART_PORT 2
#define TX_BIT 2
#define TX_PORT A
#define RX_BIT 3
#define RX_PORT A
#define RX_PULLUP

#define USB_DM_BIT 11
#define USB_DM_PORT A
#define USB_DP_BIT 12
#define USB_DP_PORT A

// Setup PWM
#define PWM0_BIT 6	// assigns PWM0 pin
#define PWM0_PORT A // assigns PWM0 pin
#define PWM0_CHANNEL 1
#define PWM0_TIMER 3

// Setup generic IO Pins
// Functionalities are set in cnc_hal_config.h file

// spindle dir
#define DOUT0_BIT 5
#define DOUT0_PORT A

// coolant
#define DOUT2_BIT 0
#define DOUT2_PORT B

// led pin
// #define DOUT31_BIT 5 // used by spindle dir
// #define DOUT31_PORT A // used by spindle dir

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 9
#define STEP0_EN_PORT A

	// analog input
	// #define ANALOG0_BIT 1
	// #define ANALOG0_PORT B
	// #define ANALOG0_CHANNEL 9

	// // servo pin
	// #define SERVO0_BIT 8
	// #define SERVO0_PORT B

	// Setup the Step Timer used has the heartbeat for µCNC
	// On STM32F1x cores this will default to Timer 2
	// #define ITP_TIMER 2

	// Setup the Timer to be used exclusively by servos in µCNC.
	// If no servos are configured then the timer is free for other functions (like PWM) (even if defined in the board)
	// On STM32F1x cores this will default to Timer 3
	// #define SERVO_TIMER 3

// I2C port
#define I2C_CLK_BIT 8
#define I2C_CLK_PORT B
#define I2C_DATA_BIT 9
#define I2C_DATA_PORT B
#define I2C_PORT 1
	// #define I2C_ADDRESS 1

#define ONESHOT_TIMER 1

#ifdef __cplusplus
}
#endif

#endif
