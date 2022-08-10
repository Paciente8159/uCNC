/*
	Name: mcu_esp32.c
	Description: Implements the µCNC HAL for ESP8266.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 05-02-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_ESP32)
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include <driver/timer.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

static volatile bool esp32_global_isr_enabled;
static volatile uint32_t mcu_runtime_ms;

void esp32_uart_init(int baud);
char esp32_uart_read(void);
void esp32_uart_write(char c);
bool esp32_uart_rx_ready(void);
bool esp32_uart_tx_ready(void);
void esp32_uart_flush(void);
void esp32_uart_process(void);

#ifndef RAM_ONLY_SETTINGS
void esp32_eeprom_init(int size);
uint8_t esp32_eeprom_read(uint16_t address);
void esp32_eeprom_write(uint16_t address, uint8_t value);
void esp32_eeprom_flush(void);
#endif

hw_timer_t *esp32_rtc_timer;
hw_timer_t *esp32_step_timer;

uint8_t esp32_pwm[16];
static IRAM_ATTR void mcu_gen_pwm(void)
{
	static uint8_t pwm_counter = 0;
	// software PWM
	if (++pwm_counter < 127)
	{
#if !(PWM0 < 0)
		if (pwm_counter > esp32_pwm[0])
		{
			mcu_clear_output(PWM0);
		}
#endif
#if !(PWM1 < 0)
		if (pwm_counter > esp32_pwm[1])
		{
			mcu_clear_output(PWM1);
		}
#endif
#if !(PWM2 < 0)
		if (pwm_counter > esp32_pwm[2])
		{
			mcu_clear_output(PWM2);
		}
#endif
#if !(PWM3 < 0)
		if (pwm_counter > esp32_pwm[3])
		{
			mcu_clear_output(PWM3);
		}
#endif
#if !(PWM4 < 0)
		if (pwm_counter > esp32_pwm[4])
		{
			mcu_clear_output(PWM4);
		}
#endif
#if !(PWM5 < 0)
		if (pwm_counter > esp32_pwm[5])
		{
			mcu_clear_output(PWM5);
		}
#endif
#if !(PWM6 < 0)
		if (pwm_counter > esp32_pwm[6])
		{
			mcu_clear_output(PWM6);
		}
#endif
#if !(PWM7 < 0)
		if (pwm_counter > esp32_pwm[7])
		{
			mcu_clear_output(PWM7);
		}
#endif
#if !(PWM8 < 0)
		if (pwm_counter > esp32_pwm[8])
		{
			mcu_clear_output(PWM8);
		}
#endif
#if !(PWM9 < 0)
		if (pwm_counter > esp32_pwm[9])
		{
			mcu_clear_output(PWM9);
		}
#endif
#if !(PWM10 < 0)
		if (pwm_counter > esp32_pwm[10])
		{
			mcu_clear_output(PWM10);
		}
#endif
#if !(PWM11 < 0)
		if (pwm_counter > esp32_pwm[11])
		{
			mcu_clear_output(PWM11);
		}
#endif
#if !(PWM12 < 0)
		if (pwm_counter > esp32_pwm[12])
		{
			mcu_clear_output(PWM12);
		}
#endif
#if !(PWM13 < 0)
		if (pwm_counter > esp32_pwm[13])
		{
			mcu_clear_output(PWM13);
		}
#endif
#if !(PWM14 < 0)
		if (pwm_counter > esp32_pwm[14])
		{
			mcu_clear_output(PWM14);
		}
#endif
#if !(PWM15 < 0)
		if (pwm_counter > esp32_pwm[15])
		{
			mcu_clear_output(PWM15);
		}
#endif
	}
	else
	{
		pwm_counter = 0;
#if !(PWM0 < 0)
		if (esp32_pwm[0])
		{
			mcu_set_output(PWM0);
		}
#endif
#if !(PWM1 < 0)
		if (esp32_pwm[1])
		{
			mcu_set_output(PWM1);
		}
#endif
#if !(PWM2 < 0)
		if (esp32_pwm[2])
		{
			mcu_set_output(PWM2);
		}
#endif
#if !(PWM3 < 0)
		if (esp32_pwm[3])
		{
			mcu_set_output(PWM3);
		}
#endif
#if !(PWM4 < 0)
		if (esp32_pwm[4])
		{
			mcu_set_output(PWM4);
		}
#endif
#if !(PWM5 < 0)
		if (esp32_pwm[5])
		{
			mcu_set_output(PWM5);
		}
#endif
#if !(PWM6 < 0)
		if (esp32_pwm[6])
		{
			mcu_set_output(PWM6);
		}
#endif
#if !(PWM7 < 0)
		if (esp32_pwm[7])
		{
			mcu_set_output(PWM7);
		}
#endif
#if !(PWM8 < 0)
		if (esp32_pwm[8])
		{
			mcu_set_output(PWM8);
		}
#endif
#if !(PWM9 < 0)
		if (esp32_pwm[9])
		{
			mcu_set_output(PWM9);
		}
#endif
#if !(PWM10 < 0)
		if (esp32_pwm[10])
		{
			mcu_set_output(PWM10);
		}
#endif
#if !(PWM11 < 0)
		if (esp32_pwm[11])
		{
			mcu_set_output(PWM11);
		}
#endif
#if !(PWM12 < 0)
		if (esp32_pwm[12])
		{
			mcu_set_output(PWM12);
		}
#endif
#if !(PWM13 < 0)
		if (esp32_pwm[13])
		{
			mcu_set_output(PWM13);
		}
#endif
#if !(PWM14 < 0)
		if (esp32_pwm[14])
		{
			mcu_set_output(PWM14);
		}
#endif
#if !(PWM15 < 0)
		if (esp32_pwm[15])
		{
			mcu_set_output(PWM15);
		}
#endif
	}
}

IRAM_ATTR void mcu_din_isr(void)
{
	mcu_inputs_changed_cb();
}

IRAM_ATTR void mcu_probe_isr(void)
{
	mcu_probe_changed_cb();
}

IRAM_ATTR void mcu_limits_isr(void)
{
	mcu_limits_changed_cb();
}

IRAM_ATTR void mcu_controls_isr(void)
{
	mcu_controls_changed_cb();
}

IRAM_ATTR void mcu_rtc_isr(void *arg)
{
	static uint8_t rtc_counter = 0;
	timer_spinlock_take(RTC_TIMER_TG);

	mcu_gen_pwm();
	rtc_counter++;
	if (rtc_counter == 128)
	{
		mcu_runtime_ms++;
		mcu_rtc_cb(mcu_runtime_ms);
		rtc_counter = 0;
	}

	timer_group_clr_intr_status_in_isr(RTC_TIMER_TG, RTC_TIMER_IDX);
	/* After the alarm has been triggered
	  we need enable it again, so it is triggered the next time */
	timer_group_enable_alarm_in_isr(RTC_TIMER_TG, RTC_TIMER_IDX);
	timer_spinlock_give(RTC_TIMER_TG);
}

