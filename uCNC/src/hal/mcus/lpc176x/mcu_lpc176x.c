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
#include "system_LPC17xx.h"

#ifdef MCU_HAS_USB
#ifdef USE_ARDUINO_CDC
extern void mcu_usb_dotasks(void);
extern void mcu_usb_init(void);
extern void mcu_usb_putc(char c);
extern void mcu_usb_flush(void);
extern char mcu_usb_getc(void);
extern uint8_t mcu_usb_available(void);
extern uint8_t mcu_usb_tx_available(void);
#else
#include <tusb_ucnc.h>
#endif
#endif

/**
 * The internal clock counter
 * Increments every millisecond
 * Can count up to almost 50 days
 **/
static volatile uint32_t mcu_runtime_ms;
volatile bool lpc_global_isr_enabled;

// define the mcu internal servo variables
#if SERVOS_MASK > 0
#define SERVO_MIN 64
static uint8_t mcu_servos[6];

static FORCEINLINE void mcu_clear_servos()
{
#if ASSERT_PIN(SERVO0)
	mcu_clear_output(SERVO0);
#endif
#if ASSERT_PIN(SERVO1)
	mcu_clear_output(SERVO1);
#endif
#if ASSERT_PIN(SERVO2)
	mcu_clear_output(SERVO2);
#endif
#if ASSERT_PIN(SERVO3)
	mcu_clear_output(SERVO3);
#endif
#if ASSERT_PIN(SERVO4)
	mcu_clear_output(SERVO4);
#endif
#if ASSERT_PIN(SERVO5)
	mcu_clear_output(SERVO5);
#endif
}

static FORCEINLINE void mcu_set_servos()
{
#if SERVOS_MASK > 0
	static uint8_t ms_servo_counter = 0;
	uint8_t servo_counter = ms_servo_counter;

	switch (servo_counter)
	{
#if ASSERT_PIN(SERVO0)
	case 0:
		mcu_set_output(SERVO0);
		SERVO_TIMER_REG->MR1 = (SERVO_MIN + mcu_servos[0]);

		break;
#endif
#if ASSERT_PIN(SERVO1)
	case 1:
		mcu_set_output(SERVO1);
		SERVO_TIMER_REG->MR1 = (SERVO_MIN + mcu_servos[1]);
		break;
#endif
#if ASSERT_PIN(SERVO2)
	case 2:
		mcu_set_output(SERVO2);
		SERVO_TIMER_REG->MR1 = (SERVO_MIN + mcu_servos[2]);
		break;
#endif
#if ASSERT_PIN(SERVO3)
	case 3:
		mcu_set_output(SERVO3);
		SERVO_TIMER_REG->MR1 = (SERVO_MIN + mcu_servos[3]);
		break;
#endif
#if ASSERT_PIN(SERVO4)
	case 4:
		mcu_set_output(SERVO4);
		SERVO_TIMER_REG->MR1 = (SERVO_MIN + mcu_servos[4]);
		break;
#endif
#if ASSERT_PIN(SERVO5)
	case 5:
		mcu_set_output(SERVO5);
		SERVO_TIMER_REG->MR1 = (SERVO_MIN + mcu_servos[5]);
		break;
#endif
	}

	servo_counter++;
	ms_servo_counter = (servo_counter != 6) ? servo_counter : 0;
#endif
}

void servo_timer_init(void)
{
	LPC_SC->PCONP |= SERVO_PCONP;
	LPC_SC->SERVO_PCLKSEL_REG &= ~SERVO_PCLKSEL_VAL; // system clk/4

	SERVO_TIMER_REG->CTCR = 0;
	SERVO_TIMER_REG->CCR &= ~0x03;
	SERVO_TIMER_REG->TC = 0;
	SERVO_TIMER_REG->PC = 0;
	SERVO_TIMER_REG->PR = 0;
	SERVO_TIMER_REG->TCR |= TIM_RESET;	// Reset Counter
	SERVO_TIMER_REG->TCR &= ~TIM_RESET; // release reset
	SERVO_TIMER_REG->EMR = 0;

	SERVO_TIMER_REG->PR = ((F_CPU >> 2) / 127500) - 1; // for 1us
	SERVO_TIMER_REG->IR = 0xFFFFFFFF;

	SERVO_TIMER_REG->MR1 = SERVO_MIN; // minimum value for servo setup
	SERVO_TIMER_REG->MR0 = 425;		  // reset @ every 3.333ms * 6 servos = 20ms->50Hz
	SERVO_TIMER_REG->MCR = 0x0B;	  // Interrupt on MC0 and MC1 and reset on MC0

	NVIC_SetPriority(SERVO_TIMER_IRQ, 10);
	NVIC_ClearPendingIRQ(SERVO_TIMER_IRQ);
	NVIC_EnableIRQ(SERVO_TIMER_IRQ);

	SERVO_TIMER_REG->TCR |= TIM_ENABLE;
}

