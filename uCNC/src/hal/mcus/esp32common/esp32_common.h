/*
	Name: mcu32_common.h
	Description: Common MCU implementations for ESP32 MCUS in µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 11-09-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef ESP32_COMMON_H
#define ESP32_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ESP32
extern volatile uint32_t i2s_mode;
#define I2S_MODE __atomic_load_n((uint32_t *)&i2s_mode, __ATOMIC_RELAXED)

void __attribute__((weak)) mcu_uart_init(void){}
void __attribute__((weak)) mcu_uart_start(void){}
void __attribute__((weak)) mcu_uart_dotasks(void){}
void __attribute__((weak)) mcu_uart2_init(void){}
void __attribute__((weak)) mcu_uart2_start(void){}
void __attribute__((weak)) mcu_uart2_dotasks(void){}
void __attribute__((weak)) mcu_eeprom_init(int size){(void)size;}
void __attribute__((weak)) mcu_spi_init(void){}
void __attribute__((weak)) mcu_spi_config(spi_config_t config, uint32_t frequency){(void)config;(void)frequency;}
void __attribute__((weak)) mcu_spi2_init(void){}
void __attribute__((weak)) mcu_spi2_config(spi_config_t config, uint32_t frequency){(void)config;(void)frequency;}
void __attribute__((weak)) mcu_usb_init(void){}
void __attribute__((weak)) mcu_usb_dotasks(void){}
void __attribute__((weak)) mcu_wifi_init(void){}
void __attribute__((weak)) mcu_wifi_dotasks(void){}
void __attribute__((weak)) mcu_bt_init(void){}
void __attribute__((weak)) mcu_bt_dotasks(void){}
void mcu_i2s_extender_init(void);
#endif

#ifdef __cplusplus
}
#endif
#endif