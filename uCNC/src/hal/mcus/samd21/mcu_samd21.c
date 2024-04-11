/*
		Name: mcu_samd21.h
		Description: Contains all the function declarations necessary to interact with the MCU.
		This provides a opac intenterface between the µCNC and the MCU unit used to power the µCNC.

		Copyright: Copyright (c) João Martins
		Author: João Martins
		Date: 09-08-2021

		µCNC is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version. Please see <http://www.gnu.org/licenses/>

		µCNC is distributed WITHOUT ANY WARRANTY;
		Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
		See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_SAMD21)
#include "core_cm0plus.h"
#include "mcumap_samd21.h"

#include "sam.h"
// #include "instance/nvmctrl.h"
#include <string.h>
#include <math.h>

// Non volatile memory
// SAMD devices page size never exceeds 1024 bytes
#define NVM_EEPROM_SIZE NVM_STORAGE_SIZE // 1Kb of emulated EEPROM is enough
#define NVM_PAGE_SIZE NVMCTRL_PAGE_SIZE
#define NVM_ROW_PAGES NVMCTRL_ROW_PAGES
#define NVM_ROW_SIZE NVMCTRL_ROW_SIZE
#define NVM_EEPROM_ROWS ((uint8_t)ceil(NVM_EEPROM_SIZE / NVMCTRL_ROW_SIZE))
#define NVM_EEPROM_BASE (FLASH_ADDR + NVMCTRL_FLASH_SIZE - (NVM_EEPROM_ROWS * NVMCTRL_ROW_SIZE))
#define NVM_MEMORY ((volatile uint16_t *)FLASH_ADDR)

#ifdef MCU_HAS_USB
#include <tusb_ucnc.h>
#endif

volatile bool samd21_global_isr_enabled;

// setups internal timers (all will run @ 8Mhz on GCLK4)
#define MAIN_CLOCK_DIV ((uint16_t)(F_CPU / F_TIMERS))
static void mcu_setup_clocks(void)
{
	PM->CPUSEL.reg = 0;
	PM->APBASEL.reg = 0;
	PM->APBBSEL.reg = 0;
	PM->APBCSEL.reg = 0;
	PM->AHBMASK.reg |= (PM_AHBMASK_NVMCTRL);
	PM->APBAMASK.reg |= (PM_APBAMASK_PM | PM_APBAMASK_SYSCTRL | PM_APBAMASK_GCLK | PM_APBAMASK_RTC);
	PM->APBBMASK.reg |= (PM_APBBMASK_NVMCTRL | PM_APBBMASK_PORT | PM_APBBMASK_USB);
	PM->APBCMASK.reg |= (PM_APBCMASK_TCC0 | PM_APBCMASK_TCC1 | PM_APBCMASK_TCC2 | PM_APBCMASK_TC3 | PM_APBCMASK_TC4 | PM_APBCMASK_TC5 | PM_APBCMASK_TC6 | PM_APBCMASK_TC7);
	PM->APBCMASK.reg |= PM_APBCMASK_ADC;

	/* Configure GCLK4's divider - to run @ 8Mhz*/
	GCLK->GENDIV.reg = GCLK_GENDIV_ID(4) | GCLK_GENDIV_DIV(MAIN_CLOCK_DIV);

	while (GCLK->STATUS.bit.SYNCBUSY)
		;

	/* Setup GCLK4 using the DFLL @48Mhz */
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(4) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;
	/* Wait for the write to complete */
	while (GCLK->STATUS.bit.SYNCBUSY)
		;

	/* Connect GCLK4 to all timers*/
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK4 | GCLK_CLKCTRL_ID_TCC0_TCC1;
	/* Wait for the write to complete. */
	while (GCLK->STATUS.bit.SYNCBUSY)
		;
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK4 | GCLK_CLKCTRL_ID_TCC2_TC3;
	/* Wait for the write to complete. */
	while (GCLK->STATUS.bit.SYNCBUSY)
		;
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK4 | GCLK_CLKCTRL_ID_TC4_TC5;
	/* Wait for the write to complete. */
	while (GCLK->STATUS.bit.SYNCBUSY)
		;
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK4 | GCLK_CLKCTRL_ID_TC6_TC7;
	/* Wait for the write to complete. */
	while (GCLK->STATUS.bit.SYNCBUSY)
		;

#if (SAMD21_EIC_MASK != 0)
	GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_EIC);
	EIC->CTRL.bit.ENABLE = 1;
	while (EIC->STATUS.bit.SYNCBUSY)
		;
	/*all external interrupts will be on pin change with filter*/
	EIC->CONFIG[0].reg = 0xbbbbbbbb;
	EIC->CONFIG[1].reg = 0xbbbbbbbb;
	NVIC_SetPriority(EIC_IRQn, 6);
	NVIC_ClearPendingIRQ(EIC_IRQn);
	NVIC_EnableIRQ(EIC_IRQn);
	EIC->EVCTRL.reg = 0;
	EIC->INTFLAG.reg = SAMD21_EIC_MASK;
	EIC->INTENSET.reg = SAMD21_EIC_MASK;
#endif
	// ADC clock
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK4 | GCLK_CLKCTRL_ID_ADC;
	/* Wait for the write to complete. */
	while (GCLK->STATUS.bit.SYNCBUSY)
		;

	// adc reset
	ADC->CTRLA.bit.SWRST = 1;
	while (ADC->STATUS.bit.SYNCBUSY)
		;
	// set resolution
	ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_10BIT_Val;
	ADC->CTRLB.bit.PRESCALER = ADC_CTRLB_PRESCALER_DIV32_Val;
	while (ADC->STATUS.bit.SYNCBUSY)
		;

	// set ref voltage
	ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;
	ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val;
	/* Wait for bus synchronization. */
	while (ADC->STATUS.bit.SYNCBUSY)
		;

	uint32_t bias = (*((uint32_t *)ADC_FUSES_BIASCAL_ADDR) & ADC_FUSES_BIASCAL_Msk) >> ADC_FUSES_BIASCAL_Pos;
	uint32_t linearity = (*((uint32_t *)ADC_FUSES_LINEARITY_0_ADDR) & ADC_FUSES_LINEARITY_0_Msk) >> ADC_FUSES_LINEARITY_0_Pos;
	linearity |= ((*((uint32_t *)ADC_FUSES_LINEARITY_1_ADDR) & ADC_FUSES_LINEARITY_1_Msk) >> ADC_FUSES_LINEARITY_1_Pos) << 5;

	/* Wait for bus synchronization. */
	while (ADC->STATUS.bit.SYNCBUSY)
		;

	/* Write the calibration data. */
	ADC->CALIB.reg = ADC_CALIB_BIAS_CAL(bias) | ADC_CALIB_LINEARITY_CAL(linearity);
	ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1;
	ADC->INPUTCTRL.bit.MUXNEG = 0x18; // select internal ground
	ADC->CTRLA.bit.ENABLE = 1;
	while (ADC->STATUS.bit.SYNCBUSY)
		;
}

