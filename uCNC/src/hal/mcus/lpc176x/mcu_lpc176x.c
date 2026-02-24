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
extern void lpc176x_usb_dotasks(void);
extern bool lpc176x_usb_available(void);
extern uint8_t lpc176x_usb_getc(void);
extern void lpc176x_usb_putc(uint8_t c);
extern void lpc176x_usb_init(void);
extern void lpc176x_usb_write(uint8_t *ptr, uint8_t len);
#else
#include <tusb_ucnc.h>
#endif
#endif

/**
 * The internal clock counter
 * Increments every millisecond
 * Can count up to almost 50 days
 **/
// provided by the framework
extern volatile uint64_t _millis;

// define the mcu internal servo variables
#if SERVOS_MASK > 0
#define SERVO_MIN 64
static uint8_t mcu_servos[6];

static FORCEINLINE void mcu_clear_servos()
{
#if ASSERT_PIN(SERVO0)
	io_clear_output(SERVO0);
#endif
#if ASSERT_PIN(SERVO1)
	io_clear_output(SERVO1);
#endif
#if ASSERT_PIN(SERVO2)
	io_clear_output(SERVO2);
#endif
#if ASSERT_PIN(SERVO3)
	io_clear_output(SERVO3);
#endif
#if ASSERT_PIN(SERVO4)
	io_clear_output(SERVO4);
#endif
#if ASSERT_PIN(SERVO5)
	io_clear_output(SERVO5);
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
		io_set_output(SERVO0);
		SERVO_TIMER_REG->MR1 = (SERVO_MIN + mcu_servos[0]);

		break;
#endif
#if ASSERT_PIN(SERVO1)
	case 1:
		io_set_output(SERVO1);
		SERVO_TIMER_REG->MR1 = (SERVO_MIN + mcu_servos[1]);
		break;
#endif
#if ASSERT_PIN(SERVO2)
	case 2:
		io_set_output(SERVO2);
		SERVO_TIMER_REG->MR1 = (SERVO_MIN + mcu_servos[2]);
		break;
#endif
#if ASSERT_PIN(SERVO3)
	case 3:
		io_set_output(SERVO3);
		SERVO_TIMER_REG->MR1 = (SERVO_MIN + mcu_servos[3]);
		break;
#endif
#if ASSERT_PIN(SERVO4)
	case 4:
		io_set_output(SERVO4);
		SERVO_TIMER_REG->MR1 = (SERVO_MIN + mcu_servos[4]);
		break;
#endif
#if ASSERT_PIN(SERVO5)
	case 5:
		io_set_output(SERVO5);
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
	LPC_SC->SERVO_PCLKSEL_REG &= ~SERVO_PCLKSEL_MASK; // system clk/4

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
}

#endif

void MCU_RTC_ISR(void)
{
	_millis++;
	mcu_rtc_cb((uint32_t)_millis);
}

void MCU_ITP_ISR(void)
{
	mcu_disable_global_isr();
	NVIC_ClearPendingIRQ(ITP_TIMER_IRQ);

	static bool resetstep = false;
	if (CHECKBIT(ITP_TIMER_REG->IR, TIM_MR0_INT))
	{
		SETBIT(ITP_TIMER_REG->IR, TIM_MR0_INT);
		if (!resetstep)
		{
					mcu_step_cb();
		}
		else
		{
			mcu_step_reset_cb();
		}
		resetstep = !resetstep;
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
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);

void MCU_COM_ISR(void)
{
	ATOMIC_CODEBLOCK_NR
	{
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
			uint8_t c = (uint8_t)(COM_INREG & UART_RBR_MASKBIT);
#if !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
			if (mcu_com_rx_cb(c))
			{
				if (!BUFFER_TRY_ENQUEUE(uart_rx, &c))
				{
					STREAM_OVF(c);
				}
			}

#else
			mcu_uart_rx_cb(c);
#endif
		}

		if (irqstatus == UART_IIR_INTID_THRE)
		{
			// UART_IntConfig(COM_USART, UART_INTCFG_THRE, DISABLE);

			mcu_enable_global_isr();
			uint8_t c = 0;

			if (!BUFFER_TRY_DEQUEUE(uart_tx, &c))
			{
				COM_UART->IER &= ~UART_IER_THREINT_EN;
				return;
			}

			COM_OUTREG = c;
		}
	}
}
#endif

