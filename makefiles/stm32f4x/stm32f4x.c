#include "stm32f4xx.h"
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

void Reset_Handler(void)
{
    /* Initialize data and bss */
    __Init_Data();
    unsigned long *pSrc = (unsigned long *)&_svtor;
    SCB->VTOR = ((unsigned long)pSrc & SCB_VTOR_TBLOFF_Msk);
    // mcu_init does this
    //  SystemInit();
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
void WEAK MemManage_Handler(void);
void WEAK BusFault_Handler(void);
void WEAK UsageFault_Handler(void);
void WEAK SVC_Handler(void);
void WEAK DebugMon_Handler(void);
void WEAK PendSV_Handler(void);
void WEAK SysTick_Handler(void);
void WEAK WWDG_Handler(void);               /*!< Window WatchDog Interrupt                                         */
void WEAK PVD_Handler(void);                /*!< PVD through EXTI Line detection Interrupt                         */
void WEAK TAMP_STAMP_Handler(void);         /*!< Tamper and TimeStamp interrupts through the EXTI line             */
void WEAK RTC_WKUP_Handler(void);           /*!< RTC Wakeup interrupt through the EXTI line                        */
void WEAK FLASH_Handler(void);              /*!< FLASH global Interrupt                                            */
void WEAK RCC_Handler(void);                /*!< RCC global Interrupt                                              */
void WEAK EXTI0_Handler(void);              /*!< EXTI Line0 Interrupt                                              */
void WEAK EXTI1_Handler(void);              /*!< EXTI Line1 Interrupt                                              */
void WEAK EXTI2_Handler(void);              /*!< EXTI Line2 Interrupt                                              */
void WEAK EXTI3_Handler(void);              /*!< EXTI Line3 Interrupt                                              */
void WEAK EXTI4_Handler(void);              /*!< EXTI Line4 Interrupt                                              */
void WEAK DMA1_Stream0_Handler(void);       /*!< DMA1 Stream 0 global Interrupt                                    */
void WEAK DMA1_Stream1_Handler(void);       /*!< DMA1 Stream 1 global Interrupt                                    */
void WEAK DMA1_Stream2_Handler(void);       /*!< DMA1 Stream 2 global Interrupt                                    */
void WEAK DMA1_Stream3_Handler(void);       /*!< DMA1 Stream 3 global Interrupt                                    */
void WEAK DMA1_Stream4_Handler(void);       /*!< DMA1 Stream 4 global Interrupt                                    */
void WEAK DMA1_Stream5_Handler(void);       /*!< DMA1 Stream 5 global Interrupt                                    */
void WEAK DMA1_Stream6_Handler(void);       /*!< DMA1 Stream 6 global Interrupt                                    */
void WEAK ADC_Handler(void);                /*!< ADC1, ADC2 and ADC3 global Interrupts                             */
void WEAK CAN1_TX_Handler(void);            /*!< CAN1 TX Interrupt                                                 */
void WEAK CAN1_RX0_Handler(void);           /*!< CAN1 RX0 Interrupt                                                */
void WEAK CAN1_RX1_Handler(void);           /*!< CAN1 RX1 Interrupt                                                */
void WEAK CAN1_SCE_Handler(void);           /*!< CAN1 SCE Interrupt                                                */
void WEAK EXTI9_5_Handler(void);            /*!< External Line[9:5] Interrupts                                     */
void WEAK TIM1_BRK_TIM9_Handler(void);      /*!< TIM1 Break interrupt and TIM9 global interrupt                    */
void WEAK TIM1_UP_TIM10_Handler(void);      /*!< TIM1 Update Interrupt and TIM10 global interrupt                  */
void WEAK TIM1_TRG_COM_TIM11_Handler(void); /*!< TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt */
void WEAK TIM1_CC_Handler(void);            /*!< TIM1 Capture Compare Interrupt                                    */
void WEAK TIM2_Handler(void);               /*!< TIM2 global Interrupt                                             */
void WEAK TIM3_Handler(void);               /*!< TIM3 global Interrupt                                             */
void WEAK TIM4_Handler(void);               /*!< TIM4 global Interrupt                                             */
void WEAK I2C1_EV_Handler(void);            /*!< I2C1 Event Interrupt                                              */
void WEAK I2C1_ER_Handler(void);            /*!< I2C1 Error Interrupt                                              */
void WEAK I2C2_EV_Handler(void);            /*!< I2C2 Event Interrupt                                              */
void WEAK I2C2_ER_Handler(void);            /*!< I2C2 Error Interrupt                                              */
void WEAK SPI1_Handler(void);               /*!< SPI1 global Interrupt                                             */
void WEAK SPI2_Handler(void);               /*!< SPI2 global Interrupt                                             */
void WEAK USART1_Handler(void);             /*!< USART1 global Interrupt                                           */
void WEAK USART2_Handler(void);             /*!< USART2 global Interrupt                                           */
void WEAK USART3_Handler(void);             /*!< USART3 global Interrupt                                           */
void WEAK EXTI15_10_Handler(void);          /*!< External Line[15:10] Interrupts                                   */
void WEAK RTC_Alarm_Handler(void);          /*!< RTC Alarm (A and B) through EXTI Line Interrupt                   */
void WEAK OTG_FS_WKUP_Handler(void);        /*!< USB OTG FS Wakeup through EXTI line interrupt                     */
void WEAK TIM8_BRK_TIM12_Handler(void);     /*!< TIM8 Break Interrupt and TIM12 global interrupt                   */
void WEAK TIM8_UP_TIM13_Handler(void);      /*!< TIM8 Update Interrupt and TIM13 global interrupt                  */
void WEAK TIM8_TRG_COM_TIM14_Handler(void); /*!< TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt */
void WEAK TIM8_CC_Handler(void);            /*!< TIM8 Capture Compare global interrupt                             */
void WEAK DMA1_Stream7_Handler(void);       /*!< DMA1 Stream7 Interrupt                                            */
void WEAK FMC_Handler(void);                /*!< FMC global Interrupt                                              */
void WEAK SDIO_Handler(void);               /*!< SDIO global Interrupt                                             */
void WEAK TIM5_Handler(void);               /*!< TIM5 global Interrupt                                             */
void WEAK SPI3_Handler(void);               /*!< SPI3 global Interrupt                                             */
void WEAK UART4_Handler(void);              /*!< UART4 global Interrupt                                            */
void WEAK UART5_Handler(void);              /*!< UART5 global Interrupt                                            */
void WEAK TIM6_DAC_Handler(void);           /*!< TIM6 global and DAC1&2 underrun error  interrupts                 */
void WEAK TIM7_Handler(void);               /*!< TIM7 global interrupt                                             */
void WEAK DMA2_Stream0_Handler(void);       /*!< DMA2 Stream 0 global Interrupt                                    */
void WEAK DMA2_Stream1_Handler(void);       /*!< DMA2 Stream 1 global Interrupt                                    */
void WEAK DMA2_Stream2_Handler(void);       /*!< DMA2 Stream 2 global Interrupt                                    */
void WEAK DMA2_Stream3_Handler(void);       /*!< DMA2 Stream 3 global Interrupt                                    */
void WEAK DMA2_Stream4_Handler(void);       /*!< DMA2 Stream 4 global Interrupt                                    */
void WEAK ETH_Handler(void);                /*!< Ethernet global Interrupt                                         */
void WEAK ETH_WKUP_Handler(void);           /*!< Ethernet Wakeup through EXTI line Interrupt                       */
void WEAK CAN2_TX_Handler(void);            /*!< CAN2 TX Interrupt                                                 */
void WEAK CAN2_RX0_Handler(void);           /*!< CAN2 RX0 Interrupt                                                */
void WEAK CAN2_RX1_Handler(void);           /*!< CAN2 RX1 Interrupt                                                */
void WEAK CAN2_SCE_Handler(void);           /*!< CAN2 SCE Interrupt                                                */
void WEAK OTG_FS_Handler(void);             /*!< USB OTG FS global Interrupt                                       */
void WEAK DMA2_Stream5_Handler(void);       /*!< DMA2 Stream 5 global interrupt                                    */
void WEAK DMA2_Stream6_Handler(void);       /*!< DMA2 Stream 6 global interrupt                                    */
void WEAK DMA2_Stream7_Handler(void);       /*!< DMA2 Stream 7 global interrupt                                    */
void WEAK USART6_Handler(void);             /*!< USART6 global interrupt                                           */
void WEAK I2C3_EV_Handler(void);            /*!< I2C3 event interrupt                                              */
void WEAK I2C3_ER_Handler(void);            /*!< I2C3 error interrupt                                              */
void WEAK OTG_HS_EP1_OUT_Handler(void);     /*!< USB OTG HS End Point 1 Out global interrupt                       */
void WEAK OTG_HS_EP1_IN_Handler(void);      /*!< USB OTG HS End Point 1 In global interrupt                        */
void WEAK OTG_HS_WKUP_Handler(void);        /*!< USB OTG HS Wakeup through EXTI interrupt                          */
void WEAK OTG_HS_Handler(void);             /*!< USB OTG HS global interrupt                                       */
void WEAK DCMI_Handler(void);               /*!< DCMI global interrupt                                             */
void WEAK CRYP_Handler(void);               /*!< CRYP crypto global interrupt                                      */
void WEAK HASH_RNG_Handler(void);           /*!< Hash and Rng global interrupt                                     */
void WEAK FPU_Handler(void);                /*!< FPU global interrupt                                              */
void WEAK UART7_Handler(void);              /*!< UART7 global interrupt                                            */
void WEAK UART8_Handler(void);              /*!< UART8 global interrupt                                            */
void WEAK SPI4_Handler(void);               /*!< SPI4 global Interrupt                                             */
void WEAK SPI5_Handler(void);               /*!< SPI5 global Interrupt                                             */
void WEAK SPI6_Handler(void);               /*!< SPI6 global Interrupt                                             */
void WEAK SAI1_Handler(void);               /*!< SAI1 global Interrupt                                             */
void WEAK LTDC_Handler(void);               /*!< LTDC global Interrupt                                              */
void WEAK LTDC_ER_Handler(void);            /*!< LTDC Error global Interrupt                                        */
void WEAK DMA2D_Handler(void);              /*!< DMA2D global Interrupt                                            */
void WEAK QUADSPI_Handler(void);            /*!< QUADSPI global Interrupt                                          */
void WEAK DSI_Handler(void);                /*!< DSI global Interrupt                                              */

__attribute__((used, section(".isr_vector"))) void (*const g_pfnVectors[])(void) = {
    (intfunc)((unsigned long *)&_estack), /* The stack pointer after relocation */
    Reset_Handler,                        /* Reset Handler */
    NMI_Handler,                          /* NMI Handler */
    HardFault_Handler,                    /* Hard Fault Handler */
    MemManage_Handler,                    /* MPU Fault Handler */
    BusFault_Handler,                     /* Bus Fault Handler */
    UsageFault_Handler,                   /* Usage Fault Handler */
    0,                                    /* Reserved */
    0,                                    /* Reserved */
    0,                                    /* Reserved */
    0,                                    /* Reserved */
    SVC_Handler,                          /* SVCall Handler */
    DebugMon_Handler,                     /* Debug Monitor Handler */
    0,                                    /* Reserved */
    PendSV_Handler,                       /* PendSV Handler */
    SysTick_Handler,                      /* SysTick Handler */

    /* External Interrupts */
    WWDG_Handler,               /*!< Window WatchDog Interrupt                                         */
    PVD_Handler,                /*!< PVD through EXTI Line detection Interrupt                         */
    TAMP_STAMP_Handler,         /*!< Tamper and TimeStamp interrupts through the EXTI line             */
    RTC_WKUP_Handler,           /*!< RTC Wakeup interrupt through the EXTI line                        */
    FLASH_Handler,              /*!< FLASH global Interrupt                                            */
    RCC_Handler,                /*!< RCC global Interrupt                                              */
    EXTI0_Handler,              /*!< EXTI Line0 Interrupt                                              */
    EXTI1_Handler,              /*!< EXTI Line1 Interrupt                                              */
    EXTI2_Handler,              /*!< EXTI Line2 Interrupt                                              */
    EXTI3_Handler,              /*!< EXTI Line3 Interrupt                                              */
    EXTI4_Handler,              /*!< EXTI Line4 Interrupt                                              */
    DMA1_Stream0_Handler,       /*!< DMA1 Stream 0 global Interrupt                                    */
    DMA1_Stream1_Handler,       /*!< DMA1 Stream 1 global Interrupt                                    */
    DMA1_Stream2_Handler,       /*!< DMA1 Stream 2 global Interrupt                                    */
    DMA1_Stream3_Handler,       /*!< DMA1 Stream 3 global Interrupt                                    */
    DMA1_Stream4_Handler,       /*!< DMA1 Stream 4 global Interrupt                                    */
    DMA1_Stream5_Handler,       /*!< DMA1 Stream 5 global Interrupt                                    */
    DMA1_Stream6_Handler,       /*!< DMA1 Stream 6 global Interrupt                                    */
    ADC_Handler,                /*!< ADC1, ADC2 and ADC3 global Interrupts                             */
    CAN1_TX_Handler,            /*!< CAN1 TX Interrupt                                                 */
    CAN1_RX0_Handler,           /*!< CAN1 RX0 Interrupt                                                */
    CAN1_RX1_Handler,           /*!< CAN1 RX1 Interrupt                                                */
    CAN1_SCE_Handler,           /*!< CAN1 SCE Interrupt                                                */
    EXTI9_5_Handler,            /*!< External Line[9:5] Interrupts                                     */
    TIM1_BRK_TIM9_Handler,      /*!< TIM1 Break interrupt and TIM9 global interrupt                    */
    TIM1_UP_TIM10_Handler,      /*!< TIM1 Update Interrupt and TIM10 global interrupt                  */
    TIM1_TRG_COM_TIM11_Handler, /*!< TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt */
    TIM1_CC_Handler,            /*!< TIM1 Capture Compare Interrupt                                    */
    TIM2_Handler,               /*!< TIM2 global Interrupt                                             */
    TIM3_Handler,               /*!< TIM3 global Interrupt                                             */
    TIM4_Handler,               /*!< TIM4 global Interrupt                                             */
    I2C1_EV_Handler,            /*!< I2C1 Event Interrupt                                              */
    I2C1_ER_Handler,            /*!< I2C1 Error Interrupt                                              */
    I2C2_EV_Handler,            /*!< I2C2 Event Interrupt                                              */
    I2C2_ER_Handler,            /*!< I2C2 Error Interrupt                                              */
    SPI1_Handler,               /*!< SPI1 global Interrupt                                             */
    SPI2_Handler,               /*!< SPI2 global Interrupt                                             */
    USART1_Handler,             /*!< USART1 global Interrupt                                           */
    USART2_Handler,             /*!< USART2 global Interrupt                                           */
    USART3_Handler,             /*!< USART3 global Interrupt                                           */
    EXTI15_10_Handler,          /*!< External Line[15:10] Interrupts                                   */
    RTC_Alarm_Handler,          /*!< RTC Alarm (A and B) through EXTI Line Interrupt                   */
    OTG_FS_WKUP_Handler,        /*!< USB OTG FS Wakeup through EXTI line interrupt                     */
    TIM8_BRK_TIM12_Handler,     /*!< TIM8 Break Interrupt and TIM12 global interrupt                   */
    TIM8_UP_TIM13_Handler,      /*!< TIM8 Update Interrupt and TIM13 global interrupt                  */
    TIM8_TRG_COM_TIM14_Handler, /*!< TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt */
    TIM8_CC_Handler,            /*!< TIM8 Capture Compare global interrupt                             */
    DMA1_Stream7_Handler,       /*!< DMA1 Stream7 Interrupt                                            */
    FMC_Handler,                /*!< FMC global Interrupt                                              */
    SDIO_Handler,               /*!< SDIO global Interrupt                                             */
    TIM5_Handler,               /*!< TIM5 global Interrupt                                             */
    SPI3_Handler,               /*!< SPI3 global Interrupt                                             */
    UART4_Handler,              /*!< UART4 global Interrupt                                            */
    UART5_Handler,              /*!< UART5 global Interrupt                                            */
    TIM6_DAC_Handler,           /*!< TIM6 global and DAC1&2 underrun error  interrupts                 */
    TIM7_Handler,               /*!< TIM7 global interrupt                                             */
    DMA2_Stream0_Handler,       /*!< DMA2 Stream 0 global Interrupt                                    */
    DMA2_Stream1_Handler,       /*!< DMA2 Stream 1 global Interrupt                                    */
    DMA2_Stream2_Handler,       /*!< DMA2 Stream 2 global Interrupt                                    */
    DMA2_Stream3_Handler,       /*!< DMA2 Stream 3 global Interrupt                                    */
    DMA2_Stream4_Handler,       /*!< DMA2 Stream 4 global Interrupt                                    */
    ETH_Handler,                /*!< Ethernet global Interrupt                                         */
    ETH_WKUP_Handler,           /*!< Ethernet Wakeup through EXTI line Interrupt                       */
    CAN2_TX_Handler,            /*!< CAN2 TX Interrupt                                                 */
    CAN2_RX0_Handler,           /*!< CAN2 RX0 Interrupt                                                */
    CAN2_RX1_Handler,           /*!< CAN2 RX1 Interrupt                                                */
    CAN2_SCE_Handler,           /*!< CAN2 SCE Interrupt                                                */
    OTG_FS_Handler,             /*!< USB OTG FS global Interrupt                                       */
    DMA2_Stream5_Handler,       /*!< DMA2 Stream 5 global interrupt                                    */
    DMA2_Stream6_Handler,       /*!< DMA2 Stream 6 global interrupt                                    */
    DMA2_Stream7_Handler,       /*!< DMA2 Stream 7 global interrupt                                    */
    USART6_Handler,             /*!< USART6 global interrupt                                           */
    I2C3_EV_Handler,            /*!< I2C3 event interrupt                                              */
    I2C3_ER_Handler,            /*!< I2C3 error interrupt                                              */
    OTG_HS_EP1_OUT_Handler,     /*!< USB OTG HS End Point 1 Out global interrupt                       */
    OTG_HS_EP1_IN_Handler,      /*!< USB OTG HS End Point 1 In global interrupt                        */
    OTG_HS_WKUP_Handler,        /*!< USB OTG HS Wakeup through EXTI interrupt                          */
    OTG_HS_Handler,             /*!< USB OTG HS global interrupt                                       */
    DCMI_Handler,               /*!< DCMI global interrupt                                             */
    CRYP_Handler,               /*!< CRYP crypto global interrupt                                      */
    HASH_RNG_Handler,           /*!< Hash and Rng global interrupt                                     */
    FPU_Handler,                /*!< FPU global interrupt                                              */
    UART7_Handler,              /*!< UART7 global interrupt                                            */
    UART8_Handler,              /*!< UART8 global interrupt                                            */
    SPI4_Handler,               /*!< SPI4 global Interrupt                                             */
    SPI5_Handler,               /*!< SPI5 global Interrupt                                             */
    SPI6_Handler,               /*!< SPI6 global Interrupt                                             */
    SAI1_Handler,               /*!< SAI1 global Interrupt                                             */
    LTDC_Handler,               /*!< LTDC global Interrupt                                              */
    LTDC_ER_Handler,            /*!< LTDC Error global Interrupt                                        */
    DMA2D_Handler,              /*!< DMA2D global Interrupt                                            */
    QUADSPI_Handler,            /*!< QUADSPI global Interrupt                                          */
    DSI_Handler                 /*!< DSI global Interrupt                                              */
};

#pragma weak NMI_Handler = Default_Handler
#pragma weak MemManage_Handler = Default_Handler
#pragma weak BusFault_Handler = Default_Handler
#pragma weak UsageFault_Handler = Default_Handler
#pragma weak SVC_Handler = Default_Handler
#pragma weak DebugMon_Handler = Default_Handler
#pragma weak PendSV_Handler = Default_Handler
#pragma weak SysTick_Handler = Default_Handler
#pragma weak WWDG_Handler = Default_Handler               /*!< Window WatchDog Interrupt                                         */
#pragma weak PVD_Handler = Default_Handler                /*!< PVD through EXTI Line detection Interrupt                         */
#pragma weak TAMP_STAMP_Handler = Default_Handler         /*!< Tamper and TimeStamp interrupts through the EXTI line             */
#pragma weak RTC_WKUP_Handler = Default_Handler           /*!< RTC Wakeup interrupt through the EXTI line                        */
#pragma weak FLASH_Handler = Default_Handler              /*!< FLASH global Interrupt                                            */
#pragma weak RCC_Handler = Default_Handler                /*!< RCC global Interrupt                                              */
#pragma weak EXTI0_Handler = Default_Handler              /*!< EXTI Line0 Interrupt                                              */
#pragma weak EXTI1_Handler = Default_Handler              /*!< EXTI Line1 Interrupt                                              */
#pragma weak EXTI2_Handler = Default_Handler              /*!< EXTI Line2 Interrupt                                              */
#pragma weak EXTI3_Handler = Default_Handler              /*!< EXTI Line3 Interrupt                                              */
#pragma weak EXTI4_Handler = Default_Handler              /*!< EXTI Line4 Interrupt                                              */
#pragma weak DMA1_Stream0_Handler = Default_Handler       /*!< DMA1 Stream 0 global Interrupt                                    */
#pragma weak DMA1_Stream1_Handler = Default_Handler       /*!< DMA1 Stream 1 global Interrupt                                    */
#pragma weak DMA1_Stream2_Handler = Default_Handler       /*!< DMA1 Stream 2 global Interrupt                                    */
#pragma weak DMA1_Stream3_Handler = Default_Handler       /*!< DMA1 Stream 3 global Interrupt                                    */
#pragma weak DMA1_Stream4_Handler = Default_Handler       /*!< DMA1 Stream 4 global Interrupt                                    */
#pragma weak DMA1_Stream5_Handler = Default_Handler       /*!< DMA1 Stream 5 global Interrupt                                    */
#pragma weak DMA1_Stream6_Handler = Default_Handler       /*!< DMA1 Stream 6 global Interrupt                                    */
#pragma weak ADC_Handler = Default_Handler                /*!< ADC1, ADC2 and ADC3 global Interrupts                             */
#pragma weak CAN1_TX_Handler = Default_Handler            /*!< CAN1 TX Interrupt                                                 */
#pragma weak CAN1_RX0_Handler = Default_Handler           /*!< CAN1 RX0 Interrupt                                                */
#pragma weak CAN1_RX1_Handler = Default_Handler           /*!< CAN1 RX1 Interrupt                                                */
#pragma weak CAN1_SCE_Handler = Default_Handler           /*!< CAN1 SCE Interrupt                                                */
#pragma weak EXTI9_5_Handler = Default_Handler            /*!< External Line[9:5] Interrupts                                     */
#pragma weak TIM1_BRK_TIM9_Handler = Default_Handler      /*!< TIM1 Break interrupt and TIM9 global interrupt                    */
#pragma weak TIM1_UP_TIM10_Handler = Default_Handler      /*!< TIM1 Update Interrupt and TIM10 global interrupt                  */
#pragma weak TIM1_TRG_COM_TIM11_Handler = Default_Handler /*!< TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt */
#pragma weak TIM1_CC_Handler = Default_Handler            /*!< TIM1 Capture Compare Interrupt                                    */
#pragma weak TIM2_Handler = Default_Handler               /*!< TIM2 global Interrupt                                             */
#pragma weak TIM3_Handler = Default_Handler               /*!< TIM3 global Interrupt                                             */
#pragma weak TIM4_Handler = Default_Handler               /*!< TIM4 global Interrupt                                             */
#pragma weak I2C1_EV_Handler = Default_Handler            /*!< I2C1 Event Interrupt                                              */
#pragma weak I2C1_ER_Handler = Default_Handler            /*!< I2C1 Error Interrupt                                              */
#pragma weak I2C2_EV_Handler = Default_Handler            /*!< I2C2 Event Interrupt                                              */
#pragma weak I2C2_ER_Handler = Default_Handler            /*!< I2C2 Error Interrupt                                              */
#pragma weak SPI1_Handler = Default_Handler               /*!< SPI1 global Interrupt                                             */
#pragma weak SPI2_Handler = Default_Handler               /*!< SPI2 global Interrupt                                             */
#pragma weak USART1_Handler = Default_Handler             /*!< USART1 global Interrupt                                           */
#pragma weak USART2_Handler = Default_Handler             /*!< USART2 global Interrupt                                           */
#pragma weak USART3_Handler = Default_Handler             /*!< USART3 global Interrupt                                           */
#pragma weak EXTI15_10_Handler = Default_Handler          /*!< External Line[15:10] Interrupts                                   */
#pragma weak RTC_Alarm_Handler = Default_Handler          /*!< RTC Alarm (A and B) through EXTI Line Interrupt                   */
#pragma weak OTG_FS_WKUP_Handler = Default_Handler        /*!< USB OTG FS Wakeup through EXTI line interrupt                     */
#pragma weak TIM8_BRK_TIM12_Handler = Default_Handler     /*!< TIM8 Break Interrupt and TIM12 global interrupt                   */
#pragma weak TIM8_UP_TIM13_Handler = Default_Handler      /*!< TIM8 Update Interrupt and TIM13 global interrupt                  */
#pragma weak TIM8_TRG_COM_TIM14_Handler = Default_Handler /*!< TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt */
#pragma weak TIM8_CC_Handler = Default_Handler            /*!< TIM8 Capture Compare global interrupt                             */
#pragma weak DMA1_Stream7_Handler = Default_Handler       /*!< DMA1 Stream7 Interrupt                                            */
#pragma weak FMC_Handler = Default_Handler                /*!< FMC global Interrupt                                              */
#pragma weak SDIO_Handler = Default_Handler               /*!< SDIO global Interrupt                                             */
#pragma weak TIM5_Handler = Default_Handler               /*!< TIM5 global Interrupt                                             */
#pragma weak SPI3_Handler = Default_Handler               /*!< SPI3 global Interrupt                                             */
#pragma weak UART4_Handler = Default_Handler              /*!< UART4 global Interrupt                                            */
#pragma weak UART5_Handler = Default_Handler              /*!< UART5 global Interrupt                                            */
#pragma weak TIM6_DAC_Handler = Default_Handler           /*!< TIM6 global and DAC1&2 underrun error  interrupts                 */
#pragma weak TIM7_Handler = Default_Handler               /*!< TIM7 global interrupt                                             */
#pragma weak DMA2_Stream0_Handler = Default_Handler       /*!< DMA2 Stream 0 global Interrupt                                    */
#pragma weak DMA2_Stream1_Handler = Default_Handler       /*!< DMA2 Stream 1 global Interrupt                                    */
#pragma weak DMA2_Stream2_Handler = Default_Handler       /*!< DMA2 Stream 2 global Interrupt                                    */
#pragma weak DMA2_Stream3_Handler = Default_Handler       /*!< DMA2 Stream 3 global Interrupt                                    */
#pragma weak DMA2_Stream4_Handler = Default_Handler       /*!< DMA2 Stream 4 global Interrupt                                    */
#pragma weak ETH_Handler = Default_Handler                /*!< Ethernet global Interrupt                                         */
#pragma weak ETH_WKUP_Handler = Default_Handler           /*!< Ethernet Wakeup through EXTI line Interrupt                       */
#pragma weak CAN2_TX_Handler = Default_Handler            /*!< CAN2 TX Interrupt                                                 */
#pragma weak CAN2_RX0_Handler = Default_Handler           /*!< CAN2 RX0 Interrupt                                                */
#pragma weak CAN2_RX1_Handler = Default_Handler           /*!< CAN2 RX1 Interrupt                                                */
#pragma weak CAN2_SCE_Handler = Default_Handler           /*!< CAN2 SCE Interrupt                                                */
#pragma weak OTG_FS_Handler = Default_Handler             /*!< USB OTG FS global Interrupt                                       */
#pragma weak DMA2_Stream5_Handler = Default_Handler       /*!< DMA2 Stream 5 global interrupt                                    */
#pragma weak DMA2_Stream6_Handler = Default_Handler       /*!< DMA2 Stream 6 global interrupt                                    */
#pragma weak DMA2_Stream7_Handler = Default_Handler       /*!< DMA2 Stream 7 global interrupt                                    */
#pragma weak USART6_Handler = Default_Handler             /*!< USART6 global interrupt                                           */
#pragma weak I2C3_EV_Handler = Default_Handler            /*!< I2C3 event interrupt                                              */
#pragma weak I2C3_ER_Handler = Default_Handler            /*!< I2C3 error interrupt                                              */
#pragma weak OTG_HS_EP1_OUT_Handler = Default_Handler     /*!< USB OTG HS End Point 1 Out global interrupt                       */
#pragma weak OTG_HS_EP1_IN_Handler = Default_Handler      /*!< USB OTG HS End Point 1 In global interrupt                        */
#pragma weak OTG_HS_WKUP_Handler = Default_Handler        /*!< USB OTG HS Wakeup through EXTI interrupt                          */
#pragma weak OTG_HS_Handler = Default_Handler             /*!< USB OTG HS global interrupt                                       */
#pragma weak DCMI_Handler = Default_Handler               /*!< DCMI global interrupt                                             */
#pragma weak CRYP_Handler = Default_Handler               /*!< CRYP crypto global interrupt                                      */
#pragma weak HASH_RNG_Handler = Default_Handler           /*!< Hash and Rng global interrupt                                     */
#pragma weak FPU_Handler = Default_Handler                /*!< FPU global interrupt                                              */
#pragma weak UART7_Handler = Default_Handler              /*!< UART7 global interrupt                                            */
#pragma weak UART8_Handler = Default_Handler              /*!< UART8 global interrupt                                            */
#pragma weak SPI4_Handler = Default_Handler               /*!< SPI4 global Interrupt                                             */
#pragma weak SPI5_Handler = Default_Handler               /*!< SPI5 global Interrupt                                             */
#pragma weak SPI6_Handler = Default_Handler               /*!< SPI6 global Interrupt                                             */
#pragma weak SAI1_Handler = Default_Handler               /*!< SAI1 global Interrupt                                             */
#pragma weak LTDC_Handler = Default_Handler               /*!< LTDC global Interrupt                                              */
#pragma weak LTDC_ER_Handler = Default_Handler            /*!< LTDC Error global Interrupt                                        */
#pragma weak DMA2D_Handler = Default_Handler              /*!< DMA2D global Interrupt                                            */
#pragma weak QUADSPI_Handler = Default_Handler            /*!< QUADSPI global Interrupt                                          */
#pragma weak DSI_Handler = Default_Handler                /*!< DSI global Interrupt                                              */

void Default_Handler(void)
{
    while (1)
    {
    }
}