#if (SAMD21_EIC_MASK != 0)

#if (PROBE_EICMASK != 0)
static bool mcu_probe_isr_enabled;
#endif

void EIC_Handler(void)
{
	mcu_disable_global_isr();
#if (LIMITS_EICMASK != 0)
	if (EIC->INTFLAG.reg & LIMITS_EICMASK)
	{
		mcu_limits_changed_cb();
	}
#endif
#if (CONTROLS_EICMASK != 0)
	if (EIC->INTFLAG.reg & CONTROLS_EICMASK)
	{
		mcu_controls_changed_cb();
	}
#endif
#if (PROBE_EICMASK != 0)
	if (EIC->INTFLAG.reg & PROBE_EICMASK && mcu_probe_isr_enabled)
	{
		mcu_probe_changed_cb();
	}
#endif
#if (DIN_IO_EICMASK != 0)
	if (EIC->INTFLAG.reg & DIN_IO_EICMASK)
	{
		mcu_inputs_changed_cb();
	}
#endif

	EIC->INTFLAG.reg = SAMD21_EIC_MASK;
	mcu_enable_global_isr();
}
#endif

void MCU_ITP_ISR(void)
{
	mcu_disable_global_isr();
	static bool resetstep = false;

#if (ITP_TIMER < 3)
	if (ITP_REG->INTFLAG.bit.MC0)
	{
		ITP_REG->INTFLAG.bit.MC0 = 1;
#else
	if (ITP_REG->COUNT16.INTFLAG.bit.MC0)
	{
		ITP_REG->COUNT16.INTFLAG.bit.MC0 = 1;
#endif
		if (!resetstep)
			mcu_step_cb();
		else
			mcu_step_reset_cb();
		resetstep = !resetstep;
	}

	mcu_enable_global_isr();
}

#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);

void mcu_com_isr()
{
	__ATOMIC_FORCEON__
	{
		if (COM_UART->USART.INTFLAG.bit.RXC && COM_UART->USART.INTENSET.bit.RXC)
		{
			COM_UART->USART.INTFLAG.bit.RXC = 1;
			uint8_t c = (0xff & COM_INREG);
#if !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(uart_rx))
				{
					c = OVF;
				}

				BUFFER_ENQUEUE(uart_rx, &c);
			}
#else
			mcu_uart_rx_cb(c);
#endif
		}
		if (COM_UART->USART.INTFLAG.bit.DRE && COM_UART->USART.INTENSET.bit.DRE)
		{
			mcu_enable_global_isr();
			if (BUFFER_EMPTY(uart_tx))
			{
				COM_UART->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
				return;
			}

			uint8_t c;
			BUFFER_DEQUEUE(uart_tx, &c);
			COM_OUTREG = c;
		}
	}
}
#endif

#if (defined(MCU_HAS_UART2))
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart2_rx, RX_BUFFER_SIZE);

void mcu_com2_isr()
{
	__ATOMIC_FORCEON__
	{
		if (COM2_UART->USART.INTFLAG.bit.RXC && COM2_UART->USART.INTENSET.bit.RXC)
		{
			COM2_UART->USART.INTFLAG.bit.RXC = 1;
			uint8_t c = (0xff & COM2_INREG);
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(uart2_rx))
				{
					c = OVF;
				}

				BUFFER_ENQUEUE(uart2_rx, &c);
			}
#else
			mcu_uart2_rx_cb(c);
#ifndef UART2_DISABLE_BUFFER
			if (BUFFER_FULL(uart2_rx))
			{
				c = OVF;
			}

			BUFFER_ENQUEUE(uart2_rx, &c);
#endif
#endif
		}
		if (COM2_UART->USART.INTFLAG.bit.DRE && COM2_UART->USART.INTENSET.bit.DRE)
		{
			// keeps sending chars until null is found
			mcu_enable_global_isr();
			if (BUFFER_EMPTY(uart2_tx))
			{
				COM2_UART->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
				return;
			}
			uint8_t c;
			BUFFER_DEQUEUE(uart2_tx, &c);
			COM2_OUTREG = c;
		}
	}
}
#endif

