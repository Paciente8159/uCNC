# µCNC
µCNC - A universal CNC firmware for microcontrollers

## µCNC for AVR
µCNC for AVR can be built in a couple of ways

## Method one - Arduino IDE
Copy the mcumap_YOUR_BOARD.h , mcumap_avr.h, mcu_avr.c and uCNC.ino file to the parent µCNC folder is (where all µCNC core code is) and open the ino file with Arduino IDE.
You need to edit these files (the ones you copied) to reflect the path changes in the #include declarations of these files and also of the [mcudefs.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcudefs.h) file.
Choose your target board and just compile and load. You're done.

## Method two - Using the makefile
First you must have AVR GCC tools installed on your computer.
You can download the latest version from [here](https://www.microchip.com/mplab/avr-support/avr-and-arm-toolchains-c-compilers)
Then edit the makefile to your specific board
```
BOARD = XX_YOUR_BOARD
MCU = (choose the avr mcu name that matches the mcu on your board)
```
Run the makefile
```
make -f makefile clean all
```

In the build folder you will have your hex file to upload to your board.