void MCU_SERVO_ISR(void)
{
	mcu_disable_global_isr();
	if (CHECKBIT(SERVO_TIMER_REG->IR, TIM_MR1_INT))
	{
		mcu_clear_servos();
		SETBIT(SERVO_TIMER_REG->IR, TIM_MR1_INT);
	}

	if (CHECKBIT(SERVO_TIMER_REG->IR, TIM_MR0_INT))
	{
		mcu_set_servos();
		SETBIT(SERVO_TIMER_REG->IR, TIM_MR0_INT);
	}
	mcu_enable_global_isr();
	// mcu_clear_servos();
	// TIM_ClearIntPending(SERVO_TIMER_REG, SERVO_INT_FLAG);
	// NVIC_ClearPendingIRQ(SERVO_TIMER_IRQ);
	// TIM_Cmd(SERVO_TIMER_REG, DISABLE);
}

#endif

void MCU_RTC_ISR(void)
{
	mcu_disable_global_isr();
	uint32_t millis = mcu_runtime_ms;
	millis++;
	mcu_runtime_ms = millis;
	mcu_rtc_cb(millis);
	mcu_enable_global_isr();
}

void MCU_ITP_ISR(void)
{
	mcu_disable_global_isr();

	if (CHECKBIT(ITP_TIMER_REG->IR, TIM_MR1_INT))
	{
		mcu_step_reset_cb();
		SETBIT(ITP_TIMER_REG->IR, TIM_MR1_INT);
	}

	if (CHECKBIT(ITP_TIMER_REG->IR, TIM_MR0_INT))
	{
		mcu_step_cb();
		SETBIT(ITP_TIMER_REG->IR, TIM_MR0_INT);
	}

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

	if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
	{
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
		DWT->CYCCNT = 0;
		DWT->CTRL |= 0x1UL;
	}
}

/**
 * The isr functions
 * The respective IRQHandler will execute these functions
//  **/
#ifdef MCU_HAS_UART
void MCU_COM_ISR(void)
{
	mcu_disable_global_isr();
	uint32_t irqstatus = UART_GetIntId(COM_UART);
	irqstatus &= UART_IIR_INTID_MASK;

	// Receive Line Status
	if (irqstatus == UART_IIR_INTID_RLS)
	{
		uint32_t linestatus = UART_GetLineStatus(COM_UART);

		// Receive Line Status
		if (linestatus & (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_RXFE | UART_LSR_BI))
		{
			// There are errors or break interrupt
			// Read LSR will clear the interrupt
			/*uint8_t dummy = */ (COM_INREG & UART_RBR_MASKBIT); // Dummy read on RX to clear interrupt, then bail out
			return;
		}
	}

	if (irqstatus == UART_IIR_INTID_RDA)
	{
		unsigned char c = (unsigned char)(COM_INREG & UART_RBR_MASKBIT);
		mcu_com_rx_cb(c);
	}

#ifndef ENABLE_SYNC_TX
	if (irqstatus == UART_IIR_INTID_THRE)
	{
		// UART_IntConfig(COM_USART, UART_INTCFG_THRE, DISABLE);
		COM_UART->IER &= ~UART_IER_THREINT_EN;
		mcu_com_tx_cb();
	}
#endif

	mcu_enable_global_isr();
}
#endif

