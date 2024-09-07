/*
	Name: boardmap_melzi114.h
	Description: Contains all MCU and PIN definitions for Arduino Melzi v1.1.4 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07-09-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_MELZI_V114_H
#define BOARDMAP_MELZI_V114_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "Melzi 1.1.4"
#endif

#define AVR6

#include "mega_ide_pins.h"

#define PCINT0_PORT A
#define PCINT1_PORT B
#define PCINT2_PORT C

// Setup step pins
#define STEP0_BIT 7
#define STEP0_PORT D
#define STEP1_BIT 6
#define STEP1_PORT C
#define STEP2_BIT 3
#define STEP2_PORT B
#define STEP3_BIT 1
#define STEP3_PORT B

// Setup dir pins
#define DIR0_BIT 5
#define DIR0_PORT C
#define DIR1_BIT 7
#define DIR1_PORT C
#define DIR2_BIT 2
#define DIR2_PORT B
#define DIR3_BIT 0
#define DIR3_PORT B

// Enable pins
#define STEP0_EN_BIT 6
#define STEP0_EN_PORT D
#define STEP1_EN_BIT 6
#define STEP1_EN_PORT D
#define STEP2_EN_BIT 5
#define STEP2_EN_PORT A
#define STEP3_EN_BIT 6
#define STEP3_EN_PORT D

	// Setup limit pins
#define LIMIT_X_BIT 2
#define LIMIT_X_PORT C
#define LIMIT_X_PULLUP
#define LIMIT_X_ISR 2
#define LIMIT_Y_BIT 3
#define LIMIT_Y_PORT C
#define LIMIT_Y_PULLUP
#define LIMIT_Y_ISR 2
#define LIMIT_Z_BIT 4
#define LIMIT_Z_PORT C
#define LIMIT_Z_PULLUP
#define LIMIT_Z_ISR 2

// Setup com pins
#define TX_BIT 1
#define TX_PORT D
#define RX_BIT 0
#define RX_PORT D
#define RX_PULLUP
// only uncomment this if other port other then 0 is used
#define UART_PORT 0

#define TX2_BIT 3
#define TX2_PORT D
#define RX2_BIT 2
#define RX2_PORT D
#define RX2_PULLUP
// only uncomment this if other port other then 0 is used
#define UART2_PORT 1

// Setup PWM
#define PWM0_BIT 4
#define PWM0_PORT B
#define PWM0_CHANNEL B
#define PWM0_TIMER 0

// Setup generic IO Pins
// Functionalities are set in config.h file

// blink led
#define DOUT31_BIT 4
#define DOUT31_PORT A

#define DOUT0_BIT 5
#define DOUT0_PORT D
#define DOUT1_BIT 6
#define DOUT1_PORT D

// hardware I2C
#define I2C_CLK_BIT 0
#define I2C_CLK_PORT D
#define I2C_DATA_BIT 1
#define I2C_DATA_PORT D

// hardware SPI
#define SPI_CLK_BIT 7
#define SPI_CLK_PORT B
#define SPI_SDI_BIT 6
#define SPI_SDI_PORT B
#define SPI_SDO_BIT 5
#define SPI_SDO_PORT B
#define SPI_CS_BIT 0
#define SPI_CS_PORT A
#define SPI_PORT 0
#define SPI_FREQ 1000000UL

// timers
#define ITP_TIMER 1
#define RTC_TIMER 2
#define ONESHOT_TIMER 0

#ifdef __cplusplus
}
#endif

#endif
