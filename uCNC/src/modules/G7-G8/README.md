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

// Assign an encoder has an RPM encoder
#define ENABLE_ENCODER_RPM

#ifdef ENABLE_ENCODER_RPM
// Assign an encoder to work as the RPM encoder
#define RPM_ENCODER ENC0
// Optional set a second encoder pin has an encoder index
// This assumes the index pulse occurs when pulse pin is also triggered
// #define RPM_INDEX_INPUT DIN8
// Resolution of the RPM encoder or Pulses Per Revolution
#define RPM_PPR 24
```

4. You must also enable RPM counter on the tool `cnc_hal_config.h`

```
// assign the tools from 1 to 16
#define TOOL1 spindle_pwm

// enable RPM encoder for spindle_pwm
// depends on encoders (below)
  #define SPINDLE_PWM_HAS_RPM_ENCODER
```

5. The last step is to enable `ENABLE_MAIN_LOOP_MODULES`, `ENABLE_PARSER_MODULES` and `ENABLE_RT_SYNC_MOTIONS` inside `cnc_config.h`
