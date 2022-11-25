_µCNC for ESP8266 can be built this way_

## Method one - PlatformIO (preferred)

1. Get [Visual Studio Code](https://code.visualstudio.com/download) and install it.
2. Install the PlatformIO extension.
3. Open uCNC folder in VSCode.
4. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
5. If needed edit the platformio.ini file environment for your board. Compile the sketch and upload it to your board.

## Method two - Arduino IDE (easiest)

1. Get [Arduino IDE](https://www.arduino.cc/en/software) and install it.
2. If you don't have install ESP8266 for Arduino with Arduino board manager has explained [here](https://github.com/esp8266/Arduino#installing-with-boards-manager)
3. Go to uCNC folder and open uCNC.ino sketch.
4. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
5. Compile the sketch and upload it to your board (via maple bootloader or other method).