#ifdef MCU_HAS_UART2
void MCU_COM2_ISR(void)
{
	mcu_disable_global_isr();
	uint32_t irqstatus = UART_GetIntId(COM2_UART);
	irqstatus &= UART_IIR_INTID_MASK;

	// Receive Line Status
	if (irqstatus == UART_IIR_INTID_RLS)
	{
		uint32_t linestatus = UART_GetLineStatus(COM2_UART);

		// Receive Line Status
		if (linestatus & (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_RXFE | UART_LSR_BI))
		{
			// There are errors or break interrupt
			// Read LSR will clear the interrupt
			/*uint8_t dummy = */ (COM2_INREG & UART_RBR_MASKBIT); // Dummy read on RX to clear interrupt, then bail out
			return;
		}
	}

	if (irqstatus == UART_IIR_INTID_RDA)
	{
		unsigned char c = (unsigned char)(COM2_INREG & UART_RBR_MASKBIT);
		mcu_com_rx_cb(c);
	}

#ifndef ENABLE_SYNC_TX
	if (irqstatus == UART_IIR_INTID_THRE)
	{
		// UART_IntConfig(COM_USART, UART_INTCFG_THRE, DISABLE);
		COM2_UART->IER &= ~UART_IER_THREINT_EN;
		mcu_com_tx_cb();
	}
#endif

	mcu_enable_global_isr();
}
#endif

#ifdef MCU_HAS_USB
#ifndef USE_ARDUINO_CDC
void USB_IRQHandler(void)
{
	mcu_disable_global_isr();
	tusb_cdc_isr_handler();
	mcu_enable_global_isr();
}
#endif
#endif

void mcu_usart_init(void)
{
#ifdef MCU_HAS_UART
	PINSEL_CFG_Type tx = {TX_PORT, TX_BIT, UART_ALT_FUNC, PINSEL_PINMODE_PULLUP, PINSEL_PINMODE_NORMAL};
	PINSEL_ConfigPin(&tx);
	PINSEL_CFG_Type rx = {RX_PORT, RX_BIT, UART_ALT_FUNC, PINSEL_PINMODE_PULLUP, PINSEL_PINMODE_NORMAL};
	PINSEL_ConfigPin(&rx);

	CLKPWR_SetPCLKDiv(COM_PCLK, CLKPWR_PCLKSEL_CCLK_DIV_4);

	UART_CFG_Type conf = {BAUDRATE, UART_PARITY_NONE, UART_DATABIT_8, UART_STOPBIT_1};
	UART_Init(COM_UART, &conf);

	// Enable UART Transmit
	UART_TxCmd(COM_UART, ENABLE);

	// Configure Interrupts
	UART_IntConfig(COM_UART, UART_INTCFG_RLS, ENABLE);
	UART_IntConfig(COM_UART, UART_INTCFG_RBR, ENABLE);

	NVIC_SetPriority(COM_IRQ, 3);
	NVIC_ClearPendingIRQ(COM_IRQ);
	NVIC_EnableIRQ(COM_IRQ);
#endif

#ifdef MCU_HAS_UART2
	PINSEL_CFG_Type tx = {TX2_PORT, TX2_BIT, UART2_ALT_FUNC, PINSEL_PINMODE_PULLUP, PINSEL_PINMODE_NORMAL};
	PINSEL_ConfigPin(&tx);
	PINSEL_CFG_Type rx = {RX2_PORT, RX2_BIT, UART2_ALT_FUNC, PINSEL_PINMODE_PULLUP, PINSEL_PINMODE_NORMAL};
	PINSEL_ConfigPin(&rx);

	CLKPWR_SetPCLKDiv(COM2_PCLK, CLKPWR_PCLKSEL_CCLK_DIV_4);

	UART_CFG_Type conf = {BAUDRATE2, UART_PARITY_NONE, UART_DATABIT_8, UART_STOPBIT_1};
	UART_Init(COM2_UART, &conf);

	// Enable UART Transmit
	UART_TxCmd(COM2_UART, ENABLE);

	// Configure Interrupts
	UART_IntConfig(COM2_UART, UART_INTCFG_RLS, ENABLE);
	UART_IntConfig(COM2_UART, UART_INTCFG_RBR, ENABLE);

	NVIC_SetPriority(COM2_IRQ, 3);
	NVIC_ClearPendingIRQ(COM2_IRQ);
	NVIC_EnableIRQ(COM2_IRQ);
#endif

#ifdef MCU_HAS_USB
#ifdef USE_ARDUINO_CDC
	mcu_usb_init();
#else
	// // // configure USB as Virtual COM port
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

	tusb_cdc_init();
#endif

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

	// Systick is initialized by the Arduino framework
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

	mcu_io_init();
	mcu_usart_init();
	// SysTick is started by the framework but is not working
	// Using timer
	mcu_rtc_init();
#if SERVOS_MASK > 0
	servo_timer_init();
#endif
#ifdef MCU_HAS_SPI
	mcu_config_af(SPI_CLK, SPI_ALT_FUNC);
	mcu_config_af(SPI_SDO, SPI_ALT_FUNC);
	mcu_config_af(SPI_SDI, SPI_ALT_FUNC);
	mcu_config_af(SPI_CS, SPI_ALT_FUNC);
	LPC_SC->PCONP |= SPI_PCONP;
	LPC_SC->SPI_PCLKSEL_REG &= ~SPI_PCLKSEL_MASK; // div clock by 4
	uint8_t div = SPI_COUNTER_DIV(SPI_FREQ);
	div += (div & 0x01) ? 1 : 0;
	SPI_REG->CPSR = div;		   // internal divider
	SPI_REG->CR0 |= SPI_MODE << 6; // clock phase
	SPI_REG->CR0 |= 7 << 0;		   // 8 bits
	SPI_REG->CR1 |= 1 << 1;		   // enable SSP*/

#endif
#ifdef MCU_HAS_I2C
	PINSEL_CFG_Type scl = {I2C_CLK_PORT, I2C_CLK_BIT, I2C_ALT_FUNC, PINSEL_PINMODE_TRISTATE, PINSEL_PINMODE_OPENDRAIN};
	PINSEL_ConfigPin(&scl);
	PINSEL_CFG_Type sda = {I2C_DATA_PORT, I2C_DATA_BIT, I2C_ALT_FUNC, PINSEL_PINMODE_TRISTATE, PINSEL_PINMODE_OPENDRAIN};
	PINSEL_ConfigPin(&sda);
	I2C_Init(I2C_REG, I2C_FREQ);
	I2C_REG->I2CONSET |= I2C_I2CONSET_I2EN;
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
	mcu_servos[servo - SERVO_PINS_OFFSET] = value;
}
#endif

