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
//#include "instance/nvmctrl.h"
#include <string.h>
#include <math.h>

// Non volatile memory
// SAMD devices page size never exceeds 1024 bytes
#define NVM_EEPROM_SIZE 0x400 // 1Kb of emulated EEPROM is enough
#define NVM_PAGE_SIZE NVMCTRL_PAGE_SIZE
#define NVM_ROW_PAGES NVMCTRL_ROW_PAGES
#define NVM_ROW_SIZE NVMCTRL_ROW_SIZE
#define NVM_EEPROM_ROWS ((uint8_t)ceil(NVM_EEPROM_SIZE / NVMCTRL_ROW_SIZE))
#define NVM_EEPROM_BASE (FLASH_ADDR + NVMCTRL_FLASH_SIZE - (NVM_EEPROM_ROWS * NVMCTRL_ROW_SIZE))
#define NVM_MEMORY ((volatile uint16_t *)FLASH_ADDR)

#if (INTERFACE == INTERFACE_USB)
#include "../../../tinyusb/tusb_config.h"
#include "../../../tinyusb/src/tusb.h"
#endif

volatile bool samd21_global_isr_enabled;

// setups internal timers (all will run @ 1Mhz on GCLK4)
#define MAIN_CLOCK_DIV ((uint16_t)(F_CPU / 1000000))
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

	/* Configure GCLK4's divider - to run @ 1Mhz*/
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
	ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_8BIT_Val;
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

#if (INTERFACE == INTERFACE_UART)
void mcu_com_isr()
{
	mcu_disable_global_isr();
#ifndef ENABLE_SYNC_RX
	if (COM->USART.INTFLAG.bit.RXC && COM->USART.INTENSET.bit.RXC)
	{
		COM->USART.INTFLAG.bit.RXC = 1;
		unsigned char c = (0xff & COM_INREG);
		mcu_com_rx_cb(c);
	}
#endif
#ifndef ENABLE_SYNC_TX
	if (COM->USART.INTFLAG.bit.DRE && COM->USART.INTENSET.bit.DRE)
	{
		COM->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
		mcu_com_tx_cb();
	}
#endif
	mcu_enable_global_isr();
}
#endif

void mcu_usart_init(void)
{
#if (INTERFACE == INTERFACE_UART)
	PM->APBCMASK.reg |= PM_APBCMASK_COM;

	/* Setup GCLK SERCOMx to use GENCLK0 */
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_COM;
	while (GCLK->STATUS.bit.SYNCBUSY)
		;

	// Start the Software Reset
	COM->USART.CTRLA.bit.SWRST = 1;

	while (COM->USART.SYNCBUSY.bit.SWRST)
		;

	COM->USART.CTRLA.bit.MODE = 1;
	COM->USART.CTRLA.bit.SAMPR = 0;			// 16x sample rate
	COM->USART.CTRLA.bit.FORM = 0;			// no parity
	COM->USART.CTRLA.bit.DORD = 1;			// LSB first
	COM->USART.CTRLA.bit.RXPO = COM_RX_PAD; // RX on PAD3
	COM->USART.CTRLA.bit.TXPO = COM_TX_PAD; // TX on PAD2
	COM->USART.CTRLB.bit.SBMODE = 0;		// one stop bit
	COM->USART.CTRLB.bit.CHSIZE = 0;		// 8 bits
	COM->USART.CTRLB.bit.RXEN = 1;			// enable receiver
	COM->USART.CTRLB.bit.TXEN = 1;			// enable transmitter

	while (COM->USART.SYNCBUSY.bit.CTRLB)
		;

	uint16_t baud = (uint16_t)(65536.0f * (1.0f - (((float)BAUDRATE) / (F_CPU >> 4))));

	COM->USART.BAUD.reg = baud;
	mcu_config_altfunc(TX);
	mcu_config_altfunc(RX);

#ifndef ENABLE_SYNC_RX
	COM->USART.INTENSET.bit.RXC = 1; // enable recieved interrupt
	COM->USART.INTENSET.bit.ERROR = 1;
#endif
#ifndef ENABLE_SYNC_TX
	COM->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
#endif
	NVIC_ClearPendingIRQ(COM_IRQ);
	NVIC_EnableIRQ(COM_IRQ);
	NVIC_SetPriority(COM_IRQ, 0);

	// enable COM
	COM->USART.CTRLA.bit.ENABLE = 1;
	while (COM->USART.SYNCBUSY.bit.ENABLE)
		;

#endif
#if (INTERFACE == INTERFACE_USB)
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
	tusb_init();
#endif
}

#if (INTERFACE == INTERFACE_USB)
void USB_Handler(void)
{
	mcu_disable_global_isr();
	tud_int_handler(0);
	mcu_enable_global_isr();
}
#endif

#if SERVOS_MASK > 0

static uint16_t mcu_servos[6];

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

// timers are running from GCLCK4 @1MHz
// servo will have prescaller of /4
// this will yield a freq of 250KHz or 250 count per ms
// in theory servo resolution should be 250
// but 245 gives a closer result
#define SERVO_RESOLUTION (245)
void servo_timer_init()
{
#if (SERVO_TIMER < 3)
	// reset timer
	SERVO_REG->CTRLA.bit.SWRST = 1;
	while (SERVO_REG->SYNCBUSY.bit.SWRST)
		;
	// enable the timer in the APB
	SERVO_REG->CTRLA.bit.PRESCALER = (uint8_t)0x2; // prescaller /4
	SERVO_REG->WAVE.bit.WAVEGEN = 1;			   // match compare
	while (SERVO_REG->SYNCBUSY.bit.WAVE)
		;
#else
	// reset timer
	SERVO_REG->COUNT16.CTRLA.bit.SWRST = 1;
	while (SERVO_REG->COUNT16.STATUS.bit.SYNCBUSY)
		;
	// enable the timer in the APB
	SERVO_REG->COUNT16.CTRLA.bit.PRESCALER = (uint8_t)0x2; // prescaller /4
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
	mcu_enable_global_isr();
}

void mcu_rtc_init()
{
	SysTick->CTRL = 0;
	SysTick->LOAD = ((F_CPU / 1024) - 1);
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
	SPICOM->SPI.CTRLA.bit.DWORD = 0;					 // MSB
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
	while (SPICOM->SPI.SYNCBUSY.bit.SWRST)
		;

#endif
#ifdef MCU_HAS_I2C
	PM->APBCMASK.reg |= PM_APBCMASK_I2CCOM;

	/* Setup GCLK SERCOM */
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(0) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_I2CCOM;
	while (GCLK->STATUS.bit.SYNCBUSY)
		;

	// Start the Software Reset
	I2CCOM->I2CM.CTRLA.bit.SWRST = 1;

	while (I2CCOM->I2C.SYNCBUSY.bit.SWRST)
		;

	I2CCOM->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN;
	while (I2CCOM->I2CM.bit.SYNCBUSY)
		;

	I2CCOM->I2CM.BAUD.reg = SERCOM_I2CM_BAUD_BAUD(F_CPU / I2C_FREQ);
	while (I2CCOM->I2CM.bit.SYNCBUSY)
		;

	I2CCOM->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_ENABLE | SERCOM_I2CM_CTRLA_MODE_I2C_MASTER | SERCOM_I2CM_CTRLA_SDAHOLD(3);
	while (I2CCOM->I2CM.bit.SYNCBUSY)
		;

	I2CCOM->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(1);
	while (I2CCOM->I2CM.bit.SYNCBUSY)
		;

	mcu_config_altfunc(I2C_SCL);
	mcu_config_altfunc(I2C_SDA);

	I2CCOM->I2C.CTRLA.bit.ENABLE = 1;
	while (I2CCOM->I2C.SYNCBUSY.bit.SWRST)
		;

#endif
	mcu_enable_global_isr();
}

/*IO functions*/
// IO functions
void mcu_set_servo(uint8_t servo, uint8_t value)
{
#if SERVOS_MASK > 0
	uint8_t scaled = (uint8_t)(((uint16_t)(value * SERVO_RESOLUTION)) >> 8);
	mcu_servos[servo - SERVO0_UCNC_INTERNAL_PIN] = scaled;
#endif
}

/**
 * gets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
uint8_t mcu_get_servo(uint8_t servo)
{
#if SERVOS_MASK > 0
	uint8_t offset = servo - SERVO0_UCNC_INTERNAL_PIN;
	uint8_t unscaled = (uint8_t)((((uint16_t)mcu_servos[offset] << 8)) / SERVO_RESOLUTION);

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
	return false;
} // Start async send
#endif

/**
 * checks if the serial hardware of the MCU has a new char ready to be read
 * */
