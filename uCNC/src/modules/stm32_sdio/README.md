# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About STM32 SDIO for µCNC

Adds SDIO support for µCNC. This module is intended to be used with the SD Card module to allow using STM32 SDIO interface.

## Adding STM32 SDIO for µCNC

To use the and SD Card v2 follow these steps:

1. Copy the the `stm32_sdio` directory and place it inside the `src/modules/` directory of µCNC
2. This module must be initialized before the SD Card module.