IRAM_ATTR void mcu_itp_isr(void *arg)
{
	static bool resetstep = false;
	timer_spinlock_take(ITP_TIMER_TG);

	if (!resetstep)
		mcu_step_cb();
	else
		mcu_step_reset_cb();
	resetstep = !resetstep;

	timer_group_clr_intr_status_in_isr(ITP_TIMER_TG, ITP_TIMER_IDX);
	/* After the alarm has been triggered
	  we need enable it again, so it is triggered the next time */
	timer_group_enable_alarm_in_isr(ITP_TIMER_TG, ITP_TIMER_IDX);
	timer_spinlock_give(ITP_TIMER_TG);
}

static void mcu_usart_init(void)
{
	esp32_uart_init(BAUDRATE);
}
/**
 * initializes the mcu
 * this function needs to:
 *   - configure all IO pins (digital IO, PWM, Analog, etc...)
 *   - configure all interrupts
 *   - configure uart or usb
 *   - start the internal RTC
 * */
void mcu_init(void)
{
#if STEP0 >= 0
	mcu_config_output(STEP0);
#endif
#if STEP1 >= 0
	mcu_config_output(STEP1);
#endif
#if STEP2 >= 0
	mcu_config_output(STEP2);
#endif
#if STEP3 >= 0
	mcu_config_output(STEP3);
#endif
#if STEP4 >= 0
	mcu_config_output(STEP4);
#endif
#if STEP5 >= 0
	mcu_config_output(STEP5);
#endif
#if STEP6 >= 0
	mcu_config_output(STEP6);
#endif
#if STEP7 >= 0
	mcu_config_output(STEP7);
#endif
#if DIR0 >= 0
	mcu_config_output(DIR0);
#endif
#if DIR1 >= 0
	mcu_config_output(DIR1);
#endif
#if DIR2 >= 0
	mcu_config_output(DIR2);
#endif
#if DIR3 >= 0
	mcu_config_output(DIR3);
#endif
#if DIR4 >= 0
	mcu_config_output(DIR4);
#endif
#if DIR5 >= 0
	mcu_config_output(DIR5);
#endif
#if DIR6 >= 0
	mcu_config_output(DIR6);
#endif
#if DIR7 >= 0
	mcu_config_output(DIR7);
#endif
#if STEP0_EN >= 0
	mcu_config_output(STEP0_EN);
#endif
#if STEP1_EN >= 0
	mcu_config_output(STEP1_EN);
#endif
#if STEP2_EN >= 0
	mcu_config_output(STEP2_EN);
#endif
#if STEP3_EN >= 0
	mcu_config_output(STEP3_EN);
#endif
#if STEP4_EN >= 0
	mcu_config_output(STEP4_EN);
#endif
#if STEP5_EN >= 0
	mcu_config_output(STEP5_EN);
#endif
#if STEP6_EN >= 0
	mcu_config_output(STEP6_EN);
#endif
#if STEP7_EN >= 0
	mcu_config_output(STEP7_EN);
#endif
#if PWM0 >= 0
	mcu_config_pwm(PWM0);
#endif
#if PWM1 >= 0
	mcu_config_pwm(PWM1);
#endif
#if PWM2 >= 0
	mcu_config_pwm(PWM2);
#endif
#if PWM3 >= 0
	mcu_config_pwm(PWM3);
#endif
#if PWM4 >= 0
	mcu_config_pwm(PWM4);
#endif
#if PWM5 >= 0
	mcu_config_pwm(PWM5);
#endif
#if PWM6 >= 0
	mcu_config_pwm(PWM6);
#endif
#if PWM7 >= 0
	mcu_config_pwm(PWM7);
#endif
#if PWM8 >= 0
	mcu_config_pwm(PWM8);
#endif
#if PWM9 >= 0
	mcu_config_pwm(PWM9);
#endif
#if PWM10 >= 0
	mcu_config_pwm(PWM10);
#endif
#if PWM11 >= 0
	mcu_config_pwm(PWM11);
#endif
#if PWM12 >= 0
	mcu_config_pwm(PWM12);
#endif
#if PWM13 >= 0
	mcu_config_pwm(PWM13);
#endif
#if PWM14 >= 0
	mcu_config_pwm(PWM14);
#endif
#if PWM15 >= 0
	mcu_config_pwm(PWM15);
#endif
#if SERVO0 >= 0
	mcu_config_output(SERVO0);
#endif
#if SERVO1 >= 0
	mcu_config_output(SERVO1);
#endif
#if SERVO2 >= 0
	mcu_config_output(SERVO2);
#endif
#if SERVO3 >= 0
	mcu_config_output(SERVO3);
#endif
#if SERVO4 >= 0
	mcu_config_output(SERVO4);
#endif
#if SERVO5 >= 0
	mcu_config_output(SERVO5);
#endif
#if DOUT0 >= 0
	mcu_config_output(DOUT0);
#endif
#if DOUT1 >= 0
	mcu_config_output(DOUT1);
#endif
#if DOUT2 >= 0
	mcu_config_output(DOUT2);
#endif
#if DOUT3 >= 0
	mcu_config_output(DOUT3);
#endif
#if DOUT4 >= 0
	mcu_config_output(DOUT4);
#endif
#if DOUT5 >= 0
	mcu_config_output(DOUT5);
#endif
#if DOUT6 >= 0
	mcu_config_output(DOUT6);
#endif
#if DOUT7 >= 0
	mcu_config_output(DOUT7);
#endif
#if DOUT8 >= 0
	mcu_config_output(DOUT8);
#endif
#if DOUT9 >= 0
	mcu_config_output(DOUT9);
#endif
#if DOUT10 >= 0
	mcu_config_output(DOUT10);
#endif
#if DOUT11 >= 0
	mcu_config_output(DOUT11);
#endif
#if DOUT12 >= 0
	mcu_config_output(DOUT12);
#endif
#if DOUT13 >= 0
	mcu_config_output(DOUT13);
#endif
#if DOUT14 >= 0
	mcu_config_output(DOUT14);
#endif
#if DOUT15 >= 0
	mcu_config_output(DOUT15);
#endif
#if DOUT16 >= 0
	mcu_config_output(DOUT16);
#endif
#if DOUT17 >= 0
	mcu_config_output(DOUT17);
#endif
#if DOUT18 >= 0
	mcu_config_output(DOUT18);
#endif
#if DOUT19 >= 0
	mcu_config_output(DOUT19);
#endif
#if DOUT20 >= 0
	mcu_config_output(DOUT20);
#endif
#if DOUT21 >= 0
	mcu_config_output(DOUT21);
#endif
#if DOUT22 >= 0
	mcu_config_output(DOUT22);
#endif
#if DOUT23 >= 0
	mcu_config_output(DOUT23);
#endif
#if DOUT24 >= 0
	mcu_config_output(DOUT24);
#endif
#if DOUT25 >= 0
	mcu_config_output(DOUT25);
#endif
#if DOUT26 >= 0
	mcu_config_output(DOUT26);
#endif
#if DOUT27 >= 0
	mcu_config_output(DOUT27);
#endif
#if DOUT28 >= 0
	mcu_config_output(DOUT28);
#endif
#if DOUT29 >= 0
	mcu_config_output(DOUT29);
#endif
#if DOUT30 >= 0
	mcu_config_output(DOUT30);
#endif
#if DOUT31 >= 0
	mcu_config_output(DOUT31);
#endif
#if LIMIT_X >= 0
	mcu_config_input(LIMIT_X);
#ifdef LIMIT_X_PULLUP
	mcu_config_pullup(LIMIT_X);
#endif
#ifdef LIMIT_X_ISR
	mcu_config_input_isr(LIMIT_X);
#endif
#endif
#if LIMIT_Y >= 0
	mcu_config_input(LIMIT_Y);
#ifdef LIMIT_Y_PULLUP
	mcu_config_pullup(LIMIT_Y);
#endif
#ifdef LIMIT_Y_ISR
	mcu_config_input_isr(LIMIT_Y);
#endif
#endif
#if LIMIT_Z >= 0
	mcu_config_input(LIMIT_Z);
#ifdef LIMIT_Z_PULLUP
	mcu_config_pullup(LIMIT_Z);
#endif
#ifdef LIMIT_Z_ISR
	mcu_config_input_isr(LIMIT_Z);
#endif
#endif
#if LIMIT_X2 >= 0
	mcu_config_input(LIMIT_X2);
#ifdef LIMIT_X2_PULLUP
	mcu_config_pullup(LIMIT_X2);
#endif
#ifdef LIMIT_X2_ISR
	mcu_config_input_isr(LIMIT_X2);
#endif
#endif
#if LIMIT_Y2 >= 0
	mcu_config_input(LIMIT_Y2);
#ifdef LIMIT_Y2_PULLUP
	mcu_config_pullup(LIMIT_Y2);
#endif
#ifdef LIMIT_Y2_ISR
	mcu_config_input_isr(LIMIT_Y2);
#endif
#endif
#if LIMIT_Z2 >= 0
	mcu_config_input(LIMIT_Z2);
#ifdef LIMIT_Z2_PULLUP
	mcu_config_pullup(LIMIT_Z2);
#endif
#ifdef LIMIT_Z2_ISR
	mcu_config_input_isr(LIMIT_Z2);
#endif
#endif
#if LIMIT_A >= 0
	mcu_config_input(LIMIT_A);
#ifdef LIMIT_A_PULLUP
	mcu_config_pullup(LIMIT_A);
#endif
#ifdef LIMIT_A_ISR
	mcu_config_input_isr(LIMIT_A);
#endif
#endif
#if LIMIT_B >= 0
	mcu_config_input(LIMIT_B);
#ifdef LIMIT_B_PULLUP
	mcu_config_pullup(LIMIT_B);
#endif
#ifdef LIMIT_B_ISR
	mcu_config_input_isr(LIMIT_B);
#endif
#endif
#if LIMIT_C >= 0
	mcu_config_input(LIMIT_C);
#ifdef LIMIT_C_PULLUP
	mcu_config_pullup(LIMIT_C);
#endif
#ifdef LIMIT_C_ISR
	mcu_config_input_isr(LIMIT_C);
#endif
#endif
#if PROBE >= 0
	mcu_config_input(PROBE);
#ifdef PROBE_PULLUP
	mcu_config_pullup(PROBE);
#endif
#ifdef PROBE_ISR
	mcu_config_input_isr(PROBE);
#endif
#endif
#if ESTOP >= 0
	mcu_config_input(ESTOP);
#ifdef ESTOP_PULLUP
	mcu_config_pullup(ESTOP);
#endif
#ifdef ESTOP_ISR
	mcu_config_input_isr(ESTOP);
#endif
#endif
#if SAFETY_DOOR >= 0
	mcu_config_input(SAFETY_DOOR);
#ifdef SAFETY_DOOR_PULLUP
	mcu_config_pullup(SAFETY_DOOR);
#endif
#ifdef SAFETY_DOOR_ISR
	mcu_config_input_isr(SAFETY_DOOR);
#endif
#endif
#if FHOLD >= 0
	mcu_config_input(FHOLD);
#ifdef FHOLD_PULLUP
	mcu_config_pullup(FHOLD);
#endif
#ifdef FHOLD_ISR
	mcu_config_input_isr(FHOLD);
#endif
#endif
#if CS_RES >= 0
	mcu_config_input(CS_RES);
#ifdef CS_RES_PULLUP
	mcu_config_pullup(CS_RES);
#endif
#ifdef CS_RES_ISR
	mcu_config_input_isr(CS_RES);
#endif
#endif
#if ANALOG0 >= 0
	mcu_config_input(ANALOG0);
#endif
#if ANALOG1 >= 0
	mcu_config_input(ANALOG1);
#endif
#if ANALOG2 >= 0
	mcu_config_input(ANALOG2);
#endif
#if ANALOG3 >= 0
	mcu_config_input(ANALOG3);
#endif
#if ANALOG4 >= 0
	mcu_config_input(ANALOG4);
#endif
#if ANALOG5 >= 0
	mcu_config_input(ANALOG5);
#endif
#if ANALOG6 >= 0
	mcu_config_input(ANALOG6);
#endif
#if ANALOG7 >= 0
	mcu_config_input(ANALOG7);
#endif
#if ANALOG8 >= 0
	mcu_config_input(ANALOG8);
#endif
#if ANALOG9 >= 0
	mcu_config_input(ANALOG9);
#endif
#if ANALOG10 >= 0
	mcu_config_input(ANALOG10);
#endif
#if ANALOG11 >= 0
	mcu_config_input(ANALOG11);
#endif
#if ANALOG12 >= 0
	mcu_config_input(ANALOG12);
#endif
#if ANALOG13 >= 0
	mcu_config_input(ANALOG13);
#endif
#if ANALOG14 >= 0
	mcu_config_input(ANALOG14);
#endif
#if ANALOG15 >= 0
	mcu_config_input(ANALOG15);
#endif
#if DIN0 >= 0
	mcu_config_input(DIN0);
#ifdef DIN0_PULLUP
	mcu_config_pullup(DIN0);
#endif
#ifdef DIN0_ISR
	mcu_config_input_isr(DIN0);
#endif
#endif
#if DIN1 >= 0
	mcu_config_input(DIN1);
#ifdef DIN1_PULLUP
	mcu_config_pullup(DIN1);
#endif
#ifdef DIN1_ISR
	mcu_config_input_isr(DIN1);
#endif
#endif
#if DIN2 >= 0
	mcu_config_input(DIN2);
#ifdef DIN2_PULLUP
	mcu_config_pullup(DIN2);
#endif
#ifdef DIN2_ISR
	mcu_config_input_isr(DIN2);
#endif
#endif
#if DIN3 >= 0
	mcu_config_input(DIN3);
#ifdef DIN3_PULLUP
	mcu_config_pullup(DIN3);
#endif
#ifdef DIN3_ISR
	mcu_config_input_isr(DIN3);
#endif
#endif
#if DIN4 >= 0
	mcu_config_input(DIN4);
#ifdef DIN4_PULLUP
	mcu_config_pullup(DIN4);
#endif
#ifdef DIN4_ISR
	mcu_config_input_isr(DIN4);
#endif
#endif
#if DIN5 >= 0
	mcu_config_input(DIN5);
#ifdef DIN5_PULLUP
	mcu_config_pullup(DIN5);
#endif
#ifdef DIN5_ISR
	mcu_config_input_isr(DIN5);
#endif
#endif
#if DIN6 >= 0
	mcu_config_input(DIN6);
#ifdef DIN6_PULLUP
	mcu_config_pullup(DIN6);
#endif
#ifdef DIN6_ISR
	mcu_config_input_isr(DIN6);
#endif
#endif
#if DIN7 >= 0
	mcu_config_input(DIN7);
#ifdef DIN7_PULLUP
	mcu_config_pullup(DIN7);
#endif
#ifdef DIN7_ISR
	mcu_config_input_isr(DIN7);
#endif
#endif
#if DIN8 >= 0
	mcu_config_input(DIN8);
#ifdef DIN8_PULLUP
	mcu_config_pullup(DIN8);
#endif
#endif
#if DIN9 >= 0
	mcu_config_input(DIN9);
#ifdef DIN9_PULLUP
	mcu_config_pullup(DIN9);
#endif
#endif
#if DIN10 >= 0
	mcu_config_input(DIN10);
#ifdef DIN10_PULLUP
	mcu_config_pullup(DIN10);
#endif
#endif
#if DIN11 >= 0
	mcu_config_input(DIN11);
#ifdef DIN11_PULLUP
	mcu_config_pullup(DIN11);
#endif
#endif
#if DIN12 >= 0
	mcu_config_input(DIN12);
#ifdef DIN12_PULLUP
	mcu_config_pullup(DIN12);
#endif
#endif
#if DIN13 >= 0
	mcu_config_input(DIN13);
#ifdef DIN13_PULLUP
	mcu_config_pullup(DIN13);
#endif
#endif
#if DIN14 >= 0
	mcu_config_input(DIN14);
#ifdef DIN14_PULLUP
	mcu_config_pullup(DIN14);
#endif
#endif
#if DIN15 >= 0
	mcu_config_input(DIN15);
#ifdef DIN15_PULLUP
	mcu_config_pullup(DIN15);
#endif
#endif
#if DIN16 >= 0
	mcu_config_input(DIN16);
#ifdef DIN16_PULLUP
	mcu_config_pullup(DIN16);
#endif
#endif
#if DIN17 >= 0
	mcu_config_input(DIN17);
#ifdef DIN17_PULLUP
	mcu_config_pullup(DIN17);
#endif
#endif
#if DIN18 >= 0
	mcu_config_input(DIN18);
#ifdef DIN18_PULLUP
	mcu_config_pullup(DIN18);
#endif
#endif
#if DIN19 >= 0
	mcu_config_input(DIN19);
#ifdef DIN19_PULLUP
	mcu_config_pullup(DIN19);
#endif
#endif
#if DIN20 >= 0
	mcu_config_input(DIN20);
#ifdef DIN20_PULLUP
	mcu_config_pullup(DIN20);
#endif
#endif
#if DIN21 >= 0
	mcu_config_input(DIN21);
#ifdef DIN21_PULLUP
	mcu_config_pullup(DIN21);
#endif
#endif
#if DIN22 >= 0
	mcu_config_input(DIN22);
#ifdef DIN22_PULLUP
	mcu_config_pullup(DIN22);
#endif
#endif
#if DIN23 >= 0
	mcu_config_input(DIN23);
#ifdef DIN23_PULLUP
	mcu_config_pullup(DIN23);
#endif
#endif
#if DIN24 >= 0
	mcu_config_input(DIN24);
#ifdef DIN24_PULLUP
	mcu_config_pullup(DIN24);
#endif
#endif
#if DIN25 >= 0
	mcu_config_input(DIN25);
#ifdef DIN25_PULLUP
	mcu_config_pullup(DIN25);
#endif
#endif
#if DIN26 >= 0
	mcu_config_input(DIN26);
#ifdef DIN26_PULLUP
	mcu_config_pullup(DIN26);
#endif
#endif
#if DIN27 >= 0
	mcu_config_input(DIN27);
#ifdef DIN27_PULLUP
	mcu_config_pullup(DIN27);
#endif
#endif
#if DIN28 >= 0
	mcu_config_input(DIN28);
#ifdef DIN28_PULLUP
	mcu_config_pullup(DIN28);
#endif
#endif
#if DIN29 >= 0
	mcu_config_input(DIN29);
#ifdef DIN29_PULLUP
	mcu_config_pullup(DIN29);
#endif
#endif
#if DIN30 >= 0
	mcu_config_input(DIN30);
#ifdef DIN30_PULLUP
	mcu_config_pullup(DIN30);
#endif
#endif
#if DIN31 >= 0
	mcu_config_input(DIN31);
#ifdef DIN31_PULLUP
	mcu_config_pullup(DIN31);
#endif
#endif
#if TX >= 0
	mcu_config_output(TX);
#endif
#if RX >= 0
	mcu_config_input(RX);
#ifdef RX_PULLUP
	mcu_config_pullup(RX);
#endif
#endif
#if USB_DM >= 0
	mcu_config_input(USB_DM);
#ifdef USB_DM_PULLUP
	mcu_config_pullup(USB_DM);
#endif
#endif
#if USB_DP >= 0
	mcu_config_input(USB_DP);
#ifdef USB_DP_PULLUP
	mcu_config_pullup(USB_DP);
#endif
#endif
#if SPI_CLK >= 0
	mcu_config_output(SPI_CLK);
#endif
#if SPI_SDI >= 0
	mcu_config_input(SPI_SDI);
#ifdef SPI_SDI_PULLUP
	mcu_config_pullup(SPI_SDI);
#endif
#endif
#if SPI_SDO >= 0
	mcu_config_output(SPI_SDO);
#endif

	mcu_usart_init();

	// initialize rtc timer
	/* Select and initialize basic parameters of the timer */
	timer_config_t config = {
		.divider = 5,
		.counter_dir = TIMER_COUNT_UP,
		.counter_en = TIMER_PAUSE,
		.alarm_en = TIMER_ALARM_EN,
		.auto_reload = true,
	}; // default clock source is APB
	timer_init(RTC_TIMER_TG, RTC_TIMER_IDX, &config);

	/* Timer's counter will initially start from value below.
	   Also, if auto_reload is set, this value will be automatically reload on alarm */
	timer_set_counter_value(RTC_TIMER_TG, RTC_TIMER_IDX, 0x00000000ULL);

	/* Configure the alarm value and the interrupt on alarm. */
	timer_set_alarm_value(RTC_TIMER_TG, RTC_TIMER_IDX, (uint64_t)125);
	timer_enable_intr(RTC_TIMER_TG, RTC_TIMER_IDX);
	timer_isr_register(RTC_TIMER_TG, RTC_TIMER_IDX, mcu_rtc_isr, NULL, 0, NULL);

	timer_start(RTC_TIMER_TG, RTC_TIMER_IDX);

	/*uint16_t timerdiv = (uint16_t)(getApbFrequency() / 128000UL);
	esp32_rtc_timer = timerBegin(RTC_TIMER, timerdiv, true);
	timerAttachInterrupt(esp32_rtc_timer, &mcu_rtc_isr, true);
	timerAlarmWrite(esp32_rtc_timer, 1, true);
	timerAlarmEnable(esp32_rtc_timer);

	// initialize stepper timer
	timerdiv = (uint16_t)(getApbFrequency() / (F_STEP_MAX << 1));
	esp32_step_timer = timerBegin(ITP_TIMER, timerdiv, true);*/

#ifndef RAM_ONLY_SETTINGS
	esp32_eeprom_init(1024); // 1K Emulated EEPROM
#endif

	mcu_enable_global_isr();
}

