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

#ifdef __cplusplus
extern "C"
{
#endif

#include "cnc.h"

#if (MCU == MCU_SAMD21)
#include "core_cm0plus.h"
#include "mcumap_samd21.h"
#include "interface/serial.h"
#include "core/interpolator.h"
#include "core/io_control.h"
#include "modules/pid_controller.h"

#include "sam.h"
#include "instance/nvmctrl.h"

#ifdef USB_VCP
#include "tusb_config.h"
#include "tusb.h"
#endif

        //setups internal timers (all will run @ 1Mhz on GCLK4)
        static void mcu_setup_clocks(void)
        {
                PM->APBCMASK.reg |= (PM_APBCMASK_TCC0 | PM_APBCMASK_TCC1 | PM_APBCMASK_TCC2 | PM_APBCMASK_TC3 | PM_APBCMASK_TC4 | PM_APBCMASK_TC5 | PM_APBCMASK_TC6 | PM_APBCMASK_TC7);

                /* Configure GCLK4's divider - in this case, divided by 1 */
                GCLK->GENDIV.reg = GCLK_GENDIV_ID(4) | GCLK_GENDIV_DIV(48);

                while (GCLK->STATUS.bit.SYNCBUSY)
                        ;

                /* Setup GCLK4 using the DFLL @48Mhz */
                GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(4) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_OE;
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

                PORT->Group[0].DIRSET.reg = (1 << 10);
                PORT->Group[0].PINCFG[10].reg |= PORT_PINCFG_PMUXEN;
                PORT->Group[0].PMUX[10 >> 1].bit.PMUXE |= PORT_PMUX_PMUXE_H;
        }

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

                NVIC_ClearPendingIRQ(ITP_IRQ);
                mcu_enable_global_isr();
        }

        void mcu_usart_init(void)
        {
#ifdef COM_PORT

#endif
#ifdef USB_VCP
                PM->AHBMASK.reg |= PM_AHBMASK_USB;

                mcu_config_input(USB_DM);
                mcu_config_input(USB_DP);
                mcu_config_altfunc(USB_DM);
                mcu_config_altfunc(USB_DP);
                NVIC_EnableIRQ(USB_IRQn);
                NVIC_SetPriority(USB_IRQn, 10);
                NVIC_ClearPendingIRQ(USB_IRQn);

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
                NVIC_ClearPendingIRQ(USB_IRQn);
                mcu_enable_global_isr();
        }