void mcu_usart_init(void)
{
#ifdef MCU_HAS_UART
	PM->APBCMASK.reg |= PM_APBCMASK_COM;

	/* Setup GCLK SERCOMx to use GENCLK0 */
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_COM;
	while (GCLK->STATUS.bit.SYNCBUSY)
		;

	// Start the Software Reset
	COM_UART->USART.CTRLA.bit.SWRST = 1;

	while (COM_UART->USART.SYNCBUSY.bit.SWRST)
		;

	COM_UART->USART.CTRLA.bit.MODE = 1;
	COM_UART->USART.CTRLA.bit.SAMPR = 0;		 // 16x sample rate
	COM_UART->USART.CTRLA.bit.FORM = 0;			 // no parity
	COM_UART->USART.CTRLA.bit.DORD = 1;			 // LSB first
	COM_UART->USART.CTRLA.bit.RXPO = COM_RX_PAD; // RX on PAD3
	COM_UART->USART.CTRLA.bit.TXPO = COM_TX_PAD; // TX on PAD2
	COM_UART->USART.CTRLB.bit.SBMODE = 0;		 // one stop bit
	COM_UART->USART.CTRLB.bit.CHSIZE = 0;		 // 8 bits
	COM_UART->USART.CTRLB.bit.RXEN = 1;			 // enable receiver
	COM_UART->USART.CTRLB.bit.TXEN = 1;			 // enable transmitter

	while (COM_UART->USART.SYNCBUSY.bit.CTRLB)
		;

	uint16_t baud = (uint16_t)(65536.0f * (1.0f - (((float)BAUDRATE) / (F_CPU >> 4))));

	COM_UART->USART.BAUD.reg = baud;
	mcu_config_altfunc(TX);
	mcu_config_altfunc(RX);
	COM_UART->USART.INTENSET.bit.RXC = 1; // enable recieved interrupt
	COM_UART->USART.INTENSET.bit.ERROR = 1;

	NVIC_ClearPendingIRQ(COM_IRQ);
	NVIC_EnableIRQ(COM_IRQ);
	NVIC_SetPriority(COM_IRQ, 0);

	// enable COM_UART
	COM_UART->USART.CTRLA.bit.ENABLE = 1;
	while (COM_UART->USART.SYNCBUSY.bit.ENABLE)
		;

#endif
#ifdef MCU_HAS_UART2
	PM->APBCMASK.reg |= PM_APBCMASK_COM2;

	/* Setup GCLK SERCOMx to use GENCLK0 */
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_COM2;
	while (GCLK->STATUS.bit.SYNCBUSY)
		;

	// Start the Software Reset
	COM2_UART->USART.CTRLA.bit.SWRST = 1;

	while (COM2_UART->USART.SYNCBUSY.bit.SWRST)
		;

	COM2_UART->USART.CTRLA.bit.MODE = 1;
	COM2_UART->USART.CTRLA.bit.SAMPR = 0;		   // 16x sample rate
	COM2_UART->USART.CTRLA.bit.FORM = 0;		   // no parity
	COM2_UART->USART.CTRLA.bit.DORD = 1;		   // LSB first
	COM2_UART->USART.CTRLA.bit.RXPO = COM2_RX_PAD; // RX on PAD3
	COM2_UART->USART.CTRLA.bit.TXPO = COM2_TX_PAD; // TX on PAD2
	COM2_UART->USART.CTRLB.bit.SBMODE = 0;		   // one stop bit
	COM2_UART->USART.CTRLB.bit.CHSIZE = 0;		   // 8 bits
	COM2_UART->USART.CTRLB.bit.RXEN = 1;		   // enable receiver
	COM2_UART->USART.CTRLB.bit.TXEN = 1;		   // enable transmitter

	while (COM2_UART->USART.SYNCBUSY.bit.CTRLB)
		;

	uint16_t baud2 = (uint16_t)(65536.0f * (1.0f - (((float)BAUDRATE2) / (F_CPU >> 4))));

	COM2_UART->USART.BAUD.reg = baud2;
	mcu_config_altfunc(TX2);
	mcu_config_altfunc(RX2);
	COM2_UART->USART.INTENSET.bit.RXC = 1; // enable recieved interrupt
	COM2_UART->USART.INTENSET.bit.ERROR = 1;

	NVIC_ClearPendingIRQ(COM2_IRQ);
	NVIC_EnableIRQ(COM2_IRQ);
	NVIC_SetPriority(COM2_IRQ, 0);

	// enable COM_UART
	COM2_UART->USART.CTRLA.bit.ENABLE = 1;
	while (COM2_UART->USART.SYNCBUSY.bit.ENABLE)
		;

#endif
#ifdef MCU_HAS_USB
	PM->AHBMASK.reg |= PM_AHBMASK_USB;

	mcu_config_input(USB_DM);
	mcu_config_input(USB_DP);
	mcu_config_altfunc(USB_DM);
	mcu_config_altfunc(USB_DP);
	NVIC_ClearPendingIRQ(USB_IRQn);
	NVIC_EnableIRQ(USB_IRQn);
	NVIC_SetPriority(USB_IRQn, 5);

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_USB;
	/* Wait for the write to complete. */
	while (GCLK->STATUS.bit.SYNCBUSY)
		;

	USB->DEVICE.INTENSET.reg = USB_DEVICE_EPINTENSET_MASK;
	USB->DEVICE.CTRLA.bit.ENABLE = 1;
	USB->DEVICE.CTRLA.bit.MODE = 0;
	USB->DEVICE.CTRLB.bit.SPDCONF = 0; //.reg &= ~USB_DEVICE_CTRLB_SPDCONF_Msk;
	// USB->DEVICE.CTRLB.reg |= USB_DEVICE_CTRLB_SPDCONF_FS;
	while (USB->DEVICE.SYNCBUSY.bit.SWRST)
		;
	tusb_cdc_init();
#endif
}

#ifdef MCU_HAS_USB
void USB_Handler(void)
{
	mcu_disable_global_isr();
	tusb_cdc_isr_handler();
	mcu_enable_global_isr();
}
#endif

#if SERVOS_MASK > 0

static uint16_t mcu_servos[6];

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

// timers are running from GCLCK4 @1MHz
// servo will have prescaller of /4
// this will yield a freq of 250KHz or 250 count per ms
// in theory servo resolution should be 250
// but 245 gives a closer result
// #define SERVO_RESOLUTION (245)
void servo_timer_init()
{
#if (SERVO_TIMER < 3)
	// reset timer
	SERVO_REG->CTRLA.bit.SWRST = 1;
	while (SERVO_REG->SYNCBUSY.bit.SWRST)
		;
	// enable the timer in the APB
	SERVO_REG->CTRLA.bit.PRESCALER = (uint8_t)0x4; // prescaller /16
	SERVO_REG->WAVE.bit.WAVEGEN = 1;			   // match compare
	while (SERVO_REG->SYNCBUSY.bit.WAVE)
		;
#else
	// reset timer
	SERVO_REG->COUNT16.CTRLA.bit.SWRST = 1;
	while (SERVO_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	// enable the timer in the APB
	SERVO_REG->COUNT16.CTRLA.bit.PRESCALER = (uint8_t)0x4; // prescaller /16
	SERVO_REG->COUNT16.CTRLA.bit.WAVEGEN = 1;			   // match compare
	while (SERVO_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
#endif
}

void servo_start_timeout(uint8_t val)
{
	NVIC_SetPriority(SERVO_IRQ, 10);
	NVIC_ClearPendingIRQ(SERVO_IRQ);
	NVIC_EnableIRQ(SERVO_IRQ);

#if (SERVO_TIMER < 3)
	SERVO_REG->CC[0].reg = (val << 1) + 125 - 4;
	SERVO_REG->COUNT.reg = 0;
	while (SERVO_REG->SYNCBUSY.bit.CC0)
		;
	SERVO_REG->INTENSET.bit.MC0 = 1;
	SERVO_REG->CTRLA.bit.ENABLE = 1; // enable timer and also write protection
	while (SERVO_REG->SYNCBUSY.bit.ENABLE)
		;
#else
	SERVO_REG->COUNT16.CC[0].reg = (val << 1) + 125 - 4;
	SERVO_REG->COUNT16.COUNT.reg = 0;
	while (SERVO_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	SERVO_REG->COUNT16.INTENSET.bit.MC0 = 1;
	SERVO_REG->COUNT16.CTRLA.bit.ENABLE = 1; // enable timer and also write protection
	while (SERVO_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
#endif
}

void MCU_SERVO_ISR(void)
{
	mcu_enable_global_isr();
#if (SERVO_TIMER < 3)
	if (SERVO_REG->INTFLAG.bit.MC0)
	{
		SERVO_REG->INTFLAG.bit.MC0 = 1;
#else
	if (SERVO_REG->COUNT16.INTFLAG.bit.MC0)
	{
		SERVO_REG->COUNT16.INTFLAG.bit.MC0 = 1;
#endif
		mcu_clear_servos();
		NVIC_DisableIRQ(SERVO_IRQ);
		SERVO_REG->COUNT16.INTENCLR.bit.MC0 = 1;
		SERVO_REG->COUNT16.CTRLA.bit.ENABLE = 0; // disable timer and also write protection
	}
}
#endif

/**
 * The internal clock counter
 * Increments every millisecond
 * Can count up to almost 50 days
 **/
static volatile uint32_t mcu_runtime_ms;

#ifndef ARDUINO_ARCH_SAMD
void SysTick_Handler(void)
#else
void sysTickHook(void)
#endif
{
	mcu_disable_global_isr();
	// counts to 20 and reloads
#if SERVOS_MASK > 0
	static uint8_t ms_servo_counter = 0;
	uint8_t servo_counter = ms_servo_counter;

	switch (servo_counter)
	{
#if ASSERT_PIN(SERVO0)
	case SERVO0_FRAME:
		servo_start_timeout(mcu_servos[0]);
		io_set_output(SERVO0);
		break;
#endif
#if ASSERT_PIN(SERVO1)
	case SERVO1_FRAME:
		io_set_output(SERVO1);
		servo_start_timeout(mcu_servos[1]);
		break;
#endif
#if ASSERT_PIN(SERVO2)
	case SERVO2_FRAME:
		io_set_output(SERVO2);
		servo_start_timeout(mcu_servos[2]);
		break;
#endif
#if ASSERT_PIN(SERVO3)
	case SERVO3_FRAME:
		io_set_output(SERVO3);
		servo_start_timeout(mcu_servos[3]);
		break;
#endif
#if ASSERT_PIN(SERVO4)
	case SERVO4_FRAME:
		io_set_output(SERVO4);
		servo_start_timeout(mcu_servos[4]);
		break;
#endif
#if ASSERT_PIN(SERVO5)
	case SERVO5_FRAME:
		io_set_output(SERVO5);
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
	mcu_enable_global_isr();
}

void mcu_rtc_init()
{
	SysTick->CTRL = 0;
	SysTick->LOAD = ((F_CPU / 1000) - 1);
	SysTick->VAL = 0;
	NVIC_SetPriority(SysTick_IRQn, 10);
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
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
	samd21_global_isr_enabled = false;
	mcu_setup_clocks();
	mcu_io_init();
	mcu_usart_init();
	mcu_rtc_init();
#if SERVOS_MASK > 0
	servo_timer_init();
#endif
#ifdef MCU_HAS_SPI
	PM->APBCMASK.reg |= PM_APBCMASK_SPICOM;

	/* Setup GCLK SERCOMx to use GENCLK0 */
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_SPICOM;
	while (GCLK->STATUS.bit.SYNCBUSY)
		;

	// Start the Software Reset
	SPICOM->SPI.CTRLA.bit.SWRST = 1;

	while (SPICOM->SPI.SYNCBUSY.bit.SWRST)
		;

	SPICOM->SPI.CTRLA.bit.MODE = 3;
	SPICOM->SPI.CTRLA.bit.DORD = 0;						 // MSB
	SPICOM->SPI.CTRLA.bit.CPHA = SPI_MODE & 0x01;		 // MODE
	SPICOM->SPI.CTRLA.bit.CPOL = (SPI_MODE >> 1) & 0x01; // MODE
	SPICOM->SPI.CTRLA.bit.FORM = 0;
	SPICOM->SPI.CTRLA.bit.DIPO = INPAD;
	SPICOM->SPI.CTRLA.bit.DOPO = OUTPAD;

	SPICOM->SPI.CTRLB.bit.RXEN = 1;
	SPICOM->SPI.CTRLB.bit.CHSIZE = 0;

	SPICOM->SPI.BAUD.reg = ((F_CPU >> 1) / SPI_FREQ) - 1;

	mcu_config_altfunc(SPI_CLK);
	mcu_config_altfunc(SPI_SDO);
	mcu_config_altfunc(SPI_SDI);

	SPICOM->SPI.CTRLA.bit.ENABLE = 1;
	while (SPICOM->SPI.SYNCBUSY.bit.ENABLE)
		;

#endif
#ifdef MCU_HAS_I2C
	mcu_i2c_config(I2C_FREQ);
#endif
	mcu_enable_global_isr();
}

/*IO functions*/
// IO functions
void mcu_set_servo(uint8_t servo, uint8_t value)
{
#if SERVOS_MASK > 0
	mcu_servos[servo - SERVO_PINS_OFFSET] = (((uint16_t)value) << 1);
#endif
}

/**
 * gets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
uint8_t mcu_get_servo(uint8_t servo)
{
#if SERVOS_MASK > 0
	uint8_t offset = servo - SERVO_PINS_OFFSET;
	uint8_t unscaled = (uint8_t)(mcu_servos[offset] >> 1);

	if ((1U << offset) & SERVOS_MASK)
	{
		return unscaled;
	}
#endif
	return 0;
}

/**
 * enables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_probe_isr
void mcu_enable_probe_isr(void)
{
#if (PROBE_EICMASK != 0)
	mcu_probe_isr_enabled = true;
#endif
}
#endif

/**
 * disables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_probe_isr
void mcu_disable_probe_isr(void)
{
#if (PROBE_EICMASK != 0)
	mcu_probe_isr_enabled = false;
#endif
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
 * checks if the serial hardware of the MCU is ready do send the next uint8_t
 * */
#ifndef mcu_tx_ready
bool mcu_tx_ready(void)
{
	return false;
} // Start async send
#endif

/**
 * sends a uint8_t either via uart (hardware, software or USB virtual COM_UART port)
 * can be defined either as a function or a macro call
 * */
#ifdef MCU_HAS_USB
DECL_BUFFER(uint8_t, usb_rx, RX_BUFFER_SIZE);

uint8_t mcu_usb_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(usb_rx, &c);
	return c;
}

uint8_t mcu_usb_available(void)
{
	return BUFFER_READ_AVAILABLE(usb_rx);
}

void mcu_usb_clear(void)
{
	BUFFER_CLEAR(usb_rx);
}

void mcu_usb_putc(uint8_t c)
{
	if (!tusb_cdc_write_available())
	{
		mcu_usb_flush();
		if (!tusb_cdc_connected)
		{
			return;
		}
	}
	tusb_cdc_write(c);
}

void mcu_usb_flush(void)
{
	tusb_cdc_flush();
	while (!tusb_cdc_write_available())
	{
		mcu_dotasks(); // tinyusb device task
	}
}
#endif

#ifdef MCU_HAS_UART
uint8_t mcu_uart_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(uart_rx, &c);
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
	while (BUFFER_FULL(uart_tx))
	{
		mcu_uart_flush();
	}
	BUFFER_ENQUEUE(uart_tx, &c);
}

void mcu_uart_flush(void)
{
	if (!(COM_UART->USART.INTENSET.reg & SERCOM_USART_INTENSET_DRE)) // not ready start flushing
	{
		COM_UART->USART.INTENSET.bit.DRE = 1; // enable recieved interrupt
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
	BUFFER_DEQUEUE(uart2_rx, &c);
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
	while (BUFFER_FULL(uart2_tx))
	{
		mcu_uart2_flush();
	}
	BUFFER_ENQUEUE(uart2_tx, &c);
}

void mcu_uart_flush(void)
{
	if (!(COM2_UART->USART.INTENSET.reg & SERCOM_USART_INTENSET_DRE)) // not ready start flushing
	{
		COM2_UART->USART.INTENSET.bit.DRE = 1; // enable tx interrupt
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
void mcu_enable_global_isr(void)
{
}
#endif

/**
 * disables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_global_isr
void mcu_disable_global_isr(void)
{
}
#endif

// Step interpolator
/**
 * convert step rate to clock cycles
 * */
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
{
	frequency = CLAMP((float)F_STEP_MIN, frequency, (float)F_STEP_MAX);

	uint32_t clocks = (uint32_t)((F_TIMERS >> 1) / frequency);
	*prescaller = 0;

	while (clocks > 0xFFFF)
	{
		clocks >>= 1;
		*prescaller++;
		if (*prescaller >= 4)
		{
			clocks >>= 1;
		}
		if (*prescaller == 7)
		{
			break;
		}
	}

	*ticks = ((uint16_t)clocks) - 1;
}

float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller)
{
	return ((float)(F_TIMERS >> 1) / (float)(((uint32_t)ticks + 1) << prescaller));
}

/**
 * starts the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
#if (ITP_TIMER < 3)
	// reset timer
	ITP_REG->CTRLA.bit.SWRST = 1;
	while (ITP_REG->SYNCBUSY.bit.SWRST)
		;
	// enable the timer in the APB
	ITP_REG->CTRLA.bit.PRESCALER = (uint8_t)prescaller; // normal counter
	ITP_REG->WAVE.bit.WAVEGEN = 1;						// match compare
	while (ITP_REG->SYNCBUSY.bit.WAVE)
		;
	ITP_REG->CC[0].reg = ticks;
	while (ITP_REG->SYNCBUSY.bit.CC0)
		;

	NVIC_SetPriority(ITP_IRQ, 1);
	NVIC_ClearPendingIRQ(ITP_IRQ);
	NVIC_EnableIRQ(ITP_IRQ);

	ITP_REG->INTENSET.bit.MC0 = 1;
	ITP_REG->CTRLA.bit.ENABLE = 1; // enable timer and also write protection
	while (ITP_REG->SYNCBUSY.bit.ENABLE)
		;
#else
	// reset timer
	ITP_REG->COUNT16.CTRLA.bit.SWRST = 1;
	while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	// enable the timer in the APB
	ITP_REG->COUNT16.CTRLA.bit.PRESCALER = (uint8_t)prescaller; // normal counter
	ITP_REG->COUNT16.CTRLA.bit.WAVEGEN = 1;						// match compare
	while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	ITP_REG->COUNT16.CC[0].reg = ticks;
	while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	NVIC_SetPriority(ITP_IRQ, 1);
	NVIC_ClearPendingIRQ(ITP_IRQ);
	NVIC_EnableIRQ(ITP_IRQ);

	ITP_REG->COUNT16.INTENSET.bit.MC0 = 1;
	ITP_REG->COUNT16.CTRLA.bit.ENABLE = 1; // enable timer and also write protection
	while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
#endif
}

/**
 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
#if (ITP_TIMER < 3)
	ITP_REG->CTRLA.bit.ENABLE = 0; // disable timer and also write protection
	while (ITP_REG->SYNCBUSY.bit.ENABLE)
		;
	ITP_REG->CTRLA.bit.PRESCALER = (uint8_t)prescaller; // normal counter
	ITP_REG->CC[0].bit.CC = ticks;
	while (ITP_REG->SYNCBUSY.bit.CC0)
		;
	ITP_REG->CTRLA.bit.ENABLE = 1; // enable timer and also write protection
	while (ITP_REG->SYNCBUSY.bit.ENABLE)
		;
#else
	ITP_REG->COUNT16.CTRLA.bit.ENABLE = 0; // disable timer and also write protection
	while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	ITP_REG->COUNT16.CTRLA.bit.PRESCALER = (uint8_t)prescaller; // normal counter
	ITP_REG->COUNT16.CC[0].bit.CC = ticks;
	while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	ITP_REG->COUNT16.CTRLA.bit.ENABLE = 1; // enable timer and also write protection
	while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
#endif
}

/**
 * stops the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_stop_itp_isr(void)
{
#if (ITP_TIMER < 3)
	ITP_REG->CTRLA.bit.ENABLE = 0; // disable timer and also write protection
	while (ITP_REG->SYNCBUSY.bit.ENABLE)
		;
#else
	ITP_REG->COUNT16.CTRLA.bit.ENABLE = 0;
	while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
#endif
	ITP_REG->COUNT16.INTENCLR.bit.MC0 = 1;
	NVIC_DisableIRQ(ITP_IRQ);
}

/**
 * gets the MCU running time in milliseconds.
 * the time counting is controled by the internal RTC
 * */
uint32_t mcu_millis()
{
	uint32_t c = mcu_runtime_ms;
	return c;
}

uint32_t mcu_micros()
{
	return ((mcu_runtime_ms * 1000) + mcu_free_micros());
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
void mcu_dotasks(void)
{
#ifdef MCU_HAS_USB
	tusb_cdc_task(); // tinyusb device task

	while (tusb_cdc_available())
	{
		uint8_t c = (uint8_t)tusb_cdc_read();
#ifndef DETACH_USB_FROM_MAIN_PROTOCOL
		if (mcu_com_rx_cb(c))
		{
			if (BUFFER_FULL(usb_rx))
			{
				c = OVF;
			}

			BUFFER_ENQUEUE(usb_rx, &c);
		}
#else
		mcu_usb_rx_cb(c);
#endif
	}
#endif
}

static uint8_t samd21_eeprom_sram[NVM_EEPROM_SIZE]; // 1kb max
static bool samd21_flash_modified = false;
static bool samd21_eeprom_loaded = false;

static void mcu_read_eeprom_buffer(void)
{
	PM->APBBMASK.bit.NVMCTRL_ = 1;
	NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;
	while (!NVMCTRL->INTFLAG.bit.READY)
		;

	mcu_disable_global_isr();
	NVMCTRL->CTRLB.bit.RWS = 0x02;
	NVMCTRL->CTRLB.bit.SLEEPPRM = 0;
	NVMCTRL->CTRLB.bit.CACHEDIS = 0;
	NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;
	while (!NVMCTRL->INTFLAG.bit.READY)
		;

	for (uint32_t i = 0; i != NVM_EEPROM_SIZE;)
	{
		uint16_t data = NVM_MEMORY[((NVM_EEPROM_BASE + i) / 2)];
		samd21_eeprom_sram[i] = (data & 0xff);
		samd21_eeprom_sram[i + 1] = (data >> 8);
		i += 2;
	}

	samd21_eeprom_loaded = true;
	samd21_flash_modified = false;
	NVMCTRL->CTRLB.bit.RWS = 0x01;
	NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;
	while (!NVMCTRL->INTFLAG.bit.READY)
		;
	mcu_enable_global_isr();
}

static void mcu_write_flash_page(const uint32_t destination_address, const uint8_t *buffer, uint16_t length)
{
	NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;

	mcu_disable_global_isr();
	// Execute "PBC" Page Buffer Clear
	NVMCTRL->ADDR.reg = (uintptr_t)&NVM_MEMORY[destination_address / 4];
	NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
	while (!NVMCTRL->INTFLAG.bit.READY)
		;

	NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;

	// Fill page buffer
	uint16_t i = 0;
	while (i != NVM_PAGE_SIZE)
	{
		uint16_t data = 0;
		if (i <= length)
		{
			data = buffer[i + 1];
			data <<= 8;
			data |= buffer[i];
		}

		NVM_MEMORY[((destination_address + i) / 2)] = data;
		// Data boundaries of the eeprom in 16bit chuncks
		i += 2;
	}

	// Execute "WP" Write Page
	NVMCTRL->ADDR.reg = (uintptr_t)&NVM_MEMORY[destination_address / 4];
	NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
	while (!NVMCTRL->INTFLAG.bit.READY)
		;

	mcu_enable_global_isr();
}

/**
 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
uint8_t mcu_eeprom_getc(uint16_t address)
{
	if (NVM_STORAGE_SIZE <= address)
	{
		DEBUG_STR("EEPROM invalid address @ ");
		DEBUG_INT(address);
		DEBUG_PUTC('\n');
		return 0;
	}
	address &= (NVM_EEPROM_SIZE - 1); // keep within 1Kb address range

	if (!samd21_eeprom_loaded)
	{
		mcu_read_eeprom_buffer();
	}

	return samd21_eeprom_sram[address];
}

/**
 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	if (NVM_STORAGE_SIZE <= address)
	{
		DEBUG_STR("EEPROM invalid address @ ");
		DEBUG_INT(address);
		DEBUG_PUTC('\n');
	}
	address &= (NVM_EEPROM_SIZE - 1);

	if (!samd21_eeprom_loaded)
	{
		mcu_read_eeprom_buffer();
	}

	if (samd21_eeprom_sram[address] != value)
	{
		samd21_flash_modified = true;
	}

	samd21_eeprom_sram[address] = value;
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
	if (samd21_flash_modified)
	{
		NVMCTRL->CTRLB.bit.RWS = 0x02;
		uint8_t cache = NVMCTRL->CTRLB.bit.CACHEDIS;
		NVMCTRL->CTRLB.bit.CACHEDIS = 1;

		// update rows
		uint32_t eeprom_offset = 0;
		uint32_t remaining = NVM_EEPROM_SIZE;
		while (remaining)
		{
			bool modified = false;
			for (uint16_t p = 0; p < NVM_ROW_PAGES; p++)
			{
				uint32_t page_offset = eeprom_offset + (p * NVM_PAGE_SIZE);
				for (uint16_t o = 0; o < NVM_PAGE_SIZE; o += 2)
				{
					uint32_t offset = page_offset + o;
					uint16_t data = NVM_MEMORY[(NVM_EEPROM_BASE + offset) / 2];
					if ((data & 0xff) != samd21_eeprom_sram[offset] || (data >> 8) != samd21_eeprom_sram[offset + 1])
					{
						modified = true;
						break;
					}
				}

				if (modified)
				{
					break;
				}
			}

			if (modified)
			{
				// set the flash address to erase/write half-word (datasheet 22.8.8)
				NVMCTRL->ADDR.reg = (uintptr_t)&NVM_MEMORY[(NVM_EEPROM_BASE + eeprom_offset) / 4];
				while (!NVMCTRL->INTFLAG.bit.READY)
					;

				NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;

				// erase region for writing
				NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
				while (!NVMCTRL->INTFLAG.bit.READY)
					;

				for (uint16_t p = 0; p < NVM_ROW_PAGES; p++)
				{
					uint32_t page_offset = eeprom_offset + (p * NVM_PAGE_SIZE);
					mcu_write_flash_page(NVM_EEPROM_BASE + page_offset, &samd21_eeprom_sram[page_offset], (remaining > NVM_PAGE_SIZE) ? NVM_PAGE_SIZE : remaining);
				}
			}

			eeprom_offset += NVM_ROW_SIZE;
			remaining -= (remaining > NVM_ROW_SIZE) ? NVM_ROW_SIZE : remaining;
		}

		NVMCTRL->CTRLB.bit.CACHEDIS = cache;
		NVMCTRL->CTRLB.bit.RWS = 0x01;
	}

	samd21_flash_modified = false;
}

#ifdef MCU_HAS_SPI
void mcu_spi_config(uint8_t mode, uint32_t frequency)
{
	mode = CLAMP(0, mode, 4);
	frequency = ((F_CPU >> 1) / frequency) - 1;
	SPICOM->SPI.CTRLA.bit.ENABLE = 0;
	while (SPICOM->SPI.SYNCBUSY.bit.ENABLE)
		;
	SPICOM->SPI.CTRLA.bit.CPHA = mode & 0x01;		 // MODE
	SPICOM->SPI.CTRLA.bit.CPOL = (mode >> 1) & 0x01; // MODE
	SPICOM->SPI.BAUD.reg = frequency;

	SPICOM->SPI.CTRLA.bit.ENABLE = 1;
	while (SPICOM->SPI.SYNCBUSY.bit.ENABLE)
		;
}
#endif

#ifdef MCU_HAS_I2C
/**
 * https://www.eevblog.com/forum/microcontrollers/i2c-atmel/
 * */
#if I2C_ADDRESS == 0
void mcu_i2c_write_stop(bool *stop)
{
	if (*stop)
	{
		I2CCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
	}
}

static uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop, uint32_t ms_timeout)
{
	bool stop __attribute__((__cleanup__(mcu_i2c_write_stop))) = send_stop;

	if (send_start)
	{
		I2CCOM->I2CM.ADDR.reg = data;
	}
	else
	{
		I2CCOM->I2CM.DATA.reg = data;
	}

	__TIMEOUT_MS__(ms_timeout)
	{
		if ((I2CCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB))
		{
			if (I2CCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
			{
				I2CCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
				return I2C_NOTOK;
			}

			return I2C_OK;
		}
	}

	stop = true;
	return I2C_NOTOK;
}

static uint8_t mcu_i2c_read(uint8_t *data, bool with_ack, bool send_stop, uint32_t ms_timeout)
{
	*data = 0xFF;
	bool stop __attribute__((__cleanup__(mcu_i2c_write_stop))) = send_stop;

	if (with_ack)
	{
		I2CCOM->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
	}
	else
	{
		I2CCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
	}

	__TIMEOUT_MS__(ms_timeout)
	{
		if (I2CCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB)
		{
			*data = I2CCOM->I2CM.DATA.reg;
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
	PM->APBCMASK.reg |= PM_APBCMASK_I2CCOM;

	/* Setup GCLK SERCOM */
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(0) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_I2CCOM;
	while (GCLK->STATUS.bit.SYNCBUSY)
		;

#if I2C_ADDRESS != 0
	// Start the Software Reset
	I2CCOM->I2CS.CTRLA.bit.SWRST = 1;

	while (I2CCOM->I2CS.SYNCBUSY.bit.SWRST)
		;

	I2CCOM->I2CS.CTRLB.reg = SERCOM_I2CS_CTRLB_AACKEN | SERCOM_I2CS_CTRLB_SMEN | SERCOM_I2CS_CTRLB_AMODE(0);
	while (I2CCOM->I2CS.SYNCBUSY.reg)
		;

	I2CCOM->I2CS.ADDR.reg = SERCOM_I2CS_ADDR_ADDR(I2C_ADDRESS) | SERCOM_I2CS_ADDR_GENCEN;
	while (I2CCOM->I2CS.SYNCBUSY.reg)
		;

	I2CCOM->I2CS.INTENSET.reg = SERCOM_I2CS_INTENSET_AMATCH | SERCOM_I2CS_INTENSET_DRDY | SERCOM_I2CS_INTENSET_PREC | SERCOM_I2CS_INTENSET_ERROR;
	NVIC_SetPriority(I2C_IRQ, 10);
	NVIC_ClearPendingIRQ(I2C_IRQ);
	NVIC_EnableIRQ(I2C_IRQ);

	I2CCOM->I2CS.CTRLA.reg = SERCOM_I2CS_CTRLA_ENABLE | SERCOM_I2CS_CTRLA_MODE_I2C_SLAVE | SERCOM_I2CS_CTRLA_SDAHOLD(3);
	while (I2CCOM->I2CS.SYNCBUSY.reg)
		;
#else
	// Start the Software Reset
	I2CCOM->I2CM.CTRLA.bit.SWRST = 1;

	while (I2CCOM->I2CM.SYNCBUSY.bit.SWRST)
		;

	I2CCOM->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN;
	while (I2CCOM->I2CM.SYNCBUSY.reg)
		;

	I2CCOM->I2CM.BAUD.reg = F_CPU / (2 * frequency) - 5 - (((F_CPU / 1000000) * 125) / (2 * 1000));
	while (I2CCOM->I2CM.SYNCBUSY.reg)
		;

	I2CCOM->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_ENABLE | SERCOM_I2CM_CTRLA_MODE_I2C_MASTER | SERCOM_I2CM_CTRLA_SDAHOLD(3);
	while (I2CCOM->I2CM.SYNCBUSY.reg)
		;

	I2CCOM->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(1);
	while (I2CCOM->I2CM.SYNCBUSY.reg)
		;

	mcu_config_altfunc(I2C_CLK);
	mcu_config_altfunc(I2C_DATA);

	I2CCOM->I2CM.CTRLA.bit.ENABLE = 1;
	while (I2CCOM->I2CM.SYNCBUSY.reg)
		;
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

	switch (I2CCOM->I2CS.INTFLAG.reg)
	{
	case SERCOM_I2CS_INTFLAG_AMATCH:
		i = 0;
		if (I2CCOM->I2CS.STATUS.bit.DIR)
		{
			// write first byte
			I2CCOM->I2CS.DATA.reg = mcu_i2c_buffer[i++];
			I2CCOM->I2CS.CTRLB.bit.ACKACT = 0;
			if (i >= datalen)
			{
				I2CCOM->I2CS.CTRLB.bit.ACKACT = 1;
				I2CCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(2);
				return;
			}
			index = i;
		}
		index = i;
		break;
	case SERCOM_I2CS_INTFLAG_DRDY:
		if (I2CCOM->I2CS.STATUS.bit.DIR)
		{
			I2CCOM->I2CS.DATA.reg = mcu_i2c_buffer[i++];
			if (i >= datalen)
			{
				I2CCOM->I2CS.CTRLB.bit.ACKACT = 1;
				I2CCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(2);
				return;
			}
		}
		else
		{
			if (i < I2C_SLAVE_BUFFER_SIZE)
			{
				mcu_i2c_buffer[i++] = I2CCOM->I2CS.DATA.reg;
				if (i >= I2C_SLAVE_BUFFER_SIZE)
				{
					I2CCOM->I2CS.CTRLB.bit.ACKACT = 1;
					I2CCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(2);
					return;
				}
			}
		}

		index = i;
		break;
	case SERCOM_I2CS_INTFLAG_PREC:
		// stop transmission
		index = 0;
		mcu_i2c_buffer[i] = 0;
		// unlock ISR and process the info request
		if (!(I2CCOM->I2CS.STATUS.bit.DIR) && i)
		{
			mcu_enable_global_isr();
			mcu_i2c_slave_cb(mcu_i2c_buffer, &i);
			datalen = MIN(i, I2C_SLAVE_BUFFER_SIZE);
		}
		break;
	case SERCOM_I2CS_INTFLAG_ERROR:
		// stop transmission
		index = 0;
		break;
	}

	I2CCOM->I2CS.CTRLB.bit.ACKACT = 0;
	I2CCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);

	NVIC_ClearPendingIRQ(I2C_IRQ);
}
#endif

#endif

#ifdef MCU_HAS_ONESHOT_TIMER

void MCU_ONESHOT_ISR(void)
{
#if (ONESHOT_TIMER < 3)
	ONESHOT_REG->INTENSET.bit.MC0 = 0;
	ONESHOT_REG->CTRLA.bit.ENABLE = 0; // disable timer and also write protection
	while (ONESHOT_REG->SYNCBUSY.bit.ENABLE)
		;
	if (ONESHOT_REG->INTFLAG.bit.MC0)
	{
		ONESHOT_REG->INTFLAG.bit.MC0 = 1;
#else
	ONESHOT_REG->COUNT16.INTENSET.bit.MC0 = 0;
	ONESHOT_REG->COUNT16.CTRLA.bit.ENABLE = 0;
	while (ONESHOT_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	if (ONESHOT_REG->COUNT16.INTFLAG.bit.MC0)
	{
		ONESHOT_REG->COUNT16.INTFLAG.bit.MC0 = 1;
#endif
	}

	if (mcu_timeout_cb)
	{
		mcu_timeout_cb();
	}

	NVIC_ClearPendingIRQ(ONESHOT_IRQ);
}

/**
 * configures a single shot timeout in us
 * */
#ifndef mcu_config_timeout
void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout)
{
	mcu_timeout_cb = fp;
	uint16_t ticks = (uint16_t)(timeout - 1);
	uint16_t prescaller = 3; // div by 8 giving one tick per us

#if (ONESHOT_TIMER < 3)
	// reset timer
	ONESHOT_REG->CTRLA.bit.SWRST = 1;
	while (ONESHOT_REG->SYNCBUSY.bit.SWRST)
		;
	// enable the timer in the APB
	ONESHOT_REG->CTRLA.bit.PRESCALER = (uint8_t)prescaller; // normal counter
	ONESHOT_REG->WAVE.bit.WAVEGEN = 1;						// match compare
	while (ONESHOT_REG->SYNCBUSY.bit.WAVE)
		;
	ONESHOT_REG->CC[0].reg = ticks;
	while (ONESHOT_REG->SYNCBUSY.bit.CC0)
		;

	NVIC_SetPriority(ONESHOT_IRQ, 3);
	NVIC_ClearPendingIRQ(ONESHOT_IRQ);
	NVIC_EnableIRQ(ONESHOT_IRQ);
#else
	// reset timer
	ONESHOT_REG->COUNT16.CTRLA.bit.SWRST = 1;
	while (ONESHOT_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	// enable the timer in the APB
	ONESHOT_REG->COUNT16.CTRLA.bit.PRESCALER = (uint8_t)prescaller; // normal counter
	ONESHOT_REG->COUNT16.CTRLA.bit.WAVEGEN = 1;						// match compare
	while (ONESHOT_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	ONESHOT_REG->COUNT16.CC[0].reg = ticks;
	while (ONESHOT_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	NVIC_SetPriority(ONESHOT_IRQ, 1);
	NVIC_ClearPendingIRQ(ONESHOT_IRQ);
	NVIC_EnableIRQ(ONESHOT_IRQ);
#endif
}
#endif

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#ifndef mcu_start_timeout
void mcu_start_timeout()
{
#if (ONESHOT_TIMER < 3)
	ONESHOT_REG->INTENSET.bit.MC0 = 1;
	ONESHOT_REG->CTRLA.bit.ENABLE = 1; // enable timer and also write protection
#else
	ONESHOT_REG->COUNT16.INTENSET.bit.MC0 = 1;
	ONESHOT_REG->COUNT16.CTRLA.bit.ENABLE = 1; // enable timer and also write protection
#endif
}
#endif
#endif

#endif
