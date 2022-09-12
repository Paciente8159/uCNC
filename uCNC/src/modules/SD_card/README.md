# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About I2C LCD for µCNC

I2C LCD allows to add an I2C LCD module to µCNC that display some basic info about the current machine position and limits state.
It requires any 2 µCNC generic digital input pins of the board. It uses software I2C so no dedicated I2C hardware is required.

## Adding I2C LCD to µCNC

To use the and I2C LCD follow these steps:

1. Copy the the `I2C_LCD` directory and place it inside the `src/modules/` directory of µCNC
2. Open `i2c_lcd.c` and define the number of rows and column of your LCD display. The default is 16x2

```
#ifndef LCD_ROWS
#define LCD_ROWS 2
#endif
#ifndef LCD_COLUMNS
#define LCD_COLUMNS 16
#endif
```

3. Also on `i2c_lcd.c` choose 2 free generic input pins. By default pins `DIN20` and `DIN21` are used but you can configure or use other pins from your board.

```
#ifndef LCD_I2C_SCL
#define LCD_I2C_SCL DIN21
#endif
#ifndef LCD_I2C_SDA
#define LCD_I2C_SDA DIN20
#endif
```

4. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(i2c_lcd);
```

5. The last step is to enable `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`