#endif

        /**
	 * The internal clock counter
	 * Increments every millisecond
	 * Can count up to almost 50 days
	 **/
        static volatile uint32_t mcu_runtime_ms;

        void SysTick_Handler(void)
        {
                mcu_runtime_ms++;
                mcu_enable_global_isr();
                pid_update_isr();
#ifdef LED
                static uint32_t last_ms = 0;
                if (mcu_runtime_ms - last_ms > 1000)
                {
                        last_ms = mcu_runtime_ms;
                        mcu_toggle_output(LED);
                }
#endif
                NVIC_ClearPendingIRQ(SysTick_IRQn);
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
                mcu_setup_clocks();
#ifdef STEP0
                mcu_config_output(STEP0);
#endif
#ifdef STEP1
                mcu_config_output(STEP1);
#endif
#ifdef STEP2
                mcu_config_output(STEP2);
#endif
#ifdef STEP3
                mcu_config_output(STEP3);
#endif
#ifdef STEP4
                mcu_config_output(STEP4);
#endif
#ifdef STEP5
                mcu_config_output(STEP5);
#endif
#ifdef STEP6
                mcu_config_output(STEP6);
#endif
#ifdef STEP7
                mcu_config_output(STEP7);
#endif
#ifdef DIR0
                mcu_config_output(DIR0);
#endif
#ifdef DIR1
                mcu_config_output(DIR1);
#endif
#ifdef DIR2
                mcu_config_output(DIR2);
#endif
#ifdef DIR3
                mcu_config_output(DIR3);
#endif
#ifdef DIR4
                mcu_config_output(DIR4);
#endif
#ifdef DIR5
                mcu_config_output(DIR5);
#endif
#ifdef STEP0_EN
                mcu_config_output(STEP0_EN);
#endif
#ifdef STEP1_EN
                mcu_config_output(STEP1_EN);
#endif
#ifdef STEP2_EN
                mcu_config_output(STEP2_EN);
#endif
#ifdef STEP3_EN
                mcu_config_output(STEP3_EN);
#endif
#ifdef STEP4_EN
                mcu_config_output(STEP4_EN);
#endif
#ifdef STEP5_EN
                mcu_config_output(STEP5_EN);
#endif
#ifdef PWM0
                mcu_config_pwm(PWM0);
#endif
#ifdef PWM1
                mcu_config_pwm(PWM1);
#endif
#ifdef PWM2
                mcu_config_pwm(PWM2);
#endif
#ifdef PWM3
                mcu_config_pwm(PWM3);
#endif
#ifdef PWM4
                mcu_config_pwm(PWM4);
#endif
#ifdef PWM5
                mcu_config_pwm(PWM5);
#endif
#ifdef PWM6
                mcu_config_pwm(PWM6);
#endif
#ifdef PWM7
                mcu_config_pwm(PWM7);
#endif
#ifdef PWM8
                mcu_config_pwm(PWM8);
#endif
#ifdef PWM9
                mcu_config_pwm(PWM9);
#endif
#ifdef PWM10
                mcu_config_pwm(PWM10);
#endif
#ifdef PWM11
                mcu_config_pwm(PWM11);
#endif
#ifdef PWM12
                mcu_config_pwm(PWM12);
#endif
#ifdef PWM13
                mcu_config_pwm(PWM13);
#endif
#ifdef PWM14
                mcu_config_pwm(PWM14);
#endif
#ifdef PWM15
                mcu_config_pwm(PWM15);
#endif
#ifdef DOUT0
                mcu_config_output(DOUT0);
#endif
#ifdef DOUT1
                mcu_config_output(DOUT1);
#endif
#ifdef DOUT2
                mcu_config_output(DOUT2);
#endif
#ifdef DOUT3
                mcu_config_output(DOUT3);
#endif
#ifdef DOUT4
                mcu_config_output(DOUT4);
#endif
#ifdef DOUT5
                mcu_config_output(DOUT5);
#endif
#ifdef DOUT6
                mcu_config_output(DOUT6);
#endif
#ifdef DOUT7
                mcu_config_output(DOUT7);
#endif
#ifdef DOUT8
                mcu_config_output(DOUT8);
#endif
#ifdef DOUT9
                mcu_config_output(DOUT9);
#endif
#ifdef DOUT10
                mcu_config_output(DOUT10);
#endif
#ifdef DOUT11
                mcu_config_output(DOUT11);
#endif
#ifdef DOUT12
                mcu_config_output(DOUT12);
#endif
#ifdef DOUT13
                mcu_config_output(DOUT13);
#endif
#ifdef DOUT14
                mcu_config_output(DOUT14);
#endif
#ifdef DOUT15
                mcu_config_output(DOUT15);
#endif
#ifdef LIMIT_X
                mcu_config_input(LIMIT_X);
#ifdef LIMIT_X_PULLUP
                mcu_config_pullup(LIMIT_X);
#endif
#ifdef LIMIT_X_ISR
                mcu_config_input_isr(LIMIT_X);
#endif
#endif
#ifdef LIMIT_Y
                mcu_config_input(LIMIT_Y);
#ifdef LIMIT_Y_PULLUP
                mcu_config_pullup(LIMIT_Y);
#endif
#ifdef LIMIT_Y_ISR
                mcu_config_input_isr(LIMIT_Y);
#endif
#endif
#ifdef LIMIT_Z
                mcu_config_input(LIMIT_Z);
#ifdef LIMIT_Z_PULLUP
                mcu_config_pullup(LIMIT_Z);
#endif
#ifdef LIMIT_Z_ISR
                mcu_config_input_isr(LIMIT_Z);
#endif
#endif
#ifdef LIMIT_X2
                mcu_config_input(LIMIT_X2);
#ifdef LIMIT_X2_PULLUP
                mcu_config_pullup(LIMIT_X2);
#endif
#ifdef LIMIT_X2_ISR
                mcu_config_input_isr(LIMIT_X2);
#endif
#endif
#ifdef LIMIT_Y2
                mcu_config_input(LIMIT_Y2);
#ifdef LIMIT_Y2_PULLUP
                mcu_config_pullup(LIMIT_Y2);
#endif
#ifdef LIMIT_Y2_ISR
                mcu_config_input_isr(LIMIT_Y2);
#endif
#endif
#ifdef LIMIT_Z2
                mcu_config_input(LIMIT_Z2);
#ifdef LIMIT_Z2_PULLUP
                mcu_config_pullup(LIMIT_Z2);
#endif
#ifdef LIMIT_Z2_ISR
                mcu_config_input_isr(LIMIT_Z2);
#endif
#endif
#ifdef LIMIT_A
                mcu_config_input(LIMIT_A);
#ifdef LIMIT_A_PULLUP
                mcu_config_pullup(LIMIT_A);
#endif
#ifdef LIMIT_A_ISR
                mcu_config_input_isr(LIMIT_A);
#endif
#endif
#ifdef LIMIT_B
                mcu_config_input(LIMIT_B);
#ifdef LIMIT_B_PULLUP
                mcu_config_pullup(LIMIT_B);
#endif
#ifdef LIMIT_B_ISR
                mcu_config_input_isr(LIMIT_B);
#endif
#endif
#ifdef LIMIT_C
                mcu_config_input(LIMIT_C);
#ifdef LIMIT_C_PULLUP
                mcu_config_pullup(LIMIT_C);
#endif
#ifdef LIMIT_C_ISR
                mcu_config_input_isr(LIMIT_C);
#endif
#endif
#ifdef PROBE
                mcu_config_input(PROBE);
#ifdef PROBE_PULLUP
                mcu_config_pullup(PROBE);
#endif
#ifdef PROBE_ISR
                mcu_config_input_isr(PROBE);
#endif
#endif
#ifdef ESTOP
                mcu_config_input(ESTOP);
#ifdef ESTOP_PULLUP
                mcu_config_pullup(ESTOP);
#endif
#ifdef ESTOP_ISR
                mcu_config_input_isr(ESTOP);
#endif
#endif
#ifdef SAFETY_DOOR
                mcu_config_input(SAFETY_DOOR);
#ifdef SAFETY_DOOR_PULLUP
                mcu_config_pullup(SAFETY_DOOR);
#endif
#ifdef SAFETY_DOOR_ISR
                mcu_config_input_isr(SAFETY_DOOR);
#endif
#endif
#ifdef FHOLD
                mcu_config_input(FHOLD);
#ifdef FHOLD_PULLUP
                mcu_config_pullup(FHOLD);
#endif
#ifdef FHOLD_ISR
                mcu_config_input_isr(FHOLD);
#endif
#endif
#ifdef CS_RES
                mcu_config_input(CS_RES);
#ifdef CS_RES_PULLUP
                mcu_config_pullup(CS_RES);
#endif
#ifdef CS_RES_ISR
                mcu_config_input_isr(CS_RES);
#endif
#endif
#ifdef ANALOG0
                mcu_config_input(ANALOG0);
#endif
#ifdef ANALOG1
                mcu_config_input(ANALOG1);
#endif
#ifdef ANALOG2
                mcu_config_input(ANALOG2);
#endif
#ifdef ANALOG3
                mcu_config_input(ANALOG3);
#endif
#ifdef ANALOG4
                mcu_config_input(ANALOG4);
#endif
#ifdef ANALOG5
                mcu_config_input(ANALOG5);
#endif
#ifdef ANALOG6
                mcu_config_input(ANALOG6);
#endif
#ifdef ANALOG7
                mcu_config_input(ANALOG7);
#endif
#ifdef ANALOG8
                mcu_config_input(ANALOG8);
#endif
#ifdef ANALOG9
                mcu_config_input(ANALOG9);
#endif
#ifdef ANALOG10
                mcu_config_input(ANALOG10);
#endif
#ifdef ANALOG11
                mcu_config_input(ANALOG11);
#endif
#ifdef ANALOG12
                mcu_config_input(ANALOG12);
#endif
#ifdef ANALOG13
                mcu_config_input(ANALOG13);
#endif
#ifdef ANALOG14
                mcu_config_input(ANALOG14);
#endif
#ifdef ANALOG15
                mcu_config_input(ANALOG15);
#endif
#ifdef DIN0
                mcu_config_input(DIN0);
#ifdef DIN0_PULLUP
                mcu_config_pullup(DIN0);
#endif
#ifdef DIN0_ISR
                mcu_config_input_isr(DIN0);
#endif
#endif
#ifdef DIN1
                mcu_config_input(DIN1);
#ifdef DIN1_PULLUP
                mcu_config_pullup(DIN1);
#endif
#ifdef DIN1_ISR
                mcu_config_input_isr(DIN1);
#endif
#endif
#ifdef DIN2
                mcu_config_input(DIN2);
#ifdef DIN2_PULLUP
                mcu_config_pullup(DIN2);
#endif
#ifdef DIN2_ISR
                mcu_config_input_isr(DIN2);
#endif
#endif
#ifdef DIN3
                mcu_config_input(DIN3);
#ifdef DIN3_PULLUP
                mcu_config_pullup(DIN3);
#endif
#ifdef DIN3_ISR
                mcu_config_input_isr(DIN3);
#endif
#endif
#ifdef DIN4
                mcu_config_input(DIN4);
#ifdef DIN4_PULLUP
                mcu_config_pullup(DIN4);
#endif
#ifdef DIN4_ISR
                mcu_config_input_isr(DIN4);
#endif
#endif
#ifdef DIN5
                mcu_config_input(DIN5);
#ifdef DIN5_PULLUP
                mcu_config_pullup(DIN5);
#endif
#ifdef DIN5_ISR
                mcu_config_input_isr(DIN5);
#endif
#endif
#ifdef DIN6
                mcu_config_input(DIN6);
#ifdef DIN6_PULLUP
                mcu_config_pullup(DIN6);
#endif
#ifdef DIN6_ISR
                mcu_config_input_isr(DIN6);
#endif
#endif
#ifdef DIN7
                mcu_config_input(DIN7);
#ifdef DIN7_PULLUP
                mcu_config_pullup(DIN7);
#endif
#ifdef DIN7_ISR
                mcu_config_input_isr(DIN7);
#endif
#endif
#ifdef DIN8
                mcu_config_input(DIN8);
#ifdef DIN8_PULLUP
                mcu_config_pullup(DIN8);
#endif
#endif
#ifdef DIN9
                mcu_config_input(DIN9);
#ifdef DIN9_PULLUP
                mcu_config_pullup(DIN9);
#endif
#endif
#ifdef DIN10
                mcu_config_input(DIN10);
#ifdef DIN10_PULLUP
                mcu_config_pullup(DIN10);
#endif
#endif
#ifdef DIN11
                mcu_config_input(DIN11);
#ifdef DIN11_PULLUP
                mcu_config_pullup(DIN11);
#endif
#endif
#ifdef DIN12
                mcu_config_input(DIN12);
#ifdef DIN12_PULLUP
                mcu_config_pullup(DIN12);
#endif
#endif
#ifdef DIN13
                mcu_config_input(DIN13);
#ifdef DIN13_PULLUP
                mcu_config_pullup(DIN13);
#endif
#endif
#ifdef DIN14
                mcu_config_input(DIN14);
#ifdef DIN14_PULLUP
                mcu_config_pullup(DIN14);
#endif
#endif
#ifdef DIN15
                mcu_config_input(DIN15);
#ifdef DIN15_PULLUP
                mcu_config_pullup(DIN15);
#endif
#endif
#ifdef TX
                mcu_config_output(TX);
#endif
#ifdef RX
                mcu_config_input(RX);
#endif
#ifdef USB_DM
                mcu_config_input(USB_DM);
#endif
#ifdef USB_DP
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
                NVIC_EnableIRQ(ITP_IRQ);
                NVIC_SetPriority(ITP_IRQ, 1);
                NVIC_ClearPendingIRQ(ITP_IRQ);
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
                NVIC_EnableIRQ(ITP_IRQ);
                NVIC_SetPriority(ITP_IRQ, 1);
                NVIC_ClearPendingIRQ(ITP_IRQ);
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

//Non volatile memory
//SAMD devices page size never exceeds 1024 bytes
#define NVM_EEPROM_SIZE 0x400 //1Kb of emulated EEPROM is enough
#define NVM_BASE_ADDRESS 0x00000000
#define NVM_PAGE_SIZE (8 * (1 + NVMCTRL->PARAM.bit.PSZ))
#define NVM_PAGE_COUNT (NVMCTRL->PARAM.bit.NVMP)
#define NVM_SIZE (NVM_PAGE_SIZE * NVM_PAGE_COUNT)
#define NVM_ROW_SIZE (NVM_PAGE_SIZE << 2) //Equivalent to a NVM Row = 4*pages
#define NVM_EEPROM_ROWS ((uint8_t)ceil(NVM_EEPROM_SIZE / NVM_ROW_SIZE))
#define NVM_EEPROM_BASE (NVM_BASE_ADDRESS + NVM_SIZE - (NVM_EEPROM_ROWS * NVM_ROW_SIZE))

        static uint8_t samd21_flash_row[NVM_EEPROM_SIZE]; //1kb max
        static bool samd21_flash_modified = false;
        static bool samd21_eeprom_loaded = false;

        /**
	 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
	 * */
        uint8_t mcu_eeprom_getc(uint16_t address)
        {
                uint16_t eepromsize = NVM_EEPROM_SIZE;
                uint32_t eeprombase = NVM_EEPROM_BASE;

                address &= (NVM_EEPROM_SIZE - 1); //keep within 1Kb address range

                if (!samd21_eeprom_loaded)
                {
                        memcpy(&samd21_flash_row[0], (const void *)(NVM_EEPROM_BASE), NVM_EEPROM_SIZE);
                        samd21_eeprom_loaded = true;
                        samd21_flash_modified = false;
                }

                return samd21_flash_row[address];
        }

        /**
	 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
	 * */
        void mcu_eeprom_putc(uint16_t address, uint8_t value)
        {
                address &= (NVM_EEPROM_SIZE - 1);

                if (!samd21_eeprom_loaded)
                {
                        memcpy(&samd21_flash_row[0], (const void *)(NVM_EEPROM_BASE), NVM_EEPROM_SIZE);
                        samd21_eeprom_loaded = true;
                        samd21_flash_modified = false;
                }

                if (samd21_flash_row[address] != value)
                {
                        samd21_flash_modified = true;
                }

                samd21_flash_row[address] = value;
        }

        /**
	 * flushes all recorded registers into the eeprom.
	 * */

        void mcu_eeprom_flush(void)
        {
                if (samd21_flash_modified)
                {
                        int32_t size = (int32_t)NVM_EEPROM_SIZE;
                        uint32_t eeprom_offset = 0;

                        volatile uint32_t *dst_addr = (volatile uint32_t *)((const volatile void *)NVM_EEPROM_BASE);
                        const uint32_t *src_addr = (uint32_t *)&samd21_flash_row[0];

                        mcu_disable_global_isr();

                        //update rows
                        // for (uint16_t r = 0; r < NVM_EEPROM_ROWS; r++)
                        // {
                        //         //row was modified
                        //         uint32_t cmpsize = (size > NVM_ROW_SIZE) ? NVM_ROW_SIZE : size;
                        //         if (memcmp(dst_addr, src_addr, cmpsize) != 0)
                        //         {
                        //                 // Enable automatic page write
                        //                 NVMCTRL->CTRLB.bit.MANW = 1;
                        //                 while (!NVMCTRL->INTFLAG.bit.READY)
                        //                         ;

                        //                 //set the flash address to erase/write half-word (datasheet 22.8.8)
                        //                 NVMCTRL->ADDR.reg = (((NVM_EEPROM_BASE + eeprom_offset) >> 1) & 0xFFFF);
                        //                 while (!NVMCTRL->INTFLAG.bit.READY)
                        //                         ;

                        //                 //erase region for writing
                        //                 NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
                        //                 while (!NVMCTRL->INTFLAG.bit.READY)
                        //                         ;

                        //                 //4 pages per NVM row
                        //                 for (uint8_t p = 0; p < 4; p++)
                        //                 {
                        //                         NVMCTRL->ADDR.reg = (((NVM_EEPROM_BASE + eeprom_offset) >> 1) & 0xFFFF);
                        //                         while (!NVMCTRL->INTFLAG.bit.READY)
                        //                                 ;

                        //                         // Execute "PBC" Page Buffer Clear
                        //                         NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
                        //                         while (!NVMCTRL->INTFLAG.bit.READY)
                        //                                 ;

                        //                         // Fill page buffer
                        //                         for (uint32_t i = 0; i < (NVM_PAGE_SIZE>>2); i++)
                        //                         {
                        //                                 *dst_addr = (size > 0) ? *src_addr : 0xffffffff;
                        //                                 dst_addr++;
                        //                                 src_addr++;
                        //                                 // Data boundaries of the eeprom in 16bit chuncks
                        //                                 eeprom_offset += 4;
                        //                                 size -= 4;
                        //                         }

                        //                         // Execute "WP" Write Page
                        //                         NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
                        //                         while (!NVMCTRL->INTFLAG.bit.READY)
                        //                                 ;
                        //                 }
                        //         }
                        //         else
                        //         {
                        //                 dst_addr += cmpsize;
                        //                 src_addr += cmpsize;
                        //                 eeprom_offset += cmpsize;
                        //                 size -= cmpsize;
                        //         }
                        // }

                        mcu_enable_global_isr();
                }

                samd21_flash_modified = false;
        }

#endif

#ifdef __cplusplus
}
#endif