#if (defined(MCU_HAS_UART2))
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart2_rx, RX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
void MCU_COM2_ISR(void)
{
	ATOMIC_CODEBLOCK_NR
	{
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
			uint8_t c = (uint8_t)(COM2_INREG & UART_RBR_MASKBIT);
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
			if (mcu_com_rx_cb(c))
			{
				if (!BUFFER_TRY_ENQUEUE(uart2_rx, &c))
				{
					STREAM_OVF(c);
				}
			}
#else
			mcu_uart2_rx_cb(c);
#ifndef UART2_DISABLE_BUFFER
			if (BUFFER_FULL(uart2_rx))
			{
				STREAM_OVF(c);
			}

			BUFFER_ENQUEUEE(uart2_rx, &c);
#endif
#endif
		}

		if (irqstatus == UART_IIR_INTID_THRE)
		{
			mcu_enable_global_isr();
			uint8_t c = 0;

			if (!BUFFER_TRY_DEQUEUE(uart2_tx, &c))
			{
				COM2_UART->IER &= ~UART_IER_THREINT_EN;
				return;
			}

			COM2_OUTREG = c;
		}
	}
}
#endif

void mcu_uart_init(void)
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
}

void mcu_uart2_init(void)
{
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
}

void mcu_usb_init(void)
{
#ifdef MCU_HAS_USB
#ifdef USE_ARDUINO_CDC
	lpc176x_usb_init();
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
void mcu_spi_init()
{
#ifdef MCU_HAS_SPI
	// powerup DMA
	// LPC_SC->PCONP |= CLKPWR_PCONP_PCGPDMA;
	// SPI_DMA_TX_CHANNEL->DMACCConfig = 0;
	// SPI_DMA_RX_CHANNEL->DMACCConfig = 0;

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
}

void mcu_spi2_init()
{
#ifdef MCU_HAS_SPI2
	// powerup DMA
	// LPC_SC->PCONP |= CLKPWR_PCONP_PCGPDMA;
	// SPI2_DMA_TX_CHANNEL->DMACCConfig = 0;
	// SPI2_DMA_RX_CHANNEL->DMACCConfig = 0;

	mcu_config_af(SPI2_CLK, SPI2_ALT_FUNC);
	mcu_config_af(SPI2_SDO, SPI2_ALT_FUNC);
	mcu_config_af(SPI2_SDI, SPI2_ALT_FUNC);
	mcu_config_af(SPI2_CS, SPI2_ALT_FUNC);
	LPC_SC->PCONP |= SPI2_PCONP;
	LPC_SC->SPI2_PCLKSEL_REG &= ~SPI2_PCLKSEL_MASK; // div clock by 4
	uint8_t div2 = SPI2_COUNTER_DIV(SPI2_FREQ);
	div2 += (div2 & 0x01) ? 1 : 0;
	SPI2_REG->CPSR = div2;			 // internal divider
	SPI2_REG->CR0 |= SPI2_MODE << 6; // clock phase
	SPI2_REG->CR0 |= 7 << 0;		 // 8 bits
	SPI2_REG->CR1 |= 1 << 1;		 // enable SSP*/

#endif
}

void mcu_i2c_init()
{
#ifdef MCU_HAS_I2C
	mcu_i2c_config(I2C_FREQ);
#endif
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
	mcu_clocks_init();
	mcu_io_init();
	mcu_usart_init();
	// SysTick is started by the framework but is not working
	// Using timer
	mcu_rtc_init();
#if SERVOS_MASK > 0
	servo_timer_init();
#endif
	GPDMA_Init();

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
uint16_t mcu_get_analog(uint8_t channel)
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
 * sends a uint8_t either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifdef MCU_HAS_UART

uint8_t mcu_uart_getc(void)
{
	uint8_t c = 0;
	BUFFER_TRY_DEQUEUE(uart_rx, &c);
	return c;
}

uint8_t mcu_uart_available(void)
{
	return BUFFER_READ_AVAILABLE(uart_rx);
}

void mcu_uart_clear(void)
{
	BUFFER_CLEAR(uart_rx);
}

void mcu_uart_putc(uint8_t c)
{
	while (!BUFFER_TRY_ENQUEUE(uart_tx, &c))
	{
		mcu_uart_flush();
	}
}

void mcu_uart_flush(void)
{
	if (!(COM_UART->IER & UART_IER_THREINT_EN)) // not ready start flushing
	{
		uint8_t c = 0;

		if (!BUFFER_TRY_DEQUEUE(uart_tx, &c))
		{
			return;
		}

		while (!CHECKBIT(COM_UART->LSR, 5))
			;
		COM_OUTREG = c;
		COM_UART->IER |= UART_IER_THREINT_EN;
#if ASSERT_PIN(ACTIVITY_LED)
		io_toggle_output(ACTIVITY_LED);
#endif
	}
}
#endif

#ifdef MCU_HAS_UART2
uint8_t mcu_uart2_getc(void)
{
	uint8_t c = 0;
	BUFFER_TRY_DEQUEUE(uart2_rx, &c);
	return c;
}

uint8_t mcu_uart2_available(void)
{
	return BUFFER_READ_AVAILABLE(uart2_rx);
}

void mcu_uart2_clear(void)
{
	BUFFER_CLEAR(uart2_rx);
}

void mcu_uart2_putc(uint8_t c)
{
	while (!BUFFER_TRY_ENQUEUE(uart2_tx, &c))
	{
		mcu_uart2_flush();
	}
}

void mcu_uart2_flush(void)
{
	if (!(COM2_UART->IER & UART_IER_THREINT_EN)) // not ready start flushing
	{
		uint8_t c = 0;

		if (!BUFFER_TRY_DEQUEUE(uart2_tx, &c))
		{
			return;
		}

		while (!CHECKBIT(COM2_UART->LSR, 5))
			;
		COM2_OUTREG = c;
		COM2_UART->IER |= UART_IER_THREINT_EN;
#if ASSERT_PIN(ACTIVITY_LED)
		io_toggle_output(ACTIVITY_LED);
#endif
	}
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
	frequency = CLAMP((float)F_STEP_MIN, frequency, (float)F_STEP_MAX);
	// up and down counter (generates half the step rate at each event)
	uint32_t totalticks = (uint32_t)((float)(F_CPU >> 3) / frequency);
	// *prescaller = 0;
	// *ticks = (uint16_t)totalticks;
	*prescaller = 0;
	while (totalticks > 0x0000FFFFUL)
	{
		(*prescaller) += 1;
		totalticks >>= 1;
	}

	*ticks = (uint16_t)totalticks;
}

float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller)
{
	return ((F_CPU >> 3) / (float)(((uint32_t)ticks) << prescaller));
}

/**
 * starts the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	uint32_t val = (uint32_t)ticks;
	val <<= prescaller;

	LPC_SC->PCONP |= ITP_PCONP;
	LPC_SC->ITP_PCLKSEL_REG &= ~ITP_PCLKSEL_MASK; // system clk/4

	ITP_TIMER_REG->CTCR = 0;
	ITP_TIMER_REG->CCR &= ~0x03;
	ITP_TIMER_REG->TC = 0;
	ITP_TIMER_REG->PC = 0;
	ITP_TIMER_REG->PR = 0;
	ITP_TIMER_REG->TCR |= TIM_RESET;  // Reset Counter
	ITP_TIMER_REG->TCR &= ~TIM_RESET; // release reset
	ITP_TIMER_REG->EMR = 0;

	ITP_TIMER_REG->PR = 0; // for higher resolution use PR = 0 that means that the timer will tick at (F_CPU/4)
	ITP_TIMER_REG->IR = 0xFFFFFFFF;

	ITP_TIMER_REG->MR0 = val;
	ITP_TIMER_REG->MCR = 0x03; // Interrupt on MC0 and MC1 and reset on MC0

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
	return (uint32_t)_millis;
}

/**
 * provides a delay in us (micro seconds)
 * the maximum allowed delay is 255 us
 * */
uint32_t mcu_micros()
{
	return ((mcu_millis() * 1000) + mcu_free_micros());
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

#ifdef MCU_HAS_USB
DECL_BUFFER(uint8_t, usb_rx, RX_BUFFER_SIZE);

#ifndef USE_ARDUINO_CDC
void USB_IRQHandler(void)
{
	mcu_disable_global_isr();
	tusb_cdc_isr_handler();
	mcu_enable_global_isr();
}

void mcu_usb_putc(uint8_t c)
{
	if (!tusb_cdc_write_available())
	{
		mcu_usb_flush();
	}
	tusb_cdc_write(c);
}

void mcu_usb_flush(void)
{
	tusb_cdc_flush();
	while (!tusb_cdc_write_available())
	{
		mcu_dotasks(); // tinyusb device task
		if (!tusb_cdc_connected)
		{
			return;
		}
	}
}
#else
#ifndef USB_TX_BUFFER_SIZE
#define USB_TX_BUFFER_SIZE 64
#endif

DECL_BUFFER(uint8_t, usb_tx, USB_TX_BUFFER_SIZE);
void mcu_usb_flush(void)
{
	while (!BUFFER_EMPTY(usb_tx))
	{
		// use this of char is not 8bits
		// uint8_t c = 0;
		// BUFFER_TRY_DEQUEUE(usb_tx, &c);
		// lpc176x_usb_putc(c);

		// bulk sending
		uint8_t tmp[USB_TX_BUFFER_SIZE + 1];
		memset(tmp, 0, sizeof(tmp));
		uint8_t r;

		BUFFER_READ(usb_tx, tmp, USB_TX_BUFFER_SIZE, r);
		lpc176x_usb_write(tmp, r);
	}
}

void mcu_usb_putc(uint8_t c)
{
	while (!BUFFER_TRY_ENQUEUE(usb_tx, &c))
	{
		mcu_usb_flush();
	}
}
#endif

uint8_t mcu_usb_getc(void)
{
	uint8_t c = 0;
	BUFFER_TRY_DEQUEUE(usb_rx, &c);
	return (uint8_t)c;
}

uint8_t mcu_usb_available(void)
{
	return BUFFER_READ_AVAILABLE(usb_rx);
}

void mcu_usb_clear(void)
{
	BUFFER_CLEAR(usb_rx);
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
	lpc176x_usb_dotasks();
#ifndef DETACH_USB_FROM_MAIN_PROTOCOL
	while (lpc176x_usb_available())
	{
		uint8_t c = lpc176x_usb_getc();
		if (mcu_com_rx_cb(c))
		{
			if (!BUFFER_TRY_ENQUEUE(usb_rx, &c))
			{
				STREAM_OVF(c);
			}
		}
	}
#else
	mcu_usb_rx_cb(c);
#endif
#else
	tusb_cdc_task(); // tinyusb device task

	while (tusb_cdc_available())
	{
		uint8_t c = (uint8_t)tusb_cdc_read();
#ifndef DETACH_USB_FROM_MAIN_PROTOCOL
		if (mcu_com_rx_cb(c))
		{
			if (!BUFFER_TRY_ENQUEUE(usb_rx, &c))
			{
				STREAM_OVF(c);
			}
		}
#else
		mcu_usb_rx_cb(c);
#endif
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
	DBGMSG("EEPROM invalid address @ %u", address);
	return 0;
}

/**
 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	DBGMSG("EEPROM invalid address @ %u", address);
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
}

#ifdef MCU_HAS_SPI
static bool spi_dma_enabled = false;
#define SPI_TX_DONE 1
#define SPI_RX_DONE 2
#define SPI_DONE (SPI_TX_DONE | SPI_RX_DONE)
#define SPI_ERROR 7
static volatile uint8_t spi_transfer_done = 0;
void mcu_spi_config(spi_config_t config, uint32_t frequency)
{
	uint8_t div = SPI_COUNTER_DIV(frequency);
	div += (div & 0x01) ? 1 : 0;
	SPI_REG->CR1 &= ~(1 << 1);		  // disable SSP
	SPI_REG->CPSR = div;			  // internal divider
	SPI_REG->CR0 |= config.mode << 6; // clock phase
	SPI_REG->CR1 |= 1 << 1;			  // enable SSP
	// SSP_DeInit(SPI_REG);

	// SSP_CFG_Type ssp_cfg = {
	// 		.ClockRate = frequency,
	// 		.Databit = SSP_DATABIT_8,
	// 		.FrameFormat = SSP_FRAME_SPI,
	// 		.Mode = SSP_MASTER_MODE,
	// 		.CPHA = (config.mode & 0x01),
	// 		.CPOL = ((config.mode >> 1) & 0x01),
	// };

	// SSP_Init(SPI_REG, &ssp_cfg);

	spi_dma_enabled = config.enable_dma;
}

uint8_t mcu_spi_xmit(uint8_t c)
{
	SPI_REG->DR = c;
	while (!(SPI_REG->SR & SSP_SR_RNE))
		;
	return SPI_REG->DR;
}

#ifndef BULK_SPI_TIMEOUT
#define BULK_SPI_TIMEOUT (1000 / INTERPOLATOR_FREQ)
#endif

// based on Marlin code
static void mcu_prep_spidma_transfer(void *buf, uint16_t length, bool is_rx)
{
	// TODO: LPC dma can only write 0xFFF bytes at once.
	GPDMA_Channel_CFG_Type GPDMACfg;

	/* Configure GPDMA channel 0 -------------------------------------------------------------*/
	/* DMA Channel 0 */
	GPDMACfg.ChannelNum = ((is_rx) ? SPI_DMA_RX_CH : SPI_DMA_TX_CH);

	if (!is_rx)
	{
		// Source memory
		GPDMACfg.SrcMemAddr = (uint32_t)buf;
		// Destination memory - Not used
		GPDMACfg.DstMemAddr = 0;
		// Source connection - unused
		GPDMACfg.SrcConn = 0;
		// Destination connection
		GPDMACfg.DstConn = SPI_DMA_TX_DEST;
	}
	else
	{
		// Source memory
		GPDMACfg.SrcMemAddr = 0;
		// Destination memory - Not used
		GPDMACfg.DstMemAddr = (uint32_t)buf;
		// Source connection - unused
		GPDMACfg.SrcConn = SPI_DMA_RX_DEST;
		// Destination connection
		GPDMACfg.DstConn = 0;
	}
	// Transfer size
	GPDMACfg.TransferSize = length;
	// Transfer width
	GPDMACfg.TransferWidth = GPDMA_WIDTH_BYTE;
	// Transfer type
	GPDMACfg.TransferType = (is_rx) ? GPDMA_TRANSFERTYPE_P2M : GPDMA_TRANSFERTYPE_M2P;

	GPDMACfg.DMALLI = 0;

	// Enable dma on SPI
	SSP_DMACmd(SPI_REG, ((is_rx) ? SSP_DMA_RX : SSP_DMA_TX), ENABLE);

	GPDMACfg.MemoryIncrease = (is_rx) ? GPDMA_DMACCxControl_DI : GPDMA_DMACCxControl_SI;

	// Setup channel with given parameter
	GPDMA_Setup(&GPDMACfg);

	// Enable DMA
	GPDMA_ChannelCmd(((is_rx) ? SPI_DMA_RX_CH : SPI_DMA_TX_CH), ENABLE);
}

static bool mcu_assert_spidma_transfer(bool is_rx)
{
	// Wait for data transfer
	if (!GPDMA_IntGetStatus(GPDMA_STAT_INTTC, ((is_rx) ? SPI_DMA_RX_CH : SPI_DMA_TX_CH)) && !GPDMA_IntGetStatus(GPDMA_STAT_INTERR, ((is_rx) ? SPI_DMA_RX_CH : SPI_DMA_TX_CH)))
	{
		return true;
	}

	// Clear err and int
	GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, ((is_rx) ? SPI_DMA_RX_CH : SPI_DMA_TX_CH));
	GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, ((is_rx) ? SPI_DMA_RX_CH : SPI_DMA_TX_CH));

	// Disable DMA
	GPDMA_ChannelCmd(((is_rx) ? SPI_DMA_RX_CH : SPI_DMA_TX_CH), DISABLE);

	if (is_rx)
	{
		while (!SSP_GetStatus(SPI_REG, SSP_STAT_TXFIFO_EMPTY))
			;
	}
	else
	{
		while (SSP_GetStatus(SPI_REG, SSP_STAT_RXFIFO_NOTEMPTY))
			;
	}

	while (SSP_GetStatus(SPI_REG, SSP_STAT_BUSY) == SET)
		;

	SSP_DMACmd(SPI_REG, ((is_rx) ? SSP_DMA_RX : SSP_DMA_TX), DISABLE);
	return false;
}

