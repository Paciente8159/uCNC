/*
	Name: boardmap_fysetc_cheetah_v2.h
	Description: Contains all MCU and PIN definitions for board Fysetc Cheetah v2 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01-04-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_FYSETC_CHEETAH_v2
#define BOARDMAP_FYSETC_CHEETAH_v2

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_STM32F4X
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "Fysetc Cheetah v2"
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

// generated via https://paciente8159.github.io/uCNC-config-builder/
#define UART_PORT 1
#define UART2_PORT 2
#define SPI_PORT 1
#define SPI2_PORT 2
#define I2C_PORT 2
#define ITP_TIMER 2
#define SERVO_TIMER 9
#define ONESHOT_TIMER 1
#define STEP0_BIT 0
#define STEP0_PORT C
#define STEP1_BIT 14
#define STEP1_PORT C
#define STEP2_BIT 9
#define STEP2_PORT B
#define STEP3_BIT 2
#define STEP3_PORT B
#define DIR0_BIT 1
#define DIR0_PORT C
#define DIR1_BIT 13
#define DIR1_PORT C
#define DIR2_BIT 8
#define DIR2_PORT B
#define DIR3_BIT 15
#define DIR3_PORT A
#define STEP0_EN_BIT 8
#define STEP0_EN_PORT A
#define STEP1_EN_BIT 15
#define STEP1_EN_PORT C
#define STEP2_EN_BIT 2
#define STEP2_EN_PORT C
#define STEP3_EN_BIT 2
#define STEP3_EN_PORT D
#define PWM0_BIT 0
#define PWM0_PORT A
#define PWM0_CHANNEL 1
#define PWM0_TIMER 5
#define PWM1_BIT 1
#define PWM1_PORT A
#define PWM1_CHANNEL 2
#define PWM1_TIMER 5
#define PWM2_BIT 0
#define PWM2_PORT B
#define PWM2_CHANNEL 3
#define PWM2_TIMER 3
#define PWM3_BIT 6
#define PWM3_PORT B
#define PWM3_CHANNEL 1
#define PWM3_TIMER 4
#define PWM4_BIT 7
#define PWM4_PORT B
#define PWM4_CHANNEL 2
#define PWM4_TIMER 4
#define DOUT0_BIT 6
#define DOUT0_PORT C
#define DOUT1_BIT 7
#define DOUT1_PORT C
#define DOUT31_BIT 13
#define DOUT31_PORT C
#define LIMIT_X_BIT 4
#define LIMIT_X_PORT B
#define LIMIT_X_ISR
#define LIMIT_Y_BIT 8
#define LIMIT_Y_PORT C
#define LIMIT_Y_ISR
#define LIMIT_Z_BIT 1
#define LIMIT_Z_PORT B
#define LIMIT_Z_ISR
#define PROBE_BIT 0
#define PROBE_PORT A
#define ANALOG0_BIT 4
#define ANALOG0_PORT C
#define ANALOG0_CHANNEL 4
#define ANALOG1_BIT 5
#define ANALOG1_PORT C
#define ANALOG1_CHANNEL 5
#define DIN16_BIT 12
#define DIN16_PORT C
#define DIN17_BIT 11
#define DIN17_PORT C
#define DIN18_BIT 10
#define DIN18_PORT C
#define DIN19_BIT 3
#define DIN19_PORT C
#define DIN19_PULLUP
#define TX_BIT 9
#define TX_PORT A
#define RX_BIT 10
#define RX_PORT A
#define USB_DM_BIT 11
#define USB_DM_PORT A
#define USB_DP_BIT 12
#define USB_DP_PORT A
#define SPI_CLK_BIT 5
#define SPI_CLK_PORT A
#define SPI_SDI_BIT 6
#define SPI_SDI_PORT A
#define SPI_SDO_BIT 7
#define SPI_SDO_PORT A
#define SPI_CS_BIT 4
#define SPI_CS_PORT A
#define I2C_CLK_BIT 3
#define I2C_CLK_PORT B
#define I2C_DATA_BIT 10
#define I2C_DATA_PORT B
#define TX2_BIT 2
#define TX2_PORT A
#define RX2_BIT 3
#define RX2_PORT A
#define SPI2_CLK_BIT 13
#define SPI2_CLK_PORT B
#define SPI2_SDI_BIT 14
#define SPI2_SDI_PORT B
#define SPI2_SDO_BIT 15
#define SPI2_SDO_PORT B
#define SPI2_CS_BIT 12
#define SPI2_CS_PORT B

//Custom configurations
#define RX_PULLUP 
#define RX_PULLUP 

#ifdef __cplusplus
}
#endif

#endif
