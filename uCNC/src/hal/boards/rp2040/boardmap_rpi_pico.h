/*
    Name: boardmap_rp_pico_w.h
    Description: Contains all MCU and PIN definitions for Raspberry Pi Pico W to run µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 16/01/2023

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_RPI_PICO_H
#define BOARDMAP_RPI_PICO_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "RPi Pico"
#endif

// SAME AS GRBL for test purposes
// Setup step pins
#define STEP2_BIT 10  // assigns STEP2 pin
#define STEP1_BIT 6  // assigns STEP1 pin
#define STEP0_BIT 2 // assigns STEP0 pin

// Setup dir pins
#define DIR2_BIT 11 // assigns DIR2 pin
#define DIR1_BIT 7 // assigns DIR1 pin
#define DIR0_BIT 3 // assigns DIR0 pin

#define LIMIT_Z_BIT 13
#define LIMIT_Y_BIT 9
#define LIMIT_X_BIT 5

// Setup control input pins
// #define ESTOP_BIT 0
// #define ESTOP_PORT A
//#define ESTOP_ISR

// Setup com pins
#define RX_BIT 1
#define TX_BIT 0
// only uncomment this if other port other then 0 is used
// #define COM_PORT 0

//forces USB
#define MCU_HAS_USB

    // Setup PWM
// #define PWM0_BIT 2  // assigns PWM0 pin
// #define PWM0_PORT D // assigns PWM0 pin

// Setup generic IO Pins
// spindle dir
// #define DOUT0_BIT 15
// #define DOUT0_PORT D

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP2_EN_BIT 12
#define STEP1_EN_BIT 8
#define STEP0_EN_BIT 4

//activity LED
#define DOUT31_BIT 25


#ifdef __cplusplus
}
#endif

#endif