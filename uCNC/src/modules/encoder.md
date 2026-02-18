<p align="center">
<img src="https://github.com/Paciente8159/uCNC/blob/master/docs/logo.png?raw=true">
</p>


# µCNC
µCNC - Universal CNC firmware for microcontrollers

# Configuring Encoders in µCNC

This document explains how to configure encoders in µCNC based on the HAL configuration file and the encoder module implementation. It covers encoder declaration, pin assignment, encoder types, incremental mode, index pins, and the features unlocked by each configuration option.

---

# 1. Enabling Encoders

To enable encoder support, define the number of encoders:

```c
#define ENCODERS <number>
```

The firmware expects sequential definitions: ENC0, ENC1, ENC2, etc. Skipping numbers is not allowed.

---

# 2. Basic Encoder Definitions

Each encoder requires two mandatory definitions:

```c
#define ENCx_PULSE <pin>
#define ENCx_DIR   <pin>
```

- **PULSE** must be an interrupt‑capable input pin.
- **DIR** is a regular input pin.

Example:

```c
#define ENC0_PULSE DIN0 // DIN0-7 are interruptable
#define ENC0_DIR   DIN8 // DIN8-49 are not interruptable
```

It's possible to invert the polarity of both `PULSE` and `DIR` for each encoder via settings `$8` and `$9`.
Also for each encoder defined Grbl type setting will be made available to allow configuration of the PPR (Pulses per full revolution) value for that encoder `$15x`.
The encoder module increments or decrements the internal counter based on the direction pin:

```c
encoders_pos[x] += (dir & ENCx_IO_MASK) ? 1 : -1;
```

---

# 3. Encoder Types

If no type is defined, µCNC defaults to a **pulse/dir** encoder:

```c
#define ENCx_TYPE ENC_TYPE_PULSE
```

Supported types:

| Type | Description |
|------|-------------|
| `ENC_TYPE_PULSE` | Standard pulse/dir incremental encoder |
| `ENC_TYPE_I2C`   | I²C absolute encoder (PULSE=SCL, DIR=SDA) |
| `ENC_TYPE_SSI`   | SSI absolute encoder (PULSE=CLK, DIR=MISO) |
| `ENC_TYPE_CUSTOM` | User‑implemented read function |

Each type automatically configures the appropriate read function:

```c
#define ENCx_READ read_encoder_mt6701_i2c(&encx)
```

or

```c
#define ENCx_READ read_encoder_mt6701_ssi(&encx)
```

for `ENC_TYPE_CUSTOM` encoder type it's possible to implement a custom encoder value read callback by implementing a custom code for:

```c
int32_t enc_custom_read(uint8_t i)
```

where `i` is the encoder index.

---

# 4. Incremental Encoders (`ENCx_IS_INCREMENTAL`)

By default, µCNC assumes **absolute** encoders. To declare an encoder as incremental:

```c
#define ENCx_IS_INCREMENTAL
```

Effects:

- The firmware performs differential reads:
  ```c
  diff = encoder_read - encoder_last_read[i];
  ```
- Wrap‑around handling is applied using the encoder resolution.
- The accumulated position is updated manually:
  ```c
  encoder_pos[i] += diff;
  ```

Incremental mode is useful for encoders that only output relative movement. But this does require the encoders to be periodically read. Also an encoder cannot rotate more then half a turn between reads otherwise the encoder calculated position will not be calculated correctly.
By default each incremental encoder is read on each run of the `cnc_io_dotasks` event with option `ENABLE_MAIN_LOOP_MODULES` enabled.

---

# 5. Index Pin Support (`ENCx_INDEX`)

An encoder can optionally define an **index pin**, which pulses once per revolution. **NOTE:** for this to work as desired the index pin must be active when the `PULSE` pin switches from inactive to active state. This is beacuse the pins state sampling is done on each transition low->high transition on the `PULSE` pin:

```c
#define ENCx_INDEX <pin>
```

When defined:

1. The encoder module checks the index pin on every pulse:
   ```c
   if (io_get_input(ENCx_INDEX)) {
       HOOK_INVOKE(encx_index);
   }
   ```
2. A hook is automatically created during module initialization:
   ```c
   CREATE_HOOK(encx_index);
   ```

**What this allows:**

- Zeroing the encoder at index
- Synchronizing spindle rotation
- Triggering measurement or logging events
- Custom user‑defined callbacks

Index hooks are one of the ways to extend functionalities in the encoder system.

**Other options:**

For custom index applications that require independent detection of the index pin transition, this options must not be set. Instead use the generic pin `input_change` event available by enabling `ENABLE_IO_MODULES`.

---

# 6. Assigning Encoders to Steppers

Encoders can be bound to stepper motors:

```c
#define STEP0_ENCODER ENC0
```

This enables:

- Closed‑loop correction (future feature)
- Resetting encoder positions during homing or interpolation:
  ```c
  encoder_reset_position(STEP0_ENCODER, origin[0]);
  ```

---

# 7. Counter Mode (Pulse‑Only)

If PULSE and DIR are assigned to the same pin:

```c
#define ENCx_PULSE DIN7
#define ENCx_DIR   DIN7
```

µCNC treats the encoder as a **simple counter**, useful for RPM sensors or tachometers.

---

# 8. RPM Measurement (`ENABLE_ENCODER_RPM`)

Any encoder can be used as an RPM source:

```c
#define ENABLE_ENCODER_RPM
```

The firmware computes RPM using:

- Position delta
- Time delta
- Encoder resolution

This option unlocks `encoder_get_rpm` function and an the necessary RPM calculation variables for all encoders:

```c
// get encoder 0 RPM
uint16_t rpm = uint16_t encoder_get_rpm(ENC0);
```

---

# 9. Main Loop Updates for Non‑Pulse Encoders

I²C, SSI, and custom encoders require periodic updates:

```c
#define ENABLE_MAIN_LOOP_MODULES
```

This enables:

```c
encoders_dotasks()
```

which calls:

```c
encoder_update(i);
```

for each encoder with a custom read function.

---

# 10. Summary of Key Macros and Their Effects

| Macro | Purpose | Effect |
|-------|---------|--------|
| `ENCODERS` | Number of encoders | Enables encoder module |
| `ENCx_PULSE`, `ENCx_DIR`, `ENCx_PPR` | Pin and resolution | Core encoder configuration |
| `ENCx_TYPE` | Encoder type | Selects pulse/dir, I²C, SSI, or custom |
| `ENCx_IS_INCREMENTAL` | Incremental mode | Differential updates and wrap‑around |
| `ENCx_INDEX` | Index pin | Creates index hook and callback support |
| `STEPx_ENCODER` | Bind encoder to stepper | Closed‑loop and homing sync |
| `ENABLE_ENCODER_RPM` | RPM measurement | Per‑encoder RPM calculation |
| `ENABLE_MAIN_LOOP_MODULES` | Background updates | Required for incremental encoders |

---

# Final Notes

The encoder subsystem in µCNC is modular and flexible. By combining encoder types, index pins, incremental mode, and stepper assignments, you can configure anything from a simple tachometer (required for example by G33 module) to a multi‑axis closed‑loop system (not implemented).

