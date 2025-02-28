/*
	Name: boards_helper.h
	Description: board configuration helper file.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-02-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDS_HELPER_H
#define BOARDS_HELPER_H

#ifdef __cplusplus
extern "C"
{
#endif

// boards list
/**
 * AVR
 */
#define BOARD_UNO 1
#define BOARD_RAMBO14 2
#define BOARD_RAMPS14 3
#define BOARD_MKS_DLC 4
#define BOARD_X_CONTROLLER 5
#define BOARD_MKS_GEN_L_V1 6
#define BOARD_UNO_SHIELD_V3 7
#define BOARD_MELZI_V1_14 8
#define BOARD_RAMPS14_MIRROR 200
/**
 * STM32F0
 */
#define BOARD_BLUEPILL_F0 1200
/**
 * STM32F1
 */
#define BOARD_BLUEPILL 1000
#define BOARD_MKS_ROBIN_NANO_V1_2 1001
/**
 * STM32F4
 */
#define BOARD_BLACKPILL 1100
#define BOARD_MKS_ROBIN_NANO_V3_1 1101
#define BOARD_SKR_PRO_V1_2 1103
#define BOARD_NUCLEO_F411RE_SHIELD_V3 1104
/**
 * SAMD21
 */
#define BOARD_MZERO 2000
#define BOARD_ZERO 2001
/**
 * LPC17XX
 */
#define BOARD_RE_ARM 3000
#define BOARD_MKS_BASE13 3001
#define BOARD_SKR_V14_TURBO 3002
/**
 * ESP8266
 */
#define BOARD_WEMOS_D1 4000
/**
 * ESP32
 */
#define BOARD_WEMOS_D1_R32 5000
#define BOARD_MKS_TINYBEE 5001
#define BOARD_MKS_DLC32 5002
#define BOARD_ESP32_SHIELD_V3 5003
/**
 * RP2040
 */
#define BOARD_RPI_PICO 6000
#define BOARD_RPI_PICO_W 6001
/**
 * RP2350
 */
#define BOARD_RPI_PICO2 6100
/**
 * Windows/Linux
 */
#define BOARD_VIRTUAL 99999
/**
 * Special purpose
 */
#define BOARD_CUSTOM -1
#define BOARD_UNDEFINED 0

#ifndef BOARD
#define BOARD BOARD_UNDEFINED
#endif

#ifdef BOARD
// AVR
#if (BOARD == BOARD_UNO)
#define BOARDMAP "avr/boardmap_uno.h"
#elif (BOARD == BOARD_RAMBO14)
#define BOARDMAP "avr/boardmap_rambo14.h"
#elif (BOARD == BOARD_RAMPS14)
#define BOARDMAP "avr/boardmap_ramps14.h"
#elif (BOARD == BOARD_MKS_DLC)
#define BOARDMAP "avr/boardmap_mks_dlc.h"
#elif (BOARD == BOARD_X_CONTROLLER)
#define BOARDMAP "avr/boardmap_x_controller.h"
#elif (BOARD == BOARD_MKS_GEN_L_V1)
#define BOARDMAP "avr/boardmap_mks_gen_l_v1.h"
#elif (BOARD == BOARD_UNO_SHIELD_V3)
#define BOARDMAP "avr/boardmap_uno_shield_v3.h"
// STM32F0
#elif (BOARD == BOARD_BLUEPILL_F0)
#define BOARDMAP "stm32/boardmap_bluepill_f0.h"
// STM32F1
#elif (BOARD == BOARD_BLUEPILL)
#define BOARDMAP "stm32/boardmap_bluepill.h"
#elif (BOARD == BOARD_MKS_ROBIN_NANO_V1_2)
#define BOARDMAP "stm32/boardmap_mks_robin_nano_v1_2.h"
// STM32F4
#elif (BOARD == BOARD_BLACKPILL)
#define BOARDMAP "stm32/boardmap_blackpill.h"
#elif (BOARD == BOARD_MKS_ROBIN_NANO_V3_1)
#define BOARDMAP "stm32/boardmap_mks_robin_nano_v3_1.h"
#elif (BOARD == BOARD_SKR_PRO_V1_2)
#define BOARDMAP "stm32/boardmap_srk_pro_v1_2.h"
#define HSE_VALUE 8000000
#define CUSTOM_PRE_MAIN
#elif (BOARD == BOARD_NUCLEO_F411RE_SHIELD_V3)
#define BOARDMAP "stm32/boardmap_nucleo_f411re_shield_v3.h"
// SAMD21
#elif (BOARD == BOARD_MZERO)
#define BOARDMAP "samd21/boardmap_mzero.h"
#elif (BOARD == BOARD_ZERO)
#define BOARDMAP "samd21/boardmap_zero.h"
// LPC17XX
#elif (BOARD == BOARD_RE_ARM)
#define BOARDMAP "lpc176x/boardmap_re_arm.h"
#elif (BOARD == BOARD_MKS_BASE13)
#define BOARDMAP "lpc176x/boardmap_mks_base13.h"
#elif (BOARD == BOARD_SKR_V14_TURBO)
#define BOARDMAP "lpc176x/boardmap_skr_v14_turbo.h"
// ESP8266
#elif (BOARD == BOARD_WEMOS_D1)
#define BOARDMAP "esp8266/boardmap_wemos_d1.h"
// ESP32
#elif (BOARD == BOARD_WEMOS_D1_R32)
#define BOARDMAP "esp32/boardmap_wemos_d1_r32.h"
#elif (BOARD == BOARD_MKS_TINYBEE)
#define BOARDMAP "esp32/boardmap_mks_dlc32.h"
#elif (BOARD == BOARD_MKS_DLC32)
#define BOARDMAP "esp32/boardmap_mks_tinybee.h"
#elif (BOARD == BOARD_ESP32_SHIELD_V3)
#define BOARDMAP "esp32/boardmap_esp32_shield_v3.h"
// RP2040
#elif (BOARD == BOARD_RPI_PICO)
#define BOARDMAP "rp2040/boardmap_rpi_pico.h"
#elif (BOARD == BOARD_RPI_PICO_W)
#define BOARDMAP "rp2040/boardmap_rpi_pico_w.h"
#define PIO_FRAMEWORK_ARDUINO_ENABLE_BLUETOOTH
// RP2350
#elif (BOARD == BOARD_RPI_PICO2)
#define BOARDMAP "rp2350/boardmap_rpi_pico2.h"
// CUSTOM
#elif (BOARD == BOARD_CUSTOM) || (BOARD == BOARD_UNDEFINED)
#define BOARDMAP "../../../boardmap_overrides.h"
#if (BOARD == BOARD_UNDEFINED)
#warning "Board configuration undefined"
#endif
#else
#error "Invalid board"
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif