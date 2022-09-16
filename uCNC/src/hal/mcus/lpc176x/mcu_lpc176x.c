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
	// TIM_ClearIntPending(RTC_TIMER_REG, RTC_INT_FLAG);
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

	mcu_io_init();
	mcu_usart_init();
	// SysTick is started by the framework but is not working
	// Using timer
	mcu_rtc_init();
#if SERVOS_MASK > 0
	servo_timer_init();
#endif
#ifdef MCU_HAS_SPI
	SPI_CFG_Type spi_config = {0};
	SPI_ConfigStructInit(&spi_config);
	spi_config.CPHA = (SPI_MODE & 0x01) ? SPI_CPHA_SECOND : SPI_CPHA_FIRST;
	spi_config.CPHA = (SPI_MODE & 0x02) ? SPI_CPOL_HI : SPI_CPOL_LO;
	spi_config.ClockRate = SPI_FREQ;
	SPI_Init(SPI_REG, &spi_config);
#endif
#ifdef MCU_HAS_I2C
	PINSEL_CFG_Type scl = {I2C_SCL_PORT, I2C_SCL_BIT, I2C_ALT_FUNC, PINSEL_PINMODE_TRISTATE, PINSEL_PINMODE_OPENDRAIN};
	PINSEL_ConfigPin(&scl);
	PINSEL_CFG_Type sda = {I2C_SDA_PORT, I2C_SDA_BIT, I2C_ALT_FUNC, PINSEL_PINMODE_TRISTATE, PINSEL_PINMODE_OPENDRAIN};
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
 * the maximum allowed delay is 255 us
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

#ifdef MCU_HAS_SPI
void mcu_spi_config(uint8_t mode, uint32_t frequency){
	mode = CLAMP(0, mode, 4);
	SPI_DeInit(SPI_REG);
	SPI_CFG_Type spi_config = {0};
	SPI_ConfigStructInit(&spi_config);
	spi_config.CPHA = (mode & 0x01) ? SPI_CPHA_SECOND : SPI_CPHA_FIRST;
	spi_config.CPHA = (mode & 0x02) ? SPI_CPOL_HI : SPI_CPOL_LO;
	spi_config.ClockRate = frequency;
	SPI_Init(SPI_REG, &spi_config);
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

#endif
