/*
	Name: mcu_lpc176x.c
	Description: Contains all the function declarations necessary to interact with the MCU.
		This provides an intenterface between the µCNC and the MCU unit used to power the µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07-06-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_LPC176X)
#include "core_cm3.h"
#include "mcumap_lpc176x.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "lpc_types.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_pwm.h"
#include "system_LPC17xx.h"

#if (INTERFACE == INTERFACE_USB)
#ifdef TX
#undef TX
#endif
#ifdef RX
#undef RX
#endif
#include "../../../tinyusb/tusb_config.h"
#include "../../../tinyusb/src/tusb.h"
#endif

/**
 * The internal clock counter
 * Increments every millisecond
 * Can count up to almost 50 days
 **/
static volatile uint32_t mcu_runtime_ms;
volatile bool lpc_global_isr_enabled;

#define mcu_config_input_isr(diopin)                                                   \
	{                                                                                  \
		SETBIT(LPC_GPIOINT->__indirect__(diopin, RISEREG), __indirect__(diopin, BIT)); \
		SETBIT(LPC_GPIOINT->__indirect__(diopin, FALLREG), __indirect__(diopin, BIT)); \
		NVIC_SetPriority(EINT3_IRQn, 5);                                               \
		NVIC_ClearPendingIRQ(EINT3_IRQn);                                              \
		NVIC_EnableIRQ(EINT3_IRQn);                                                    \
	}

#define mcu_config_pullup(diopin)                                                                                             \
	{                                                                                                                         \
		LPC_PINCON->__helper__(PINMODE, __indirect__(diopin, PINCON), ) &= ~(3 << (0x1F & (__indirect__(diopin, BIT) << 1))); \
	}

#define mcu_config_pwm(diopin)                                                                                                                                   \
	{                                                                                                                                                            \
		mcu_config_output(diopin);                                                                                                                               \
		CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCPWM1, ENABLE);                                                                                                          \
		CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_PWM1, CLKPWR_PCLKSEL_CCLK_DIV_4);                                                                                       \
		LPC_PWM1->IR = 0xFF & PWM_IR_BITMASK;                                                                                                                    \
		LPC_PWM1->TCR = 0;                                                                                                                                       \
		LPC_PWM1->CTCR = 0;                                                                                                                                      \
		LPC_PWM1->MCR = 0;                                                                                                                                       \
		LPC_PWM1->CCR = 0;                                                                                                                                       \
		LPC_PWM1->PCR &= 0xFF00;                                                                                                                                 \
		LPC_PWM1->LER |= (1UL << 0) | (1UL << __indirect__(diopin, CHANNEL));                                                                                    \
		LPC_PWM1->PCR |= (1UL << (8 + __indirect__(diopin, CHANNEL)));                                                                                           \
		LPC_PWM1->PR = (CLKPWR_GetPCLK(CLKPWR_PCLKSEL_PWM1) / (255 * 1000)) - 1;                                                                                 \
		LPC_PWM1->MCR = (1UL << 1);                                                                                                                              \
		LPC_PWM1->MR0 = 255;                                                                                                                                     \
		LPC_PWM1->TCR = (1UL << 3) | (1UL << 0);                                                                                                                 \
		mcu_config_output(diopin);                                                                                                                               \
		PINSEL_CFG_Type pwm = {__indirect__(diopin, PORT), __indirect__(diopin, BIT), __indirect__(diopin, FUNC), PINSEL_PINMODE_PULLUP, PINSEL_PINMODE_NORMAL}; \
		PINSEL_ConfigPin(&pwm);                                                                                                                                  \
		mcu_set_pwm(diopin, 0);                                                                                                                                  \
	}

// define the mcu internal servo variables
#if SERVOS_MASK > 0

static uint8_t mcu_servos[6];

static FORCEINLINE void mcu_clear_servos()
{
#if SERVO0 >= 0
	mcu_clear_output(SERVO0);
#endif
#if SERVO1 >= 0
	mcu_clear_output(SERVO1);
#endif
#if SERVO2 >= 0
	mcu_clear_output(SERVO2);
#endif
#if SERVO3 >= 0
	mcu_clear_output(SERVO3);
#endif
#if SERVO4 >= 0
	mcu_clear_output(SERVO4);
#endif
#if SERVO5 >= 0
	mcu_clear_output(SERVO5);
#endif
}

