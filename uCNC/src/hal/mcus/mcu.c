/*
	Name: mcu.c
	Description: Contains common MCU (agnostic) functions.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 26/08/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"

#ifndef ENABLE_SYNC_TX
uint8_t mcu_com_tx_buffer[TX_BUFFER_SIZE];
volatile uint8_t mcu_com_tx_head;
#if defined(MCU_HAS_UART) && !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
uint8_t mcu_uart_tx_tail;
#endif
#if defined(MCU_HAS_UART2) && !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
uint8_t mcu_uart2_tx_tail;
#endif
#if defined(MCU_HAS_WIFI) && !defined(DETACH_WIFI_FROM_MAIN_PROTOCOL)
uint8_t mcu_wifi_tx_tail;
#endif
#endif

#ifdef MCU_HAS_ONESHOT_TIMER
MCU_CALLBACK mcu_timeout_delgate mcu_timeout_cb;
#endif

// most MCU can perform some sort of loop within 4 to 6 CPU cycles + a small function call overhead
// the amount of cycles per loop and overhead can be tuned with a scope or by inspecting the produced asm
// and adjusted in each MCU
// by adding the noinline and the empty asm instruction prevents optimization from removing the code executing the loop
// if used inside atomic operations can execute delays with good precision in any MCU
__attribute__((noinline, optimize("O3"))) void mcu_delay_loop(uint16_t loops)
{
	do
	{
		asm volatile("");
	} while (--loops);
}

void __attribute__((weak)) mcu_io_init(void)
{
#if ASSERT_PIN(STEP0)
	mcu_config_output(STEP0);
#endif
#if ASSERT_PIN(STEP1)
	mcu_config_output(STEP1);
#endif
#if ASSERT_PIN(STEP2)
	mcu_config_output(STEP2);
#endif
#if ASSERT_PIN(STEP3)
	mcu_config_output(STEP3);
#endif
#if ASSERT_PIN(STEP4)
	mcu_config_output(STEP4);
#endif
#if ASSERT_PIN(STEP5)
	mcu_config_output(STEP5);
#endif
#if ASSERT_PIN(STEP6)
	mcu_config_output(STEP6);
#endif
#if ASSERT_PIN(STEP7)
	mcu_config_output(STEP7);
#endif
#if ASSERT_PIN(DIR0)
	mcu_config_output(DIR0);
#endif
#if ASSERT_PIN(DIR1)
	mcu_config_output(DIR1);
#endif
#if ASSERT_PIN(DIR2)
	mcu_config_output(DIR2);
#endif
#if ASSERT_PIN(DIR3)
	mcu_config_output(DIR3);
#endif
#if ASSERT_PIN(DIR4)
	mcu_config_output(DIR4);
#endif
#if ASSERT_PIN(DIR5)
	mcu_config_output(DIR5);
#endif
#if ASSERT_PIN(DIR6)
	mcu_config_output(DIR6);
#endif
#if ASSERT_PIN(DIR7)
	mcu_config_output(DIR7);
#endif
#if ASSERT_PIN(STEP0_EN)
	mcu_config_output(STEP0_EN);
#endif
#if ASSERT_PIN(STEP1_EN)
	mcu_config_output(STEP1_EN);
#endif
#if ASSERT_PIN(STEP2_EN)
	mcu_config_output(STEP2_EN);
#endif
#if ASSERT_PIN(STEP3_EN)
	mcu_config_output(STEP3_EN);
#endif
#if ASSERT_PIN(STEP4_EN)
	mcu_config_output(STEP4_EN);
#endif
#if ASSERT_PIN(STEP5_EN)
	mcu_config_output(STEP5_EN);
#endif
#if ASSERT_PIN(STEP6_EN)
	mcu_config_output(STEP6_EN);
#endif
#if ASSERT_PIN(STEP7_EN)
	mcu_config_output(STEP7_EN);
#endif
#if ASSERT_PIN(PWM0)
	mcu_config_pwm(PWM0, 1000);
#endif
#if ASSERT_PIN(PWM1)
	mcu_config_pwm(PWM1, 1000);
#endif
#if ASSERT_PIN(PWM2)
	mcu_config_pwm(PWM2, 1000);
#endif
#if ASSERT_PIN(PWM3)
	mcu_config_pwm(PWM3, 1000);
#endif
#if ASSERT_PIN(PWM4)
	mcu_config_pwm(PWM4, 1000);
#endif
#if ASSERT_PIN(PWM5)
	mcu_config_pwm(PWM5, 1000);
#endif
#if ASSERT_PIN(PWM6)
	mcu_config_pwm(PWM6, 1000);
#endif
#if ASSERT_PIN(PWM7)
	mcu_config_pwm(PWM7, 1000);
#endif
#if ASSERT_PIN(PWM8)
	mcu_config_pwm(PWM8, 1000);
#endif
#if ASSERT_PIN(PWM9)
	mcu_config_pwm(PWM9, 1000);
#endif
#if ASSERT_PIN(PWM10)
	mcu_config_pwm(PWM10, 1000);
#endif
#if ASSERT_PIN(PWM11)
	mcu_config_pwm(PWM11, 1000);
#endif
#if ASSERT_PIN(PWM12)
	mcu_config_pwm(PWM12, 1000);
#endif
#if ASSERT_PIN(PWM13)
	mcu_config_pwm(PWM13, 1000);
#endif
#if ASSERT_PIN(PWM14)
	mcu_config_pwm(PWM14, 1000);
#endif
#if ASSERT_PIN(PWM15)
	mcu_config_pwm(PWM15, 1000);
#endif
#if ASSERT_PIN(SERVO0)
	mcu_config_output(SERVO0);
#endif
#if ASSERT_PIN(SERVO1)
	mcu_config_output(SERVO1);
#endif
#if ASSERT_PIN(SERVO2)
	mcu_config_output(SERVO2);
#endif
#if ASSERT_PIN(SERVO3)
	mcu_config_output(SERVO3);
#endif
#if ASSERT_PIN(SERVO4)
	mcu_config_output(SERVO4);
#endif
#if ASSERT_PIN(SERVO5)
	mcu_config_output(SERVO5);
#endif
#if ASSERT_PIN(DOUT0)
	mcu_config_output(DOUT0);
#endif
#if ASSERT_PIN(DOUT1)
	mcu_config_output(DOUT1);
#endif
#if ASSERT_PIN(DOUT2)
	mcu_config_output(DOUT2);
#endif
#if ASSERT_PIN(DOUT3)
	mcu_config_output(DOUT3);
#endif
#if ASSERT_PIN(DOUT4)
	mcu_config_output(DOUT4);
#endif
#if ASSERT_PIN(DOUT5)
	mcu_config_output(DOUT5);
#endif
#if ASSERT_PIN(DOUT6)
	mcu_config_output(DOUT6);
#endif
#if ASSERT_PIN(DOUT7)
	mcu_config_output(DOUT7);
#endif
#if ASSERT_PIN(DOUT8)
	mcu_config_output(DOUT8);
#endif
#if ASSERT_PIN(DOUT9)
	mcu_config_output(DOUT9);
#endif
#if ASSERT_PIN(DOUT10)
	mcu_config_output(DOUT10);
#endif
#if ASSERT_PIN(DOUT11)
	mcu_config_output(DOUT11);
#endif
#if ASSERT_PIN(DOUT12)
	mcu_config_output(DOUT12);
#endif
#if ASSERT_PIN(DOUT13)
	mcu_config_output(DOUT13);
#endif
#if ASSERT_PIN(DOUT14)
	mcu_config_output(DOUT14);
#endif
#if ASSERT_PIN(DOUT15)
	mcu_config_output(DOUT15);
#endif
#if ASSERT_PIN(DOUT16)
	mcu_config_output(DOUT16);
#endif
#if ASSERT_PIN(DOUT17)
	mcu_config_output(DOUT17);
#endif
#if ASSERT_PIN(DOUT18)
	mcu_config_output(DOUT18);
#endif
#if ASSERT_PIN(DOUT19)
	mcu_config_output(DOUT19);
#endif
#if ASSERT_PIN(DOUT20)
	mcu_config_output(DOUT20);
#endif
#if ASSERT_PIN(DOUT21)
	mcu_config_output(DOUT21);
#endif
#if ASSERT_PIN(DOUT22)
	mcu_config_output(DOUT22);
#endif
#if ASSERT_PIN(DOUT23)
	mcu_config_output(DOUT23);
#endif
#if ASSERT_PIN(DOUT24)
	mcu_config_output(DOUT24);
#endif
#if ASSERT_PIN(DOUT25)
	mcu_config_output(DOUT25);
#endif
#if ASSERT_PIN(DOUT26)
	mcu_config_output(DOUT26);
#endif
#if ASSERT_PIN(DOUT27)
	mcu_config_output(DOUT27);
#endif
#if ASSERT_PIN(DOUT28)
	mcu_config_output(DOUT28);
#endif
#if ASSERT_PIN(DOUT29)
	mcu_config_output(DOUT29);
#endif
#if ASSERT_PIN(DOUT30)
	mcu_config_output(DOUT30);
#endif
#if ASSERT_PIN(DOUT31)
	mcu_config_output(DOUT31);
#endif
#if ASSERT_PIN(LIMIT_X)
	mcu_config_input(LIMIT_X);
#ifdef LIMIT_X_PULLUP
	mcu_config_pullup(LIMIT_X);
#endif
#ifdef LIMIT_X_ISR
	mcu_config_input_isr(LIMIT_X);
#endif
#endif
#if ASSERT_PIN(LIMIT_Y)
	mcu_config_input(LIMIT_Y);
#ifdef LIMIT_Y_PULLUP
	mcu_config_pullup(LIMIT_Y);
#endif
#ifdef LIMIT_Y_ISR
	mcu_config_input_isr(LIMIT_Y);
#endif
#endif
#if ASSERT_PIN(LIMIT_Z)
	mcu_config_input(LIMIT_Z);
#ifdef LIMIT_Z_PULLUP
	mcu_config_pullup(LIMIT_Z);
#endif
#ifdef LIMIT_Z_ISR
	mcu_config_input_isr(LIMIT_Z);
#endif
#endif
#if ASSERT_PIN(LIMIT_X2)
	mcu_config_input(LIMIT_X2);
#ifdef LIMIT_X2_PULLUP
	mcu_config_pullup(LIMIT_X2);
#endif
#ifdef LIMIT_X2_ISR
	mcu_config_input_isr(LIMIT_X2);
#endif
#endif
#if ASSERT_PIN(LIMIT_Y2)
	mcu_config_input(LIMIT_Y2);
#ifdef LIMIT_Y2_PULLUP
	mcu_config_pullup(LIMIT_Y2);
#endif
#ifdef LIMIT_Y2_ISR
	mcu_config_input_isr(LIMIT_Y2);
#endif
#endif
#if ASSERT_PIN(LIMIT_Z2)
	mcu_config_input(LIMIT_Z2);
#ifdef LIMIT_Z2_PULLUP
	mcu_config_pullup(LIMIT_Z2);
#endif
#ifdef LIMIT_Z2_ISR
	mcu_config_input_isr(LIMIT_Z2);
#endif
#endif
#if ASSERT_PIN(LIMIT_A)
	mcu_config_input(LIMIT_A);
#ifdef LIMIT_A_PULLUP
	mcu_config_pullup(LIMIT_A);
#endif
#ifdef LIMIT_A_ISR
	mcu_config_input_isr(LIMIT_A);
#endif
#endif
#if ASSERT_PIN(LIMIT_B)
	mcu_config_input(LIMIT_B);
#ifdef LIMIT_B_PULLUP
	mcu_config_pullup(LIMIT_B);
#endif
#ifdef LIMIT_B_ISR
	mcu_config_input_isr(LIMIT_B);
#endif
#endif
#if ASSERT_PIN(LIMIT_C)
	mcu_config_input(LIMIT_C);
#ifdef LIMIT_C_PULLUP
	mcu_config_pullup(LIMIT_C);
#endif
#ifdef LIMIT_C_ISR
	mcu_config_input_isr(LIMIT_C);
#endif
#endif
#if ASSERT_PIN(PROBE)
	mcu_config_input(PROBE);
#ifdef PROBE_PULLUP
	mcu_config_pullup(PROBE);
#endif
#ifdef PROBE_ISR
	mcu_config_input_isr(PROBE);
#endif
#endif
#if ASSERT_PIN(ESTOP)
	mcu_config_input(ESTOP);
#ifdef ESTOP_PULLUP
	mcu_config_pullup(ESTOP);
#endif
#ifdef ESTOP_ISR
	mcu_config_input_isr(ESTOP);
#endif
#endif
#if ASSERT_PIN(SAFETY_DOOR)
	mcu_config_input(SAFETY_DOOR);
#ifdef SAFETY_DOOR_PULLUP
	mcu_config_pullup(SAFETY_DOOR);
#endif
#ifdef SAFETY_DOOR_ISR
	mcu_config_input_isr(SAFETY_DOOR);
#endif
#endif
#if ASSERT_PIN(FHOLD)
	mcu_config_input(FHOLD);
#ifdef FHOLD_PULLUP
	mcu_config_pullup(FHOLD);
#endif
#ifdef FHOLD_ISR
	mcu_config_input_isr(FHOLD);
#endif
#endif
#if ASSERT_PIN(CS_RES)
	mcu_config_input(CS_RES);
#ifdef CS_RES_PULLUP
	mcu_config_pullup(CS_RES);
#endif
#ifdef CS_RES_ISR
	mcu_config_input_isr(CS_RES);
#endif
#endif
#if ASSERT_PIN(ANALOG0)
	mcu_config_analog(ANALOG0);
#endif
#if ASSERT_PIN(ANALOG1)
	mcu_config_analog(ANALOG1);
#endif
#if ASSERT_PIN(ANALOG2)
	mcu_config_analog(ANALOG2);
#endif
#if ASSERT_PIN(ANALOG3)
	mcu_config_analog(ANALOG3);
#endif
#if ASSERT_PIN(ANALOG4)
	mcu_config_analog(ANALOG4);
#endif
#if ASSERT_PIN(ANALOG5)
	mcu_config_analog(ANALOG5);
#endif
#if ASSERT_PIN(ANALOG6)
	mcu_config_analog(ANALOG6);
#endif
#if ASSERT_PIN(ANALOG7)
	mcu_config_analog(ANALOG7);
#endif
#if ASSERT_PIN(ANALOG8)
	mcu_config_analog(ANALOG8);
#endif
#if ASSERT_PIN(ANALOG9)
	mcu_config_analog(ANALOG9);
#endif
#if ASSERT_PIN(ANALOG10)
	mcu_config_analog(ANALOG10);
#endif
#if ASSERT_PIN(ANALOG11)
	mcu_config_analog(ANALOG11);
#endif
#if ASSERT_PIN(ANALOG12)
	mcu_config_analog(ANALOG12);
#endif
#if ASSERT_PIN(ANALOG13)
	mcu_config_analog(ANALOG13);
#endif
#if ASSERT_PIN(ANALOG14)
	mcu_config_analog(ANALOG14);
#endif
#if ASSERT_PIN(ANALOG15)
	mcu_config_analog(ANALOG15);
#endif
#if ASSERT_PIN(DIN0)
	mcu_config_input(DIN0);
#ifdef DIN0_PULLUP
	mcu_config_pullup(DIN0);
#endif
#ifdef DIN0_ISR
	mcu_config_input_isr(DIN0);
#endif
#endif
#if ASSERT_PIN(DIN1)
	mcu_config_input(DIN1);
#ifdef DIN1_PULLUP
	mcu_config_pullup(DIN1);
#endif
#ifdef DIN1_ISR
	mcu_config_input_isr(DIN1);
#endif
#endif
#if ASSERT_PIN(DIN2)
	mcu_config_input(DIN2);
#ifdef DIN2_PULLUP
	mcu_config_pullup(DIN2);
#endif
#ifdef DIN2_ISR
	mcu_config_input_isr(DIN2);
#endif
#endif
#if ASSERT_PIN(DIN3)
	mcu_config_input(DIN3);
#ifdef DIN3_PULLUP
	mcu_config_pullup(DIN3);
#endif
#ifdef DIN3_ISR
	mcu_config_input_isr(DIN3);
#endif
#endif
#if ASSERT_PIN(DIN4)
	mcu_config_input(DIN4);
#ifdef DIN4_PULLUP
	mcu_config_pullup(DIN4);
#endif
#ifdef DIN4_ISR
	mcu_config_input_isr(DIN4);
#endif
#endif
#if ASSERT_PIN(DIN5)
	mcu_config_input(DIN5);
#ifdef DIN5_PULLUP
	mcu_config_pullup(DIN5);
#endif
#ifdef DIN5_ISR
	mcu_config_input_isr(DIN5);
#endif
#endif
#if ASSERT_PIN(DIN6)
	mcu_config_input(DIN6);
#ifdef DIN6_PULLUP
	mcu_config_pullup(DIN6);
#endif
#ifdef DIN6_ISR
	mcu_config_input_isr(DIN6);
#endif
#endif
#if ASSERT_PIN(DIN7)
	mcu_config_input(DIN7);
#ifdef DIN7_PULLUP
	mcu_config_pullup(DIN7);
#endif
#ifdef DIN7_ISR
	mcu_config_input_isr(DIN7);
#endif
#endif
#if ASSERT_PIN(DIN8)
	mcu_config_input(DIN8);
#ifdef DIN8_PULLUP
	mcu_config_pullup(DIN8);
#endif
#endif
#if ASSERT_PIN(DIN9)
	mcu_config_input(DIN9);
#ifdef DIN9_PULLUP
	mcu_config_pullup(DIN9);
#endif
#endif
#if ASSERT_PIN(DIN10)
	mcu_config_input(DIN10);
#ifdef DIN10_PULLUP
	mcu_config_pullup(DIN10);
#endif
#endif
#if ASSERT_PIN(DIN11)
	mcu_config_input(DIN11);
#ifdef DIN11_PULLUP
	mcu_config_pullup(DIN11);
#endif
#endif
#if ASSERT_PIN(DIN12)
	mcu_config_input(DIN12);
#ifdef DIN12_PULLUP
	mcu_config_pullup(DIN12);
#endif
#endif
#if ASSERT_PIN(DIN13)
	mcu_config_input(DIN13);
#ifdef DIN13_PULLUP
	mcu_config_pullup(DIN13);
#endif
#endif
#if ASSERT_PIN(DIN14)
	mcu_config_input(DIN14);
#ifdef DIN14_PULLUP
	mcu_config_pullup(DIN14);
#endif
#endif
#if ASSERT_PIN(DIN15)
	mcu_config_input(DIN15);
#ifdef DIN15_PULLUP
	mcu_config_pullup(DIN15);
#endif
#endif
#if ASSERT_PIN(DIN16)
	mcu_config_input(DIN16);
#ifdef DIN16_PULLUP
	mcu_config_pullup(DIN16);
#endif
#endif
#if ASSERT_PIN(DIN17)
	mcu_config_input(DIN17);
#ifdef DIN17_PULLUP
	mcu_config_pullup(DIN17);
#endif
#endif
#if ASSERT_PIN(DIN18)
	mcu_config_input(DIN18);
#ifdef DIN18_PULLUP
	mcu_config_pullup(DIN18);
#endif
#endif
#if ASSERT_PIN(DIN19)
	mcu_config_input(DIN19);
#ifdef DIN19_PULLUP
	mcu_config_pullup(DIN19);
#endif
#endif
#if ASSERT_PIN(DIN20)
	mcu_config_input(DIN20);
#ifdef DIN20_PULLUP
	mcu_config_pullup(DIN20);
#endif
#endif
#if ASSERT_PIN(DIN21)
	mcu_config_input(DIN21);
#ifdef DIN21_PULLUP
	mcu_config_pullup(DIN21);
#endif
#endif
#if ASSERT_PIN(DIN22)
	mcu_config_input(DIN22);
#ifdef DIN22_PULLUP
	mcu_config_pullup(DIN22);
#endif
#endif
#if ASSERT_PIN(DIN23)
	mcu_config_input(DIN23);
#ifdef DIN23_PULLUP
	mcu_config_pullup(DIN23);
#endif
#endif
#if ASSERT_PIN(DIN24)
	mcu_config_input(DIN24);
#ifdef DIN24_PULLUP
	mcu_config_pullup(DIN24);
#endif
#endif
#if ASSERT_PIN(DIN25)
	mcu_config_input(DIN25);
#ifdef DIN25_PULLUP
	mcu_config_pullup(DIN25);
#endif
#endif
#if ASSERT_PIN(DIN26)
	mcu_config_input(DIN26);
#ifdef DIN26_PULLUP
	mcu_config_pullup(DIN26);
#endif
#endif
#if ASSERT_PIN(DIN27)
	mcu_config_input(DIN27);
#ifdef DIN27_PULLUP
	mcu_config_pullup(DIN27);
#endif
#endif
#if ASSERT_PIN(DIN28)
	mcu_config_input(DIN28);
#ifdef DIN28_PULLUP
	mcu_config_pullup(DIN28);
#endif
#endif
#if ASSERT_PIN(DIN29)
	mcu_config_input(DIN29);
#ifdef DIN29_PULLUP
	mcu_config_pullup(DIN29);
#endif
#endif
#if ASSERT_PIN(DIN30)
	mcu_config_input(DIN30);
#ifdef DIN30_PULLUP
	mcu_config_pullup(DIN30);
#endif
#endif
#if ASSERT_PIN(DIN31)
	mcu_config_input(DIN31);
#ifdef DIN31_PULLUP
	mcu_config_pullup(DIN31);
#endif
#endif
#if ASSERT_PIN(TX)
	mcu_config_output(TX);
#endif
#if ASSERT_PIN(RX)
	mcu_config_input(RX);
#ifdef RX_PULLUP
	mcu_config_pullup(RX);
#endif
#endif
#if ASSERT_PIN(USB_DM)
	mcu_config_input(USB_DM);
#ifdef USB_DM_PULLUP
	mcu_config_pullup(USB_DM);
#endif
#endif
#if ASSERT_PIN(USB_DP)
	mcu_config_input(USB_DP);
#ifdef USB_DP_PULLUP
	mcu_config_pullup(USB_DP);
#endif
#endif
#if ASSERT_PIN(SPI_CLK)
	mcu_config_output(SPI_CLK);
#endif
#if ASSERT_PIN(SPI_SDI)
	mcu_config_input(SPI_SDI);
#ifdef SPI_SDI_PULLUP
	mcu_config_pullup(SPI_SDI);
#endif
#endif
#if ASSERT_PIN(SPI_SDO)
	mcu_config_output(SPI_SDO);
#endif
#if ASSERT_PIN(SPI_CS)
	mcu_config_output(SPI_CS);
#endif
#if ASSERT_PIN(I2C_CLK)
	mcu_config_input(I2C_CLK);
	mcu_config_pullup(I2C_CLK);
#endif
#if ASSERT_PIN(I2C_DATA)
	mcu_config_input(I2C_DATA);
	mcu_config_pullup(I2C_DATA);
#endif
#if ASSERT_PIN(TX2)
	mcu_config_output(TX2);
#endif
#if ASSERT_PIN(RX2)
	mcu_config_input(RX2);
#ifdef RX2_PULLUP
	mcu_config_pullup(RX2);
#endif
#endif
}

#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
uint8_t __attribute__((weak)) mcu_custom_grbl_cmd(char *grbl_cmd_str, uint8_t grbl_cmd_len, char next_char)
{
	return STATUS_INVALID_STATEMENT;
}
#endif

void mcu_putc(uint8_t c)
{
	// USB, WiFi and BT have usually dedicated buffers
#if defined(MCU_HAS_USB) && !defined(DETACH_USB_FROM_MAIN_PROTOCOL)
	mcu_usb_putc(c);
#endif
#if defined(MCU_HAS_BLUETOOTH) && !defined(DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL)
	mcu_bt_putc(c);
#endif
#if defined(MCU_HAS_UART) && !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
	mcu_uart_putc(c);
#endif
#if defined(MCU_HAS_UART2) && !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
	mcu_uart2_putc(c);
#endif
#if defined(MCU_HAS_WIFI) && !defined(DETACH_WIFI_FROM_MAIN_PROTOCOL)
	mcu_wifi_putc(c);
#endif
}

void mcu_flush(void)
{
#if defined(MCU_HAS_USB) && !defined(DETACH_USB_FROM_MAIN_PROTOCOL)
	mcu_usb_flush();
#endif
#if defined(MCU_HAS_BLUETOOTH) && !defined(DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL)
	mcu_bt_flush();
#endif
#if defined(MCU_HAS_UART) && !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
	mcu_uart_flush();
#endif
#if defined(MCU_HAS_UART2) && !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
	mcu_uart2_flush();
#endif
#if defined(MCU_HAS_WIFI) && !defined(DETACH_WIFI_FROM_MAIN_PROTOCOL)
	mcu_wifi_flush();
#endif
}

#if (defined(MCU_HAS_I2C))
#if defined(MCU_SUPPORTS_I2C_SLAVE) && (I2C_ADDRESS != 0)
void __attribute__((weak)) mcu_i2c_slave_cb(uint8_t *data, uint8_t *datalen)
{
}
#endif
#endif
