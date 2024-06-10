_µCNC for STM32F0x can be built this way_

## Method one - PlatformIO (preferred)

1. Get [Visual Studio Code](https://code.visualstudio.com/download) and install it.
2. Install the PlatformIO extension.
3. Open uCNC folder in VSCode.
4. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
5. You might need to adapt the ```src/hal/boards/stm32/stm32.ini file``` and/or ```boards/genericSTM32F0.json file``` to your specific needs/board. 
6. If needed edit the platformio.ini file environment for your board. Compile the sketch and upload it to your board.

## Method two - Arduino IDE (easiest)
**WARNING:** _Arduino IDE will produce a larger output file. The makefile method has better compilation optimizations that produce a smaller binary file._

1. Get [Arduino IDE](https://www.arduino.cc/en/software) and install it.
2. If you don't have install STM32duino with Arduino board manager has explained [here](https://github.com/stm32duino/wiki/wiki/Getting-Started)
3. Go to uCNC folder and open uCNC.ino sketch.
4. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
5. Compile the sketch and upload it to your board (via maple bootloader or other method).

_**Notes:** Your board should have USART and USB support options disabled. µCNC takes care of these by it's own._
