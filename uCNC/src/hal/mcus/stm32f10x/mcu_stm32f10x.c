/*
	Name: mcu_stm32f10x.h
	Description: Contains all the function declarations necessary to interact with the MCU.
        This provides a opac intenterface between the µCNC and the MCU unit used to power the µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_STM32F10X)
#include "core_cm3.h"
#include "mcumap_stm32f10x.h"
#include "../../../interface/serial.h"
#include "../../../core/interpolator.h"
#include "../../../core/io_control.h"

#ifdef USB_VCP
#include "../../../tinyusb/tusb_config.h"
#include "../../../tinyusb/src/tusb.h"

#endif

/**
	 * The internal clock counter
	 * Increments every millisecond
	 * Can count up to almost 50 days
	 **/
static volatile uint32_t mcu_runtime_ms;
volatile bool stm32_global_isr_enabled;

#define mcu_config_output(diopin)                                                                                           \
	{                                                                                                                       \
		RCC->APB2ENR |= __indirect__(diopin, APB2EN);                                                                       \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << (__indirect__(diopin, CROFF) << 2U));       \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (GPIO_OUT_PP_50MHZ << (__indirect__(diopin, CROFF) << 2U)); \
	}

#define mcu_config_input(diopin)                                                                                        \
	{                                                                                                                   \
		RCC->APB2ENR |= __indirect__(diopin, APB2EN);                                                                   \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << (__indirect__(diopin, CROFF) << 2U));   \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (GPIO_IN_FLOAT << (__indirect__(diopin, CROFF) << 2U)); \
	}

#define mcu_config_pullup(diopin) (                                                                                   \
	{                                                                                                                 \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << (__indirect__(diopin, CROFF) << 2U)); \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (GPIO_IN_PUP << (__indirect__(diopin, CROFF) << 2U)); \
		__indirect__(diopin, GPIO)->BSRR = (1U << __indirect__(diopin, BIT));                                         \
	})

#define mcu_config_pwm(diopin) (                                                                                                 \
	{                                                                                                                            \
		RCC->APB2ENR |= 0x1U;                                                                                                    \
		__indirect__(diopin, ENREG) |= __indirect__(diopin, APBEN);                                                              \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << ((__indirect__(diopin, CROFF)) << 2U));          \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (GPIO_OUTALT_PP_50MHZ << ((__indirect__(diopin, CROFF)) << 2U)); \
		__indirect__(diopin, TIMREG)->CR1 = 0;                                                                                   \
		__indirect__(diopin, TIMREG)->PSC = (uint16_t)(F_CPU / 1000000UL) - 1;                                                   \
		__indirect__(diopin, TIMREG)->ARR = (uint16_t)(1000000UL / __indirect__(diopin, FREQ)) - 1;                              \
		__indirect__(diopin, TIMREG)->__indirect__(diopin, CCR) = 0;                                                             \
		__indirect__(diopin, TIMREG)->__indirect__(diopin, CCMREG) = __indirect__(diopin, MODE);                                 \
		__indirect__(diopin, TIMREG)->CCER |= (1U << ((__indirect__(diopin, CHANNEL) - 1) << 2));                                \
		__indirect__(diopin, TIMREG)->BDTR |= (1 << 15);                                                                         \
		__indirect__(diopin, TIMREG)->CR1 |= 0x01U;                                                                              \
		__indirect__(diopin, ENOUTPUT);                                                                                          \
	})

#define mcu_config_input_isr(diopin) (                                                                          \
	{                                                                                                           \
		RCC->APB2ENR |= 0x1U;                                                                                   \
		AFIO->EXTICR[(__indirect__(diopin, EXTIREG))] &= ~(0xF << (((__indirect__(diopin, BIT)) & 0x03) << 2)); \
		AFIO->EXTICR[(__indirect__(diopin, EXTIREG))] |= (__indirect__(diopin, EXTIVAL));                       \
		SETBIT(EXTI->RTSR, __indirect__(diopin, BIT));                                                          \
		SETBIT(EXTI->FTSR, __indirect__(diopin, BIT));                                                          \
		SETBIT(EXTI->IMR, __indirect__(diopin, BIT));                                                           \
		NVIC_SetPriority(__indirect__(diopin, IRQ), 5);                                                         \
		NVIC_ClearPendingIRQ(__indirect__(diopin, IRQ));                                                        \
		NVIC_EnableIRQ(__indirect__(diopin, IRQ));                                                              \
	})

#define mcu_config_analog(diopin) (                                                                                     \
	{                                                                                                                   \
		RCC->CFGR &= ~(RCC_CFGR_ADCPRE);                                                                                \
		RCC->CFGR |= RCC_CFGR_ADCPRE_DIV8;                                                                              \
		RCC->APB2ENR |= (RCC_APB2ENR_ADC1EN | __indirect__(diopin, APB2EN) | 0x1U);                                     \
		ADC1->SQR1 = 1; /*one conversion*/                                                                              \
		ADC1->SMPR1 = 0x00ffffff & 0x36DB6DB6;                                                                          \
		ADC1->SMPR2 = 0x36DB6DB6;                                                                                       \
		ADC1->CR2 &= ~ADC_CR2_CONT; /*single conversion mode*/                                                          \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << ((__indirect__(diopin, CROFF)) << 2U)); \
		ADC1->CR2 |= ADC_CR2_ADON;	 /*enable adc*/                                                                     \
		ADC1->CR2 |= ADC_CR2_RSTCAL; /*reset calibration*/                                                              \
		while (ADC1->CR2 & ADC_CR2_RSTCAL)                                                                              \
			;                                                                                                           \
		ADC1->CR2 |= ADC_CR2_CAL; /*start calibration*/                                                                 \
		while (ADC1->CR2 & ADC_CR2_CAL)                                                                                 \
			;                                                                                                           \
		ADC1->CR2 |= (ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG); /*external start trigger software*/                            \
	})

/**
 * The isr functions
 * The respective IRQHandler will execute these functions 
 **/
#ifdef COM_PORT
void mcu_serial_isr(void)
{
	mcu_disable_global_isr();
#ifndef ENABLE_SYNC_RX
	if (COM_USART->SR & USART_SR_RXNE)
	{
		unsigned char c = COM_INREG;
		serial_rx_isr(c);
	}
#endif

#ifndef ENABLE_SYNC_TX
	if (COM_USART->SR & (USART_SR_TXE | USART_SR_TC))
	{
		COM_USART->CR1 &= ~(USART_CR1_TXEIE);
		serial_tx_isr();
	}
#endif
	mcu_enable_global_isr();
}
#elif defined(USB_VCP)
void USB_HP_CAN1_TX_IRQHandler(void)
{
	mcu_disable_global_isr();
	tud_int_handler(0);
	mcu_enable_global_isr();
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
	mcu_disable_global_isr();
	tud_int_handler(0);
	mcu_enable_global_isr();
}

void USBWakeUp_IRQHandler(void)
{
	mcu_disable_global_isr();
	tud_int_handler(0);
	mcu_enable_global_isr();
}

#endif

void mcu_timer_isr(void)
{
	mcu_disable_global_isr();

	static bool resetstep = false;
	if ((TIMER_REG->SR & 1))
	{
		if (!resetstep)
			itp_step_isr();
		else
			itp_step_reset_isr();
		resetstep = !resetstep;
	}
	TIMER_REG->SR = 0;
	
	mcu_enable_global_isr();
}

#define LIMITS_EXTIBITMASK (LIMIT_X_EXTIBITMASK | LIMIT_Y_EXTIBITMASK | LIMIT_Z_EXTIBITMASK | LIMIT_X2_EXTIBITMASK | LIMIT_Y2_EXTIBITMASK | LIMIT_Z2_EXTIBITMASK | LIMIT_A_EXTIBITMASK | LIMIT_B_EXTIBITMASK | LIMIT_C_EXTIBITMASK)
#define CONTROLS_EXTIBITMASK (ESTOP_EXTIBITMASK | SAFETY_DOOR_EXTIBITMASK | FHOLD_EXTIBITMASK | CS_RES_EXTIBITMASK)
#define DIN_IO_EXTIBITMASK (DIN0_EXTIBITMASK | DIN1_EXTIBITMASK | DIN2_EXTIBITMASK | DIN3_EXTIBITMASK | DIN4_EXTIBITMASK | DIN5_EXTIBITMASK | DIN6_EXTIBITMASK | DIN7_EXTIBITMASK)
#define ALL_EXTIBITMASK (LIMITS_EXTIBITMASK | CONTROLS_EXTIBITMASK | PROBE_EXTIBITMASK | DIN_IO_EXTIBITMASK)