/**
 * gets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_servo
uint8_t mcu_get_servo(uint8_t servo)
{
	return mcu_servos[servo - SERVO_PINS_OFFSET];
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
 * sends a char either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_putc
void mcu_putc(char c)
{
#ifdef ENABLE_SYNC_TX
	while (!mcu_tx_ready())
	{
#ifdef MCU_HAS_USB
#ifdef USE_ARDUINO_CDC
		mcu_usb_flush();
#else
		tusb_cdc_flush();
#endif
#endif
	}
#endif

#ifdef MCU_HAS_UART
	COM_OUTREG = c;
#ifndef ENABLE_SYNC_TX
	COM_UART->IER |= UART_IER_THREINT_EN;
#endif
#endif
#ifdef MCU_HAS_UART2
	COM2_OUTREG = c;
#ifndef ENABLE_SYNC_TX
	COM2_UART->IER |= UART_IER_THREINT_EN;
#endif
#endif

#ifdef MCU_HAS_USB
#ifdef USE_ARDUINO_CDC
	if (c != 0)
	{
		mcu_usb_putc(c);
	}
	if (c == '\r' || c == 0)
	{
		mcu_usb_flush();
	}
#else
	if (c != 0)
	{
		tusb_cdc_write(c);
	}
	if (c == '\r' || c == 0)
	{
		tusb_cdc_flush();
	}
#endif
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
	uint32_t totalticks = (uint32_t)((float)1000000UL / frequency);
	// *prescaller = 0;
	// *ticks = (uint16_t)totalticks;
	*prescaller = 0;
	while (totalticks > 0x0000FFFFUL)
	{
		*prescaller++;
		totalticks >>= 1;
	}

	*ticks = (uint16_t)totalticks;
}

float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller)
{
	return (1000000.0f / (float)(((uint32_t)ticks) << prescaller));
}

/**
 * starts the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	uint32_t val = (uint32_t)ticks;
	val <<= prescaller;
	LPC_SC->PCONP |= ITP_PCONP;
	LPC_SC->ITP_PCLKSEL_REG &= ~ITP_PCLKSEL_VAL; // system clk/4

	ITP_TIMER_REG->CTCR = 0;
	ITP_TIMER_REG->CCR &= ~0x03;
	ITP_TIMER_REG->TC = 0;
	ITP_TIMER_REG->PC = 0;
	ITP_TIMER_REG->PR = 0;
	ITP_TIMER_REG->TCR |= TIM_RESET;  // Reset Counter
	ITP_TIMER_REG->TCR &= ~TIM_RESET; // release reset
	ITP_TIMER_REG->EMR = 0;

	ITP_TIMER_REG->PR = ((F_CPU >> 2) / 1000000UL) - 1; // for 1us
	ITP_TIMER_REG->IR = 0xFFFFFFFF;

	ITP_TIMER_REG->MR1 = val >> 1;
	ITP_TIMER_REG->MR0 = val;
	ITP_TIMER_REG->MCR = 0x0B; // Interrupt on MC0 and MC1 and reset on MC0

	NVIC_SetPriority(ITP_TIMER_IRQ, 1);
	NVIC_ClearPendingIRQ(ITP_TIMER_IRQ);
	NVIC_EnableIRQ(ITP_TIMER_IRQ);

	// TIM_Cmd(ITP_TIMER_REG, ENABLE);
	ITP_TIMER_REG->TCR |= TIM_ENABLE;
}

/**
 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	uint32_t val = (uint32_t)ticks;
	val <<= prescaller;
	ITP_TIMER_REG->TCR &= ~TIM_ENABLE;
	ITP_TIMER_REG->MR1 = val >> 1;
	ITP_TIMER_REG->MR0 = val;
	ITP_TIMER_REG->TCR |= TIM_RESET;
	ITP_TIMER_REG->TCR &= ~TIM_RESET;
	ITP_TIMER_REG->TCR |= TIM_ENABLE;
}

/**
 * stops the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_stop_itp_isr(void)
{
	ITP_TIMER_REG->TCR &= ~TIM_ENABLE;
	ITP_TIMER_REG->TCR |= TIM_RESET;
	ITP_TIMER_REG->TCR &= ~TIM_RESET;
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
 * the maximum allowed delay is 255 us
 * */
