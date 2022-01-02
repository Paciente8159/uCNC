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
#include "../../../interface/serial.h"
#include "../../../core/interpolator.h"
#include "../../../core/io_control.h"

#include "sam.h"
//#include "instance/nvmctrl.h"
#include <string.h>

//Non volatile memory
//SAMD devices page size never exceeds 1024 bytes
#define NVM_EEPROM_SIZE 0x400 //1Kb of emulated EEPROM is enough
#define NVM_PAGE_SIZE NVMCTRL_PAGE_SIZE
#define NVM_ROW_PAGES NVMCTRL_ROW_PAGES
#define NVM_ROW_SIZE NVMCTRL_ROW_SIZE
#define NVM_EEPROM_ROWS ((uint8_t)ceil(NVM_EEPROM_SIZE / NVMCTRL_ROW_SIZE))
#define NVM_EEPROM_BASE (FLASH_ADDR + NVMCTRL_FLASH_SIZE - (NVM_EEPROM_ROWS * NVMCTRL_ROW_SIZE))
#define NVM_MEMORY ((volatile uint16_t *)FLASH_ADDR)

#ifdef USB_VCP
#include "../../../tinyusb/tusb_config.h"
#include "../../../tinyusb/src/tusb.h"
#endif

volatile bool samd21_global_isr_enabled;

//setups internal timers (all will run @ 1Mhz on GCLK4)
#define MAIN_CLOCK_DIV ((uint16_t)(F_CPU/1000000))
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
        //ADC clock
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK4 | GCLK_CLKCTRL_ID_ADC;
        /* Wait for the write to complete. */
        while (GCLK->STATUS.bit.SYNCBUSY)
                ;

        //adc reset
        ADC->CTRLA.bit.SWRST = 1;
        while (ADC->STATUS.bit.SYNCBUSY)
                ;
        //set resolution
        ADC->CTRLB.bit.RESSEL = ADC_CTRLB_RESSEL_8BIT_Val;
        ADC->CTRLB.bit.PRESCALER = ADC_CTRLB_PRESCALER_DIV32_Val;
        while (ADC->STATUS.bit.SYNCBUSY)
                ;

        //set ref voltage
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
        ADC->INPUTCTRL.bit.MUXNEG = 0x18; //select internal ground
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
        static bool running = false;
        mcu_disable_global_isr();
        if (running)
        {
                EIC->INTFLAG.reg = SAMD21_EIC_MASK;
                mcu_enable_global_isr();
                return;
        }

        running = true;
        mcu_enable_global_isr();

#if (LIMITS_EICMASK != 0)
        if (EIC->INTFLAG.reg & LIMITS_EICMASK)
        {
                io_limits_isr();
        }
#endif
#if (CONTROLS_EICMASK != 0)
        if (EIC->INTFLAG.reg & CONTROLS_EICMASK)
        {
                io_controls_isr();
        }
#endif
#if (PROBE_EICMASK != 0)
        if (EIC->INTFLAG.reg & PROBE_EICMASK && mcu_probe_isr_enabled)
        {
                io_probe_isr();
        }
#endif
#if (DIN_IO_EICMASK != 0)
        if (EIC->INTFLAG.reg & DIN_IO_EICMASK)
        {
                io_inputs_isr();
        }
#endif

        //clears interrupt flags
        mcu_disable_global_isr();
        running = false;
        EIC->INTFLAG.reg = SAMD21_EIC_MASK;
        mcu_enable_global_isr();
}
#endif

void mcu_timer_isr(void)
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
                        itp_step_isr();
                else
                        itp_step_reset_isr();
                resetstep = !resetstep;
        }

        mcu_enable_global_isr();
}

#ifdef COM_PORT
void mcu_com_isr()
{
        mcu_disable_global_isr();
#ifndef ENABLE_SYNC_RX
        if (COM->USART.INTFLAG.bit.RXC && COM->USART.INTENSET.bit.RXC)
        {
                COM->USART.INTFLAG.bit.RXC = 1;
                unsigned char c = (0xff & COM_INREG);
                serial_rx_isr(c);
        }
#endif
#ifndef ENABLE_SYNC_TX
        if (COM->USART.INTFLAG.bit.DRE && COM->USART.INTENSET.bit.DRE)
        {
                COM->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
                serial_tx_isr();
        }
#endif
        mcu_enable_global_isr();
}
#endif

void mcu_usart_init(void)
{
#ifdef COM_PORT
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
        COM->USART.CTRLA.bit.SAMPR = 0;         //16x sample rate
        COM->USART.CTRLA.bit.FORM = 0;          //no parity
        COM->USART.CTRLA.bit.DORD = 1;          //LSB first
        COM->USART.CTRLA.bit.RXPO = COM_RX_PAD; // RX on PAD3
        COM->USART.CTRLA.bit.TXPO = COM_TX_PAD; // TX on PAD2
        COM->USART.CTRLB.bit.SBMODE = 0;        //one stop bit
        COM->USART.CTRLB.bit.CHSIZE = 0;        //8 bits
        COM->USART.CTRLB.bit.RXEN = 1;          // enable receiver
        COM->USART.CTRLB.bit.TXEN = 1;          // enable transmitter

        while (COM->USART.SYNCBUSY.bit.CTRLB)
                ;

        uint16_t baud = (uint16_t)(65536.0f * (1.0f - (((float)BAUDRATE) / (F_CPU >> 4))));

        COM->USART.BAUD.reg = baud;
        mcu_config_altfunc(TX);
        mcu_config_altfunc(RX);

#ifndef ENABLE_SYNC_RX
        COM->USART.INTENSET.bit.RXC = 1; //enable recieved interrupt
        COM->USART.INTENSET.bit.ERROR = 1;
#endif
        NVIC_ClearPendingIRQ(COM_IRQ);
        NVIC_EnableIRQ(COM_IRQ);
        NVIC_SetPriority(COM_IRQ, 0);

        //enable COM
        COM->USART.CTRLA.bit.ENABLE = 1;
        while (COM->USART.SYNCBUSY.bit.ENABLE)
                ;

#endif
#ifdef USB_VCP
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
        //USB->DEVICE.CTRLB.reg |= USB_DEVICE_CTRLB_SPDCONF_FS;
        while (USB->DEVICE.SYNCBUSY.bit.SWRST)
                ;
        tusb_init();
#endif
}

#ifdef USB_VCP
void USB_Handler(void)
{
        mcu_disable_global_isr();
        tud_int_handler(0);
        mcu_enable_global_isr();
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
        mcu_runtime_ms++;
        cnc_scheduletasks();
}

void mcu_tick_init()
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
        mcu_get_analog(ANALOG0);
#endif
#if ANALOG1 >= 0
        mcu_config_analog(ANALOG1);
        mcu_get_analog(ANALOG1);
#endif
#if ANALOG2 >= 0
        mcu_config_analog(ANALOG2);
        mcu_get_analog(ANALOG2);
#endif
#if ANALOG3 >= 0
        mcu_config_analog(ANALOG3);
        mcu_get_analog(ANALOG3);
#endif
#if ANALOG4 >= 0
        mcu_config_analog(ANALOG4);
        mcu_get_analog(ANALOG4);
#endif
#if ANALOG5 >= 0
        mcu_config_analog(ANALOG5);
        mcu_get_analog(ANALOG5);
#endif
#if ANALOG6 >= 0
        mcu_config_analog(ANALOG6);
        mcu_get_analog(ANALOG6);
#endif
#if ANALOG7 >= 0
        mcu_config_analog(ANALOG7);
        mcu_get_analog(ANALOG7);
#endif
#if ANALOG8 >= 0
        mcu_config_analog(ANALOG8);
        mcu_get_analog(ANALOG8);
#endif
#if ANALOG9 >= 0
        mcu_config_analog(ANALOG9);
        mcu_get_analog(ANALOG9);
#endif
#if ANALOG10 >= 0
        mcu_config_analog(ANALOG10);
        mcu_get_analog(ANALOG10);
#endif
#if ANALOG11 >= 0
        mcu_config_analog(ANALOG11);
        mcu_get_analog(ANALOG11);
#endif
#if ANALOG12 >= 0
        mcu_config_analog(ANALOG12);
        mcu_get_analog(ANALOG12);
#endif
#if ANALOG13 >= 0
        mcu_config_analog(ANALOG13);
        mcu_get_analog(ANALOG13);
#endif
#if ANALOG14 >= 0
        mcu_config_analog(ANALOG14);
        mcu_get_analog(ANALOG14);
#endif
#if ANALOG15 >= 0
        mcu_config_analog(ANALOG15);
        mcu_get_analog(ANALOG15);
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
        mcu_enable_global_isr();
}

/*IO functions*/

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
} //Start async send
#endif

/**
 * checks if the serial hardware of the MCU has a new char ready to be read
 * */
#ifndef mcu_rx_ready
bool mcu_rx_ready(void)
{
        return false;
} //Stop async send
#endif

