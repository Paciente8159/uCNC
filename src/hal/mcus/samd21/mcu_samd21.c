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

#include "samd21.h"
#include "instance/nvmctrl.h"

    static void mcu_setup_clocks(void)
    {
        /* Set the correct number of wait states for 48 MHz @ 3.3v */
        NVMCTRL->CTRLB.bit.RWS = 1;
        /* This works around a quirk in the hardware (errata 1.2.1) -
   the DFLLCTRL register must be manually reset to this value before
   configuration. */
        while (!SYSCTRL->PCLKSR.bit.DFLLRDY)
            ;
        SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;
        while (!SYSCTRL->PCLKSR.bit.DFLLRDY)
            ;

        /* Write the coarse and fine calibration from NVM. */
        uint32_t coarse =
            ((*(uint32_t *)SYSCTRL_FUSES_DFLL48M_COARSE_CAL_ADDR) & SYSCTRL_FUSES_DFLL48M_COARSE_CAL_Msk) >> SYSCTRL_FUSES_DFLL48M_COARSE_CAL_Pos;
        /*uint32_t fine =
            ((*(uint32_t *)FUSES_DFLL48M_FINE_CAL_ADDR) & FUSES_DFLL48M_FINE_CAL_Msk) >> FUSES_DFLL48M_FINE_CAL_Pos;*/

        SYSCTRL->DFLLVAL.reg = SYSCTRL_FUSES_DFLL48M_COARSE_CAL(coarse);

        /* Wait for the write to finish. */
        while (!SYSCTRL->PCLKSR.bit.DFLLRDY)
            ;

#ifdef USB_VCP
        SYSCTRL->DFLLCTRL.reg |=
            /* Enable USB clock recovery mode */
            SYSCTRL_DFLLCTRL_USBCRM |
            /* Disable chill cycle as per datasheet to speed up locking.
       This is specified in section 17.6.7.2.2, and chill cycles
       are described in section 17.6.7.2.1. */
            SYSCTRL_DFLLCTRL_CCDIS;

        /* Configure the DFLL to multiply the 1 kHz clock to 48 MHz */
        SYSCTRL->DFLLMUL.reg =
            /* This value is output frequency / reference clock frequency,
       so 48 MHz / 1 kHz */
            SYSCTRL_DFLLMUL_MUL(48000) |
            /* The coarse and fine values can be set to their minimum
       since coarse is fixed in USB clock recovery mode and
       fine should lock on quickly. */
            SYSCTRL_DFLLMUL_FSTEP(1) |
            SYSCTRL_DFLLMUL_CSTEP(1);
        /* Closed loop mode */
        SYSCTRL->DFLLCTRL.bit.MODE = 1;
#endif

        /* Enable the DFLL */
        SYSCTRL->DFLLCTRL.bit.ENABLE = 1;

        /* Wait for the write to finish */
        while (!SYSCTRL->PCLKSR.bit.DFLLRDY)
            ;

        /* Setup GCLK0 using the DFLL @ 48 MHz */
        GCLK->GENCTRL.reg =
            GCLK_GENCTRL_ID(0) |
            GCLK_GENCTRL_SRC_DFLL48M |
            /* Improve the duty cycle. */
            GCLK_GENCTRL_IDC |
            GCLK_GENCTRL_GENEN;

        /* Wait for the write to complete */
        while (GCLK->STATUS.bit.SYNCBUSY)
            ;
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
    }
#endif

/**
 * gets a char either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_getc
    char mcu_getc(void)
    {
        return 0;
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
    void mcu_dotasks(void) {}

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