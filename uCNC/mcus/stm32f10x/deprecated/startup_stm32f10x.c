#include "config.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>
//#include "system_stm32f10x.h"

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void (*const pfunc)(void);

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t _stext;  /* start of program code */
extern uint32_t _etext;  /* end of end of program code */
extern uint32_t _sdata;  /* start of initialized global and static variables */
extern uint32_t _edata;  /* end of initialized global and static variables */
extern uint32_t _sbss;   /* start of uninitialized global and static variables */
extern uint32_t _ebss;   /* end of uninitialized global and static variables */
extern uint32_t _sstack; /* start of stack */
extern uint32_t _estack; /* end of stack */

extern void main(void) __attribute__((noreturn));

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
static void Default_IRQHandler(void) __attribute__((interrupt, noreturn));
static void Reset_IRQHandler(void) __attribute__((interrupt, noreturn));

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/

/*void *****  Cortex-M3 Processor Exceptions Numbers ***************************************************/
void NonMaskableInt_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void HardFault_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void MemoryManagement_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void BusFault_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void UsageFault_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SVCall_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DebugMonitor_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void PendSV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SysTick_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));

/******  STM32 specific Interrupt Numbers *********************************************************/
void WWDG_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void PVD_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TAMPER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void RTC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void FLASH_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void RCC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI0_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA1_Channel1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA1_Channel2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA1_Channel3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA1_Channel4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA1_Channel5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA1_Channel6_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA1_Channel7_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));

#ifdef STM32F10X_LD
void ADC1_2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USB_HP_CAN1_TX_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_RX1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_SCE_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_BRK_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_UP_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_TRG_COM_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void RTCAlarm_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USBWakeUp_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
#endif /* STM32F10X_LD */  

#ifdef STM32F10X_LD_VL
void ADC1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_BRK_TIM15_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_UP_TIM16_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_TRG_COM_TIM17_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void RTCAlarm_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CEC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM6_DAC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM7_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
#endif /* STM32F10X_LD_VL */

#ifdef STM32F10X_MD
void ADC1_2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USB_HP_CAN1_TX_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_RX1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_SCE_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_BRK_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_UP_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_TRG_COM_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void RTCAlarm_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USBWakeUp_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
#endif /* STM32F10X_MD */  

#ifdef STM32F10X_MD_VL
void ADC1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_BRK_TIM15_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_UP_TIM16_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_TRG_COM_TIM17_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void RTCAlarm_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CEC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM6_DAC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM7_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
#endif /* STM32F10X_MD_VL */

#ifdef STM32F10X_HD
void ADC1_2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USB_HP_CAN1_TX_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_RX1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_SCE_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_BRK_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_UP_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_TRG_COM_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void RTCAlarm_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USBWakeUp_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM8_BRK_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM8_UP_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM8_TRG_COM_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM8_CC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void ADC3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void FSMC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SDIO_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void UART4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void UART5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM6_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM7_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel4_5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
#endif /* STM32F10X_HD */  

#ifdef STM32F10X_HD_VL
void ADC1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_BRK_TIM15_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_UP_TIM16_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_TRG_COM_TIM17_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void RTCAlarm_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CEC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM12_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM13_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM14_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void UART4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void UART5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM6_DAC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM7_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel4_5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
                                mapped at position 60 only if the MISC_REMAP bit in 
                                the AFIO_MAPR2 register is set)                      */       
#endif /* STM32F10X_HD_VL */

#ifdef STM32F10X_XL
void ADC1_2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USB_HP_CAN1_TX_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_RX1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_SCE_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_BRK_TIM9_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_UP_TIM10_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_TRG_COM_TIM11_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void RTCAlarm_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USBWakeUp_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM8_BRK_TIM12_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM8_UP_TIM13_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM8_TRG_COM_TIM14_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM8_CC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void ADC3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void FSMC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SDIO_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void UART4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void UART5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM6_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM7_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel4_5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
#endif /* STM32F10X_XL */  