#ifndef mcu_rx_ready
bool mcu_rx_ready(void)
{
	return false;
} // Stop async send
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
#if (INTERFACE == INTERFACE_USB)
	if (c != 0)
	{
		tud_cdc_write_char(c);
	}
	if (c == '\r' || c == 0)
	{
		tud_cdc_write_flush();
	}
#else
#if (INTERFACE == INTERFACE_UART)
#ifdef ENABLE_SYNC_TX
	while (!mcu_tx_ready())
		;
#endif
	COM_OUTREG = c;
#ifndef ENABLE_SYNC_TX
	COM->USART.INTENSET.bit.DRE = 1; // enable recieved interrupt
#endif
#endif
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
#if (INTERFACE == INTERFACE_USB)
	while (!tud_cdc_available())
	{
		tud_task();
	}

	return (unsigned char)tud_cdc_read_char();
#else
#if (INTERFACE == INTERFACE_UART)
#ifdef ENABLE_SYNC_RX
	while (!mcu_rx_ready())
		;
#endif
	return (char)(0xff & COM_INREG);
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
	if (frequency < F_STEP_MIN)
		frequency = F_STEP_MIN;
	if (frequency > F_STEP_MAX)
		frequency = F_STEP_MAX;

	float clockcounter = 1000000;
	frequency *= 2.0f;

	if (frequency >= 16)
	{
		*prescaller = 0;
	}
	else if (frequency >= 8)
	{
		*prescaller = 1;
		clockcounter *= 0.5;
	}
	else if (frequency >= 4)
	{
		*prescaller = 2;
		clockcounter *= 0.25;
	}
	else if (frequency >= 2)
	{
		*prescaller = 3;
		clockcounter *= 0.125;
	}
	else if (frequency >= 1)
	{
		*prescaller = 4;
		clockcounter *= 0.0625;
	}
	else
	{
		*prescaller = 7;
		clockcounter *= 0.0009765625;
	}

	*ticks = floorf((clockcounter / frequency)) - 1;
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

void mcu_delay_us(uint8_t delay)
{
	uint32_t loops;
	if (!delay)
	{
		return;
	}
	else
	{
		loops = (delay * (F_CPU / 1000000) / 6) - 2;
	}
	while (loops--)
		asm("nop");
}

/**
 * runs all internal tasks of the MCU.
 * for the moment these are:
 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
 *   - if ENABLE_SYNC_RX is enabled check if there are any chars in the rx transmitter (or the tinyUSB buffer) and read them to the mcu_com_rx_cb
 *   - if ENABLE_SYNC_TX is enabled check if serial_tx_empty is false and run mcu_com_tx_cb
 * */
void mcu_dotasks(void)
{
#if (INTERFACE == INTERFACE_USB)
	tud_cdc_write_flush();
	tud_task(); // tinyusb device task
#endif
#ifdef ENABLE_SYNC_RX
	while (mcu_rx_ready())
	{
		unsigned char c = mcu_getc();
		mcu_com_rx_cb(c);
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

#ifdef MCU_HAS_I2C
/**
 * https://www.eevblog.com/forum/microcontrollers/i2c-atmel/
 * */
#ifndef mcu_i2c_write
uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop)
{
	if (send_start)
	{
		I2CCOM->I2CM.ADDR.reg = data;
	}
	else
	{
		I2CCOM->I2CM.DATA.reg = data;
	}

	while (0 == (I2C_I2CCOMSERCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB))
		;

	if (I2CCOM->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
	{
		I2CCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
		return 0;
	}

	if (stop)
	{
		I2CCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
	}

	return 1;
}
#endif

#ifndef mcu_i2c_read
uint8_t mcu_i2c_read(bool with_ack, bool send_stop)
{
	if (with_ack)
	{
		I2CCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
	}
	else
	{
		I2CCOM->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
	}

	while (0 == (I2CCOM->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB))
		;

	uint8_t data = I2CCOM->I2CM.DATA.reg;

	if (stop)
	{
		I2CCOM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
	}

	return c;
}
#endif
#endif

#endif