void servo_timer_init(void)
{
    TIM_Cmd(SERVO_TIMER_REG, DISABLE);
	TIM_TIMERCFG_Type tmrconfig;
	TIM_ConfigStructInit(TIM_TIMER_MODE, &tmrconfig);
	TIM_Init(SERVO_TIMER_REG, TIM_TIMER_MODE, &tmrconfig);
	NVIC_SetPriority(SERVO_TIMER_IRQ, 10);
	NVIC_ClearPendingIRQ(SERVO_TIMER_IRQ);
	NVIC_EnableIRQ(SERVO_TIMER_IRQ);
}

void servo_start_timeout(uint8_t val)
{
	TIM_Cmd(SERVO_TIMER_REG, DISABLE);
	TIM_MATCHCFG_Type tmrmatch;
	tmrmatch.MatchChannel = SERVO_TIMER;
	tmrmatch.IntOnMatch = ENABLE;
	tmrmatch.StopOnMatch = DISABLE;
	tmrmatch.ResetOnMatch = ENABLE;
	tmrmatch.MatchValue = ((uint32_t)((float)val * 7.875f)) + 500;
	TIM_ClearIntPending(SERVO_TIMER_REG, SERVO_INT_FLAG);
	NVIC_ClearPendingIRQ(SERVO_TIMER_IRQ);
	TIM_ConfigMatch(SERVO_TIMER_REG, &tmrmatch);

	TIM_Cmd(SERVO_TIMER_REG, ENABLE);
}

void MCU_SERVO_ISR(void)
{
	mcu_clear_servos();
	TIM_ClearIntPending(SERVO_TIMER_REG, SERVO_INT_FLAG);
	NVIC_ClearPendingIRQ(SERVO_TIMER_IRQ);
	TIM_Cmd(SERVO_TIMER_REG, DISABLE);
}

#endif

void MCU_RTC_ISR(void)
{
	mcu_disable_global_isr();
#if SERVOS_MASK > 0
	static uint8_t ms_servo_counter = 0;
	uint8_t servo_counter = ms_servo_counter;

	switch (servo_counter)
	{
#if SERVO0 >= 0
	case SERVO0_FRAME:
		servo_start_timeout(mcu_servos[0]);
		mcu_set_output(SERVO0);
		break;
#endif
#if SERVO1 >= 0
	case SERVO1_FRAME:
		mcu_set_output(SERVO1);
		servo_start_timeout(mcu_servos[1]);
		break;
#endif
#if SERVO2 >= 0
	case SERVO2_FRAME:
		mcu_set_output(SERVO2);
		servo_start_timeout(mcu_servos[2]);
		break;
#endif
#if SERVO3 >= 0
	case SERVO3_FRAME:
		mcu_set_output(SERVO3);
		servo_start_timeout(mcu_servos[3]);
		break;
#endif
#if SERVO4 >= 0
	case SERVO4_FRAME:
		mcu_set_output(SERVO4);
		servo_start_timeout(mcu_servos[4]);
		break;
#endif
#if SERVO5 >= 0
	case SERVO5_FRAME:
		mcu_set_output(SERVO5);
		servo_start_timeout(mcu_servos[5]);
		break;
#endif
	}

	servo_counter++;
	ms_servo_counter = (servo_counter != 20) ? servo_counter : 0;

#endif

	uint32_t millis = mcu_runtime_ms;
	millis++;
	mcu_runtime_ms = millis;
	mcu_rtc_cb(millis);
	//TIM_ClearIntPending(RTC_TIMER_REG, RTC_INT_FLAG);
	mcu_enable_global_isr();
}

void MCU_ITP_ISR(void)
{
	mcu_disable_global_isr();

	static bool resetstep = false;
	if (TIM_GetIntStatus(ITP_TIMER_REG, ITP_INT_FLAG))
	{
		if (!resetstep)
			mcu_step_cb();
		else
			mcu_step_reset_cb();
		resetstep = !resetstep;
	}
	TIM_ClearIntPending(ITP_TIMER_REG, ITP_INT_FLAG);
	mcu_enable_global_isr();
}