/**
 * enables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_probe_isr
void mcu_enable_probe_isr(void)
{
}
#endif

/**
 * disables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_probe_isr
void mcu_disable_probe_isr(void)
{
}
#endif

/**
 * gets the voltage value of a built-in ADC pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_analog
uint8_t mcu_get_analog(uint8_t channel)
{
	return 0;
}
#endif

/**
 * sets the pwm value of a built-in pwm pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_pwm
void mcu_set_pwm(uint8_t pwm, uint8_t value)
{
}
#endif

/**
 * gets the configured pwm value of a built-in pwm pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_pwm
uint8_t mcu_get_pwm(uint8_t pwm)
{
	return 0;
}
#endif

/**
 * checks if the serial hardware of the MCU is ready do send the next char
 * */
#ifndef mcu_tx_ready
bool mcu_tx_ready(void)
{
	return esp32_uart_tx_ready();
}
#endif

/**
 * checks if the serial hardware of the MCU has a new char ready to be read
 * */
#ifndef mcu_rx_ready
bool mcu_rx_ready(void)
{
	return esp32_uart_rx_ready();
}
#endif

/**
 * sends a char either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_putc

void mcu_putc(char c)
{
#if !(LED < 0)
	mcu_toggle_output(LED);
#endif
#ifdef ENABLE_SYNC_TX
	while (!mcu_tx_ready())
		;
#endif

	esp32_uart_write(c);
}
#endif

/**
 * gets a char either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_getc
char mcu_getc(void)
{
#if !(LED < 0)
	mcu_toggle_output(LED);
#endif
#ifdef ENABLE_SYNC_RX
	while (!mcu_rx_ready())
		;
#endif

	return esp32_uart_read();
}
#endif

// ISR
/**
 * enables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_global_isr
void mcu_enable_global_isr(void)
{
	// ets_intr_unlock();
	esp32_global_isr_enabled = true;
}
#endif

/**
 * disables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_global_isr
void mcu_disable_global_isr(void)
{
	esp32_global_isr_enabled = false;
	// ets_intr_lock();
}
#endif

/**
 * gets global interrupts state on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_global_isr
bool mcu_get_global_isr(void)
{
	return esp32_global_isr_enabled;
}
#endif

// Step interpolator
/**
 * convert step rate to clock cycles
 * */
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
{
	// up and down counter (generates half the step rate at each event)
	uint32_t totalticks = (uint32_t)(500000.0f / frequency);
	*prescaller = 1;
	while (totalticks > 0xFFFF)
	{
		(*prescaller) <<= 1;
		totalticks >>= 1;
	}

	*ticks = (uint16_t)totalticks;
}