uint32_t mcu_micros()
{
	return ((mcu_runtime_ms * 1000) + ((SysTick->LOAD - SysTick->VAL) / (F_CPU / 1000000)));
}

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
 * */
void mcu_dotasks()
{
#ifdef MCU_HAS_USB
#ifdef USE_ARDUINO_CDC
	mcu_usb_flush();
	mcu_usb_dotasks();
	while (mcu_usb_available())
	{
		unsigned char c = (unsigned char)mcu_usb_getc();
		mcu_com_rx_cb(c);
	}
#else
	tusb_cdc_flush();
	tusb_cdc_task(); // tinyusb device task

	while (tusb_cdc_available())
	{
		unsigned char c = (unsigned char)tusb_cdc_read();
		mcu_com_rx_cb(c);
	}
#endif
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

#ifdef MCU_HAS_SPI
void mcu_spi_config(uint8_t mode, uint32_t frequency)
{
	uint8_t div = SPI_COUNTER_DIV(frequency);
	div += (div & 0x01) ? 1 : 0;
	mode = CLAMP(0, mode, 3);
	SPI_REG->CR1 &= ~(1 << 1); // disable SSP
	SPI_REG->CPSR = div;	   // internal divider
	SPI_REG->CR0 |= mode << 6; // clock phase
	SPI_REG->CR1 |= 1 << 1;	   // enable SSP
}

#endif

#ifdef MCU_HAS_I2C
#ifndef mcu_i2c_write
uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop)
{
	if (send_start)
	{
		// Enter to Master Transmitter mode
		I2C_REG->I2CONSET = I2C_I2CONSET_STA;
		// Wait for complete
		while (!(I2C_REG->I2CONSET & I2C_I2CONSET_SI))
			;
		I2C_REG->I2CONCLR = I2C_I2CONCLR_STAC;
		if ((I2C_REG->I2STAT & I2C_STAT_CODE_BITMASK) != 0x08)
		{
			I2C_REG->I2CONSET = I2C_I2CONSET_STO;
			I2C_REG->I2CONCLR = I2C_I2CONCLR_SIC;
			// Wait for complete
			while (!(I2C_REG->I2CONSET & I2C_I2CONSET_STO))
				;
			return 0;
		}
	}

	// Reset STA, STO, SI
	I2C_REG->I2CONCLR = I2C_I2CONCLR_SIC | I2C_I2CONCLR_STOC | I2C_I2CONCLR_STAC;

	/* Make sure start bit is not active */
	I2C_REG->I2DAT = data & I2C_I2DAT_BITMASK;
	// Wait for complete
	while (!(I2C_REG->I2CONSET & I2C_I2CONSET_SI))
		;

	switch ((I2C_REG->I2STAT & I2C_STAT_CODE_BITMASK))
	{
	case 0x18:
	case 0x28:
	case 0x40:
	case 0x50:
		break;
	default:
		/* Make sure start bit is not active */
		I2C_REG->I2CONSET = I2C_I2CONSET_STO;
		I2C_REG->I2CONCLR = I2C_I2CONCLR_SIC;
		// Wait for complete
		while (!(I2C_REG->I2CONSET & I2C_I2CONSET_STO))
			;
		return 0;
	}

	if (send_stop)
	{
		/* Make sure start bit is not active */
		I2C_REG->I2CONSET = I2C_I2CONSET_STO;
		I2C_REG->I2CONCLR = I2C_I2CONCLR_SIC;
		// Wait for complete
		while (!(I2C_REG->I2CONSET & I2C_I2CONSET_STO))
			;
	}

	return 1;
}
#endif

