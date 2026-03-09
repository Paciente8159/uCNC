/*
	Name: boardmap_skr3.h
	Description: Contains all PIN definitions for BTT SKR3 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 28-02-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_SKR3_H
#define BOARDMAP_SKR3_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_STM32H7X
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "BTT SKR3"
#endif

#define RAM_ONLY_SETTINGS

// Setup step pins
#define STEP0_BIT 4	 // assigns STEP0 pin
#define STEP0_PORT D // assigns STEP0 port
#define STEP1_BIT 15 // assigns STEP1 pin
#define STEP1_PORT A // assigns STEP1 port
#define STEP2_BIT 2	 // assigns STEP2 pin
#define STEP2_PORT E // assigns STEP2 port
#define STEP3_BIT 15 // assigns STEP3 pin
#define STEP3_PORT D // assigns STEP3 port
#define STEP4_BIT 11 // assigns STEP4 pin
#define STEP4_PORT D // assigns STEP4 port

// Setup dir pins
#define DIR0_BIT 3	// assigns DIR0 pin
#define DIR0_PORT D // assigns DIR0 port
#define DIR1_BIT 8	// assigns DIR1 pin
#define DIR1_PORT A // assigns DIR1 port
#define DIR2_BIT 3	// assigns DIR2 pin
#define DIR2_PORT E // assigns DIR2 port
#define DIR3_BIT 14 // assigns DIR3 pin
#define DIR3_PORT D // assigns DIR3 port
#define DIR4_BIT 10 // assigns DIR4 pin
#define DIR4_PORT D // assigns DIR4 port

// Setup step EN pins
#define STEP0_EN_BIT 6	// assigns STEP0 pin
#define STEP0_EN_PORT D // assigns STEP0 port
#define STEP1_EN_BIT 1	// assigns STEP1 pin
#define STEP1_EN_PORT D // assigns STEP1 port
#define STEP2_EN_BIT 0	// assigns STEP2 pin
#define STEP2_EN_PORT E // assigns STEP2 port
#define STEP3_EN_BIT 7	// assigns STEP3 pin
#define STEP3_EN_PORT C // assigns STEP3 port
#define STEP4_EN_BIT 13 // assigns STEP4 pin
#define STEP4_EN_PORT D // assigns STEP4 port

// Setup step SPI (software emulated)
#define DIN29_BIT 15
#define DIN29_PORT E
#define DOUT29_BIT 13
#define DOUT29_PORT E
#define DOUT30_BIT 14
#define DOUT30_PORT E

// Setup step uart pins (one wire)
#define DOUT20_BIT 5
#define DOUT20_PORT D
#define DOUT21_BIT 0
#define DOUT21_PORT D
#define DOUT22_BIT 1
#define DOUT22_PORT E
#define DOUT23_BIT 6
#define DOUT23_PORT C
#define DOUT24_BIT 12
#define DOUT24_PORT D

// Setup limit pins
#define LIMIT_X_BIT 1	 // assigns LIMIT_X pin
#define LIMIT_X_PORT C // assigns LIMIT_X port
#define LIMIT_Y_BIT 3	 // assigns LIMIT_Y pin
#define LIMIT_Y_PORT C // assigns LIMIT_Y port
#define LIMIT_Z_BIT 0	 // assigns LIMIT_Z pin
#define LIMIT_Z_PORT C // assigns LIMIT_Z port
#define LIMIT_A_BIT 2	 // assigns LIMIT_A pin
#define LIMIT_A_PORT C // assigns LIMIT_A port
#define LIMIT_B_BIT 0	 // assigns LIMIT_B pin
#define LIMIT_B_PORT A // assigns LIMIT_B port

// Enable limits switch interrupt
// #define LIMIT_X_ISR
// #define LIMIT_Y_ISR
// #define LIMIT_Z_ISR
// #define LIMIT_A_ISR

// Setup control input pins
// #define ESTOP_BIT 5
// #define ESTOP_PORT B
// #define FHOLD_BIT 6
// #define FHOLD_PORT B
// #define CS_RES_BIT 7
// #define CS_RES_PORT B
// #define SAFETY_DOOR_BIT 8
// #define SAFETY_DOOR_PORT B
// Setup probe pin
#define PROBE_BIT 13
#define PROBE_PORT C

	// Enable controls switch interrupt
	// #define ESTOP_ISR
	// #define FHOLD_ISR
	// #define CS_RES_ISR
	// #define SAFETY_DOOR_ISR

#define UART_PORT 1
#define TX_BIT 9
#define TX_PORT A
#define RX_BIT 10
#define RX_PORT A
#define RX_PULLUP

// PIN A10 is also used because of the USB ID (USB OTG)
#define USB_DM_BIT 11
#define USB_DM_PORT A
#define USB_DP_BIT 12
#define USB_DP_PORT A

// Setup PWM
#define PWM0_BIT 7	// assigns PWM0 pin
#define PWM0_PORT B // assigns PWM0 pin
#define PWM0_CHANNEL 2
#define PWM0_TIMER 4

#define PWM1_BIT 6	// assigns PWM0 pin
#define PWM1_PORT B // assigns PWM0 pin
#define PWM1_CHANNEL 1
#define PWM1_TIMER 4

#define PWM2_BIT 5	// assigns PWM0 pin
#define PWM2_PORT B // assigns PWM0 pin
#define PWM2_CHANNEL 2
#define PWM2_TIMER 3

// Setup generic IO Pins
// Functionalities are set in cnc_hal_config.h file

// spindle dir
#define DOUT0_BIT 7
#define DOUT0_PORT D

// coolant and mist
#define DOUT2_BIT 3
#define DOUT2_PORT B
#define DOUT3_BIT 4
#define DOUT3_PORT B

// led pin
#define DOUT31_BIT 13
#define DOUT31_PORT A

// analog input
// #define ANALOG0_BIT 2
// #define ANALOG0_PORT A
// #define ANALOG0_CHANNEL 14
// #define ANALOG0_BIT 3
// #define ANALOG0_PORT A
// #define ANALOG0_CHANNEL 15

// servo pin
#define SERVO0_BIT 5
#define SERVO0_PORT E

// Setup the Step Timer used has the heartbeat for µCNC
// #define ITP_TIMER 2

// Setup the Timer to be used exclusively by servos in µCNC.
// If no servos are configured then the timer is free for other functions (like PWM) (even if defined in the board)
// On STM32F1x cores this will default to Timer 3
#define SERVO_TIMER 5

	// #define I2C_CLK_BIT 8
	// #define I2C_CLK_PORT B
	// #define I2C_DATA_BIT 9
	// #define I2C_DATA_PORT B
	// #define I2C_PORT 1
	// #define DIN7_BIT 5
	// #define DIN7_PORT A
	// #define DIN7_ISR
	// #define DIN7_PULLUP

	// #define SPI_CLK_PORT A
	// #define SPI_CLK_BIT 5
	// #define SPI_SDI_PORT A
	// #define SPI_SDI_BIT 6
	// #define SPI_SDO_PORT A
	// #define SPI_SDO_BIT 7
	// #define SPI_CS_PORT A
	// #define SPI_CS_BIT 4
	// #define SPI_PORT 1

	// #define SPI2_CLK_PORT B
	// #define SPI2_CLK_BIT 13
	// #define SPI2_SDI_PORT B
	// #define SPI2_SDI_BIT 14
	// #define SPI2_SDO_PORT B
	// #define SPI2_SDO_BIT 15
	// #define SPI2_PORT 2

#define ONESHOT_TIMER 1

#ifdef __cplusplus
}
#endif

#endif
