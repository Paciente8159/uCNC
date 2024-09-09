/*
	Name: boardmap_mks_robin_nano_v1_2.h
	Description: Contains all MCU and PIN definitions for board MKS Robin Nano V1.2 to run µCNC.

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

#ifndef BOARDMAP_MKS_ROBIN_NANO_V31_H
#define BOARDMAP_MKS_ROBIN_NANO_V31_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_STM32F4X
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "MKS Robin Nano 3.1"
#endif

// custom clocks setup
#define CUSTOM_CLOCKS_INIT
#ifndef HSE_VALUE
#define HSE_VALUE 8000000UL
#endif
#define PLL_M (HSE_VALUE / 1000000)
#define PLL_N 336
#define PLL_P 2
#define PLL_Q 7
#define FLASH_LATENCY 5

// Setup step pins
#define STEP0_BIT 3	 // assigns STEP0 pin
#define STEP0_PORT E // assigns STEP0 port
#define STEP1_BIT 0	 // assigns STEP1 pin
#define STEP1_PORT E // assigns STEP1 port
#define STEP2_BIT 5	 // assigns STEP2 pin
#define STEP2_PORT B // assigns STEP2 port
#define STEP3_BIT 6	 // assigns STEP3 pin
#define STEP3_PORT D // assigns STEP3 port
#define STEP4_BIT 15 // assigns STEP3 pin
#define STEP4_PORT D // assigns STEP3 port

// Setup dir pins
#define DIR0_BIT 2	// assigns DIR0 pin
#define DIR0_PORT E // assigns DIR0 port
#define DIR1_BIT 9	// assigns DIR1 pin
#define DIR1_PORT B // assigns DIR1 port
#define DIR2_BIT 4	// assigns DIR2 pin
#define DIR2_PORT B // assigns DIR2 port
#define DIR3_BIT 3	// assigns DIR3 pin
#define DIR3_PORT D // assigns DIR3 port
#define DIR4_BIT 1	// assigns DIR3 pin
#define DIR4_PORT A // assigns DIR3 port

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 4
#define STEP0_EN_PORT E
#define STEP1_EN_BIT 1
#define STEP1_EN_PORT E
#define STEP2_EN_BIT 8
#define STEP2_EN_PORT B
#define STEP3_EN_BIT 3
#define STEP3_EN_PORT B
#define STEP4_EN_BIT 3
#define STEP4_EN_PORT A

// Setup limit pins
#define LIMIT_X_BIT 15	// assigns LIMIT_X pin
#define LIMIT_X_PORT A	// assigns LIMIT_X port
#define LIMIT_Y_BIT 2		// assigns LIMIT_Y pin
#define LIMIT_Y_PORT D	// assigns LIMIT_Y port
#define LIMIT_Z_BIT 4		// assigns LIMIT_Z+ pin
#define LIMIT_Z_PORT C	// assigns LIMIT_Z+ port
#define LIMIT_Z2_BIT 8	// assigns LIMIT_Z- pin
#define LIMIT_Z2_PORT C // assigns LIMIT_Z- port

// Enable limits switch interrupt
#define LIMIT_X_ISR
#define LIMIT_Y_ISR
#define LIMIT_Z_ISR
#define LIMIT_Z2_ISR

// Setup probe pin
#define PROBE_BIT 4
#define PROBE_PORT A
#define PROBE_ISR

// Setup COM pins
#define UART_PORT 1
#define TX_BIT 9
#define TX_PORT A
#define RX_BIT 10
#define RX_PORT A
#define RX_PULLUP

#define USB_DM_BIT 11
#define USB_DM_PORT A
#define USB_DP_BIT 12
#define USB_DP_PORT A

// Setup PWM
#define PWM0_BIT 0	// assigns PWM0 pin
#define PWM0_PORT A // assigns PWM0 pin
#define PWM0_CHANNEL 1
#define PWM0_TIMER 2
#define PWM1_BIT 5	// assigns PWM2 pin
#define PWM1_PORT E // assigns PWM2 pin
#define PWM1_CHANNEL 1
#define PWM1_TIMER 9
#define PWM2_BIT 0	// assigns PWM3 pin
#define PWM2_PORT B // assigns PWM3 pin
#define PWM2_CHANNEL 3
#define PWM2_TIMER 3

// spindle dir
// analog input
#define ANALOG0_BIT 0
#define ANALOG0_PORT C
#define ANALOG0_CHANNEL 10
#define ANALOG1_BIT 1
#define ANALOG1_PORT C
#define ANALOG1_CHANNEL 11
#define ANALOG2_BIT 2
#define ANALOG2_PORT C
#define ANALOG2_CHANNEL 12

#define SERVO0_BIT 8
#define SERVO0_PORT A

// Setup the Step Timer used has the heartbeat for µCNC
#define ITP_TIMER 5
#define SERVO_TIMER 4

// SPI for card access
#define SPI_CS_BIT 9
#define SPI_CS_PORT C
#define SPI_CLK_BIT 10
#define SPI_CLK_PORT C
#define SPI_SDO_BIT 12
#define SPI_SDO_PORT C
#define SPI_SDI_BIT 11
#define SPI_SDI_PORT C
#define SPI_PORT 3
// SD detect pin
#define DIN19_BIT 12
#define DIN19_PORT D

// SPI for displays
#define SPI2_CS_BIT 11
#define SPI2_CS_PORT D
#define SPI2_CLK_BIT 5
#define SPI2_CLK_PORT A
#define SPI2_SDO_BIT 7
#define SPI2_SDO_PORT A
#define SPI2_SDI_BIT 6
#define SPI2_SDI_PORT A
#define SPI2_PORT 1

#define ONESHOT_TIMER 8

#ifdef __cplusplus
}
#endif

#endif