#ifndef mcu_i2c_read
uint8_t mcu_i2c_read(bool with_ack, bool send_stop)
{
	uint8_t c = 0;

	if (with_ack)
	{
		I2C_REG->I2CONSET = I2C_I2CONSET_AA;
	}
	else
	{
		I2C_REG->I2CONCLR = I2C_I2CONCLR_AAC;
	}

	I2C_REG->I2CONCLR = I2C_I2CONCLR_SIC;

	// Wait for complete
	while (!(I2C_REG->I2CONSET & I2C_I2CONSET_SI))
		;

	c = (uint8_t)(I2C_REG->I2DAT & I2C_I2DAT_BITMASK);

	if (send_stop)
	{
		/* Make sure start bit is not active */
		I2C_REG->I2CONSET = I2C_I2CONSET_STO;
		I2C_REG->I2CONCLR = I2C_I2CONCLR_SIC;
		// Wait for complete
		while (!(I2C_REG->I2CONSET & I2C_I2CONSET_STO))
			;
	}

	return c;
}
#endif
#endif

#ifdef MCU_HAS_ONESHOT_TIMER

void MCU_ONESHOT_ISR(void)
{
	if (mcu_timeout_cb)
	{
		mcu_timeout_cb();
	}

	NVIC_ClearPendingIRQ(ONESHOT_TIMER_IRQ);
}

/**
 * configures a single shot timeout in us
 * */
#ifndef mcu_config_timeout
void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout)
{
	mcu_timeout_cb = fp;
	LPC_SC->PCONP |= ONESHOT_PCONP;
	LPC_SC->ONESHOT_PCLKSEL_REG &= ~ONESHOT_PCLKSEL_VAL; // system clk/4

	ONESHOT_TIMER_REG->CTCR = 0;
	ONESHOT_TIMER_REG->CCR &= ~0x03;
	ONESHOT_TIMER_REG->TC = 0;
	ONESHOT_TIMER_REG->PC = 0;
	ONESHOT_TIMER_REG->PR = 0;
	ONESHOT_TIMER_REG->TCR |= TIM_RESET;  // Reset Counter
	ONESHOT_TIMER_REG->TCR &= ~TIM_RESET; // release reset
	ONESHOT_TIMER_REG->EMR = 0;

	ONESHOT_TIMER_REG->PR = ((F_CPU >> 2) / 1000000UL) - 1; // for 1us
	ONESHOT_TIMER_REG->IR = 0xFFFFFFFF;

	ONESHOT_TIMER_REG->MR0 = timeout;
	ONESHOT_TIMER_REG->MCR = 0x07; // Interrupt reset and stop on MC0

	NVIC_SetPriority(ONESHOT_TIMER_IRQ, 3);
	NVIC_ClearPendingIRQ(ONESHOT_TIMER_IRQ);
	NVIC_EnableIRQ(ONESHOT_TIMER_IRQ);

	// TIM_Cmd(ONESHOT_TIMER_REG, ENABLE);
	// ONESHOT_TIMER_REG->TCR |= TIM_ENABLE;
}
#endif

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#ifndef mcu_start_timeout
void mcu_start_timeout()
{
}
#endif
#endif

#endif
