_µCNC for STM32F1x/STM32F4x can be built this way_

## Method one - PlatformIO (preferred)

1. Get [Visual Studio Code](https://code.visualstudio.com/download) and install it.
2. Install the PlatformIO extension.
3. Open uCNC folder in VSCode.
4. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
5. If needed edit the platformio.ini file environment for your board. Compile the sketch and upload it to your board.

## Method two - Arduino IDE (easiest)
**WARNING:** _Arduino IDE will produce a larger output file. The makefile method has better compilation optimizations that produce a smaller binary file._

1. Get [Arduino IDE](https://www.arduino.cc/en/software) and install it.
2. If you don't have install STM32duino with Arduino board manager has explained [here](https://github.com/stm32duino/wiki/wiki/Getting-Started)
3. Go to uCNC folder and open uCNC.ino sketch.
4. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
5. Compile the sketch and upload it to your board (via maple bootloader or other method).

_**Notes:** Your board should have USART and USB support options disabled. µCNC takes care of these by it's own._

## Method three - Using the makefile (optimized binary alternative)

1. Download and install GCC tools for ARM inside your PC. You can download the latest version of GCC tool for ARM from [here](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads).
   * If your are compiling with this method on a Windows machine you will also need to install Make. You can download Make for Windows from [here](http://gnuwin32.sourceforge.net/packages/make.htm) and CoreUtils [here](http://gnuwin32.sourceforge.net/packages/coreutils.htm).
2. Go to the ```uCNC folder``` and edit the board ```cnc_config.h file``` and ```cnc_hal_config.h file``` if you need to select a different ARM board or different functionalities. µCNC is configured by default to mimic ```Grbl``` pin configuration in the Arduino UNO board.
3. Open a command console inside ```makefiles/stm32f1x``` / ```makefiles/stm32f4x``` folder and run ```make clean all```
4. If everything went well you should have a hex file inside ```makefiles/stm32f1x/build``` / ```makefiles/stm32f4x/build```folder.
5. Now just upload µCNC to your board using an appropriate tool and programmer.
