# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About web pendant for µCNC

This module adds GrblHAL keypad to µCNC.

## Adding GrblHAL keypad to µCNC

To use the GrblHAL keypad module follow these steps:

1. Copy the the `grblhal_keypad` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

3. You must configure the necessary interface (HW/SW I2C/UART) and pins that connect to the keypad in `cnc_hal_overrides.h`
These are the default values

Example using HW I2C
```
// to use hardware I2C define the interface
#define KEYPAD_PORT KEYPAD_PORT_HW_I2C
//define the strobe pin and interrupt mask
#define KEYPAD_DOWN DIN7
#define KEYPAD_DOWN_MASK DIN7_MASK //mask much match pin
```

Example using SW I2C
```
// to use software I2C define the interface
#define KEYPAD_PORT KEYPAD_PORT_SW_I2C
// define the strobe pin and interrupt mask
// must be an interruptable pin
#define KEYPAD_DOWN DIN7
#define KEYPAD_DOWN_MASK DIN7_MASK //mask much match pin

//define the pins used to bit-bang I2C
#define KEYPAD_SCL DIN16
#define KEYPAD_SDA DIN17
```

Example using HW UART2
```
// to use hardware UART2 define the interface
#define KEYPAD_PORT KEYPAD_PORT_HW_UART2
// you need to detach UART2 from the main protocol to be able to use it for this case
#define DETACH_UART2_FROM_MAIN_PROTOCOL
```

Example using SW UART
```
// to use software UART define the interface
#define KEYPAD_PORT KEYPAD_PORT_SW_UART
// define the pins used to bit-bang UART
// RX pin must be interrupt capable to detect incomming keypad requests
#define KEYPAD_RX DIN7
#define KEYPAD_RX_MASK DIN7_MASK //mask much match pin
#define KEYPAD_TX DOUT20
```

You can also enable MPG mode by adding
```
#define KEYPAD_MPG_MODE_ENABLED
```

```
LOAD_MODULE(grblhal_keypad);
```

4. The last step is to enable `ENABLE_IO_MODULES` and `ENABLE_MAIN_LOOP_MODULES` inside `cnc_config.h`

5. Custom codes for extra buttons can be added by implementing the function

```
void keypad_extended_code(uint8_t *c)
```

where c is a pointer to the keycode pressed.

