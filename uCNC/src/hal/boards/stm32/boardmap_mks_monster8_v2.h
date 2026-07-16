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

	** Also see for pin assign **
	https://github.com/makerbase-mks/MKS-Monster8/blob/main/hardware/MKS%20Monster8%20V2.0_003/MKS%20Monster8%20V2.0_003%20PIN.pdf
*/

#ifndef BOARDMAP_MKS_MONSTER8_V2_H
#define BOARDMAP_MKS_MONSTER8_V2_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_STM32F4X
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "MKS Monster8 V2"
#endif

// custom clocks setup
#define CUSTOM_CLOCKS_INIT true
#ifndef HSE_VALUE
#define HSE_VALUE 8000000UL
#endif
#define PLL_M (HSE_VALUE / 1000000)
#define PLL_N 336
#define PLL_P 2
#define PLL_Q 7
#define FLASH_LATENCY 5

// Setup step pins
#define STEP0_BIT 14
#define STEP0_PORT C
#define STEP1_BIT 5
#define STEP1_PORT E
#define STEP2_BIT 1
#define STEP2_PORT E
#define STEP3_BIT 5
#define STEP3_PORT B
#define STEP4_BIT 6
#define STEP4_PORT D
#define STEP5_BIT 2
#define STEP5_PORT D
#define STEP6_BIT 7
#define STEP6_PORT C
#define STEP7_BIT 13
#define STEP7_PORT D


// Setup dir pins
#define DIR0_BIT 13
#define DIR0_PORT C
#define DIR1_BIT 4
#define DIR1_PORT E
#define DIR2_BIT 0
#define DIR2_PORT E
#define DIR3_BIT 4
#define DIR3_PORT B
#define DIR4_BIT 5
#define DIR4_PORT D
#define DIR5_BIT 1
#define DIR5_PORT D
#define DIR6_BIT 6
#define DIR6_PORT C
#define DIR7_BIT 12
#define DIR7_PORT D


// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 15
#define STEP0_EN_PORT C
#define STEP1_EN_BIT 15
#define STEP1_EN_PORT C
#define STEP2_EN_BIT 2
#define STEP2_EN_PORT E
#define STEP3_EN_BIT 6
#define STEP3_EN_PORT B
#define STEP4_EN_BIT 7
#define STEP4_EN_PORT D
#define STEP5_EN_BIT 3
#define STEP5_EN_PORT D
#define STEP6_EN_BIT 8
#define STEP6_EN_PORT C
#define STEP7_EN_BIT 6
#define STEP7_EN_PORT B

// Setup limit pins
#define LIMIT_X_BIT 14
#define LIMIT_X_PORT A
#define LIMIT_X_PULLUP
#define LIMIT_Y_BIT 15
#define LIMIT_Y_PORT A
#define LIMIT_Y_PULLUP
#define LIMIT_Z_BIT 12 // use Z+ Limit SW connector
#define LIMIT_Z_PORT B // use Z+ Limit SW connector
#define LIMIT_Z_PULLUP // use Z+ Limit SW connector
#define LIMIT_Z2_BIT 13 // use Z- Limit SW connector
#define LIMIT_Z2_PORT B // use Z- Limit SW connector
#define LIMIT_Z2_PULLUP // use Z- Limit SW connector

// Enable limits switch interrupt
#define LIMIT_X_ISR
#define LIMIT_Y_ISR
#define LIMIT_Z_ISR
#define LIMIT_Z2_ISR

// Setup probe pin
// use MT_DET Limit SW connector
#define PROBE_BIT 13
#define PROBE_PORT A 
#define PROBE_ISR

// Setup COM pins
#define UART_PORT 1
#define TX_BIT 9
#define TX_PORT A
#define RX_BIT 10
#define RX_PORT A

#define UART2_PORT 0

#define I2C_PORT 1
#define I2C_CLK_BIT 8
#define I2C_CLK_PORT B
#define I2C_DATA_BIT 9
#define I2C_DATA_PORT B

#define USB_DM_BIT 11
#define USB_DM_PORT A
#define USB_DP_BIT 12
#define USB_DP_PORT A

// Setup PWM
#define PWM0_BIT 1     // HE0 connector
#define PWM0_PORT B
#define PWM0_CHANNEL 4
#define PWM0_TIMER 3 
#define PWM1_BIT 0     // HE1 connector
#define PWM1_PORT B
#define PWM1_CHANNEL 3
#define PWM1_TIMER 3
#define PWM2_BIT 3     // HE2 connector
#define PWM2_PORT A
#define PWM2_CHANNEL 4
#define PWM2_TIMER 5
#define PWM3_BIT 10    // H-BED connector
#define PWM3_PORT B
#define PWM3_CHANNEL 3
#define PWM3_TIMER 2

// digital out
#define DOUT0_BIT 2   // FAN0 connector
#define DOUT0_PORT A
#define DOUT1_BIT 1   // FAN1 connector
#define DOUT1_PORT A
#define DOUT2_BIT 0   // FAN2 connector
#define DOUT2_PORT A

// spindle dir
// analog input
#define ANALOG0_BIT 0        // TB connector
#define ANALOG0_PORT C
#define ANALOG0_CHANNEL 10
#define ANALOG1_BIT 1        // TH0 connector
#define ANALOG1_PORT C
#define ANALOG1_CHANNEL 11
#define ANALOG2_BIT 2        // TH1 connector
#define ANALOG2_PORT C
#define ANALOG2_CHANNEL 12
#define ANALOG3_CHANNEL -1
#define ANALOG4_CHANNEL -1
#define ANALOG5_CHANNEL -1
#define ANALOG6_CHANNEL -1
#define ANALOG7_CHANNEL -1
#define ANALOG8_CHANNEL -1
#define ANALOG9_CHANNEL -1
#define ANALOG10_CHANNEL -1
#define ANALOG11_CHANNEL -1
#define ANALOG12_CHANNEL -1
#define ANALOG13_CHANNEL -1
#define ANALOG14_CHANNEL -1
#define ANALOG15_CHANNEL -1


#define SERVO0_BIT 8    // 3DTOUCH connector
#define SERVO0_PORT A

// Setup the Step Timer used has the heartbeat for µCNC
#define ITP_TIMER 7
#define SERVO_TIMER 4

// SPI for card access
#define SPI_CLK_BIT 10
#define SPI_CLK_PORT C
#define SPI_SDI_BIT 11
#define SPI_SDI_PORT C
#define SPI_SDO_BIT 12
#define SPI_SDO_PORT C
#define SPI_CS_BIT 9
#define SPI_CS_PORT C
#define SPI_PORT 3

// SD detect pin
#define DIN19_BIT 4
#define DIN19_PORT C

// SPI for displays
#define SPI2_CS_BIT 15
#define SPI2_CS_PORT E
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
