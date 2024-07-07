# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About graphic_display for µCNC

This module adds graphic display support for µCNC.

## Adding graphic_display to µCNC

To use the graphic_display parser module follow these steps:

1. Copy the the `graphic_display` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(graphic_display);
```

3. You must choose the communication bus used by the display (SPI/I2C) and type of bus (Hardware or Software emulated) and configure the necessary pins that operate the display and the rotary encoder `cnc_hal_config.h`
These are the default values

```
// choose the communication interface
#define GRAPHIC_DISPLAY_INTERFACE GRAPHIC_DISPLAY_HW_SPI /* or GRAPHIC_DISPLAY_HW_I2C or GRAPHIC_DISPLAY_SW_SPI or GRAPHIC_DISPLAY_SW_I2C */
// choose one of the available display drivers
#define GRAPHIC_DISPLAY_DRIVER st7920_128x64_spi

#ifndef GRAPHIC_DISPLAY_SPI_CLOCK
#define GRAPHIC_DISPLAY_SPI_CLOCK DOUT4
#endif
#ifndef GRAPHIC_DISPLAY_SPI_MOSI
#define GRAPHIC_DISPLAY_SPI_MOSI DOUT5
#endif
#ifndef GRAPHIC_DISPLAY_SPI_CS
#define GRAPHIC_DISPLAY_SPI_CS DOUT6
#endif
#ifndef GRAPHIC_DISPLAY_ENCODER_BTN
#define GRAPHIC_DISPLAY_ENCODER_BTN DIN11
#endif
#ifndef GRAPHIC_DISPLAY_ENCODER_ENC1
#define GRAPHIC_DISPLAY_ENCODER_ENC1 DIN12
#endif
#ifndef GRAPHIC_DISPLAY_ENCODER_ENC2
#define GRAPHIC_DISPLAY_ENCODER_ENC2 DIN13
#endif
```

4. The last step is to enable `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`