#ifdef STM32F10X_CL
void ADC1_2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_TX_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_RX0_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_RX1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN1_SCE_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_BRK_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_UP_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_TRG_COM_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void USART3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void RTCAlarm_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void OTG_FS_WKUP_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void SPI3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void UART4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void UART5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM6_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void TIM7_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel2_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel3_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel4_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void DMA2_Channel5_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void ETH_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void ETH_WKUP_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN2_TX_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN2_RX0_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN2_RX1_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void CAN2_SCE_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
void OTG_FS_IRQHandler(void) __attribute__((weak, interrupt, interrupt, alias("Default_IRQHandler")));
#endif /* STM32F10X_CL */   

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
__attribute__((section(".isr_vector"))) void (*const g_pfnVectors[])(void) = {
    (pfunc)((unsigned long)&_estack),
    Reset_IRQHandler,      /*     Reset Handler */
    NonMaskableInt_IRQHandler,        /* -14 NMI Handler */
    HardFault_IRQHandler,  /* -13 Hard Fault Handler */
    MemoryManagement_IRQHandler,  /* -12 MPU Fault Handler */
    BusFault_IRQHandler,   /* -11 Bus Fault Handler */
    UsageFault_IRQHandler, /* -10 Usage Fault Handler */
    0,              /*     Reserved */
    SVCall_IRQHandler,     /*  -5 SVCall Handler */
    DebugMonitor_IRQHandler,   /*  -4 Debug Monitor Handler */
    0,              /*     Reserved */
    PendSV_IRQHandler,     /*  -2 PendSV Handler */
    SysTick_IRQHandler,    /*  -1 SysTick Handler */

    /* Interrupts */
    /******  STM32 specific Interrupt Numbers *********************************************************/
    WWDG_IRQHandler,          /*!< Window WatchDog Interrupt                            */
    PVD_IRQHandler,           /*!< PVD through EXTI Line detection Interrupt            */
    TAMPER_IRQHandler,        /*!< Tamper Interrupt                                     */
    RTC_IRQHandler,           /*!< RTC global Interrupt                                 */
    FLASH_IRQHandler,         /*!< FLASH global Interrupt                               */
    RCC_IRQHandler,           /*!< RCC global Interrupt                                 */
    EXTI0_IRQHandler,         /*!< EXTI Line0 Interrupt                                 */
    EXTI1_IRQHandler,         /*!< EXTI Line1 Interrupt                                 */
    EXTI2_IRQHandler,         /*!< EXTI Line2 Interrupt                                 */
    EXTI3_IRQHandler,         /*!< EXTI Line3 Interrupt                                 */
    EXTI4_IRQHandler,         /*!< EXTI Line4 Interrupt                                 */
    DMA1_Channel1_IRQHandler, /*!< DMA1 Channel 1 global Interrupt                      */
    DMA1_Channel2_IRQHandler, /*!< DMA1 Channel 2 global Interrupt                      */
    DMA1_Channel3_IRQHandler, /*!< DMA1 Channel 3 global Interrupt                      */
    DMA1_Channel4_IRQHandler, /*!< DMA1 Channel 4 global Interrupt                      */
    DMA1_Channel5_IRQHandler, /*!< DMA1 Channel 5 global Interrupt                      */
    DMA1_Channel6_IRQHandler, /*!< DMA1 Channel 6 global Interrupt                      */
    DMA1_Channel7_IRQHandler, /*!< DMA1 Channel 7 global Interrupt                      */

#ifdef STM32F10X_LD
    ADC1_2_IRQHandler,          /*!< ADC1 and ADC2 global Interrupt                       */
    USB_HP_CAN1_TX_IRQHandler,  /*!< USB Device High Priority or CAN1 TX Interrupts       */
    USB_LP_CAN1_RX0_IRQHandler, /*!< USB Device Low Priority or CAN1 RX0 Interrupts       */
    CAN1_RX1_IRQHandler,        /*!< CAN1 RX1 Interrupt                                   */
    CAN1_SCE_IRQHandler,        /*!< CAN1 SCE Interrupt                                   */
    EXTI9_5_IRQHandler,         /*!< External Line[9:5] Interrupts                        */
    TIM1_BRK_IRQHandler,        /*!< TIM1 Break Interrupt                                 */
    TIM1_UP_IRQHandler,         /*!< TIM1 Update Interrupt                                */
    TIM1_TRG_COM_IRQHandler,    /*!< TIM1 Trigger and Commutation Interrupt               */
    TIM1_CC_IRQHandler,         /*!< TIM1 Capture Compare Interrupt                       */
    TIM2_IRQHandler,            /*!< TIM2 global Interrupt                                */
    TIM3_IRQHandler,            /*!< TIM3 global Interrupt                                */
    I2C1_EV_IRQHandler,         /*!< I2C1 Event Interrupt                                 */
    I2C1_ER_IRQHandler,         /*!< I2C1 Error Interrupt                                 */
    SPI1_IRQHandler,            /*!< SPI1 global Interrupt                                */
    USART1_IRQHandler,          /*!< USART1 global Interrupt                              */
    USART2_IRQHandler,          /*!< USART2 global Interrupt                              */
    EXTI15_10_IRQHandler,       /*!< External Line[15:10] Interrupts                      */
    RTCAlarm_IRQHandler,        /*!< RTC Alarm through EXTI Line Interrupt                */
    USBWakeUp_IRQHandler        /*!< USB Device WakeUp from suspend through EXTI Line Interrupt */
#endif                   /* STM32F10X_LD */

#ifdef STM32F10X_LD_VL
        ADC1_IRQHandler,           /*!< ADC1 global Interrupt                                */
    EXTI9_5_IRQHandler,            /*!< External Line[9:5] Interrupts                        */
    TIM1_BRK_TIM15_IRQHandler,     /*!< TIM1 Break and TIM15 Interrupts                      */
    TIM1_UP_TIM16_IRQHandler,      /*!< TIM1 Update and TIM16 Interrupts                     */
    TIM1_TRG_COM_TIM17_IRQHandler, /*!< TIM1 Trigger and Commutation and TIM17 Interrupt     */
    TIM1_CC_IRQHandler,            /*!< TIM1 Capture Compare Interrupt                       */
    TIM2_IRQHandler,               /*!< TIM2 global Interrupt                                */
    TIM3_IRQHandler,               /*!< TIM3 global Interrupt                                */
    I2C1_EV_IRQHandler,            /*!< I2C1 Event Interrupt                                 */
    I2C1_ER_IRQHandler,            /*!< I2C1 Error Interrupt                                 */
    SPI1_IRQHandler,               /*!< SPI1 global Interrupt                                */
    USART1_IRQHandler,             /*!< USART1 global Interrupt                              */
    USART2_IRQHandler,             /*!< USART2 global Interrupt                              */
    EXTI15_10_IRQHandler,          /*!< External Line[15:10] Interrupts                      */
    RTCAlarm_IRQHandler,           /*!< RTC Alarm through EXTI Line Interrupt                */
    CEC_IRQHandler,                /*!< HDMI-CEC Interrupt                                   */
    TIM6_DAC_IRQHandler,           /*!< TIM6 and DAC underrun Interrupt                      */
    TIM7_IRQHandler                /*!< TIM7 Interrupt                                       */
#endif                      /* STM32F10X_LD_VL */

#ifdef STM32F10X_MD
        ADC1_2_IRQHandler,      /*!< ADC1 and ADC2 global Interrupt                       */
    USB_HP_CAN1_TX_IRQHandler,  /*!< USB Device High Priority or CAN1 TX Interrupts       */
    USB_LP_CAN1_RX0_IRQHandler, /*!< USB Device Low Priority or CAN1 RX0 Interrupts       */
    CAN1_RX1_IRQHandler,        /*!< CAN1 RX1 Interrupt                                   */
    CAN1_SCE_IRQHandler,        /*!< CAN1 SCE Interrupt                                   */
    EXTI9_5_IRQHandler,         /*!< External Line[9:5] Interrupts                        */
    TIM1_BRK_IRQHandler,        /*!< TIM1 Break Interrupt                                 */
    TIM1_UP_IRQHandler,         /*!< TIM1 Update Interrupt                                */
    TIM1_TRG_COM_IRQHandler,    /*!< TIM1 Trigger and Commutation Interrupt               */
    TIM1_CC_IRQHandler,         /*!< TIM1 Capture Compare Interrupt                       */
    TIM2_IRQHandler,            /*!< TIM2 global Interrupt                                */
    TIM3_IRQHandler,            /*!< TIM3 global Interrupt                                */
    TIM4_IRQHandler,            /*!< TIM4 global Interrupt                                */
    I2C1_EV_IRQHandler,         /*!< I2C1 Event Interrupt                                 */
    I2C1_ER_IRQHandler,         /*!< I2C1 Error Interrupt                                 */
    I2C2_EV_IRQHandler,         /*!< I2C2 Event Interrupt                                 */
    I2C2_ER_IRQHandler,         /*!< I2C2 Error Interrupt                                 */
    SPI1_IRQHandler,            /*!< SPI1 global Interrupt                                */
    SPI2_IRQHandler,            /*!< SPI2 global Interrupt                                */
    USART1_IRQHandler,          /*!< USART1 global Interrupt                              */
    USART2_IRQHandler,          /*!< USART2 global Interrupt                              */
    USART3_IRQHandler,          /*!< USART3 global Interrupt                              */
    EXTI15_10_IRQHandler,       /*!< External Line[15:10] Interrupts                      */
    RTCAlarm_IRQHandler,        /*!< RTC Alarm through EXTI Line Interrupt                */
    USBWakeUp_IRQHandler        /*!< USB Device WakeUp from suspend through EXTI Line Interrupt */
#endif                   /* STM32F10X_MD */

#ifdef STM32F10X_MD_VL
        ADC1_IRQHandler,           /*!< ADC1 global Interrupt                                */
    EXTI9_5_IRQHandler,            /*!< External Line[9:5] Interrupts                        */
    TIM1_BRK_TIM15_IRQHandler,     /*!< TIM1 Break and TIM15 Interrupts                      */
    TIM1_UP_TIM16_IRQHandler,      /*!< TIM1 Update and TIM16 Interrupts                     */
    TIM1_TRG_COM_TIM17_IRQHandler, /*!< TIM1 Trigger and Commutation and TIM17 Interrupt     */
    TIM1_CC_IRQHandler,            /*!< TIM1 Capture Compare Interrupt                       */
    TIM2_IRQHandler,               /*!< TIM2 global Interrupt                                */
    TIM3_IRQHandler,               /*!< TIM3 global Interrupt                                */
    TIM4_IRQHandler,               /*!< TIM4 global Interrupt                                */
    I2C1_EV_IRQHandler,            /*!< I2C1 Event Interrupt                                 */
    I2C1_ER_IRQHandler,            /*!< I2C1 Error Interrupt                                 */
    I2C2_EV_IRQHandler,            /*!< I2C2 Event Interrupt                                 */
    I2C2_ER_IRQHandler,            /*!< I2C2 Error Interrupt                                 */
    SPI1_IRQHandler,               /*!< SPI1 global Interrupt                                */
    SPI2_IRQHandler,               /*!< SPI2 global Interrupt                                */
    USART1_IRQHandler,             /*!< USART1 global Interrupt                              */
    USART2_IRQHandler,             /*!< USART2 global Interrupt                              */
    USART3_IRQHandler,             /*!< USART3 global Interrupt                              */
    EXTI15_10_IRQHandler,          /*!< External Line[15:10] Interrupts                      */
    RTCAlarm_IRQHandler,           /*!< RTC Alarm through EXTI Line Interrupt                */
    CEC_IRQHandler,                /*!< HDMI-CEC Interrupt                                   */
    TIM6_DAC_IRQHandler,           /*!< TIM6 and DAC underrun Interrupt                      */
    TIM7_IRQHandler                /*!< TIM7 Interrupt                                       */
#endif                      /* STM32F10X_MD_VL */

#ifdef STM32F10X_HD
        ADC1_2_IRQHandler,      /*!< ADC1 and ADC2 global Interrupt                       */
    USB_HP_CAN1_TX_IRQHandler,  /*!< USB Device High Priority or CAN1 TX Interrupts       */
    USB_LP_CAN1_RX0_IRQHandler, /*!< USB Device Low Priority or CAN1 RX0 Interrupts       */
    CAN1_RX1_IRQHandler,        /*!< CAN1 RX1 Interrupt                                   */
    CAN1_SCE_IRQHandler,        /*!< CAN1 SCE Interrupt                                   */
    EXTI9_5_IRQHandler,         /*!< External Line[9:5] Interrupts                        */
    TIM1_BRK_IRQHandler,        /*!< TIM1 Break Interrupt                                 */
    TIM1_UP_IRQHandler,         /*!< TIM1 Update Interrupt                                */
    TIM1_TRG_COM_IRQHandler,    /*!< TIM1 Trigger and Commutation Interrupt               */
    TIM1_CC_IRQHandler,         /*!< TIM1 Capture Compare Interrupt                       */
    TIM2_IRQHandler,            /*!< TIM2 global Interrupt                                */
    TIM3_IRQHandler,            /*!< TIM3 global Interrupt                                */
    TIM4_IRQHandler,            /*!< TIM4 global Interrupt                                */
    I2C1_EV_IRQHandler,         /*!< I2C1 Event Interrupt                                 */
    I2C1_ER_IRQHandler,         /*!< I2C1 Error Interrupt                                 */
    I2C2_EV_IRQHandler,         /*!< I2C2 Event Interrupt                                 */
    I2C2_ER_IRQHandler,         /*!< I2C2 Error Interrupt                                 */
    SPI1_IRQHandler,            /*!< SPI1 global Interrupt                                */
    SPI2_IRQHandler,            /*!< SPI2 global Interrupt                                */
    USART1_IRQHandler,          /*!< USART1 global Interrupt                              */
    USART2_IRQHandler,          /*!< USART2 global Interrupt                              */
    USART3_IRQHandler,          /*!< USART3 global Interrupt                              */
    EXTI15_10_IRQHandler,       /*!< External Line[15:10] Interrupts                      */
    RTCAlarm_IRQHandler,        /*!< RTC Alarm through EXTI Line Interrupt                */
    USBWakeUp_IRQHandler,       /*!< USB Device WakeUp from suspend through EXTI Line Interrupt */
    TIM8_BRK_IRQHandler,        /*!< TIM8 Break Interrupt                                 */
    TIM8_UP_IRQHandler,         /*!< TIM8 Update Interrupt                                */
    TIM8_TRG_COM_IRQHandler,    /*!< TIM8 Trigger and Commutation Interrupt               */
    TIM8_CC_IRQHandler,         /*!< TIM8 Capture Compare Interrupt                       */
    ADC3_IRQHandler,            /*!< ADC3 global Interrupt                                */
    FSMC_IRQHandler,            /*!< FSMC global Interrupt                                */
    SDIO_IRQHandler,            /*!< SDIO global Interrupt                                */
    TIM5_IRQHandler,            /*!< TIM5 global Interrupt                                */
    SPI3_IRQHandler,            /*!< SPI3 global Interrupt                                */
    UART4_IRQHandler,           /*!< UART4 global Interrupt                               */
    UART5_IRQHandler,           /*!< UART5 global Interrupt                               */
    TIM6_IRQHandler,            /*!< TIM6 global Interrupt                                */
    TIM7_IRQHandler,            /*!< TIM7 global Interrupt                                */
    DMA2_Channel1_IRQHandler,   /*!< DMA2 Channel 1 global Interrupt                      */
    DMA2_Channel2_IRQHandler,   /*!< DMA2 Channel 2 global Interrupt                      */
    DMA2_Channel3_IRQHandler,   /*!< DMA2 Channel 3 global Interrupt                      */
    DMA2_Channel4_5_IRQHandler  /*!< DMA2 Channel 4 and Channel 5 global Interrupt        */
#endif                   /* STM32F10X_HD */

#ifdef STM32F10X_HD_VL
        ADC1_IRQHandler,           /*!< ADC1 global Interrupt                                */
    EXTI9_5_IRQHandler,            /*!< External Line[9:5] Interrupts                        */
    TIM1_BRK_TIM15_IRQHandler,     /*!< TIM1 Break and TIM15 Interrupts                      */
    TIM1_UP_TIM16_IRQHandler,      /*!< TIM1 Update and TIM16 Interrupts                     */
    TIM1_TRG_COM_TIM17_IRQHandler, /*!< TIM1 Trigger and Commutation and TIM17 Interrupt     */
    TIM1_CC_IRQHandler,            /*!< TIM1 Capture Compare Interrupt                       */
    TIM2_IRQHandler,               /*!< TIM2 global Interrupt                                */
    TIM3_IRQHandler,               /*!< TIM3 global Interrupt                                */
    TIM4_IRQHandler,               /*!< TIM4 global Interrupt                                */
    I2C1_EV_IRQHandler,            /*!< I2C1 Event Interrupt                                 */
    I2C1_ER_IRQHandler,            /*!< I2C1 Error Interrupt                                 */
    I2C2_EV_IRQHandler,            /*!< I2C2 Event Interrupt                                 */
    I2C2_ER_IRQHandler,            /*!< I2C2 Error Interrupt                                 */
    SPI1_IRQHandler,               /*!< SPI1 global Interrupt                                */
    SPI2_IRQHandler,               /*!< SPI2 global Interrupt                                */
    USART1_IRQHandler,             /*!< USART1 global Interrupt                              */
    USART2_IRQHandler,             /*!< USART2 global Interrupt                              */
    USART3_IRQHandler,             /*!< USART3 global Interrupt                              */
    EXTI15_10_IRQHandler,          /*!< External Line[15:10] Interrupts                      */
    RTCAlarm_IRQHandler,           /*!< RTC Alarm through EXTI Line Interrupt                */
    CEC_IRQHandler,                /*!< HDMI-CEC Interrupt                                   */
    TIM12_IRQHandler,              /*!< TIM12 global Interrupt                               */
    TIM13_IRQHandler,              /*!< TIM13 global Interrupt                               */
    TIM14_IRQHandler,              /*!< TIM14 global Interrupt                               */
    TIM5_IRQHandler,               /*!< TIM5 global Interrupt                                */
    SPI3_IRQHandler,               /*!< SPI3 global Interrupt                                */
    UART4_IRQHandler,              /*!< UART4 global Interrupt                               */
    UART5_IRQHandler,              /*!< UART5 global Interrupt                               */
    TIM6_DAC_IRQHandler,           /*!< TIM6 and DAC underrun Interrupt                      */
    TIM7_IRQHandler,               /*!< TIM7 Interrupt                                       */
    DMA2_Channel1_IRQHandler,      /*!< DMA2 Channel 1 global Interrupt                      */
    DMA2_Channel2_IRQHandler,      /*!< DMA2 Channel 2 global Interrupt                      */
    DMA2_Channel3_IRQHandler,      /*!< DMA2 Channel 3 global Interrupt                      */
    DMA2_Channel4_5_IRQHandler,    /*!< DMA2 Channel 4 and Channel 5 global Interrupt        */
    DMA2_Channel5_IRQHandler       /*!< DMA2 Channel 5 global Interrupt (DMA2 Channel 5 is 
                                             mapped at position 60 only if the MISC_REMAP bit in 
                                             the AFIO_MAPR2 register is set)                      */
#endif                      /* STM32F10X_HD_VL */

#ifdef STM32F10X_XL
        ADC1_2_IRQHandler,         /*!< ADC1 and ADC2 global Interrupt                       */
    USB_HP_CAN1_TX_IRQHandler,     /*!< USB Device High Priority or CAN1 TX Interrupts       */
    USB_LP_CAN1_RX0_IRQHandler,    /*!< USB Device Low Priority or CAN1 RX0 Interrupts       */
    CAN1_RX1_IRQHandler,           /*!< CAN1 RX1 Interrupt                                   */
    CAN1_SCE_IRQHandler,           /*!< CAN1 SCE Interrupt                                   */
    EXTI9_5_IRQHandler,            /*!< External Line[9:5] Interrupts                        */
    TIM1_BRK_TIM9_IRQHandler,      /*!< TIM1 Break Interrupt and TIM9 global Interrupt       */
    TIM1_UP_TIM10_IRQHandler,      /*!< TIM1 Update Interrupt and TIM10 global Interrupt     */
    TIM1_TRG_COM_TIM11_IRQHandler, /*!< TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt */
    TIM1_CC_IRQHandler,            /*!< TIM1 Capture Compare Interrupt                       */
    TIM2_IRQHandler,               /*!< TIM2 global Interrupt                                */
    TIM3_IRQHandler,               /*!< TIM3 global Interrupt                                */
    TIM4_IRQHandler,               /*!< TIM4 global Interrupt                                */
    I2C1_EV_IRQHandler,            /*!< I2C1 Event Interrupt                                 */
    I2C1_ER_IRQHandler,            /*!< I2C1 Error Interrupt                                 */
    I2C2_EV_IRQHandler,            /*!< I2C2 Event Interrupt                                 */
    I2C2_ER_IRQHandler,            /*!< I2C2 Error Interrupt                                 */
    SPI1_IRQHandler,               /*!< SPI1 global Interrupt                                */
    SPI2_IRQHandler,               /*!< SPI2 global Interrupt                                */
    USART1_IRQHandler,             /*!< USART1 global Interrupt                              */
    USART2_IRQHandler,             /*!< USART2 global Interrupt                              */
    USART3_IRQHandler,             /*!< USART3 global Interrupt                              */
    EXTI15_10_IRQHandler,          /*!< External Line[15:10] Interrupts                      */
    RTCAlarm_IRQHandler,           /*!< RTC Alarm through EXTI Line Interrupt                */
    USBWakeUp_IRQHandler,          /*!< USB Device WakeUp from suspend through EXTI Line Interrupt */
    TIM8_BRK_TIM12_IRQHandler,     /*!< TIM8 Break Interrupt and TIM12 global Interrupt      */
    TIM8_UP_TIM13_IRQHandler,      /*!< TIM8 Update Interrupt and TIM13 global Interrupt     */
    TIM8_TRG_COM_TIM14_IRQHandler, /*!< TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt */
    TIM8_CC_IRQHandler,            /*!< TIM8 Capture Compare Interrupt                       */
    ADC3_IRQHandler,               /*!< ADC3 global Interrupt                                */
    FSMC_IRQHandler,               /*!< FSMC global Interrupt                                */
    SDIO_IRQHandler,               /*!< SDIO global Interrupt                                */
    TIM5_IRQHandler,               /*!< TIM5 global Interrupt                                */
    SPI3_IRQHandler,               /*!< SPI3 global Interrupt                                */
    UART4_IRQHandler,              /*!< UART4 global Interrupt                               */
    UART5_IRQHandler,              /*!< UART5 global Interrupt                               */
    TIM6_IRQHandler,               /*!< TIM6 global Interrupt                                */
    TIM7_IRQHandler,               /*!< TIM7 global Interrupt                                */
    DMA2_Channel1_IRQHandler,      /*!< DMA2 Channel 1 global Interrupt                      */
    DMA2_Channel2_IRQHandler,      /*!< DMA2 Channel 2 global Interrupt                      */
    DMA2_Channel3_IRQHandler,      /*!< DMA2 Channel 3 global Interrupt                      */
    DMA2_Channel4_5_IRQHandler     /*!< DMA2 Channel 4 and Channel 5 global Interrupt        */
#endif                      /* STM32F10X_XL */

#ifdef STM32F10X_CL
        ADC1_2_IRQHandler,    /*!< ADC1 and ADC2 global Interrupt                       */
    CAN1_TX_IRQHandler,       /*!< USB Device High Priority or CAN1 TX Interrupts       */
    CAN1_RX0_IRQHandler,      /*!< USB Device Low Priority or CAN1 RX0 Interrupts       */
    CAN1_RX1_IRQHandler,      /*!< CAN1 RX1 Interrupt                                   */
    CAN1_SCE_IRQHandler,      /*!< CAN1 SCE Interrupt                                   */
    EXTI9_5_IRQHandler,       /*!< External Line[9:5] Interrupts                        */
    TIM1_BRK_IRQHandler,      /*!< TIM1 Break Interrupt                                 */
    TIM1_UP_IRQHandler,       /*!< TIM1 Update Interrupt                                */
    TIM1_TRG_COM_IRQHandler,  /*!< TIM1 Trigger and Commutation Interrupt               */
    TIM1_CC_IRQHandler,       /*!< TIM1 Capture Compare Interrupt                       */
    TIM2_IRQHandler,          /*!< TIM2 global Interrupt                                */
    TIM3_IRQHandler,          /*!< TIM3 global Interrupt                                */
    TIM4_IRQHandler,          /*!< TIM4 global Interrupt                                */
    I2C1_EV_IRQHandler,       /*!< I2C1 Event Interrupt                                 */
    I2C1_ER_IRQHandler,       /*!< I2C1 Error Interrupt                                 */
    I2C2_EV_IRQHandler,       /*!< I2C2 Event Interrupt                                 */
    I2C2_ER_IRQHandler,       /*!< I2C2 Error Interrupt                                 */
    SPI1_IRQHandler,          /*!< SPI1 global Interrupt                                */
    SPI2_IRQHandler,          /*!< SPI2 global Interrupt                                */
    USART1_IRQHandler,        /*!< USART1 global Interrupt                              */
    USART2_IRQHandler,        /*!< USART2 global Interrupt                              */
    USART3_IRQHandler,        /*!< USART3 global Interrupt                              */
    EXTI15_10_IRQHandler,     /*!< External Line[15:10] Interrupts                      */
    RTCAlarm_IRQHandler,      /*!< RTC Alarm through EXTI Line Interrupt                */
    OTG_FS_WKUP_IRQHandler,   /*!< USB OTG FS WakeUp from suspend through EXTI Line Interrupt */
    TIM5_IRQHandler,          /*!< TIM5 global Interrupt                                */
    SPI3_IRQHandler,          /*!< SPI3 global Interrupt                                */
    UART4_IRQHandler,         /*!< UART4 global Interrupt                               */
    UART5_IRQHandler,         /*!< UART5 global Interrupt                               */
    TIM6_IRQHandler,          /*!< TIM6 global Interrupt                                */
    TIM7_IRQHandler,          /*!< TIM7 global Interrupt                                */
    DMA2_Channel1_IRQHandler, /*!< DMA2 Channel 1 global Interrupt                      */
    DMA2_Channel2_IRQHandler, /*!< DMA2 Channel 2 global Interrupt                      */
    DMA2_Channel3_IRQHandler, /*!< DMA2 Channel 3 global Interrupt                      */
    DMA2_Channel4_IRQHandler, /*!< DMA2 Channel 4 global Interrupt                      */
    DMA2_Channel5_IRQHandler, /*!< DMA2 Channel 5 global Interrupt                      */
    ETH_IRQHandler,           /*!< Ethernet global Interrupt                            */
    ETH_WKUP_IRQHandler,      /*!< Ethernet Wakeup through EXTI line Interrupt          */
    CAN2_TX_IRQHandler,       /*!< CAN2 TX Interrupt                                    */
    CAN2_RX0_IRQHandler,      /*!< CAN2 RX0 Interrupt                                   */
    CAN2_RX1_IRQHandler,      /*!< CAN2 RX1 Interrupt                                   */
    CAN2_SCE_IRQHandler,      /*!< CAN2 SCE Interrupt                                   */
    OTG_FS_IRQHandler         /*!< USB OTG FS global Interrupt                          */
#endif                 /* STM32F10X_CL */
};

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_IRQHandler(void)
{
    unsigned long *src, *dst;
    /* copy the data segment into ram */
    src = &_etext;
    dst = &_sdata;
    if (src != dst)
    {
        while (dst < &_edata)
        {
            *(dst++) = *(src++);
        }
    }

    /* zero the bss segment */
    dst = &_sbss;
    while (dst < &_ebss)
    {
        *(dst++) = 0;
    }

    /* initializes the clocks, pll and vector table realocation */
    SystemInit();

    main(); /* Enter main */
}

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_IRQHandler(void)
{
    while (1)
        ;
}
