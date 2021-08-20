/**
 * start up file for SAMD21
 * This is directly derived from the SMT32F1 startup file
 */

#include "sam.h"

// /* Initialize segments */
typedef void (*const intfunc)(void);

#define WEAK __attribute__((weak))
extern unsigned long _svtor;
extern unsigned long _etext;
extern unsigned long _sidata;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sbss;
extern unsigned long _ebss;
extern unsigned long _estack;

void Reset_Handler(void) __attribute__((__interrupt__));
void __Init_Data(void);
void SystemInit(void);
void Default_Handler(void);
extern void main(void);

/* Init Data
 * Loads data from addresses defined in linker file into RAM
 * Zero bss (statically allocated uninitialized variables)
 */
void __Init_Data(void)
{
        unsigned long *src, *dst;
        /* copy the data segment into ram */
        src = &_sidata;
        dst = &_sdata;
        if (src != dst)
                while (dst < &_edata)
                        *(dst++) = *(src++);

        /* zero the bss segment */
        dst = &_sbss;
        while (dst < &_ebss)
                *(dst++) = 0;
}

/* This function is straight from the system_stm32f10x.c library file and
 * is called within the startup file:
 * 1. After each device reset the HSI is used as System clock source.
 * 2. This function assumes that an external 8MHz crystal is used to drive the System clock.
 */
void SystemInit(void)
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
            ((*(uint32_t *)FUSES_DFLL48M_COARSE_CAL_ADDR) & FUSES_DFLL48M_COARSE_CAL_Msk) >> FUSES_DFLL48M_COARSE_CAL_Pos;
        uint32_t fine =
            ((*(uint32_t *)FUSES_DFLL48M_FINE_CAL_ADDR) & FUSES_DFLL48M_FINE_CAL_Msk) >> FUSES_DFLL48M_FINE_CAL_Pos;

        SYSCTRL->DFLLVAL.reg = SYSCTRL_DFLLVAL_COARSE(coarse) | SYSCTRL_DFLLVAL_FINE(fine);

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

#ifdef USE_VCP
        USB->DEVICE.QOSCTRL.bit.CQOS = 2;
        USB->DEVICE.QOSCTRL.bit.DQOS = 2;
#endif
}

void Reset_Handler(void)
{
        /* Initialize data and bss */
        __Init_Data();
        unsigned long *pSrc = (unsigned long *)&_svtor;
        SCB->VTOR = ((unsigned long)pSrc & SCB_VTOR_TBLOFF_Msk);
        SystemInit();
        while (1)
        {
                main();
        }
        while (1)
        {
        }
}

void WEAK NMI_Handler(void);
void WEAK HardFault_Handler(void);
void WEAK SVC_Handler(void);
void WEAK PendSV_Handler(void);
void WEAK SysTick_Handler(void);
void WEAK PM_Handler(void);
void WEAK SYSCTRL_Handler(void);
void WEAK WDT_Handler(void);
void WEAK RTC_Handler(void);
void WEAK EIC_Handler(void);
void WEAK NVMCTRL_Handler(void);
void WEAK DMAC_Handler(void);
void WEAK USB_Handler(void);
void WEAK EVSYS_Handler(void);
void WEAK SERCOM0_Handler(void);
void WEAK SERCOM1_Handler(void);
void WEAK SERCOM2_Handler(void);
void WEAK SERCOM3_Handler(void);
void WEAK SERCOM4_Handler(void);
void WEAK SERCOM5_Handler(void);
void WEAK TCC0_Handler(void);
void WEAK TCC1_Handler(void);
void WEAK TCC2_Handler(void);
void WEAK TC3_Handler(void);
void WEAK TC4_Handler(void);
void WEAK TC5_Handler(void);
void WEAK TC6_Handler(void);
void WEAK TC7_Handler(void);
void WEAK ADC_Handler(void);
void WEAK AC_Handler(void);
void WEAK DAC_Handler(void);
void WEAK PTC_Handler(void);
void WEAK I2S_Handler(void);

__attribute__((section(".isr_vector"))) void (*const g_pfnVectors[])(void) = {
    (intfunc)((unsigned long *)&_estack), /* The stack pointer after relocation */
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    SVC_Handler,
    0,
    0,
    PendSV_Handler,
    SysTick_Handler,
    PM_Handler,
    SYSCTRL_Handler,
    WDT_Handler,
    RTC_Handler,
    EIC_Handler,
    NVMCTRL_Handler,
    DMAC_Handler,
    USB_Handler,
    EVSYS_Handler,
    SERCOM0_Handler,
    SERCOM1_Handler,
    SERCOM2_Handler,
    SERCOM3_Handler,
    SERCOM4_Handler,
    SERCOM5_Handler,
    TCC0_Handler,
    TCC1_Handler,
    TCC2_Handler,
    TC3_Handler,
    TC4_Handler,
    TC5_Handler,
    TC6_Handler,
    TC7_Handler,
    ADC_Handler,
    AC_Handler,
    DAC_Handler,
    PTC_Handler,
    I2S_Handler,
};

#pragma weak NMI_Handler = Default_Handler
#pragma weak HardFault_Handler = Default_Handler
#pragma weak SVC_Handler = Default_Handler
#pragma weak PendSV_Handler = Default_Handler
#pragma weak SysTick_Handler = Default_Handler
#pragma weak PM_Handler = Default_Handler
#pragma weak SYSCTRL_Handler = Default_Handler
#pragma weak WDT_Handler = Default_Handler
#pragma weak RTC_Handler = Default_Handler
#pragma weak EIC_Handler = Default_Handler
#pragma weak NVMCTRL_Handler = Default_Handler
#pragma weak DMAC_Handler = Default_Handler
#pragma weak USB_Handler = Default_Handler
#pragma weak EVSYS_Handler = Default_Handler
#pragma weak SERCOM0_Handler = Default_Handler
#pragma weak SERCOM1_Handler = Default_Handler
#pragma weak SERCOM2_Handler = Default_Handler
#pragma weak SERCOM3_Handler = Default_Handler
#pragma weak SERCOM4_Handler = Default_Handler
#pragma weak SERCOM5_Handler = Default_Handler
#pragma weak TCC0_Handler = Default_Handler
#pragma weak TCC1_Handler = Default_Handler
#pragma weak TCC2_Handler = Default_Handler
#pragma weak TC3_Handler = Default_Handler
#pragma weak TC4_Handler = Default_Handler
#pragma weak TC5_Handler = Default_Handler
#pragma weak TC6_Handler = Default_Handler
#pragma weak TC7_Handler = Default_Handler
#pragma weak ADC_Handler = Default_Handler
#pragma weak AC_Handler = Default_Handler
#pragma weak DAC_Handler = Default_Handler
#pragma weak PTC_Handler = Default_Handler
#pragma weak I2S_Handler = Default_Handler

void Default_Handler(void)
{
        while (1)
        {
        }
}