/**
 * sends a char either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_putc
void mcu_putc(char c)
{
#ifdef LED
        mcu_toggle_output(LED);
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
#else
#ifdef COM
        if (c != 0)
        {
#ifdef ENABLE_SYNC_TX
                while (!mcu_tx_ready())
                        ;
#endif
                COM_OUTREG = c;
        }
#ifndef ENABLE_SYNC_TX
        COM->USART.INTENSET.bit.DRE = 1; //enable recieved interrupt
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
#ifdef LED
        mcu_toggle_output(LED);
#endif
#ifdef USB_VCP
        while (!tud_cdc_available())
        {
                tud_task();
        }

        return (unsigned char)tud_cdc_read_char();
#else
#ifdef COM
#ifdef ENABLE_SYNC_RX
        while (!mcu_rx_ready())
                ;
#endif
        return (char)(0xff & COM_INREG);
#endif
#endif
}
#endif

//ISR
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

//Step interpolator
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
        //reset timer
        ITP_REG->CTRLA.bit.SWRST = 1;
        while (ITP_REG->SYNCBUSY.bit.SWRST)
                ;
        //enable the timer in the APB
        ITP_REG->CTRLA.bit.PRESCALER = (uint8_t)prescaller; //normal counter
        ITP_REG->WAVE.bit.WAVEGEN = 1;                      // match compare
        while (ITP_REG->SYNCBUSY.bit.WAVE)
                ;
        ITP_REG->CC[0].reg = ticks;
        while (ITP_REG->SYNCBUSY.bit.CC0)
                ;

        NVIC_SetPriority(ITP_IRQ, 1);
        NVIC_ClearPendingIRQ(ITP_IRQ);
        NVIC_EnableIRQ(ITP_IRQ);

        ITP_REG->INTENSET.bit.MC0 = 1;
        ITP_REG->CTRLA.bit.ENABLE = 1; //enable timer and also write protection
        while (ITP_REG->SYNCBUSY.bit.ENABLE)
                ;
#else
        //reset timer
        ITP_REG->COUNT16.CTRLA.bit.SWRST = 1;
        while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
                ;
        //enable the timer in the APB
        ITP_REG->COUNT16.CTRLA.bit.PRESCALER = (uint8_t)prescaller; //normal counter
        ITP_REG->COUNT16.CTRLA.bit.WAVEGEN = 1;                     // match compare
        while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
                ;
        ITP_REG->COUNT16.CC[0].reg = ticks;
        while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
                ;
        NVIC_SetPriority(ITP_IRQ, 1);
        NVIC_ClearPendingIRQ(ITP_IRQ);
        NVIC_EnableIRQ(ITP_IRQ);

        ITP_REG->COUNT16.INTENSET.bit.MC0 = 1;
        ITP_REG->COUNT16.CTRLA.bit.ENABLE = 1; //enable timer and also write protection
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
        ITP_REG->CTRLA.bit.ENABLE = 0; //disable timer and also write protection
        while (ITP_REG->SYNCBUSY.bit.ENABLE)
                ;
        ITP_REG->CTRLA.bit.PRESCALER = (uint8_t)prescaller; //normal counter
        ITP_REG->CC[0].bit.CC = ticks;
        while (ITP_REG->SYNCBUSY.bit.CC0)
                ;
        ITP_REG->CTRLA.bit.ENABLE = 1; //enable timer and also write protection
        while (ITP_REG->SYNCBUSY.bit.ENABLE)
                ;
#else
        ITP_REG->COUNT16.CTRLA.bit.ENABLE = 0; //disable timer and also write protection
        while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
                ;
        ITP_REG->COUNT16.CTRLA.bit.PRESCALER = (uint8_t)prescaller; //normal counter
        ITP_REG->COUNT16.CC[0].bit.CC = ticks;
        while (ITP_REG->COUNT16.STATUS.bit.SYNCBUSY)
                ;
        ITP_REG->COUNT16.CTRLA.bit.ENABLE = 1; //enable timer and also write protection
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
        ITP_REG->CTRLA.bit.ENABLE = 0; //disable timer and also write protection
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

/**
	 * runs all internal tasks of the MCU.
	 * for the moment these are:
	 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
	 *   - if ENABLE_SYNC_RX is enabled check if there are any chars in the rx transmitter (or the tinyUSB buffer) and read them to the serial_rx_isr
	 *   - if ENABLE_SYNC_TX is enabled check if serial_tx_empty is false and run serial_tx_isr
	 * */
void mcu_dotasks(void)
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

static uint8_t samd21_eeprom_sram[NVM_EEPROM_SIZE]; //1kb max
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

        //Execute "WP" Write Page
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
        address &= (NVM_EEPROM_SIZE - 1); //keep within 1Kb address range

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

                //update rows
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
                                //set the flash address to erase/write half-word (datasheet 22.8.8)
                                NVMCTRL->ADDR.reg = (uintptr_t)&NVM_MEMORY[(NVM_EEPROM_BASE + eeprom_offset) / 4];
                                while (!NVMCTRL->INTFLAG.bit.READY)
                                        ;

                                NVMCTRL->STATUS.reg |= NVMCTRL_STATUS_MASK;

                                //erase region for writing
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

                NVMCTRL->CTRLB.bit.CACHEDIS = 1;
                NVMCTRL->CTRLB.bit.RWS = 0x01;
        }

        samd21_flash_modified = false;
}

#endif
