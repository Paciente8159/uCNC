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

# 5. Index Support (`ENCx_INDEX` and virtual index)

µCNC supports two index sources for each encoder:

- a physical **index pin** (`ENCx_INDEX`), and
- a **virtual index** generated from the encoder position (`ENCx_VIRTUAL_INDEX`).

Both sources call the same `encX_index` simple hook. Physical index detection runs from the encoder pulse ISR, while virtual index detection runs from the `cnc_io_dotasks` event. Keep hook callbacks short and non-blocking, especially for physical index pins.

## 5.1 Physical index pin (`ENCx_INDEX`)

An encoder can optionally define an **index pin**, which pulses once per revolution. **NOTE:** for this to work as desired the index pin must be active when the `PULSE` pin switches from inactive to active state. This is beacuse the pins state sampling is done on each transition low->high transition on the `PULSE` pin:

```c
#define ENCx_INDEX <pin>
```

When defined:

1. The encoder module checks the index pin on every pulse:
   ```c
   if (io_get_input(ENCx_INDEX)) {
       HOOK_INVOKE(enc0_index); // enc0_index through enc7_index, matching the encoder number
   }
   ```
2. A hook is automatically created during module initialization:
   ```c
   CREATE_HOOK(enc0_index); // one hook per configured encoder (enc0_index ... enc7_index)
   ```

**What this allows:**

- Zeroing the encoder at index
- Synchronizing spindle rotation
- Triggering measurement or logging events
- Custom user‑defined callbacks

Index hooks are one of the ways to extend functionalities in the encoder system.

**Other options:**

For custom index applications that require independent detection of the index pin transition, this options must not be set. Instead use the generic pin `input_change` event available by enabling `ENABLE_IO_MODULES`.

## 5.2 Virtual index (`ENCx_VIRTUAL_INDEX`)

A virtual index does not require an index input pin. The encoder module derives index events from the encoder position and a configured counts-per-revolution value. Enable it per encoder:

```c
#define ENC0_VIRTUAL_INDEX 1
#define ENC0_VIRTUAL_INDEX_CPR 2000 // counts per revolution; falls back to the $15x PPR setting when 0
```

Optional tuning macros:

| Macro | Default | Purpose |
|-------|---------|---------|
| `ENCx_VIRTUAL_INDEX_CPR` | `0` | Counts per revolution used to calculate index slots. `0` uses the encoder PPR setting |
| `ENCx_VIRTUAL_INDEX_OFFSET` | `0` | Position offset applied when the index origin is established |
| `ENCx_VIRTUAL_INDEX_HYSTERESIS` | `1` | Minimum distance from a slot boundary required to accept a virtual index |
| `ENCx_VIRTUAL_INDEX_ONLY` | `0` | When set, a configured physical `ENCx_INDEX` pin is ignored and only the virtual index is used |
| `ENCx_INDEX_RESET_POSITION` | not set | If defined, the virtual index resets the encoder position to this value and records the index reference |
| `ENCODER_VIRTUAL_INDEX_FIRE_HOOK` | `1` | Set to `0` to keep virtual index bookkeeping without invoking the `encX_index` hook |

Virtual index events are accepted when the encoder crosses a slot boundary defined by `ENCx_VIRTUAL_INDEX_CPR`, subject to `ENCx_VIRTUAL_INDEX_HYSTERESIS`. The module records the index reference (`origin`, last position, delta statistics) and, when `ENCODER_VIRTUAL_INDEX_FIRE_HOOK` is enabled, invokes the matching `encX_index` hook.

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

# 8. RPM Measurement

Any encoder can be used as an RPM source:
The firmware computes RPM using:

- Position delta
- Time delta
- Encoder resolution

`encoder_get_rpm` function can be used to get a specific encoder RPM:

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
| `ENCx_INDEX` | Physical index pin | Creates index hook and callback support |
| `ENCx_VIRTUAL_INDEX`, `ENCx_VIRTUAL_INDEX_CPR`, `ENCx_VIRTUAL_INDEX_OFFSET`, `ENCx_VIRTUAL_INDEX_HYSTERESIS`, `ENCx_VIRTUAL_INDEX_ONLY`, `ENCx_INDEX_RESET_POSITION` | Virtual index | Derives index events from position; calls the same index hook |
| `STEPx_ENCODER` | Bind encoder to stepper | Closed‑loop and homing sync |
| `ENABLE_MAIN_LOOP_MODULES` | Background updates | Required for incremental encoders |

---

# Final Notes

The encoder subsystem in µCNC is modular and flexible. By combining encoder types, index pins, incremental mode, and stepper assignments, you can configure anything from a simple tachometer (required for example by G33 module) to a multi‑axis closed‑loop system (not implemented).

