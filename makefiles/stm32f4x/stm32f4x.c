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

void Reset_IRQHandler(void) __attribute__((__interrupt__));
void __Init_Data(void);
void SystemInit(void);
void Default_IRQHandler(void);
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

void Reset_IRQHandler(void)
{
	/* Initialize data and bss */
	__Init_Data();
	SystemInit();
	unsigned long *pSrc = (unsigned long *)&_svtor;
	SCB->VTOR = ((unsigned long)pSrc & SCB_VTOR_TBLOFF_Msk);
	// mcu_init does this

	while (1)
	{
		main();
	}
	while (1)
	{
	}
}

void WEAK NMI_IRQHandler(void);
void WEAK HardFault_IRQHandler(void);
void WEAK MemManage_IRQHandler(void);
void WEAK BusFault_IRQHandler(void);
void WEAK UsageFault_IRQHandler(void);
void WEAK SVC_IRQHandler(void);
void WEAK DebugMon_IRQHandler(void);
void WEAK PendSV_IRQHandler(void);
void WEAK SysTick_IRQHandler(void);
void WEAK WWDG_IRQHandler(void);							 /*!< Window WatchDog Interrupt                                         */
void WEAK PVD_IRQHandler(void);								 /*!< PVD through EXTI Line detection Interrupt                         */
void WEAK TAMP_STAMP_IRQHandler(void);				 /*!< Tamper and TimeStamp interrupts through the EXTI line             */
void WEAK RTC_WKUP_IRQHandler(void);					 /*!< RTC Wakeup interrupt through the EXTI line                        */
void WEAK FLASH_IRQHandler(void);							 /*!< FLASH global Interrupt                                            */
void WEAK RCC_IRQHandler(void);								 /*!< RCC global Interrupt                                              */
void WEAK EXTI0_IRQHandler(void);							 /*!< EXTI Line0 Interrupt                                              */
void WEAK EXTI1_IRQHandler(void);							 /*!< EXTI Line1 Interrupt                                              */
void WEAK EXTI2_IRQHandler(void);							 /*!< EXTI Line2 Interrupt                                              */
void WEAK EXTI3_IRQHandler(void);							 /*!< EXTI Line3 Interrupt                                              */
void WEAK EXTI4_IRQHandler(void);							 /*!< EXTI Line4 Interrupt                                              */
void WEAK DMA1_Stream0_IRQHandler(void);			 /*!< DMA1 Stream 0 global Interrupt                                    */
void WEAK DMA1_Stream1_IRQHandler(void);			 /*!< DMA1 Stream 1 global Interrupt                                    */
void WEAK DMA1_Stream2_IRQHandler(void);			 /*!< DMA1 Stream 2 global Interrupt                                    */
void WEAK DMA1_Stream3_IRQHandler(void);			 /*!< DMA1 Stream 3 global Interrupt                                    */
void WEAK DMA1_Stream4_IRQHandler(void);			 /*!< DMA1 Stream 4 global Interrupt                                    */
void WEAK DMA1_Stream5_IRQHandler(void);			 /*!< DMA1 Stream 5 global Interrupt                                    */
void WEAK DMA1_Stream6_IRQHandler(void);			 /*!< DMA1 Stream 6 global Interrupt                                    */
void WEAK ADC_IRQHandler(void);								 /*!< ADC1, ADC2 and ADC3 global Interrupts                             */
void WEAK CAN1_TX_IRQHandler(void);						 /*!< CAN1 TX Interrupt                                                 */
void WEAK CAN1_RX0_IRQHandler(void);					 /*!< CAN1 RX0 Interrupt                                                */
void WEAK CAN1_RX1_IRQHandler(void);					 /*!< CAN1 RX1 Interrupt                                                */
void WEAK CAN1_SCE_IRQHandler(void);					 /*!< CAN1 SCE Interrupt                                                */
void WEAK EXTI9_5_IRQHandler(void);						 /*!< External Line[9:5] Interrupts                                     */
void WEAK TIM1_BRK_TIM9_IRQHandler(void);			 /*!< TIM1 Break interrupt and TIM9 global interrupt                    */
void WEAK TIM1_UP_TIM10_IRQHandler(void);			 /*!< TIM1 Update Interrupt and TIM10 global interrupt                  */
void WEAK TIM1_TRG_COM_TIM11_IRQHandler(void); /*!< TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt */
void WEAK TIM1_CC_IRQHandler(void);						 /*!< TIM1 Capture Compare Interrupt                                    */
void WEAK TIM2_IRQHandler(void);							 /*!< TIM2 global Interrupt                                             */
void WEAK TIM3_IRQHandler(void);							 /*!< TIM3 global Interrupt                                             */
void WEAK TIM4_IRQHandler(void);							 /*!< TIM4 global Interrupt                                             */
void WEAK I2C1_EV_IRQHandler(void);						 /*!< I2C1 Event Interrupt                                              */
void WEAK I2C1_ER_IRQHandler(void);						 /*!< I2C1 Error Interrupt                                              */
void WEAK I2C2_EV_IRQHandler(void);						 /*!< I2C2 Event Interrupt                                              */
void WEAK I2C2_ER_IRQHandler(void);						 /*!< I2C2 Error Interrupt                                              */
void WEAK SPI1_IRQHandler(void);							 /*!< SPI1 global Interrupt                                             */
void WEAK SPI2_IRQHandler(void);							 /*!< SPI2 global Interrupt                                             */
void WEAK USART1_IRQHandler(void);						 /*!< USART1 global Interrupt                                           */
void WEAK USART2_IRQHandler(void);						 /*!< USART2 global Interrupt                                           */
void WEAK USART3_IRQHandler(void);						 /*!< USART3 global Interrupt                                           */
void WEAK EXTI15_10_IRQHandler(void);					 /*!< External Line[15:10] Interrupts                                   */
void WEAK RTC_Alarm_IRQHandler(void);					 /*!< RTC Alarm (A and B) through EXTI Line Interrupt                   */
void WEAK OTG_FS_WKUP_IRQHandler(void);				 /*!< USB OTG FS Wakeup through EXTI line interrupt                     */
void WEAK TIM8_BRK_TIM12_IRQHandler(void);		 /*!< TIM8 Break Interrupt and TIM12 global interrupt                   */
void WEAK TIM8_UP_TIM13_IRQHandler(void);			 /*!< TIM8 Update Interrupt and TIM13 global interrupt                  */
void WEAK TIM8_TRG_COM_TIM14_IRQHandler(void); /*!< TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt */
void WEAK TIM8_CC_IRQHandler(void);						 /*!< TIM8 Capture Compare global interrupt                             */
void WEAK DMA1_Stream7_IRQHandler(void);			 /*!< DMA1 Stream7 Interrupt                                            */
void WEAK FSMC_IRQHandler(void);							 /*!< FMC global Interrupt                                              */
void WEAK SDIO_IRQHandler(void);							 /*!< SDIO global Interrupt                                             */
void WEAK TIM5_IRQHandler(void);							 /*!< TIM5 global Interrupt                                             */
void WEAK SPI3_IRQHandler(void);							 /*!< SPI3 global Interrupt                                             */
void WEAK UART4_IRQHandler(void);							 /*!< UART4 global Interrupt                                            */
void WEAK UART5_IRQHandler(void);							 /*!< UART5 global Interrupt                                            */
void WEAK TIM6_DAC_IRQHandler(void);					 /*!< TIM6 global and DAC1&2 underrun error  interrupts                 */
void WEAK TIM7_IRQHandler(void);							 /*!< TIM7 global interrupt                                             */
void WEAK DMA2_Stream0_IRQHandler(void);			 /*!< DMA2 Stream 0 global Interrupt                                    */
void WEAK DMA2_Stream1_IRQHandler(void);			 /*!< DMA2 Stream 1 global Interrupt                                    */
void WEAK DMA2_Stream2_IRQHandler(void);			 /*!< DMA2 Stream 2 global Interrupt                                    */
void WEAK DMA2_Stream3_IRQHandler(void);			 /*!< DMA2 Stream 3 global Interrupt                                    */
void WEAK DMA2_Stream4_IRQHandler(void);			 /*!< DMA2 Stream 4 global Interrupt                                    */
void WEAK ETH_IRQHandler(void);								 /*!< Ethernet global Interrupt                                         */
void WEAK ETH_WKUP_IRQHandler(void);					 /*!< Ethernet Wakeup through EXTI line Interrupt                       */
void WEAK CAN2_TX_IRQHandler(void);						 /*!< CAN2 TX Interrupt                                                 */
void WEAK CAN2_RX0_IRQHandler(void);					 /*!< CAN2 RX0 Interrupt                                                */
void WEAK CAN2_RX1_IRQHandler(void);					 /*!< CAN2 RX1 Interrupt                                                */
void WEAK CAN2_SCE_IRQHandler(void);					 /*!< CAN2 SCE Interrupt                                                */
void WEAK OTG_FS_IRQHandler(void);						 /*!< USB OTG FS global Interrupt                                       */
void WEAK DMA2_Stream5_IRQHandler(void);			 /*!< DMA2 Stream 5 global interrupt                                    */
void WEAK DMA2_Stream6_IRQHandler(void);			 /*!< DMA2 Stream 6 global interrupt                                    */
void WEAK DMA2_Stream7_IRQHandler(void);			 /*!< DMA2 Stream 7 global interrupt                                    */
void WEAK USART6_IRQHandler(void);						 /*!< USART6 global interrupt                                           */
void WEAK I2C3_EV_IRQHandler(void);						 /*!< I2C3 event interrupt                                              */
void WEAK I2C3_ER_IRQHandler(void);						 /*!< I2C3 error interrupt                                              */
void WEAK OTG_HS_EP1_OUT_IRQHandler(void);		 /*!< USB OTG HS End Point 1 Out global interrupt                       */
void WEAK OTG_HS_EP1_IN_IRQHandler(void);			 /*!< USB OTG HS End Point 1 In global interrupt                        */
void WEAK OTG_HS_WKUP_IRQHandler(void);				 /*!< USB OTG HS Wakeup through EXTI interrupt                          */
void WEAK OTG_HS_IRQHandler(void);						 /*!< USB OTG HS global interrupt                                       */
void WEAK DCMI_IRQHandler(void);							 /*!< DCMI global interrupt                                             */
void WEAK CRYP_IRQHandler(void);							 /*!< CRYP crypto global interrupt                                      */
void WEAK HASH_RNG_IRQHandler(void);					 /*!< Hash and Rng global interrupt                                     */
void WEAK FPU_IRQHandler(void);								 /*!< FPU global interrupt                                              */
void WEAK UART7_IRQHandler(void);							 /*!< UART7 global interrupt                                            */
void WEAK UART8_IRQHandler(void);							 /*!< UART8 global interrupt                                            */
void WEAK SPI4_IRQHandler(void);							 /*!< SPI4 global Interrupt                                             */
void WEAK SPI5_IRQHandler(void);							 /*!< SPI5 global Interrupt                                             */
void WEAK SPI6_IRQHandler(void);							 /*!< SPI6 global Interrupt                                             */
void WEAK SAI1_IRQHandler(void);							 /*!< SAI1 global Interrupt                                             */
void WEAK LTDC_IRQHandler(void);							 /*!< LTDC global Interrupt                                              */
void WEAK LTDC_ER_IRQHandler(void);						 /*!< LTDC Error global Interrupt                                        */
void WEAK DMA2D_IRQHandler(void);							 /*!< DMA2D global Interrupt                                            */
void WEAK QUADSPI_IRQHandler(void);						 /*!< QUADSPI global Interrupt                                          */
void WEAK DSI_IRQHandler(void);								 /*!< DSI global Interrupt                                              */