void mcu_clocks_init(void)
{
	// disable pullups
	LPC_PINCON->PINMODE0 = 0xAAAAAAAA;
	LPC_PINCON->PINMODE1 = 0xAAAAAAAA;
	LPC_PINCON->PINMODE2 = 0xAAAAAAAA;
	LPC_PINCON->PINMODE3 = 0xAAAAAAAA;
	LPC_PINCON->PINMODE4 = 0xAAAAAAAA;
	LPC_PINCON->PINMODE5 = 0xAAAAAAAA;
	LPC_PINCON->PINMODE6 = 0xAAAAAAAA;
	LPC_PINCON->PINMODE7 = 0xAAAAAAAA;
	LPC_PINCON->PINMODE8 = 0xAAAAAAAA;
	LPC_PINCON->PINMODE9 = 0xAAAAAAAA;
}

/**
 * The isr functions
 * The respective IRQHandler will execute these functions
//  **/
#if (INTERFACE == INTERFACE_USART)
void MCU_COM_ISR(void)
{
	mcu_disable_global_isr();
	uint32_t irqstatus = UART_GetIntId(COM_USART);
	irqstatus &= UART_IIR_INTID_MASK;

	// Receive Line Status
	if (irqstatus == UART_IIR_INTID_RLS)
	{
		uint32_t linestatus = UART_GetLineStatus(COM_USART);

		// Receive Line Status
		if (linestatus & (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_RXFE | UART_LSR_BI))
		{
			// There are errors or break interrupt
			// Read LSR will clear the interrupt
			uint8_t dummy = (COM_INREG & UART_RBR_MASKBIT); // Dummy read on RX to clear interrupt, then bail out
			return;
		}
	}

#ifndef ENABLE_SYNC_RX
	if (irqstatus == UART_IIR_INTID_RDA)
	{
		unsigned char c = (unsigned char)(COM_INREG & UART_RBR_MASKBIT);
		mcu_com_rx_cb(c);
	}
#endif

#ifndef ENABLE_SYNC_TX
	if (irqstatus == UART_IIR_INTID_THRE)
	{
		UART_IntConfig(COM_USART, UART_INTCFG_THRE, DISABLE);
		mcu_com_tx_cb();
	}
#endif
	mcu_enable_global_isr();
}
#elif (INTERFACE == INTERFACE_USB)
void USB_IRQHandler(void)
{
	mcu_disable_global_isr();
	tud_int_handler(0);
	mcu_enable_global_isr();
}
#endif

void mcu_usart_init(void)
{
	//     LPC_SC->PCONP &= ~((1 << 3) | (1 << 4) | (1 << 24) | (1 << 25) | (1 << 31));
#if (INTERFACE == INTERFACE_UART)
	PINSEL_CFG_Type tx = {TX_PORT, TX_BIT, TX_ALT_FUNC, PINSEL_PINMODE_PULLUP, PINSEL_PINMODE_NORMAL};
	PINSEL_ConfigPin(&tx);
	PINSEL_CFG_Type rx = {RX_PORT, RX_BIT, RX_ALT_FUNC, PINSEL_PINMODE_PULLUP, PINSEL_PINMODE_NORMAL};
	PINSEL_ConfigPin(&rx);

	CLKPWR_SetPCLKDiv(COM_PCLK, CLKPWR_PCLKSEL_CCLK_DIV_1);

	UART_CFG_Type conf = {BAUDRATE, UART_PARITY_NONE, UART_DATABIT_8, UART_STOPBIT_1};
	UART_Init(COM_USART, &conf);

	// Enable UART Transmit
	UART_TxCmd(COM_USART, ENABLE);

	// Configure Interrupts
	UART_IntConfig(COM_USART, UART_INTCFG_RLS, ENABLE);
#ifndef ENABLE_SYNC_RX
	UART_IntConfig(COM_USART, UART_INTCFG_RBR, ENABLE);
#endif

#if (!defined(ENABLE_SYNC_TX) || !defined(ENABLE_SYNC_RX))
	NVIC_SetPriority(COM_IRQ, 3);
	NVIC_ClearPendingIRQ(COM_IRQ);
	NVIC_EnableIRQ(COM_IRQ);
#endif

#elif (INTERFACE == INTERFACE_USB)
	// // configure USB as Virtual COM port
	LPC_PINCON->PINSEL1 &= ~((3 << 26) | (3 << 28)); /* P0.29 D+, P0.30 D- */
	LPC_PINCON->PINSEL1 |= ((1 << 26) | (1 << 28));	 /* PINSEL1 26.27, 28.29  = 01 */

	// todo: VBUS not used by smoothieboard (though spec requires it for self powered devices), pin used for beeper
	// todo: Goodlink used for servo4?
	// LPC_PINCON->PINSEL3 &= ~((3<< 4)|(3<<28));   /* P1.18 GoodLink, P1.30 VBUS */
	// LPC_PINCON->PINSEL3 |=  ((1<< 4)|(2<<28));   /* PINSEL3 4.5 = 01, 28.29 = 10 */

	LPC_PINCON->PINSEL4 &= ~((3 << 18)); /* P2.9 SoftConnect */
	LPC_PINCON->PINSEL4 |= ((1 << 18));	 /* PINSEL4 18.19 = 01 */

	LPC_SC->PCONP |= (1UL << 31); /* USB PCLK -> enable USB Per.       */

	LPC_USB->USBClkCtrl = 0x1A; /* Dev, PortSel, AHB clock enable */
	while ((LPC_USB->USBClkSt & 0x1A) != 0x1A)
		;

	NVIC_SetPriority(USB_IRQn, 10);
	NVIC_ClearPendingIRQ(USB_IRQn);
	NVIC_EnableIRQ(USB_IRQn);

	tusb_init();
#endif
}

