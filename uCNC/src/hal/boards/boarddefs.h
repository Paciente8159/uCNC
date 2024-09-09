/*
	Name: boarddefs.h
	Description: Defines the available board types.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/02/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDDEFS_H
#define BOARDDEFS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "boards.h"
#include "../mcus/mcus.h"

/*
	MCU port map
*/
#ifndef BOARD
#error Undefined board
#endif

#if (!BOARD)
#error Invalid board configuration
#endif

#if (BOARD == BOARD_UNO)
#define MCU MCU_AVR
#include "avr/boardmap_uno.h"
#endif

#if (BOARD == BOARD_UNO_SHIELD_V3)
#define MCU MCU_AVR
#include "avr/boardmap_uno_shield_v3.h"
#endif

#if (BOARD == BOARD_X_CONTROLLER)
#define MCU MCU_AVR
#include "avr/boardmap_x_controller.h"
#endif

#if (BOARD == BOARD_MKS_DLC)
#define MCU MCU_AVR
#include "avr/boardmap_mks_dlc.h"
#endif

#if (BOARD == BOARD_RAMBO14)
#define MCU MCU_AVR
#include "avr/boardmap_rambo14.h"
#endif

#if (BOARD == BOARD_RAMPS14)
#define MCU MCU_AVR
#include "avr/boardmap_ramps14.h"
#endif

#if (BOARD == BOARD_RAMPS14_MIRROR)
#define MCU MCU_AVR
#include "avr/boardmap_ramps14_mirror.h"
#endif

#if (BOARD == BOARD_MKS_GEN_L_V1)
#define MCU MCU_AVR
#include "avr/boardmap_mks_gen_l_v1.h"
#endif

#if (BOARD == BOARD_MELZI_V114)
#define MCU MCU_AVR
#include "avr/boardmap_melzi_v114.h"
#endif

#if (BOARD == BOARD_BLUEPILL)
#ifndef MCU
#define MCU MCU_STM32F1X
#endif
#include "stm32/boardmap_bluepill.h"
#endif

#if (BOARD == BOARD_BLUEPILL_F0)
#ifndef MCU
#define MCU MCU_STM32F0X
#endif
#include "stm32/boardmap_bluepill_f0.h"
#endif

#if (BOARD == BOARD_BLACKPILL)
#ifndef MCU
#define MCU MCU_STM32F4X
#endif
#include "stm32/boardmap_blackpill.h"
#endif

#if (BOARD == BOARD_NUCLEO_F411RE_SHIELD_V3)
#define MCU MCU_STM32F4X
#include "stm32/boardmap_nucleo_f411re_shield_v3.h"
#endif

#if (BOARD == BOARD_MKS_ROBIN_NANO_V1_2)
#define MCU MCU_STM32F1X
#include "stm32/boardmap_mks_robin_nano_v1_2.h"
#endif

#if (BOARD == BOARD_MKS_ROBIN_NANO_V3_1)
#define MCU MCU_STM32F4X
#include "stm32/boardmap_mks_robin_nano_v3_1.h"
#endif

#if (BOARD == BOARD_SKR_PRO_V1_2)
#define MCU MCU_STM32F4X
#include "stm32/boardmap_srk_pro_v1_2.h"
#endif

#if (BOARD == BOARD_MZERO)
#define MCU MCU_SAMD21
#include "samd21/boardmap_mzero.h"
#endif

#if (BOARD == BOARD_ZERO)
#define MCU MCU_SAMD21
#include "samd21/boardmap_zero.h"
#endif

#if (BOARD == BOARD_RE_ARM)
#define MCU MCU_LPC176X
#include "lpc176x/boardmap_re_arm.h"
#endif

#if (BOARD == BOARD_MKS_BASE13)
#define MCU MCU_LPC176X
#include "lpc176x/boardmap_mks_base13.h"
#endif

#if (BOARD == BOARD_SKR_V14_TURBO)
#define MCU MCU_LPC176X
#include "lpc176x/boardmap_skr_v14_turbo.h"
#endif

#if (BOARD == BOARD_WEMOS_D1)
#define MCU MCU_ESP8266
#include "esp8266/boardmap_wemos_d1.h"
#endif

#if (BOARD == BOARD_WEMOS_D1_R32)
#define MCU MCU_ESP32
#include "esp32/boardmap_wemos_d1_r32.h"
#endif

#if (BOARD == BOARD_MKS_TINYBEE)
#define MCU MCU_ESP32
#include "esp32/boardmap_mks_tinybee.h"
#endif

#if (BOARD == BOARD_MKS_DLC32)
#define MCU MCU_ESP32
#include "esp32/boardmap_mks_dlc32.h"
#endif

#if (BOARD == BOARD_ESP32_SHIELD_V3)
#define MCU MCU_ESP32
#include "esp32/boardmap_esp32_shield_v3.h"
#endif

#if (BOARD == BOARD_RPI_PICO)
#define MCU MCU_RP2040
#include "rp2040/boardmap_rpi_pico.h"
#endif

#if (BOARD == BOARD_RPI_PICO_W)
#define MCU MCU_RP2040
#include "rp2040/boardmap_rpi_pico_w.h"
#endif

#if (BOARD == BOARD_VIRTUAL)
#ifndef __linux__
#define MCU MCU_VIRTUAL_WIN
#endif
#endif

#ifndef BOARD
#error Undefined board
#endif

#include "../../../boardmap_overrides.h"
#include "pin_mapping_helper.h"
#include "../mcus/mcudefs.h" //configures the MCU for the selected board

#ifdef __cplusplus
}
#endif

#endif
