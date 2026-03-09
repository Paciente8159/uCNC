_µCNC for RP2040 can be built this way_

## Method one - PlatformIO (preferred)

1. Get [Visual Studio Code](https://code.visualstudio.com/download) and install it.
2. Install the PlatformIO extension.
3. Open uCNC folder in VSCode.
4. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
5. If needed edit the platformio.ini file environment for your board. Compile the sketch and upload it to your board.

## Method two - Arduino IDE (easiest)

1. Get [Arduino IDE](https://www.arduino.cc/en/software) and install it.
2. If you don't have install ESP8266 for Arduino with Arduino board manager has explained [here](https://github.com/earlephilhower/arduino-pico#installing-via-arduino-boards-manager)
3. Go to uCNC folder and open uCNC.ino sketch.
4. Edit ```cnc_config.h file``` and ```cnc_hal_config.h file``` to fit your needs and board.
5. Compile the sketch and upload it to your board.

## Using PIO custom 74HC595 block to drive up to 4 chained 74HC595 ICs

You can expand the IO output capabilities a MCU using the 74HC595 module. There are several options to run this. Either via a 3 output pins with bit-banging or using hardware SPI. In the case of the RP2040 a specialized option that takes advantage of the RP2040 PIO blocks can be used to create a 74HC595 driver capable of controlling up to 4 chained (daisy-chained) 74HC595 IC.

For this you just need to enable the custom 74HC595 IO shift option and define the pins used by the PIO to control the 74HC595 like this

```
#define IC74HC595_CUSTOM_SHIFT_IO //Enables custom MCU data shift transmission. In RP2040 that is via a PIO
#define IC74HC595_PIO_DATA 26 //use GPIO26 for the 74HC595 data
#define IC74HC595_PIO_CLK 27 //use GPIO27 for the 74HC595 clock
#define IC74HC595_PIO_LATCH 28 //use GPIO28 for the 74HC595 data latch
```

By default this will drive the 74HC595 at it's maximum speed of 20MHz. You can also modify the frequency by customizing the PIO clock speed like this:
```
#define IC74HC595_PIO_FREQ 10000000 //Run the 74HC595 at 10MHz
```