void mcu_rtc_init()
{
	// TIM_Cmd(RTC_TIMER_REG, DISABLE);
	// TIM_TIMERCFG_Type tmrconfig;
	// TIM_ConfigStructInit(TIM_TIMER_MODE, &tmrconfig);
	// TIM_Init(RTC_TIMER_REG, TIM_TIMER_MODE, &tmrconfig);
	// TIM_MATCHCFG_Type tmrmatch;
	// tmrmatch.MatchChannel = RTC_TIMER;
	// tmrmatch.IntOnMatch = ENABLE;
	// tmrmatch.StopOnMatch = DISABLE;
	// tmrmatch.ResetOnMatch = ENABLE;
	// tmrmatch.MatchValue = 1000;
	// TIM_ConfigMatch(RTC_TIMER_REG, &tmrmatch);
	// NVIC_SetPriority(RTC_TIMER_IRQ, 10);
	// NVIC_ClearPendingIRQ(RTC_TIMER_IRQ);
	// NVIC_EnableIRQ(RTC_TIMER_IRQ);
	// TIM_Cmd(RTC_TIMER_REG, ENABLE);

	// Systick is initialized by the framework
}

/*IO functions*/

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
	mcu_clocks_init();

	lpc_global_isr_enabled = false;

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
#if (INTERFACE == INTERFACE_UART)
#if TX >= 0
	mcu_config_output(TX);
#endif
#if RX >= 0
	mcu_config_input(RX);
#ifdef RX_PULLUP
	mcu_config_pullup(RX);
#endif
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
	// SysTick is started by the framework but is not working
	// Using timer
	mcu_rtc_init();
#if SERVOS_MASK > 0
	servo_timer_init();
#endif
	mcu_disable_probe_isr();
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
 * sets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_servo
void mcu_set_servo(uint8_t servo, uint8_t value)
{
	mcu_servos[servo - SERVO0_UCNC_INTERNAL_PIN] = value;
}
#endif

/**
 * gets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_servo
uint8_t mcu_get_servo(uint8_t servo)
{
	return mcu_servos[servo - SERVO0_UCNC_INTERNAL_PIN];
}
#endif

/**
 * checks if the serial hardware of the MCU is ready do send the next char
 * */
#ifndef mcu_tx_ready
bool mcu_tx_ready(void)
{
}
#endif

/**
 * checks if the serial hardware of the MCU has a new char ready to be read
 * */
