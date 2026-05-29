# About G33 for µCNC

This module adds custom G33 code to the µCNC parser. This similar to Linux CNC G33 and allows to make motions synched with the spindle in µCNC.

## Changelog

### 2026-04-06

- new math motion equations to produce an accurate pitch threading start synchronization
- filtered debug after threading finnishes

### 2026-03-23

- remove FP operation from ISR (crashes on ESP32)

### 2026-02-17

- updated to version 1.15.1
- new encoder library and configurations

### 2023-05-21

- updated to version 1.8 (#29)

### 2023-05-02

- updated to version 1.7 (#17)

### 2022-12-11

- initial release (#15)



