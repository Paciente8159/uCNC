# µCNC
µCNC - A universal CNC firmware for microcontrollers

## How to add a custom MCU to µCNC
µCNC can be extended to othe MCU as well. For that you only need to create two files:
  1. A correspondent uCNC/mcumap.h file that defines the pins in your MCU and some custom defines to access ROM/Flash memory (check [mcumap_virtual.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcus/virtual/mcumap_virtual.h) and [mcumap_atmega328p.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcus/avr/uno/mcumap_atmega328p.h) to get an ideia).
  2. A file that implements all the functions defined in the [mcu.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcu.h) header file.
  3. You must also edit [mcus.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcus.h) and [mcudefs.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcudefs.h) to add you custom MCU.

  Compile it and test it.
  