#if (ALL_EXTIBITMASK != 0)
static void mcu_input_isr(void)
{
	
#if (LIMITS_EXTIBITMASK != 0)
	if (EXTI->PR & LIMITS_EXTIBITMASK)
	{
		io_limits_isr();
	}
#endif
#if (CONTROLS_EXTIBITMASK != 0)
	if (EXTI->PR & CONTROLS_EXTIBITMASK)
	{
		io_controls_isr();
	}
#endif
#if (PROBE_EXTIBITMASK & 0x01)
	if (EXTI->PR & PROBE_EXTIBITMASK)
	{
		io_probe_isr();
	}
#endif
#if (DIN_IO_EXTIBITMASK != 0)
	if (EXTI->PR & DIN_IO_EXTIBITMASK)
	{
		io_inputs_isr();
	}
#endif

	EXTI->PR = ALL_EXTIBITMASK;
	mcu_enable_global_isr();
}

#if (ALL_EXTIBITMASK & 0x0001)
void EXTI0_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0x0002)
void EXTI1_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0x0004)
void EXTI2_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0x0008)
void EXTI3_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0x0010)
void EXTI4_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0x03E0)
void EXTI9_5_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0xFC00)
void EXTI15_10_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#endif

#ifndef ARDUINO_ARCH_STM32
void SysTick_Handler(void)
#else
void osSystickHandler(void)
#endif
{
	mcu_disable_global_isr();
	cnc_scheduletasks(++mcu_runtime_ms);
	mcu_enable_global_isr();
}

/**
	 * 
 * Initializes the mcu:
 *   1. Configures all IO
 *   2. Configures UART/USB
 *   3. Starts internal clock (RTC)
 **/
static void mcu_tick_init(void);
static void mcu_usart_init(void);

void mcu_init(void)
{
	stm32_global_isr_enabled = false;
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
	mcu_config_analog(ANALOG0);
#endif
#if ANALOG1 >= 0
	mcu_config_analog(ANALOG1);
#endif
#if ANALOG2 >= 0
	mcu_config_analog(ANALOG2);
#endif
#if ANALOG3 >= 0
	mcu_config_analog(ANALOG3);
#endif
#if ANALOG4 >= 0
	mcu_config_analog(ANALOG4);
#endif
#if ANALOG5 >= 0
	mcu_config_analog(ANALOG5);
#endif
#if ANALOG6 >= 0
	mcu_config_analog(ANALOG6);
#endif
#if ANALOG7 >= 0
	mcu_config_analog(ANALOG7);
#endif
#if ANALOG8 >= 0
	mcu_config_analog(ANALOG8);
#endif
#if ANALOG9 >= 0
	mcu_config_analog(ANALOG9);
#endif
#if ANALOG10 >= 0
	mcu_config_analog(ANALOG10);
#endif
#if ANALOG11 >= 0
	mcu_config_analog(ANALOG11);
#endif
#if ANALOG12 >= 0
	mcu_config_analog(ANALOG12);
#endif
#if ANALOG13 >= 0
	mcu_config_analog(ANALOG13);
#endif
#if ANALOG14 >= 0
	mcu_config_analog(ANALOG14);
#endif
#if ANALOG15 >= 0
	mcu_config_analog(ANALOG15);
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
#if TX >= 0
	mcu_config_output(TX);
#endif
#if RX >= 0
	mcu_config_input(RX);
#endif
#if USB_DM >= 0
	mcu_config_input(USB_DM);
#endif
#if USB_DP >= 0
	mcu_config_input(USB_DP);
#endif

	mcu_usart_init();
	mcu_tick_init();
	mcu_disable_probe_isr();
	mcu_enable_global_isr();
}