bool mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	static bool is_running = false;

	if (!is_running)
	{
		is_running = true;
		if (spi_dma_enabled && !in) // DMA only seems to work correctly in send only transmissions
		{
			// if (in)
			// {
			// 	mcu_prep_dma_transfer(in, len, true);
			// }
			mcu_prep_spidma_transfer((void *)out, len, false);
			return true;
		}
		else
		{
			uint32_t timeout = BULK_SPI_TIMEOUT + mcu_millis();
			while (len--)
			{
				uint8_t c = mcu_spi_xmit(*out++);
				if (in)
				{
					*in++ = c;
				}

				if (timeout < mcu_millis())
				{
					timeout = BULK_SPI_TIMEOUT + mcu_millis();
					TASK_YIELD();
				}
			}
			is_running = false;
			return false;
		}
	}
	else
	{
		if (spi_dma_enabled && !in)
		{
			if (!mcu_assert_spidma_transfer(false))
			{
				// if (in)
				// {
				// 	while (mcu_assert_dma_transfer(true))
				// 		;
				// }
				return false;
			}
		}
	}

	return true;
}

#endif

#ifdef MCU_HAS_SPI2
static bool spi2_dma_enabled = false;
#define SPI2_TX_DONE 1
#define SPI2_RX_DONE 2
#define SPI2_DONE (SPI2_TX_DONE | SPI2_RX_DONE)
#define SPI2_ERROR 7
static volatile uint8_t spi2_transfer_done = 0;
void mcu_spi2_config(spi_config_t config, uint32_t frequency)
{
	uint8_t div = SPI2_COUNTER_DIV(frequency);
	div += (div & 0x01) ? 1 : 0;
	SPI2_REG->CR1 &= ~(1 << 1);		   // disable SSP
	SPI2_REG->CPSR = div;			   // internal divider
	SPI2_REG->CR0 |= config.mode << 6; // clock phase
	SPI2_REG->CR1 |= 1 << 1;		   // enable SSP
	// SSP_DeInit(SPI2_REG);

	// SSP_CFG_Type ssp_cfg = {
	// 		.ClockRate = frequency,
	// 		.Databit = SSP_DATABIT_8,
	// 		.FrameFormat = SSP_FRAME_SPI2,
	// 		.Mode = SSP_MASTER_MODE,
	// 		.CPHA = (config.mode & 0x01),
	// 		.CPOL = ((config.mode >> 1) & 0x01),
	// };

	// SSP_Init(SPI2_REG, &ssp_cfg);

	spi2_dma_enabled = config.enable_dma;
}

