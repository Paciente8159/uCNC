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

#define LIMIT_Z_PULLUP
#define LIMIT_Y_PULLUP
#define LIMIT_X_PULLUP

// Setup control input pins
// #define ESTOP_BIT 0
// #define ESTOP_PORT A
// #define ESTOP_ISR

// Setup com pins
#define RX_BIT 1
#define RX_PULLUP
#define TX_BIT 0
// only uncomment this if other port other then 0 is used
// #define UART_PORT 0

//forces USB
#define MCU_HAS_USB

    // Setup PWM
#define PWM0_BIT 14  // assigns PWM0 pin

// Setup generic IO Pins
// spindle dir
#define DOUT0_BIT 15

// Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP2_EN_BIT 12
#define STEP1_EN_BIT 8
#define STEP0_EN_BIT 4

//activity LED
#define DOUT31_BIT 25

//disable EEPROM emulation
// #ifndef RAM_ONLY_SETTINGS
// #define RAM_ONLY_SETTINGS
// #endif

#define I2C_CLK_BIT 27
#define I2C_DATA_BIT 26
#define I2C_PORT 1
#define I2C_ADDRESS 1

/**
 * This is an example of how to use RP2040 PIO to control
 * up to 4 chainned 74hc595 (32 output pins) using only 3 pins
 * from the board.
 * The 3 pins should be sequential (for example GPIO's 26, 27 and 28)
 * 
 * RP2040 does not yet support software generate PWM
 * 
 * **/

// // Use PIO to shift data to 74HC595 shift registers (up to 4)
// // IO pins should be sequencial GPIO pins starting by data, then clock then the latch pin
// #define IC74HC595_CUSTOM_SHIFT_IO //Enables custom MCU data shift transmission. In RP2040 that is via a PIO
// #define IC74HC595_PIO_DATA 26
// #define IC74HC595_PIO_CLK 27
// #define IC74HC595_PIO_LATCH 28
// // enabling IC74HC595_CUSTOM_SHIFT_IO will force IC74HC595_COUNT to be set to 4 no matter what
// // support up to 4 chained 74HC595. Less can be used (overflow bits will be discarded like in the ESP32 I2S implementation)
// #define IC74HC595_COUNT 4

// #define STEP0_EN_IO_OFFSET 0
// #define STEP0_IO_OFFSET 1
// #define DIR0_IO_OFFSET 2
// #define STEP1_EN_IO_OFFSET 3
// #define STEP1_IO_OFFSET 4
// #define DIR1_IO_OFFSET 5
// #define STEP2_EN_IO_OFFSET 6
// #define STEP2_IO_OFFSET 7
// #define DIR2_IO_OFFSET 8
// #define STEP3_EN_IO_OFFSET 9
// #define STEP3_IO_OFFSET 10
// #define DIR3_IO_OFFSET 11
// #define STEP4_EN_IO_OFFSET 12
// #define STEP4_IO_OFFSET 13
// #define DIR4_IO_OFFSET 14
// #define DOUT0_IO_OFFSET 15

#ifdef __cplusplus
}
#endif

#endif