/*IO functions*/
#ifndef mcu_get_input
uint8_t mcu_get_input(uint8_t pin)
{
}
#endif

#ifndef mcu_get_output
uint8_t mcu_get_output(uint8_t pin)
{
}
#endif

#ifndef mcu_set_output
void mcu_set_output(uint8_t pin)
{
}
#endif

#ifndef mcu_clear_output
void mcu_clear_output(uint8_t pin)
{
}
#endif

#ifndef mcu_toggle_output
void mcu_toggle_output(uint8_t pin)
{
}
#endif

#ifndef mcu_enable_probe_isr
void mcu_enable_probe_isr(void)
{
}
#endif
#ifndef mcu_disable_probe_isr
void mcu_disable_probe_isr(void)
{
}
#endif

//Analog input
#ifndef mcu_get_analog
uint8_t mcu_get_analog(uint8_t channel)
{
}
#endif

//PWM
#ifndef mcu_set_pwm
void mcu_set_pwm(uint8_t pwm, uint8_t value)
{
}
#endif

#ifndef mcu_get_pwm
uint8_t mcu_get_pwm(uint8_t pwm)
{
}
#endif

static uint8_t mcu_tx_buffer[TX_BUFFER_SIZE];

void mcu_usart_init(void)
{
#ifdef COM_PORT
	/*enables RCC clocks and GPIO*/
	RCC->APB2ENR |= (RCC_APB2ENR_AFIOEN);
	RCC->COM_APB |= (COM_APBEN);
	RCC->APB2ENR |= __indirect__(TX, APB2EN);
	__indirect__(TX, GPIO)->__indirect__(TX, CR) &= ~(GPIO_RESET << ((__indirect__(TX, CROFF)) << 2));
	__indirect__(TX, GPIO)->__indirect__(TX, CR) |= (GPIO_OUTALT_PP_50MHZ << ((__indirect__(TX, CROFF)) << 2));
	RCC->APB2ENR |= __indirect__(RX, APB2EN);
	__indirect__(RX, GPIO)->__indirect__(RX, CR) &= ~(GPIO_RESET << ((__indirect__(RX, CROFF)) << 2));
	__indirect__(RX, GPIO)->__indirect__(RX, CR) |= (GPIO_IN_FLOAT << ((__indirect__(RX, CROFF)) << 2));
	/*setup UART*/
	COM_USART->CR1 = 0; //8 bits No parity M=0 PCE=0
	COM_USART->CR2 = 0; //1 stop bit STOP=00
	COM_USART->CR3 = 0;
	COM_USART->SR = 0;
	// //115200 baudrate
	float baudrate = ((float)(F_CPU >> 4) / ((float)BAUDRATE));
	uint16_t brr = (uint16_t)baudrate;
	baudrate -= brr;
	brr <<= 4;
	brr += (uint16_t)roundf(16.0f * baudrate);
	COM_USART->BRR = brr;
#if (defined(ENABLE_SYNC_TX) || defined(ENABLE_SYNC_RX))
	NVIC_SetPriority(COM_IRQ, 3);
	NVIC_ClearPendingIRQ(COM_IRQ);
	NVIC_EnableIRQ(COM_IRQ);
#endif
	COM_USART->CR1 |= (USART_CR1_RE | USART_CR1_TE); // enable TE, RE
#ifndef ENABLE_SYNC_TX
	COM_USART->CR1 |= (USART_CR1_TXEIE); // enable TXEIE
#endif
#ifndef ENABLE_SYNC_RX
	COM_USART->CR1 |= USART_CR1_RXNEIE; // enable RXNEIE
#endif
	COM_USART->CR1 |= USART_CR1_UE; //Enable UART
#ifdef ENABLE_SYNC_TX
	//this null char is needed to set TXE bit by the harware
	COM_OUTREG = 0;
#endif
#else
	//configure USB as Virtual COM port
	RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
	mcu_config_input(USB_DM);
	mcu_config_input(USB_DP);
	NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 10);
	NVIC_ClearPendingIRQ(USB_HP_CAN1_TX_IRQn);
	NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
	NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 10);
	NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	NVIC_SetPriority(USBWakeUp_IRQn, 10);
	NVIC_ClearPendingIRQ(USBWakeUp_IRQn);
	NVIC_EnableIRQ(USBWakeUp_IRQn);

	//Enable USB interrupts and enable usb
	USB->CNTR |= (USB_CNTR_WKUPM | USB_CNTR_SOFM | USB_CNTR_ESOFM | USB_CNTR_CTRM);
	RCC->APB1ENR |= RCC_APB1ENR_USBEN;
	tusb_init();