uint8_t mcu_spi2_xmit(uint8_t c)
{
	SPI2_REG->DR = c;
	while (!(SPI2_REG->SR & SSP_SR_RNE))
		;
	return SPI2_REG->DR;
}

#ifndef BULK_SPI2_TIMEOUT
#define BULK_SPI2_TIMEOUT (1000 / INTERPOLATOR_FREQ)
#endif

// based on Marlin code
static void mcu_prep_spi2dma_transfer(void *buf, uint16_t length, bool is_rx)
{
	// TODO: LPC dma can only write 0xFFF bytes at once.
	GPDMA_Channel_CFG_Type GPDMACfg;

	/* Configure GPDMA channel 0 -------------------------------------------------------------*/
	/* DMA Channel 0 */
	GPDMACfg.ChannelNum = ((is_rx) ? SPI2_DMA_RX_CH : SPI2_DMA_TX_CH);

	if (!is_rx)
	{
		// Source memory
		GPDMACfg.SrcMemAddr = (uint32_t)buf;
		// Destination memory - Not used
		GPDMACfg.DstMemAddr = 0;
		// Source connection - unused
		GPDMACfg.SrcConn = 0;
		// Destination connection
		GPDMACfg.DstConn = SPI2_DMA_TX_DEST;
	}
	else
	{
		// Source memory
		GPDMACfg.SrcMemAddr = 0;
		// Destination memory - Not used
		GPDMACfg.DstMemAddr = (uint32_t)buf;
		// Source connection - unused
		GPDMACfg.SrcConn = SPI2_DMA_RX_DEST;
		// Destination connection
		GPDMACfg.DstConn = 0;
	}
	// Transfer size
	GPDMACfg.TransferSize = length;
	// Transfer width
	GPDMACfg.TransferWidth = GPDMA_WIDTH_BYTE;
	// Transfer type
	GPDMACfg.TransferType = (is_rx) ? GPDMA_TRANSFERTYPE_P2M : GPDMA_TRANSFERTYPE_M2P;

	GPDMACfg.DMALLI = 0;

	// Enable dma on SPI2
	SSP_DMACmd(SPI2_REG, ((is_rx) ? SSP_DMA_RX : SSP_DMA_TX), ENABLE);

	GPDMACfg.MemoryIncrease = (is_rx) ? GPDMA_DMACCxControl_DI : GPDMA_DMACCxControl_SI;

	// Setup channel with given parameter
	GPDMA_Setup(&GPDMACfg);

	// Enable DMA
	GPDMA_ChannelCmd(((is_rx) ? SPI2_DMA_RX_CH : SPI2_DMA_TX_CH), ENABLE);
}

