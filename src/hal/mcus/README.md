# µCNC
µCNC - A universal CNC firmware for microcontrollers

## How to add a custom MCU to µCNC
µCNC can be extended to other custom MCU as well. For that you only need to define how your mcu responds to the HAL interface:
  1. Create the needed files that implements all the functions defined in the [mcu.h](https://github.com/Paciente8159/uCNC/blob/master/src/hal/mcus/mcu.h) header file. mcu.h acts has the HAL interface and contains all the needed mcu function declarations that µCNC needs (direct IO operations, pwm outputs, analog reading, communications, etc...).
  2. You must also edit [mcus.h](https://github.com/Paciente8159/uCNC/blob/master/src/hal/mcus/mcus.h) and [mcudefs.h](https://github.com/Paciente8159/uCNC/blob/master/src/hal/mcus/mcudefs.h) to add you custom MCU.

  Compile it and test it.
  