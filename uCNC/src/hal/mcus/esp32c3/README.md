_µCNC for ESP32 can be built this way_

## Method one - PlatformIO (preferred)

1. Get [Visual Studio Code](https://code.visualstudio.com/download) and install it.
2. Install the PlatformIO extension.
3. Open uCNC folder in VSCode.
4. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
5. If needed edit the platformio.ini file environment for your board. Compile the sketch and upload it to your board.

ESP32 on PlatformIO now prevents 2 flavors of the same souce code. The normal Arduino compilation and a version using ESPIDF and Arduino as a component. This second version adds an optimizations to timer ISR that make step generation smoother in particular using 74HC595 IO extension. It's possible to also make changes to build more performant code if for example you want to compile code with only WiFi or Bluetooth, by tweaking the options on the ```sdkconfig.deafults``` file.

**Note** ESP32 boards build environments are available in two flavors. Regular Arduino and an ESPIDF with Arduino as a component that has some optimized options for low latency timer ISR. 

## Method two - Arduino IDE (easiest)

1. Get [Arduino IDE](https://www.arduino.cc/en/software) and install it.
2. If you don't have install ESP32 for Arduino with Arduino board manager has explained [here](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
3. Go to uCNC folder and open uCNC.ino sketch.
4. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
5. Compile the sketch and upload it to your board.