static bool mcu_assert_spi2dma_transfer(bool is_rx)
{
	// Wait for data transfer
	if (!GPDMA_IntGetStatus(GPDMA_STAT_INTTC, ((is_rx) ? SPI2_DMA_RX_CH : SPI2_DMA_TX_CH)) && !GPDMA_IntGetStatus(GPDMA_STAT_INTERR, ((is_rx) ? SPI2_DMA_RX_CH : SPI2_DMA_TX_CH)))
	{
		return true;
	}

	// Clear err and int
	GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, ((is_rx) ? SPI2_DMA_RX_CH : SPI2_DMA_TX_CH));
	GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, ((is_rx) ? SPI2_DMA_RX_CH : SPI2_DMA_TX_CH));

	// Disable DMA
	GPDMA_ChannelCmd(((is_rx) ? SPI2_DMA_RX_CH : SPI2_DMA_TX_CH), DISABLE);

	if (is_rx)
	{
		while (!SSP_GetStatus(SPI2_REG, SSP_STAT_TXFIFO_EMPTY))
			;
	}
	else
	{
		while (SSP_GetStatus(SPI2_REG, SSP_STAT_RXFIFO_NOTEMPTY))
			;
	}

	while (SSP_GetStatus(SPI2_REG, SSP_STAT_BUSY) == SET)
		;

	SSP_DMACmd(SPI2_REG, ((is_rx) ? SSP_DMA_RX : SSP_DMA_TX), DISABLE);
	return false;
}

