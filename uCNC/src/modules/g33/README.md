# uCNC-modules

Addon modules for µCNC - Universal CNC firmware for microcontrollers

## About G33 for µCNC

This module adds custom G33 code to the µCNC parser. This similar to Linux CNC G33 and allows to make motions synched with the spindle in µCNC.

## Adding G33 to µCNC

To use the G33 parser module follow these steps:

1. Copy the the `G33` directory and place it inside the `src/modules/` directory of µCNC
2. Then you need load the module inside µCNC. Open `src/module.c` and at the bottom of the file add the following lines inside the function `load_modules()`

```
LOAD_MODULE(g33);
```

3. You must configure the encoder that will handle the RPM reading in the `cnc_hal_config.h`

```
#define ENCODERS 1

// Uses DIN7 input as ENCODER0 in counter mode
#define ENC0_PULSE DIN7
#define ENC0_DIR DIN7

// define an encoder index pin if the pin also changes state synchronized with the PULSE pin
#define ENC0_INDEX DIN8

/** OR **/

// define a custom interruptable pin that will be used to dispatch the index event for G33 like this
#define G33_INDEX_PIN DIN6

// Assign which encoder will be used by G33
#define G33_ENCODER ENC0
```

Inside the index ISR a floating point math operation is performed. If this causes issues on a specific architecture you can enable an option to replace it by a fixed point operation.

`#define G33_REPLACE_FP_OPERATION_IN_ISR`

4. You should also enable RPM counter on the tool `cnc_hal_config.h`. This will allow reading the tool actual speed and not the programmed speed. For example for spindle_pwm tool it's done like this:

```
// assign the tools from 1 to 16
#define TOOL1 spindle_pwm

// assign ENC0 to the spindle rpm encoder
  #define SPINDLE_RPM_ENCODER ENC0
```

5. The last step is to enable `ENABLE_MAIN_LOOP_MODULES`, `ENABLE_PARSER_MODULES`, `ENABLE_IO_MODULES` and `ENABLE_RT_SYNC_MOTIONS` inside `cnc_config.h`
