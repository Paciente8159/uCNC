# µCNC
µCNC - A universal CNC firmware for microcontrollers

## How to add a custom MCU to µCNC
µCNC can be extended to other custom MCU as well. For that you only need to create two files:
  1. A correspondent `mcumap_YOUR-MCU.h` file that defines the pins equivalent masks in your MCU, some custom definitions to access ROM/Flash memory, and special math operations (check [mcumap_virtual.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcus/virtual/mcumap_virtual.h) and [mcumap_atmega328p.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcus/avr/uno/mcumap_atmega328p.h) to get an idea).
  2. A `mcu_YOUR-MCU.c` file that implements all the functions defined in the `mcu_YOUR-MCU.h` header file. Note: [mcu.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcu.h) already has implemented the necessary instructions to perform direct IO operations to digital pins.
  3. You must also edit [mcus.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcus.h) and [mcudefs.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcudefs.h) to add you custom MCU.

  Compile and test it.
  