bool mcu_spi2_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	static bool is_running = false;

	if (!is_running)
	{
		is_running = true;
		if (spi2_dma_enabled && !in) // DMA only seems to work correctly in send only transmissions
		{
			// if (in)
			// {
			// 	mcu_prep_dma_transfer(in, len, true);
			// }
			mcu_prep_spi2dma_transfer((void *)out, len, false);
			return true;
		}
		else
		{
			uint32_t timeout = BULK_SPI2_TIMEOUT + mcu_millis();
			while (len--)
			{
				uint8_t c = mcu_spi2_xmit(*out++);
				if (in)
				{
					*in++ = c;
				}

				if (timeout < mcu_millis())
				{
					timeout = BULK_SPI2_TIMEOUT + mcu_millis();
					TASK_YIELD();
				}
			}
			is_running = false;
			return false;
		}
	}
	else
	{
		if (spi2_dma_enabled && !in)
		{
			if (!mcu_assert_spi2dma_transfer(false))
			{
				// if (in)
				// {
				// 	while (mcu_assert_dma_transfer(true))
				// 		;
				// }
				return false;
			}
		}
	}

	return true;
}

#endif

#if defined(MCU_HAS_I2C) && !defined(USE_ARDUINO_WIRE)
#if I2C_ADDRESS == 0
static void mcu_i2c_write_stop(bool *stop)
{
	if (*stop)
	{
		uint32_t ms_timeout = 25;

		I2C_REG->I2CONCLR = I2C_I2CONCLR_SIC;
		I2C_REG->I2CONSET = I2C_I2CONSET_STO;
		// Wait for complete
		__TIMEOUT_MS__(ms_timeout)
		{
			if (I2C_REG->I2CONSET & I2C_I2CONSET_STO)
			{
				return;
			}
		}
	}
}

static uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop, uint32_t ms_timeout)
{
	bool stop __attribute__((__cleanup__(mcu_i2c_write_stop))) = send_stop;
	int32_t timeout = ms_timeout;

	if (send_start)
	{
		// manual 19.9.7.3
		if (!I2C_REG->I2STAT || (I2C_REG->I2CONSET & I2C_I2CONSET_STA) || (I2C_REG->I2CONSET & I2C_I2CONSET_STO))
		{
			I2C_REG->I2CONCLR = I2C_I2CONCLR_SIC;
			I2C_REG->I2CONSET = I2C_I2CONSET_STO;
			// Wait for complete
			__TIMEOUT_MS__(timeout)
			{
				if (I2C_REG->I2CONSET & I2C_I2CONSET_STO)
				{
					break;
				}
			}
		}
		// Enter to Master Transmitter mode
		I2C_REG->I2CONSET = I2C_I2CONSET_STA;
		// Wait for complete
		timeout = ms_timeout;
		__TIMEOUT_MS__(timeout)
		{
			if (I2C_REG->I2CONSET & I2C_I2CONSET_SI)
			{
				break;
			}
		}
		__TIMEOUT_ASSERT__(timeout)
		{
			stop = true;
			return I2C_NOTOK;
		}
		I2C_REG->I2CONCLR = I2C_I2CONCLR_STAC;
		if ((I2C_REG->I2STAT & I2C_STAT_CODE_BITMASK) != 0x08)
		{
			I2C_REG->I2CONSET = I2C_I2CONSET_STO;
			I2C_REG->I2CONCLR = I2C_I2CONCLR_SIC;
			// Wait for complete
			timeout = ms_timeout;
			__TIMEOUT_MS__(timeout)
			{
				if (I2C_REG->I2CONSET & I2C_I2CONSET_STO)
				{
					break;
				}
			}
			__TIMEOUT_ASSERT__(timeout)
			{
				stop = true;
				return I2C_NOTOK;
			}
		}
	}

	// Reset STA, STO, SI
	I2C_REG->I2CONCLR = I2C_I2CONCLR_SIC | I2C_I2CONCLR_STOC | I2C_I2CONCLR_STAC;

	/* Make sure start bit is not active */
	I2C_REG->I2DAT = data & I2C_I2DAT_BITMASK;
	// Wait for complete
	timeout = ms_timeout;
	__TIMEOUT_MS__(timeout)
	{
		if (I2C_REG->I2CONSET & I2C_I2CONSET_SI)
		{
			break;
		}
	}

	__TIMEOUT_ASSERT__(timeout)
	{
		stop = true;
		return I2C_NOTOK;
	}

	switch ((I2C_REG->I2STAT & I2C_STAT_CODE_BITMASK))
	{
	case 0x18:
	case 0x28:
	case 0x40:
	case 0x50:
		break;
	default:
		stop = true;
		return I2C_NOTOK;
	}

	return I2C_OK;
}

static uint8_t mcu_i2c_read(uint8_t *data, bool with_ack, bool send_stop, uint32_t ms_timeout)
{
	*data = 0xFF;
	bool stop __attribute__((__cleanup__(mcu_i2c_write_stop))) = send_stop;

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
	__TIMEOUT_MS__(ms_timeout)
	{
		if (I2C_REG->I2CONSET & I2C_I2CONSET_SI)
		{
			*data = (uint8_t)(I2C_REG->I2DAT & I2C_I2DAT_BITMASK);
			return I2C_OK;
		}
	}

	stop = true;
	return I2C_NOTOK;
}

#ifndef mcu_i2c_send
// master sends command to slave
uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen, bool release, uint32_t ms_timeout)
{
	if (data && datalen)
	{
		if (mcu_i2c_write(address << 1, true, false, ms_timeout) == I2C_OK) // start, send address, write
		{
			// send data, stop
			do
			{
				datalen--;
				bool last = (datalen == 0);
				if (mcu_i2c_write(*data, false, (release & last), ms_timeout) != I2C_OK)
				{
					return I2C_NOTOK;
				}
				data++;

			} while (datalen);

			return I2C_OK;
		}
	}

	return I2C_NOTOK;
}
#endif