#endif
}

void mcu_putc(char c)
{
#ifdef LED
	mcu_toggle_output(LED);
#endif
#ifdef COM_PORT

	if (c != 0)
	{
#ifdef ENABLE_SYNC_TX
		while (!(COM_USART->SR & USART_SR_TXE))
			;
#endif
		COM_OUTREG = c;
	}
#ifndef ENABLE_SYNC_TX
	COM_USART->CR1 |= (USART_CR1_TXEIE);
#endif
#endif
#ifdef USB_VCP
	if (c != 0)
	{
		tud_cdc_write_char(c);
	}
	if (c == '\r' || c == 0)
	{
		tud_cdc_write_flush();
	}
#endif
}

char mcu_getc(void)
{
#ifdef LED
	mcu_toggle_output(LED);
#endif
#ifdef COM_PORT
#ifdef ENABLE_SYNC_RX
	while (!(COM_USART->SR & USART_SR_RXNE))
		;
#endif
	return COM_INREG;
#endif
#ifdef USB_VCP
	while (!tud_cdc_available())
	{
		tud_task();
	}

	return (unsigned char)tud_cdc_read_char();
#endif
}

//ISR
//enables all interrupts on the mcu. Must be called to enable all IRS functions
#ifndef mcu_enable_global_isr
#error "mcu_enable_global_isr undefined"
#endif
//disables all ISR functions
#ifndef mcu_disable_global_isr
#error "mcu_disable_global_isr undefined"
#endif

//Timers
//convert step rate to clock cycles
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
{
	//up and down counter (generates half the step rate at each event)
	uint32_t totalticks = (uint32_t)((float)(F_CPU >> 2) / frequency);
	*prescaller = 1;
	while (totalticks > 0xFFFF)
	{
		*prescaller <<= 1;
		totalticks >>= 1;
	}

	*prescaller--;
	*ticks = (uint16_t)totalticks;
}

//starts a constant rate pulse at a given frequency.
void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	RCC->TIMER_ENREG |= TIMER_APB;
	TIMER_REG->CR1 = 0;
	TIMER_REG->DIER = 0;
	TIMER_REG->PSC = prescaller;
	TIMER_REG->ARR = ticks;
	TIMER_REG->EGR |= 0x01;
	TIMER_REG->SR &= ~0x01;

	NVIC_SetPriority(TIMER_IRQ, 1);
	NVIC_ClearPendingIRQ(TIMER_IRQ);
	NVIC_EnableIRQ(TIMER_IRQ);

	TIMER_REG->DIER |= 1;
	TIMER_REG->CR1 |= 1; //enable timer upcounter no preload
}

//modifies the pulse frequency
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	TIMER_REG->ARR = ticks;
	TIMER_REG->PSC = prescaller;
	TIMER_REG->EGR |= 0x01;
}

//stops the pulse
void mcu_stop_itp_isr(void)
{
	TIMER_REG->CR1 &= ~0x1;
	TIMER_REG->DIER &= ~0x1;
	TIMER_REG->SR &= ~0x01;
	NVIC_DisableIRQ(TIMER_IRQ);
}

//Custom delay function
//gets the mcu running time in ms
uint32_t mcu_millis()
{
	uint32_t val = mcu_runtime_ms;
	return val;
}

