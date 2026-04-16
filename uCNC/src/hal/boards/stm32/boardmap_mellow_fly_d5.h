/*
	Name: boardmap_mellow_fly_d5.h
	Description: Contains all MCU and PIN definitions for Bluepill F0 variant to run µCNC.

	Copyright: Copyright (c) Abdul M Waraich
	Author: Abdul M Waraich
	Date: 08-03-2026

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_MELLOW_FLY_D5_H
#define BOARDMAP_MELLOW_FLY_D5_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_STM32F0X
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "Mellow Fly D5"
#endif

#define BAUDRATE2 BAUDRATE

#define UART_PORT 3
//#define UART2_PORT 3
#define SPI_PORT 2
#define I2C_PORT 1
#define ITP_TIMER 2
#define SERVO_TIMER 14
#define ONESHOT_TIMER 15
#define IC74HC595_COUNT 0
#define IC74HC165_COUNT 0

#define STEP0_BIT 15
#define STEP0_PORT C
#define STEP1_BIT 1
#define STEP1_PORT A
#define STEP2_BIT 5
#define STEP2_PORT A
#define STEP3_BIT 5
#define STEP3_PORT C
#define STEP4_BIT 10
#define STEP4_PORT B
#define DIR0_BIT 14
#define DIR0_PORT C
#define DIR1_BIT 0
#define DIR1_PORT A
#define DIR2_BIT 4
#define DIR2_PORT A
#define DIR3_BIT 4
#define DIR3_PORT C
#define DIR4_BIT 2
#define DIR4_PORT B
#define STEP0_EN_BIT 2
#define STEP0_EN_PORT C
#define STEP1_EN_BIT 2
#define STEP1_EN_PORT A
#define STEP2_EN_BIT 6
#define STEP2_EN_PORT A
#define STEP3_EN_BIT 0
#define STEP3_EN_PORT B
#define STEP4_EN_BIT 11
#define STEP4_EN_PORT B

#define DOUT31_BIT 8	//LED pin to test the board
#define DOUT31_PORT C	//LED port to test the board 

//#define PWM0_BIT 8
//#define PWM0_PORT C
//#define PWM0_CHANNEL 3
//#define PWM0_TIMER 3

#define SERVO0_BIT 7
#define SERVO0_PORT A

//#define DOUT0_BIT 13
//#define DOUT0_PORT A
#define DOUT1_BIT 10
#define DOUT1_PORT A
#define DOUT2_BIT 9
#define DOUT2_PORT C
#define DOUT3_BIT 6
#define DOUT3_PORT C

#define LIMIT_X_BIT 4
#define LIMIT_X_PORT B
#define LIMIT_X_PULLUP
#define LIMIT_X_ISR
#define LIMIT_Y_BIT 3
#define LIMIT_Y_PORT B
#define LIMIT_Y_PULLUP
#define LIMIT_Y_ISR 2
#define LIMIT_Z_BIT 2
#define LIMIT_Z_PORT D
#define LIMIT_Z_PULLUP
#define LIMIT_Z_ISR
#define LIMIT_Y2_BIT 9
#define LIMIT_Y2_PORT A
#define LIMIT_Y2_PULLUP
#define LIMIT_Y2_ISR
#define PROBE_BIT 5
#define PROBE_PORT B
#define PROBE_PULLUP
#define PROBE_ISR
/*
#define ESTOP_BIT 1
#define ESTOP_PORT C
#define ESTOP_PULLUP
#define ESTOP_ISR
#define SAFETY_DOOR_BIT 12
#define SAFETY_DOOR_PORT C
#define SAFETY_DOOR_PULLUP
#define FHOLD_BIT 0
#define FHOLD_PORT C
#define FHOLD_PULLUP
#define FHOLD_ISR
#define CS_RES_BIT 3
#define CS_RES_PORT C
#define CS_RES_PULLUP
#define CS_RES_ISR
*/

//#define DIN0_BIT 14
//#define DIN0_PORT A
//#define DIN0_PULLUP
//#define DIN0_ISR

//USB pins are defined in variant_MELLOW_STM32F072.h as they are used by the USB stack and need to be defined before including the USB stack headers
#define USB_DM_BIT 11
#define USB_DM_PORT A
#define USB_DP_BIT 12
#define USB_DP_PORT A

#define I2C_CLK_BIT 6
#define I2C_CLK_PORT B
#define I2C_DATA_BIT 7
#define I2C_DATA_PORT B

#define TX_BIT 10
#define TX_PORT C
#define RX_BIT 11
#define RX_PORT C

#define SPI_CLK_BIT 13
#define SPI_CLK_PORT B
#define SPI_SDI_BIT 14
#define SPI_SDI_PORT B
#define SPI_SDO_BIT 15
#define SPI_SDO_PORT B
#define SPI_CS_BIT 12
#define SPI_CS_PORT B

#ifdef __cplusplus
}
#endif
#endif
