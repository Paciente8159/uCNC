/*
	Name: boardmap_skr_v14_turbo.h
	Description: Contains all MCU and PIN definitions for MKS Base V1.3 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/10/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_SKR_V14_TURBO_H
#define BOARDMAP_SKR_V14_TURBO_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "SKR v1.4 Turbo"
#endif

#ifndef F_CPU
#define F_CPU 100000000UL
#endif

// Setup step pins
#define STEP0_BIT 2	 // assigns STEP0 pin
#define STEP0_PORT 2 // assigns STEP0 port
#define STEP1_BIT 19	 // assigns STEP1 pin
#define STEP1_PORT 0 // assigns STEP1 port
#define STEP2_BIT 22	 // assigns STEP2 pin
#define STEP2_PORT 0 // assigns STEP2 port
#define STEP3_BIT 13	 // assigns STEP3 pin
#define STEP3_PORT 2 // assigns STEP3 port
#define STEP4_BIT 15	 // assigns STEP4 pin
#define STEP4_PORT 1 // assigns STEP4 port

// Setup dir pins
#define DIR0_BIT 6 // assigns DIR0 pin
#define DIR0_PORT 2 // assigns DIR0 port
#define DIR1_BIT 20 // assigns DIR1 pin
#define DIR1_PORT 0 // assigns DIR1 port
#define DIR2_BIT 11 // assigns DIR2 pin
#define DIR2_PORT 2 // assigns DIR2 port
#define DIR3_BIT 11	// assigns DIR2 pin
#define DIR3_PORT 0 // assigns DIR2 port
#define DIR4_BIT 14 // assigns DIR2 pin
#define DIR4_PORT 1 // assigns DIR2 port

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 1
#define STEP0_EN_PORT 2
#define STEP1_EN_BIT 8
#define STEP1_EN_PORT 2
#define STEP2_EN_BIT 21
#define STEP2_EN_PORT 0
#define STEP3_EN_BIT 12
#define STEP3_EN_PORT 2
#define STEP4_EN_BIT 16
#define STEP4_EN_PORT 1

	// Setup limit pins

#define LIMIT_X_BIT 29 // assigns LIMIT_X pin
#define LIMIT_X_PORT 1 // assigns LIMIT_X port
// #define LIMIT_X_ISR -6	// assigns LIMIT_X ISR
#define LIMIT_X2_BIT 26 // assigns LIMIT_X2 pin
#define LIMIT_X2_PORT 1 // assigns LIMIT_X2 port
// #define LIMIT_X2_ISR -5 // assigns LIMIT_X2 ISR
#define LIMIT_Y_BIT 28 // assigns LIMIT_Y pin
#define LIMIT_Y_PORT 1 // assigns LIMIT_Y port
// #define LIMIT_Y_ISR 1	// assigns LIMIT_Y ISR
#define LIMIT_Y2_BIT 25 // assigns LIMIT_Y2 pin
#define LIMIT_Y2_PORT 1 // assigns LIMIT_Y2 port
// #define LIMIT_Y2_ISR 1	// assigns LIMIT_Y2 ISR
#define LIMIT_Z_BIT 27 // assigns LIMIT_Z pin
#define LIMIT_Z_PORT 1 // assigns LIMIT_Z port
// #define LIMIT_Z_ISR -4	// assigns LIMIT_Z ISR

// Setup probe pin
#define PROBE_BIT 10
#define PROBE_PORT 0
// #define PROBE_ISR -3

// Setup com pins
// #define RX_BIT 3
// #define TX_BIT 2
// #define RX_PORT 0
// #define TX_PORT 0
// // only uncomment this if other port other then 0 is used
// //#define UART_PORT 0

#define USB_DM_BIT 30
#define USB_DM_PORT 0
#define USB_DP_BIT 29
#define USB_DP_PORT 0

// // Setup PWM
#define PWM0_BIT 5	// assigns PWM0 pin
#define PWM0_PORT 2 // assigns PWM0 pin
#define PWM0_CHANNEL 6

#define PWM1_BIT 4	// assigns PWM1 pin
#define PWM1_PORT 2 // assigns PWM1 pin
#define PWM1_CHANNEL 5

#define PWM2_BIT 3	// assigns PWM1 pin
#define PWM2_PORT 2 // assigns PWM1 pin
#define PWM2_CHANNEL 4

// Setup generic IO Pins
// Functionalities are set in config.h file

// // blink led
#define DOUT31_BIT 18
#define DOUT31_PORT 1

	// Setup the Step Timer used has the heartbeat for µCNC
	// Timer 0 is used by default
	//#define ITP_TIMER 0
	// Setup the SERVO Timer used by µCNC
	// Timer 1 is set by default
	//#define SERVO_TIMER 1

#define SERVO3_BIT 18
#define SERVO3_PORT 1
#define SERVO2_BIT 19
#define SERVO2_PORT 1
#define SERVO1_BIT 21
#define SERVO1_PORT 1
#define SERVO0_BIT 20
#define SERVO0_PORT 1

#define ANALOG0_BIT 23
#define ANALOG0_PORT 0
#define ANALOG0_CHANNEL 0
#define ANALOG1_BIT 24
#define ANALOG1_PORT 0
#define ANALOG1_CHANNEL 1
#define ANALOG2_BIT 25
#define ANALOG2_PORT 0
#define ANALOG2_CHANNEL 2
#define ANALOG3_BIT 26
#define ANALOG3_PORT 0
#define ANALOG3_CHANNEL 3

// hardware I2C
#define I2C_CLK_BIT 1
#define I2C_CLK_PORT 0
#define I2C_DATA_BIT 0
#define I2C_DATA_PORT 0
#define I2C_PORT 1
// software I2C
// #define DIN30_BIT 1
// #define DIN30_PORT 0
// #define DIN31_BIT 0
// #define DIN31_PORT 0

// hardware SPI(onboard)
// #define SPI_SDO_BIT 9
// #define SPI_SDO_PORT 0
// #define SPI_SDI_BIT 8
// #define SPI_SDI_PORT 0
// #define SPI_CLK_BIT 7
// #define SPI_CLK_PORT 0
// #define SPI_CS_BIT 6
// #define SPI_CS_PORT 0
// #define SPI_PORT 1
// #define SPI_FREQ 100000UL

// hardware SPI (display adapter)
#define SPI_SDO_BIT 18
#define SPI_SDO_PORT 0
#define SPI_SDI_BIT 17
#define SPI_SDI_PORT 0
#define SPI_CLK_BIT 15
#define SPI_CLK_PORT 0
#define SPI_CS_BIT 16
#define SPI_CS_PORT 0

// software SPI
// #define DOUT29_BIT 9
// #define DOUT29_PORT 0
// #define DIN29_BIT 8
// #define DIN29_PORT 0
// #define DOUT30_BIT 7
// #define DOUT30_PORT 0

// sd card detect
#define DIN19_BIT 31
#define DIN19_PORT 1
#define DIN19_PULLUP

//pins for smart adapter
//clk
#define DOUT4_BIT 19
#define DOUT4_PORT 1
//data
#define DOUT5_BIT 20
#define DOUT5_PORT 1
//cs
#define DOUT6_BIT 18
#define DOUT6_PORT 1
//beep
#define DOUT7_BIT 30
#define DOUT7_PORT 1
//enc btn
#define DIN16_BIT 28
#define DIN16_PORT 0
#define DIN16_PULLUP
//enc 1
#define DIN17_BIT 25
#define DIN17_PORT 3
#define DIN17_PULLUP
//enc 2
#define DIN18_BIT 226
#define DIN18_PORT 3
#define DIN18_PULLUP

// TMC0 UART
#define DOUT20_BIT 10
#define DOUT20_PORT 1
#define DIN20_BIT 10
#define DIN20_PORT 1
#define DIN20_PULLUP

// TMC1 UART
#define DOUT21_BIT 9
#define DOUT21_PORT 1
#define DIN21_BIT 9
#define DIN21_PORT 1
#define DIN21_PULLUP

// TMC2 UART
#define DOUT22_BIT 8
#define DOUT22_PORT 1
#define DIN22_BIT 8
#define DIN22_PORT 1
#define DIN22_PULLUP

// TMC3 UART
#define DOUT23_BIT 4
#define DOUT23_PORT 1
#define DIN23_BIT 4
#define DIN23_PORT 1
#define DIN23_PULLUP

// TMC4 UART
#define DOUT24_BIT 1
#define DOUT24_PORT 1
#define DIN24_BIT 1
#define DIN24_PORT 1
#define DIN24_PULLUP

#define ONESHOT_TIMER 2

#ifdef __cplusplus
}
#endif

#endif