void mcu_tick_init()
{
	SysTick->CTRL = 0;
	SysTick->LOAD = (((F_CPU >> 3) / 1000) - 1);
	SysTick->VAL = 0;
	NVIC_SetPriority(SysTick_IRQn, 10);
	SysTick->CTRL = 3; //Start SysTick (ABH clock/8)
}

void mcu_dotasks()
{
#ifdef USB_VCP
	tud_cdc_write_flush();
	tud_task(); // tinyusb device task
#endif
#ifdef ENABLE_SYNC_RX
	while (mcu_rx_ready())
	{
		unsigned char c = mcu_getc();
		serial_rx_isr(c);
	}
#endif
}

static uint8_t stm32_flash_page[0x400];
static uint16_t stm32_flash_current_page;
static bool stm32_flash_modified;
//checks if the current page is loaded to ram
//if not loads it
static uint16_t mcu_access_flash_page(uint16_t address)
{
	uint16_t address_page = address & 0xfc00;
	uint16_t address_offset = address & 0x03ff;
	if (stm32_flash_current_page != address_page)
	{
		stm32_flash_modified = false;
		stm32_flash_current_page = address_page;
		uint8_t counter = 255;
		uint32_t *ptr = ((uint32_t *)&stm32_flash_page[0]);
		volatile uint32_t *eeprom = ((volatile uint32_t *)(FLASH_BASE + address_page));
		while (counter--)
		{
			*ptr = *eeprom;
			eeprom++;
			ptr++;
		}
	}

	return address_offset;
}

//Non volatile memory
uint8_t mcu_eeprom_getc(uint16_t address)
{
	uint16_t offset = mcu_access_flash_page(address);
	return stm32_flash_page[offset];
}

static void mcu_eeprom_erase(uint16_t address)
{
	while (FLASH->SR & FLASH_SR_BSY)
		; // wait while busy
	//unlock flash if locked
	if (FLASH->CR & FLASH_CR_LOCK)
	{
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;
	}
	FLASH->CR = 0;			   // Ensure PG bit is low
	FLASH->CR |= FLASH_CR_PER; // set the PER bit
	FLASH->AR = (FLASH_BASE + address);
	FLASH->CR |= FLASH_CR_STRT; // set the start bit
	while (FLASH->SR & FLASH_SR_BSY)
		; // wait while busy
	FLASH->CR = 0;
}

extern void protocol_send_error(uint8_t error);

void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	uint16_t offset = mcu_access_flash_page(address);

	if (stm32_flash_page[offset] != value)
	{
		stm32_flash_modified = true;
	}

	stm32_flash_page[offset] = value;
}

void mcu_eeprom_flush()
{
	if (stm32_flash_modified)
	{
		mcu_eeprom_erase(stm32_flash_current_page);
		volatile uint16_t *eeprom = ((volatile uint16_t *)(FLASH_BASE + stm32_flash_current_page));
		uint16_t *ptr = ((uint16_t *)&stm32_flash_page[0]);
		uint16_t counter = 512;
		while (counter--)
		{
			while (FLASH->SR & FLASH_SR_BSY)
				; // wait while busy
			mcu_disable_global_isr();
			// unlock flash if locked
			if (FLASH->CR & FLASH_CR_LOCK)
			{
				FLASH->KEYR = 0x45670123;
				FLASH->KEYR = 0xCDEF89AB;
			}
			FLASH->CR = 0;
			FLASH->CR |= FLASH_CR_PG; // Ensure PG bit is high
			*eeprom = *ptr;
			while (FLASH->SR & FLASH_SR_BSY)
				; // wait while busy
			mcu_enable_global_isr();
			if (FLASH->SR & FLASH_SR_PGERR)
				protocol_send_error(42); // STATUS_SETTING_WRITE_FAIL
			if (FLASH->SR & FLASH_SR_WRPRTERR)
				protocol_send_error(43); // STATUS_SETTING_PROTECTED_FAIL
			FLASH->CR = 0;				 // Ensure PG bit is low
			FLASH->SR = 0;
			eeprom++;
			ptr++;
		}
		stm32_flash_modified = false;
		// Restore interrupt flag state.*/
	}
}

#endif
