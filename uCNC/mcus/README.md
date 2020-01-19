# uCNC
uCNC - A universal CNC firmware for microcontrollers

## How to add a custom MCU to uCNC
uCNC can be extended to othe MCU as well. For that you only need to create two files:
  1. A correspondent mcumap.h file that defines the pins in your MCU and some custom defines to access ROM/Flash memory (check mcumap_virtual and mcumap_atmega328p to get an ideia).
  2. A file that implements all the functions defined in the mcu.h header file.

  Compile it and test it.
  