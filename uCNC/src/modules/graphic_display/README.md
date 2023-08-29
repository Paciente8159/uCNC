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

3. You must configure the necessary pins that operate the display and the rotary encoder `cnc_hal_config.h`
These are the default values

```
#ifndef U8X8_MSG_GPIO_SPI_CLOCK_PIN
#define U8X8_MSG_GPIO_SPI_CLOCK_PIN DOUT8
#endif
#ifndef U8X8_MSG_GPIO_SPI_DATA_PIN
#define U8X8_MSG_GPIO_SPI_DATA_PIN DOUT9
#endif
#ifndef U8X8_MSG_GPIO_CS_PIN
#define U8X8_MSG_GPIO_CS_PIN DOUT10
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

4. You must also enable RPM counter on the tool `cnc_hal_config.h`

```
// assign the tools from 1 to 16
#define TOOL1 spindle_pwm

// enable RPM encoder for spindle_pwm
// depends on encoders (below)
  #define SPINDLE_PWM_HAS_RPM_ENCODER
```

5. The last step is to enable `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`