__attribute__((used, section(".isr_vector"))) void (*const g_pfnVectors[])(void) = {
		(intfunc)((unsigned long *)&_estack), /* The stack pointer after relocation */
		Reset_IRQHandler,											/* Reset Handler */
		NMI_IRQHandler,												/* NMI Handler */
		HardFault_IRQHandler,									/* Hard Fault Handler */
		MemManage_IRQHandler,									/* MPU Fault Handler */
		BusFault_IRQHandler,									/* Bus Fault Handler */
		UsageFault_IRQHandler,								/* Usage Fault Handler */
		0,																		/* Reserved */
		0,																		/* Reserved */
		0,																		/* Reserved */
		0,																		/* Reserved */
		SVC_IRQHandler,												/* SVCall Handler */
		DebugMon_IRQHandler,									/* Debug Monitor Handler */
		0,																		/* Reserved */
		PendSV_IRQHandler,										/* PendSV Handler */
		SysTick_IRQHandler,										/* SysTick Handler */

		/* External Interrupts */
		WWDG_IRQHandler,							 /*!< Window WatchDog Interrupt                                         */
		PVD_IRQHandler,								 /*!< PVD through EXTI Line detection Interrupt                         */
		TAMP_STAMP_IRQHandler,				 /*!< Tamper and TimeStamp interrupts through the EXTI line             */
		RTC_WKUP_IRQHandler,					 /*!< RTC Wakeup interrupt through the EXTI line                        */
		FLASH_IRQHandler,							 /*!< FLASH global Interrupt                                            */
		RCC_IRQHandler,								 /*!< RCC global Interrupt                                              */
		EXTI0_IRQHandler,							 /*!< EXTI Line0 Interrupt                                              */
		EXTI1_IRQHandler,							 /*!< EXTI Line1 Interrupt                                              */
		EXTI2_IRQHandler,							 /*!< EXTI Line2 Interrupt                                              */
		EXTI3_IRQHandler,							 /*!< EXTI Line3 Interrupt                                              */
		EXTI4_IRQHandler,							 /*!< EXTI Line4 Interrupt                                              */
		DMA1_Stream0_IRQHandler,			 /*!< DMA1 Stream 0 global Interrupt                                    */
		DMA1_Stream1_IRQHandler,			 /*!< DMA1 Stream 1 global Interrupt                                    */
		DMA1_Stream2_IRQHandler,			 /*!< DMA1 Stream 2 global Interrupt                                    */
		DMA1_Stream3_IRQHandler,			 /*!< DMA1 Stream 3 global Interrupt                                    */
		DMA1_Stream4_IRQHandler,			 /*!< DMA1 Stream 4 global Interrupt                                    */
		DMA1_Stream5_IRQHandler,			 /*!< DMA1 Stream 5 global Interrupt                                    */
		DMA1_Stream6_IRQHandler,			 /*!< DMA1 Stream 6 global Interrupt                                    */
		ADC_IRQHandler,								 /*!< ADC1, ADC2 and ADC3 global Interrupts                             */
		CAN1_TX_IRQHandler,						 /*!< CAN1 TX Interrupt                                                 */
		CAN1_RX0_IRQHandler,					 /*!< CAN1 RX0 Interrupt                                                */
		CAN1_RX1_IRQHandler,					 /*!< CAN1 RX1 Interrupt                                                */
		CAN1_SCE_IRQHandler,					 /*!< CAN1 SCE Interrupt                                                */
		EXTI9_5_IRQHandler,						 /*!< External Line[9:5] Interrupts                                     */
		TIM1_BRK_TIM9_IRQHandler,			 /*!< TIM1 Break interrupt and TIM9 global interrupt                    */
		TIM1_UP_TIM10_IRQHandler,			 /*!< TIM1 Update Interrupt and TIM10 global interrupt                  */
		TIM1_TRG_COM_TIM11_IRQHandler, /*!< TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt */
		TIM1_CC_IRQHandler,						 /*!< TIM1 Capture Compare Interrupt                                    */
		TIM2_IRQHandler,							 /*!< TIM2 global Interrupt                                             */
		TIM3_IRQHandler,							 /*!< TIM3 global Interrupt                                             */
		TIM4_IRQHandler,							 /*!< TIM4 global Interrupt                                             */
		I2C1_EV_IRQHandler,						 /*!< I2C1 Event Interrupt                                              */
		I2C1_ER_IRQHandler,						 /*!< I2C1 Error Interrupt                                              */
		I2C2_EV_IRQHandler,						 /*!< I2C2 Event Interrupt                                              */
		I2C2_ER_IRQHandler,						 /*!< I2C2 Error Interrupt                                              */
		SPI1_IRQHandler,							 /*!< SPI1 global Interrupt                                             */
		SPI2_IRQHandler,							 /*!< SPI2 global Interrupt                                             */
		USART1_IRQHandler,						 /*!< USART1 global Interrupt                                           */
		USART2_IRQHandler,						 /*!< USART2 global Interrupt                                           */
		USART3_IRQHandler,						 /*!< USART3 global Interrupt                                           */
		EXTI15_10_IRQHandler,					 /*!< External Line[15:10] Interrupts                                   */
		RTC_Alarm_IRQHandler,					 /*!< RTC Alarm (A and B) through EXTI Line Interrupt                   */
		OTG_FS_WKUP_IRQHandler,				 /*!< USB OTG FS Wakeup through EXTI line interrupt                     */
		TIM8_BRK_TIM12_IRQHandler,		 /*!< TIM8 Break Interrupt and TIM12 global interrupt                   */
		TIM8_UP_TIM13_IRQHandler,			 /*!< TIM8 Update Interrupt and TIM13 global interrupt                  */
		TIM8_TRG_COM_TIM14_IRQHandler, /*!< TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt */
		TIM8_CC_IRQHandler,						 /*!< TIM8 Capture Compare global interrupt                             */
		DMA1_Stream7_IRQHandler,			 /*!< DMA1 Stream7 Interrupt                                            */
		FSMC_IRQHandler,							 /*!< FMC global Interrupt                                              */
		SDIO_IRQHandler,							 /*!< SDIO global Interrupt                                             */
		TIM5_IRQHandler,							 /*!< TIM5 global Interrupt                                             */
		SPI3_IRQHandler,							 /*!< SPI3 global Interrupt                                             */
		UART4_IRQHandler,							 /*!< UART4 global Interrupt                                            */
		UART5_IRQHandler,							 /*!< UART5 global Interrupt                                            */
		TIM6_DAC_IRQHandler,					 /*!< TIM6 global and DAC1&2 underrun error  interrupts                 */
		TIM7_IRQHandler,							 /*!< TIM7 global interrupt                                             */
		DMA2_Stream0_IRQHandler,			 /*!< DMA2 Stream 0 global Interrupt                                    */
		DMA2_Stream1_IRQHandler,			 /*!< DMA2 Stream 1 global Interrupt                                    */
		DMA2_Stream2_IRQHandler,			 /*!< DMA2 Stream 2 global Interrupt                                    */
		DMA2_Stream3_IRQHandler,			 /*!< DMA2 Stream 3 global Interrupt                                    */
		DMA2_Stream4_IRQHandler,			 /*!< DMA2 Stream 4 global Interrupt                                    */
		ETH_IRQHandler,								 /*!< Ethernet global Interrupt                                         */
		ETH_WKUP_IRQHandler,					 /*!< Ethernet Wakeup through EXTI line Interrupt                       */
		CAN2_TX_IRQHandler,						 /*!< CAN2 TX Interrupt                                                 */
		CAN2_RX0_IRQHandler,					 /*!< CAN2 RX0 Interrupt                                                */
		CAN2_RX1_IRQHandler,					 /*!< CAN2 RX1 Interrupt                                                */
		CAN2_SCE_IRQHandler,					 /*!< CAN2 SCE Interrupt                                                */
		OTG_FS_IRQHandler,						 /*!< USB OTG FS global Interrupt                                       */
		DMA2_Stream5_IRQHandler,			 /*!< DMA2 Stream 5 global interrupt                                    */
		DMA2_Stream6_IRQHandler,			 /*!< DMA2 Stream 6 global interrupt                                    */
		DMA2_Stream7_IRQHandler,			 /*!< DMA2 Stream 7 global interrupt                                    */
		USART6_IRQHandler,						 /*!< USART6 global interrupt                                           */
		I2C3_EV_IRQHandler,						 /*!< I2C3 event interrupt                                              */
		I2C3_ER_IRQHandler,						 /*!< I2C3 error interrupt                                              */
		OTG_HS_EP1_OUT_IRQHandler,		 /*!< USB OTG HS End Point 1 Out global interrupt                       */
		OTG_HS_EP1_IN_IRQHandler,			 /*!< USB OTG HS End Point 1 In global interrupt                        */
		OTG_HS_WKUP_IRQHandler,				 /*!< USB OTG HS Wakeup through EXTI interrupt                          */
		OTG_HS_IRQHandler,						 /*!< USB OTG HS global interrupt                                       */
		DCMI_IRQHandler,							 /*!< DCMI global interrupt                                             */
		CRYP_IRQHandler,							 /*!< CRYP crypto global interrupt                                      */
		HASH_RNG_IRQHandler,					 /*!< Hash and Rng global interrupt                                     */
		FPU_IRQHandler,								 /*!< FPU global interrupt                                              */
		UART7_IRQHandler,							 /*!< UART7 global interrupt                                            */
		UART8_IRQHandler,							 /*!< UART8 global interrupt                                            */
		SPI4_IRQHandler,							 /*!< SPI4 global Interrupt                                             */
		SPI5_IRQHandler,							 /*!< SPI5 global Interrupt                                             */
		SPI6_IRQHandler,							 /*!< SPI6 global Interrupt                                             */
		SAI1_IRQHandler,							 /*!< SAI1 global Interrupt                                             */
		LTDC_IRQHandler,							 /*!< LTDC global Interrupt                                              */
		LTDC_ER_IRQHandler,						 /*!< LTDC Error global Interrupt                                        */
		DMA2D_IRQHandler,							 /*!< DMA2D global Interrupt                                            */
		QUADSPI_IRQHandler,						 /*!< QUADSPI global Interrupt                                          */
		DSI_IRQHandler								 /*!< DSI global Interrupt                                              */
};

