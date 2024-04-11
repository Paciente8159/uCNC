_µCNC for AVR can be built in a several ways_

## Method one - PlatformIO (preferred)

1. Get [Visual Studio Code](https://code.visualstudio.com/download) and install it.
2. Install the PlatformIO extension.
3. Open uCNC folder in VSCode.
4. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
5. f needed edit the platformio.ini file environment for your board. Compile the sketch and upload it to your board.

## Method two - Arduino IDE (easiest)
**WARNING:** _Arduino IDE will produce a larger output file. The makefile method has better compilation optimizations that produce a smaller binary file. With that in mind remember that some options can be activated using the makefile and others won't fit the device flash in Arduino IDE (UNO is already near max capabilities)._

1. Get [Arduino IDE](https://www.arduino.cc/en/software) and install it.
2. Go to uCNC folder and open uCNC.ino sketch.
3. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
4. Compile the sketch and upload it to your board.

## Method three - Using the makefile (optimized alternative)

1. Download and install GCC tools for AVR inside your PC. You can download the latest version of GCC tool for AVR from Microchip from [here](https://www.microchip.com/mplab/avr-support/avr-and-arm-toolchains-c-compilers).
   * If your are compiling with this method on a Windows machine you will also need to install Make. You can download Make for Windows from [here](http://gnuwin32.sourceforge.net/packages/make.htm) and CoreUtils [here](http://gnuwin32.sourceforge.net/packages/coreutils.htm).
2. Go to the ```uCNC folder``` and edit the board ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board. µCNC is configured by default to mimic ```Grbl``` pin configuration in the Arduino UNO board.
3. Go to the ```makefiles/avr folder```.
   * The makefile is configured by default to compile the code for the MCU (atmega328p) and working frequency (16Mhz) in Arduino UNO board. If the chosen board has a different MCU/working frequency the makefile must be adjusted by modifying ```CPU = atmega328p``` and ```FREQ = 16000000UL```
4. Open a command console inside ```makefiles/avr``` folder and run ```make clean all```
5. If everything went well you should have a hex file inside ```makefiles/avr/build``` folder
6. Now just upload µCNC to your board using an appropriate tool. [xLoader](http://www.hobbytronics.co.uk/download/XLoader.zip) for AVR is an easy tool to use.