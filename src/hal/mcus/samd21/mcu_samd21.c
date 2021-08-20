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

        //     static void mcu_setup_clocks(void)
        //     {
        //         /* Set the correct number of wait states for 48 MHz @ 3.3v */
        //         NVMCTRL->CTRLB.bit.RWS = 1;
        //         /* This works around a quirk in the hardware (errata 1.2.1) -
        //    the DFLLCTRL register must be manually reset to this value before
        //    configuration. */
        //         while (!SYSCTRL->PCLKSR.bit.DFLLRDY)
        //             ;
        //         SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;
        //         while (!SYSCTRL->PCLKSR.bit.DFLLRDY)
        //             ;

        //         /* Write the coarse and fine calibration from NVM. */
        //         uint32_t coarse =
        //             ((*(uint32_t *)SYSCTRL_FUSES_DFLL48M_COARSE_CAL_ADDR) & SYSCTRL_FUSES_DFLL48M_COARSE_CAL_Msk) >> SYSCTRL_FUSES_DFLL48M_COARSE_CAL_Pos;
        //         /*uint32_t fine =
        //             ((*(uint32_t *)FUSES_DFLL48M_FINE_CAL_ADDR) & FUSES_DFLL48M_FINE_CAL_Msk) >> FUSES_DFLL48M_FINE_CAL_Pos;*/

        //         SYSCTRL->DFLLVAL.reg = SYSCTRL_FUSES_DFLL48M_COARSE_CAL(coarse);

        //         /* Wait for the write to finish. */
        //         while (!SYSCTRL->PCLKSR.bit.DFLLRDY)
        //             ;

        // #ifdef USB_VCP
        //         SYSCTRL->DFLLCTRL.reg |=
        //             /* Enable USB clock recovery mode */
        //             SYSCTRL_DFLLCTRL_USBCRM |
        //             /* Disable chill cycle as per datasheet to speed up locking.
        //        This is specified in section 17.6.7.2.2, and chill cycles
        //        are described in section 17.6.7.2.1. */
        //             SYSCTRL_DFLLCTRL_CCDIS;

        //         /* Configure the DFLL to multiply the 1 kHz clock to 48 MHz */
        //         SYSCTRL->DFLLMUL.reg =
        //             /* This value is output frequency / reference clock frequency,
        //        so 48 MHz / 1 kHz */
        //             SYSCTRL_DFLLMUL_MUL(48000) |
        //             /* The coarse and fine values can be set to their minimum
        //        since coarse is fixed in USB clock recovery mode and
        //        fine should lock on quickly. */
        //             SYSCTRL_DFLLMUL_FSTEP(1) |
        //             SYSCTRL_DFLLMUL_CSTEP(1);
        //         /* Closed loop mode */
        //         SYSCTRL->DFLLCTRL.bit.MODE = 1;
        // #endif

        //         /* Enable the DFLL */
        //         SYSCTRL->DFLLCTRL.bit.ENABLE = 1;

        //         /* Wait for the write to finish */
        //         while (!SYSCTRL->PCLKSR.bit.DFLLRDY)
        //             ;

        //         /* Setup GCLK0 using the DFLL @ 48 MHz */
        //         GCLK->GENCTRL.reg =
        //             GCLK_GENCTRL_ID(0) |
        //             GCLK_GENCTRL_SRC_DFLL48M |
        //             /* Improve the duty cycle. */
        //             GCLK_GENCTRL_IDC |
        //             GCLK_GENCTRL_GENEN;

        //         /* Wait for the write to complete */
        //         while (GCLK->STATUS.bit.SYNCBUSY)
        //             ;
        //     }

        void mcu_usart_init(void)
        {
#ifdef COM_PORT

#endif
#ifdef USB_VCP
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
                {
                };
                PM->AHBMASK.reg |= PM_AHBMASK_USB;
                USB->DEVICE.INTENSET.reg = USB_DEVICE_EPINTENSET_MASK;
                USB->DEVICE.CTRLA.bit.ENABLE = 1;
                USB->DEVICE.CTRLA.bit.MODE = 0;
                USB->DEVICE.CTRLB.bit.SPDCONF = 0; //.reg &= ~USB_DEVICE_CTRLB_SPDCONF_Msk;
                //USB->DEVICE.CTRLB.reg |= USB_DEVICE_CTRLB_SPDCONF_FS;
                while (USB->DEVICE.SYNCBUSY.bit.SWRST)
                {
                };
                tusb_init();
#endif
        }

#ifdef USB_VCP
        void USB_Handler(void)
        {
                tud_int_handler(0);
                NVIC_ClearPendingIRQ(USB_IRQn);
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
#ifdef STEPPER_ENABLE0
                mcu_config_output(STEPPER_ENABLE0);
#endif
#ifdef STEPPER_ENABLE1
                mcu_config_output(STEPPER_ENABLE1);
#endif
#ifdef STEPPER_ENABLE2
                mcu_config_output(STEPPER_ENABLE2);
#endif
#ifdef STEPPER_ENABLE3
                mcu_config_output(STEPPER_ENABLE3);
#endif
#ifdef STEPPER_ENABLE4
                mcu_config_output(STEPPER_ENABLE4);
#endif
#ifdef STEPPER_ENABLE5
                mcu_config_output(STEPPER_ENABLE5);
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

                //mcu_setup_clocks();
                mcu_tick_init();
                mcu_usart_init();
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
        void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller) {}

        /**
	 * starts the timer interrupt that generates the step pulses for the interpolator
	 * */
        void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller) {}

        /**
	 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
	 * */
        void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller) {}

        /**
	 * stops the timer interrupt that generates the step pulses for the interpolator
	 * */
        void mcu_stop_itp_isr(void) {}

        /**
	 * gets the MCU running time in milliseconds.
	 * the time counting is controled by the internal RTC
	 * */
        uint32_t mcu_millis() { return 0; }

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
        /**
	 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
	 * */
        uint8_t mcu_eeprom_getc(uint16_t address) { return 0; }

        /**
	 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
	 * */
        void mcu_eeprom_putc(uint16_t address, uint8_t value) {}

        /**
	 * flushes all recorded registers into the eeprom.
	 * */
        void mcu_eeprom_flush(void) {}

#endif

#ifdef __cplusplus
}
#endif