#pragma weak NMI_IRQHandler = Default_IRQHandler
#pragma weak MemManage_IRQHandler = Default_IRQHandler
#pragma weak BusFault_IRQHandler = Default_IRQHandler
#pragma weak UsageFault_IRQHandler = Default_IRQHandler
#pragma weak SVC_IRQHandler = Default_IRQHandler
#pragma weak DebugMon_IRQHandler = Default_IRQHandler
#pragma weak PendSV_IRQHandler = Default_IRQHandler
#pragma weak SysTick_IRQHandler = Default_IRQHandler
#pragma weak WWDG_IRQHandler = Default_IRQHandler								/*!< Window WatchDog Interrupt                                         */
#pragma weak PVD_IRQHandler = Default_IRQHandler								/*!< PVD through EXTI Line detection Interrupt                         */
#pragma weak TAMP_STAMP_IRQHandler = Default_IRQHandler					/*!< Tamper and TimeStamp interrupts through the EXTI line             */
#pragma weak RTC_WKUP_IRQHandler = Default_IRQHandler						/*!< RTC Wakeup interrupt through the EXTI line                        */
#pragma weak FLASH_IRQHandler = Default_IRQHandler							/*!< FLASH global Interrupt                                            */
#pragma weak RCC_IRQHandler = Default_IRQHandler								/*!< RCC global Interrupt                                              */
#pragma weak EXTI0_IRQHandler = Default_IRQHandler							/*!< EXTI Line0 Interrupt                                              */
#pragma weak EXTI1_IRQHandler = Default_IRQHandler							/*!< EXTI Line1 Interrupt                                              */
#pragma weak EXTI2_IRQHandler = Default_IRQHandler							/*!< EXTI Line2 Interrupt                                              */
#pragma weak EXTI3_IRQHandler = Default_IRQHandler							/*!< EXTI Line3 Interrupt                                              */
#pragma weak EXTI4_IRQHandler = Default_IRQHandler							/*!< EXTI Line4 Interrupt                                              */
#pragma weak DMA1_Stream0_IRQHandler = Default_IRQHandler				/*!< DMA1 Stream 0 global Interrupt                                    */
#pragma weak DMA1_Stream1_IRQHandler = Default_IRQHandler				/*!< DMA1 Stream 1 global Interrupt                                    */
#pragma weak DMA1_Stream2_IRQHandler = Default_IRQHandler				/*!< DMA1 Stream 2 global Interrupt                                    */
#pragma weak DMA1_Stream3_IRQHandler = Default_IRQHandler				/*!< DMA1 Stream 3 global Interrupt                                    */
#pragma weak DMA1_Stream4_IRQHandler = Default_IRQHandler				/*!< DMA1 Stream 4 global Interrupt                                    */
#pragma weak DMA1_Stream5_IRQHandler = Default_IRQHandler				/*!< DMA1 Stream 5 global Interrupt                                    */
#pragma weak DMA1_Stream6_IRQHandler = Default_IRQHandler				/*!< DMA1 Stream 6 global Interrupt                                    */
#pragma weak ADC_IRQHandler = Default_IRQHandler								/*!< ADC1, ADC2 and ADC3 global Interrupts                             */
#pragma weak CAN1_TX_IRQHandler = Default_IRQHandler						/*!< CAN1 TX Interrupt                                                 */
#pragma weak CAN1_RX0_IRQHandler = Default_IRQHandler						/*!< CAN1 RX0 Interrupt                                                */
#pragma weak CAN1_RX1_IRQHandler = Default_IRQHandler						/*!< CAN1 RX1 Interrupt                                                */
#pragma weak CAN1_SCE_IRQHandler = Default_IRQHandler						/*!< CAN1 SCE Interrupt                                                */
#pragma weak EXTI9_5_IRQHandler = Default_IRQHandler						/*!< External Line[9:5] Interrupts                                     */
#pragma weak TIM1_BRK_TIM9_IRQHandler = Default_IRQHandler			/*!< TIM1 Break interrupt and TIM9 global interrupt                    */
#pragma weak TIM1_UP_TIM10_IRQHandler = Default_IRQHandler			/*!< TIM1 Update Interrupt and TIM10 global interrupt                  */
#pragma weak TIM1_TRG_COM_TIM11_IRQHandler = Default_IRQHandler /*!< TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt */
#pragma weak TIM1_CC_IRQHandler = Default_IRQHandler						/*!< TIM1 Capture Compare Interrupt                                    */
#pragma weak TIM2_IRQHandler = Default_IRQHandler								/*!< TIM2 global Interrupt                                             */
#pragma weak TIM3_IRQHandler = Default_IRQHandler								/*!< TIM3 global Interrupt                                             */
#pragma weak TIM4_IRQHandler = Default_IRQHandler								/*!< TIM4 global Interrupt                                             */
#pragma weak I2C1_EV_IRQHandler = Default_IRQHandler						/*!< I2C1 Event Interrupt                                              */
#pragma weak I2C1_ER_IRQHandler = Default_IRQHandler						/*!< I2C1 Error Interrupt                                              */
#pragma weak I2C2_EV_IRQHandler = Default_IRQHandler						/*!< I2C2 Event Interrupt                                              */
#pragma weak I2C2_ER_IRQHandler = Default_IRQHandler						/*!< I2C2 Error Interrupt                                              */
#pragma weak SPI1_IRQHandler = Default_IRQHandler								/*!< SPI1 global Interrupt                                             */
#pragma weak SPI2_IRQHandler = Default_IRQHandler								/*!< SPI2 global Interrupt                                             */
#pragma weak USART1_IRQHandler = Default_IRQHandler							/*!< USART1 global Interrupt                                           */
#pragma weak USART2_IRQHandler = Default_IRQHandler							/*!< USART2 global Interrupt                                           */
#pragma weak USART3_IRQHandler = Default_IRQHandler							/*!< USART3 global Interrupt                                           */
#pragma weak EXTI15_10_IRQHandler = Default_IRQHandler					/*!< External Line[15:10] Interrupts                                   */
#pragma weak RTC_Alarm_IRQHandler = Default_IRQHandler					/*!< RTC Alarm (A and B) through EXTI Line Interrupt                   */
#pragma weak OTG_FS_WKUP_IRQHandler = Default_IRQHandler				/*!< USB OTG FS Wakeup through EXTI line interrupt                     */
#pragma weak TIM8_BRK_TIM12_IRQHandler = Default_IRQHandler			/*!< TIM8 Break Interrupt and TIM12 global interrupt                   */
#pragma weak TIM8_UP_TIM13_IRQHandler = Default_IRQHandler			/*!< TIM8 Update Interrupt and TIM13 global interrupt                  */
#pragma weak TIM8_TRG_COM_TIM14_IRQHandler = Default_IRQHandler /*!< TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt */
#pragma weak TIM8_CC_IRQHandler = Default_IRQHandler						/*!< TIM8 Capture Compare global interrupt                             */
#pragma weak DMA1_Stream7_IRQHandler = Default_IRQHandler				/*!< DMA1 Stream7 Interrupt                                            */
#pragma weak FSMC_IRQHandler = Default_IRQHandler								/*!< FMC global Interrupt                                              */
#pragma weak SDIO_IRQHandler = Default_IRQHandler								/*!< SDIO global Interrupt                                             */
#pragma weak TIM5_IRQHandler = Default_IRQHandler								/*!< TIM5 global Interrupt                                             */
#pragma weak SPI3_IRQHandler = Default_IRQHandler								/*!< SPI3 global Interrupt                                             */
#pragma weak UART4_IRQHandler = Default_IRQHandler							/*!< UART4 global Interrupt                                            */
#pragma weak UART5_IRQHandler = Default_IRQHandler							/*!< UART5 global Interrupt                                            */
#pragma weak TIM6_DAC_IRQHandler = Default_IRQHandler						/*!< TIM6 global and DAC1&2 underrun error  interrupts                 */
#pragma weak TIM7_IRQHandler = Default_IRQHandler								/*!< TIM7 global interrupt                                             */
#pragma weak DMA2_Stream0_IRQHandler = Default_IRQHandler				/*!< DMA2 Stream 0 global Interrupt                                    */
#pragma weak DMA2_Stream1_IRQHandler = Default_IRQHandler				/*!< DMA2 Stream 1 global Interrupt                                    */
#pragma weak DMA2_Stream2_IRQHandler = Default_IRQHandler				/*!< DMA2 Stream 2 global Interrupt                                    */
#pragma weak DMA2_Stream3_IRQHandler = Default_IRQHandler				/*!< DMA2 Stream 3 global Interrupt                                    */
#pragma weak DMA2_Stream4_IRQHandler = Default_IRQHandler				/*!< DMA2 Stream 4 global Interrupt                                    */
#pragma weak ETH_IRQHandler = Default_IRQHandler								/*!< Ethernet global Interrupt                                         */
#pragma weak ETH_WKUP_IRQHandler = Default_IRQHandler						/*!< Ethernet Wakeup through EXTI line Interrupt                       */
#pragma weak CAN2_TX_IRQHandler = Default_IRQHandler						/*!< CAN2 TX Interrupt                                                 */
#pragma weak CAN2_RX0_IRQHandler = Default_IRQHandler						/*!< CAN2 RX0 Interrupt                                                */
#pragma weak CAN2_RX1_IRQHandler = Default_IRQHandler						/*!< CAN2 RX1 Interrupt                                                */
#pragma weak CAN2_SCE_IRQHandler = Default_IRQHandler						/*!< CAN2 SCE Interrupt                                                */
#pragma weak OTG_FS_IRQHandler = Default_IRQHandler							/*!< USB OTG FS global Interrupt                                       */
#pragma weak DMA2_Stream5_IRQHandler = Default_IRQHandler				/*!< DMA2 Stream 5 global interrupt                                    */
#pragma weak DMA2_Stream6_IRQHandler = Default_IRQHandler				/*!< DMA2 Stream 6 global interrupt                                    */
#pragma weak DMA2_Stream7_IRQHandler = Default_IRQHandler				/*!< DMA2 Stream 7 global interrupt                                    */
#pragma weak USART6_IRQHandler = Default_IRQHandler							/*!< USART6 global interrupt                                           */
#pragma weak I2C3_EV_IRQHandler = Default_IRQHandler						/*!< I2C3 event interrupt                                              */
#pragma weak I2C3_ER_IRQHandler = Default_IRQHandler						/*!< I2C3 error interrupt                                              */
#pragma weak OTG_HS_EP1_OUT_IRQHandler = Default_IRQHandler			/*!< USB OTG HS End Point 1 Out global interrupt                       */
#pragma weak OTG_HS_EP1_IN_IRQHandler = Default_IRQHandler			/*!< USB OTG HS End Point 1 In global interrupt                        */
#pragma weak OTG_HS_WKUP_IRQHandler = Default_IRQHandler				/*!< USB OTG HS Wakeup through EXTI interrupt                          */
#pragma weak OTG_HS_IRQHandler = Default_IRQHandler							/*!< USB OTG HS global interrupt                                       */
#pragma weak DCMI_IRQHandler = Default_IRQHandler								/*!< DCMI global interrupt                                             */
#pragma weak CRYP_IRQHandler = Default_IRQHandler								/*!< CRYP crypto global interrupt                                      */
#pragma weak HASH_RNG_IRQHandler = Default_IRQHandler						/*!< Hash and Rng global interrupt                                     */
#pragma weak FPU_IRQHandler = Default_IRQHandler								/*!< FPU global interrupt                                              */
#pragma weak UART7_IRQHandler = Default_IRQHandler							/*!< UART7 global interrupt                                            */
#pragma weak UART8_IRQHandler = Default_IRQHandler							/*!< UART8 global interrupt                                            */
#pragma weak SPI4_IRQHandler = Default_IRQHandler								/*!< SPI4 global Interrupt                                             */
#pragma weak SPI5_IRQHandler = Default_IRQHandler								/*!< SPI5 global Interrupt                                             */
#pragma weak SPI6_IRQHandler = Default_IRQHandler								/*!< SPI6 global Interrupt                                             */
#pragma weak SAI1_IRQHandler = Default_IRQHandler								/*!< SAI1 global Interrupt                                             */
#pragma weak LTDC_IRQHandler = Default_IRQHandler								/*!< LTDC global Interrupt                                              */
#pragma weak LTDC_ER_IRQHandler = Default_IRQHandler						/*!< LTDC Error global Interrupt                                        */
#pragma weak DMA2D_IRQHandler = Default_IRQHandler							/*!< DMA2D global Interrupt                                            */
#pragma weak QUADSPI_IRQHandler = Default_IRQHandler						/*!< QUADSPI global Interrupt                                          */
#pragma weak DSI_IRQHandler = Default_IRQHandler								/*!< DSI global Interrupt                                              */

void Default_IRQHandler(void)
{
	while (1)
	{
	}
}