#ifndef mcu_rx_ready
bool mcu_rx_ready(void)
{
	return true;
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
#if (INTERFACE == INTERFACE_UART)
#ifdef ENABLE_SYNC_TX
	while (!mcu_tx_ready())
		;
#endif
	COM_OUTREG = c;
#ifndef ENABLE_SYNC_TX
	UART_IntConfig(COM_USART, UART_INTCFG_THRE, ENABLE);
#endif
#elif (INTERFACE == INTERFACE_USB)
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
#if (INTERFACE == INTERFACE_UART)
#ifdef ENABLE_SYNC_RX
	while (!mcu_rx_ready())
		;
#endif
	return (COM_INREG & UART_RBR_MASKBIT);
#elif (INTERFACE == INTERFACE_USB)
	while (!tud_cdc_available())
	{
		tud_task();
	}

	return (unsigned char)tud_cdc_read_char();
#endif
}
#endif

// ISR
/**
 * enables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_global_isr
#error "mcu_enable_global_isr undefined"
#endif
/**
 * disables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_global_isr
#error "mcu_disable_global_isr undefined"
#endif
/**
 * gets global interrupts state on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_global_isr
#error "mcu_get_global_isr undefined"
#endif

// Step interpolator
/**
 * convert step rate to clock cycles
 * */
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
{
	// up and down counter (generates half the step rate at each event)
	uint32_t totalticks = (uint32_t)((float)(1000000UL >> 1) / frequency);
	*prescaller = 0;
	*ticks = (uint16_t)totalticks;
}

/**
 * starts the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	TIM_Cmd(ITP_TIMER_REG, DISABLE);
	TIM_TIMERCFG_Type tmrconfig;
	TIM_ConfigStructInit(TIM_TIMER_MODE, &tmrconfig);
	TIM_Init(ITP_TIMER_REG, TIM_TIMER_MODE, &tmrconfig);
	TIM_MATCHCFG_Type tmrmatch;
	tmrmatch.MatchChannel = ITP_TIMER;
	tmrmatch.IntOnMatch = ENABLE;
	tmrmatch.StopOnMatch = DISABLE;
	tmrmatch.ResetOnMatch = ENABLE;
	tmrmatch.MatchValue = ticks;
	TIM_ConfigMatch(ITP_TIMER_REG, &tmrmatch);
	NVIC_SetPriority(ITP_TIMER_IRQ, 1);
	NVIC_ClearPendingIRQ(ITP_TIMER_IRQ);
	NVIC_EnableIRQ(ITP_TIMER_IRQ);

	TIM_Cmd(ITP_TIMER_REG, ENABLE);
}

/**
 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	TIM_UpdateMatchValue(ITP_TIMER_REG, ITP_TIMER, ticks);
}

/**
 * stops the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_stop_itp_isr(void)
{
	TIM_Cmd(ITP_TIMER_REG, DISABLE);
	NVIC_DisableIRQ(ITP_TIMER_IRQ);
}

/**
 * gets the MCU running time in milliseconds.
 * the time counting is controled by the internal RTC
 * */
uint32_t mcu_millis()
{
	uint32_t val = mcu_runtime_ms;
	return val;
}

/**
 * provides a delay in us (micro seconds)
 * */
#define mcu_micros ((mcu_runtime_ms * 1000) + ((SysTick->LOAD - SysTick->VAL) / (SystemCoreClock / 1000000)))
#ifndef mcu_delay_us
void mcu_delay_us(uint16_t delay)
{
	// lpc176x_delay_us(delay);
	uint32_t target = mcu_micros + delay;
	while (target > mcu_micros)
		;
}
#endif

/**
 * runs all internal tasks of the MCU.
 * for the moment these are:
 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
 *   - if ENABLE_SYNC_RX is enabled check if there are any chars in the rx transmitter (or the tinyUSB buffer) and read them to the mcu_com_rx_cb
 *   - if ENABLE_SYNC_TX is enabled check if com_tx_empty is false and run mcu_com_tx_cb
 * */
void mcu_dotasks(void)
{
#if (INTERFACE == INTERFACE_USB)
	tud_cdc_write_flush();
	tud_task(); // tinyusb device task
#endif
#if (defined(ENABLE_SYNC_TX) || defined(ENABLE_SYNC_RX))
	// lpc176x_uart_flush();
#endif
#ifdef ENABLE_SYNC_RX
	while (mcu_rx_ready())
	{
		unsigned char c = mcu_getc();
		mcu_com_rx_cb(c);
	}
#endif
}

// Non volatile memory
/**
 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
uint8_t mcu_eeprom_getc(uint16_t address)
{
	return 0;
}

/**
 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
}

#endif