#ifndef mcu_i2c_receive
// master receive response from slave
uint8_t mcu_i2c_receive(uint8_t address, uint8_t *data, uint8_t datalen, uint32_t ms_timeout)
{
	if (data && datalen)
	{
		if (mcu_i2c_write((address << 1) | 0x01, true, false, ms_timeout) == I2C_OK) // start, send address, write
		{
			do
			{
				datalen--;
				bool last = (datalen == 0);
				if (mcu_i2c_read(data, !last, last, ms_timeout) != I2C_OK)
				{
					return I2C_NOTOK;
				}
				data++;
			} while (datalen);
			return I2C_OK;
		}
	}

	return I2C_NOTOK;
}
#endif
#endif

#ifndef mcu_i2c_config
void mcu_i2c_config(uint32_t frequency)
{
	I2C_DeInit(I2C_REG);
	PINSEL_CFG_Type scl = {I2C_CLK_PORT, I2C_CLK_BIT, I2C_ALT_FUNC, PINSEL_PINMODE_TRISTATE, PINSEL_PINMODE_OPENDRAIN};
	PINSEL_ConfigPin(&scl);
	PINSEL_CFG_Type sda = {I2C_DATA_PORT, I2C_DATA_BIT, I2C_ALT_FUNC, PINSEL_PINMODE_TRISTATE, PINSEL_PINMODE_OPENDRAIN};
	PINSEL_ConfigPin(&sda);
	I2C_Init(I2C_REG, frequency);
#if I2C_ADDRESS != 0
	I2C_OWNSLAVEADDR_CFG_Type i2c_slave = {0};
	i2c_slave.SlaveAddr_7bit = I2C_ADDRESS;
	i2c_slave.GeneralCallState = ENABLE;
	i2c_slave.SlaveAddrChannel = I2C_PORT;
	I2C_SetOwnSlaveAddr(I2C_REG, &i2c_slave);
	// slave mode
	I2C_Cmd(I2C_REG, I2C_SLAVE_MODE, ENABLE);
	I2C_IntCmd(I2C_REG, ENABLE);
#else
	I2C_Cmd(I2C_REG, I2C_MASTER_MODE, ENABLE);
#endif
}
#endif

#if I2C_ADDRESS != 0
uint8_t mcu_i2c_buffer[I2C_SLAVE_BUFFER_SIZE];

void I2C_ISR(void)
{
	static uint8_t index = 0;
	static uint8_t datalen = 0;

	uint8_t i = index;

	switch (I2C_REG->I2STAT)
	{
	/*slave receiver*/
	case 0x80:
	case 0x90:
	case 0x68:
	case 0x78:
		index++;
		__FALL_THROUGH__
	case 0xA0: // stop or repeated start condition received
		// sends the data
		if (i < I2C_SLAVE_BUFFER_SIZE)
		{
			mcu_i2c_buffer[i] = I2C_REG->I2DAT;
		}
		if (I2C_REG->I2STAT == 0xA0)
		{
			index = 0;
			mcu_i2c_buffer[i] = 0;
			// unlock ISR and process the info request
			mcu_enable_global_isr();
			mcu_i2c_slave_cb(mcu_i2c_buffer, &i);
			datalen = MIN(i, I2C_SLAVE_BUFFER_SIZE);
		}
		break;
	/*slave trasnmitter*/
	case 0xA8: // addressed, returned ack
	case 0xB0: // arbitration lost, returned ack
		i = 0;
		__FALL_THROUGH__
	case 0xB8: // byte sent, ack returned
		// copy data to output register
		I2C_REG->I2DAT = mcu_i2c_buffer[i++];
		// if there is more to send, ack, otherwise nack
		if (i >= datalen)
		{
			I2C_REG->I2CONCLR |= I2C_I2CONCLR_SIC;
			return;
		}
		index = i;
		break;
	case 0x00: // bus error, illegal stop/start
		index = 0;
		// restart I2C
		uint32_t clkh = I2Cx->I2SCLH;
		uint32_t clkl = I2Cx->I2SCLL;
		I2C_Cmd(I2C_REG, I2C_MASTER_MODE, DISABLE);
		I2C_DeInit(I2C_REG);
		I2C_Init(I2C_REG, I2C_FREQ);
		I2Cx->I2SCLH = clkh;
		I2Cx->I2SCLL = clkl;
		I2C_Cmd(I2C_REG, I2C_MASTER_MODE, ENABLE);
		break;
	default: // other cases like reset data and prepare ACK to receive data
		index = 0;
		break;
	}

	// clear and reenable I2C ISR by default this falls to NACK if ACK is not set
	I2C_REG->I2CONCLR |= I2C_I2CONCLR_SIC | I2C_I2CONSET_AA;
}
#endif
// this is similar to AVR

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
	LPC_SC->ONESHOT_PCLKSEL_REG &= ~ONESHOT_PCLKSEL_MASK; // system clk/4

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