/**
 * starts the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	/*timerAttachInterrupt(esp32_step_timer, &mcu_itp_isr, true);
	timerAlarmWrite(esp32_step_timer, (uint32_t)ticks * (uint32_t)prescaller, true);
	timerAlarmEnable(esp32_step_timer); // Just Enable*/
	timer_config_t config = {
		.divider = 80,
		.counter_dir = TIMER_COUNT_UP,
		.counter_en = TIMER_PAUSE,
		.alarm_en = TIMER_ALARM_EN,
		.auto_reload = true,
	}; // default clock source is APB
	timer_init(ITP_TIMER_TG, ITP_TIMER_IDX, &config);

	/* Timer's counter will initially start from value below.
	   Also, if auto_reload is set, this value will be automatically reload on alarm */
	timer_set_counter_value(ITP_TIMER_TG, ITP_TIMER_IDX, 0x00000000ULL);

	/* Configure the alarm value and the interrupt on alarm. */
	timer_set_alarm_value(ITP_TIMER_TG, ITP_TIMER_IDX, (uint64_t)ticks * prescaller);
	timer_enable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
	timer_isr_register(ITP_TIMER_TG, ITP_TIMER_IDX, mcu_itp_isr, NULL, 0, NULL);

	timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
}

/**
 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
	timer_set_alarm_value(ITP_TIMER_TG, ITP_TIMER_IDX, (uint64_t)ticks * prescaller);
	timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
}

/**
 * stops the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_stop_itp_isr(void)
{
	// timerAlarmDisable(esp32_step_timer);
	timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
	timer_disable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
}

/**
 * gets the MCU running time in milliseconds.
 * the time counting is controled by the internal RTC
 * */
uint32_t mcu_millis()
{
	return mcu_runtime_ms;
}

#ifndef mcu_delay_us
void mcu_delay_us(uint8_t delay)
{
	int64_t time = esp_timer_get_time() + delay;
	while (time > esp_timer_get_time())
		;
}
#endif

/**
 * runs all internal tasks of the MCU.
 * for the moment these are:
 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
 *   - if ENABLE_SYNC_RX is enabled check if there are any chars in the rx transmitter (or the tinyUSB buffer) and read them to the serial_rx_isr
 *   - if ENABLE_SYNC_TX is enabled check if serial_tx_empty is false and run serial_tx_isr
 * */
void mcu_dotasks(void)
{
	// reset WDT
	esp_task_wdt_reset();
	esp32_uart_process();
}

// Non volatile memory
/**
 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
uint8_t mcu_eeprom_getc(uint16_t address)
{
#ifndef RAM_ONLY_SETTINGS
	return esp32_eeprom_read(address);
#else
	return 0;
#endif
}

/**
 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
#ifndef RAM_ONLY_SETTINGS
	esp32_eeprom_write(address, value);
#endif
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
#ifndef RAM_ONLY_SETTINGS
	esp32_eeprom_flush();
#endif
}

#endif
