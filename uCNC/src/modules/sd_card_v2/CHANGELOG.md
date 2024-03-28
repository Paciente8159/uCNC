# About SD Card PF for µCNC

SD card allows to add an SD card read/write support to µCNC.
The SD card can be navigated using grbl commands similar to shell commands
For best performance the hardware SPI should be used.
This version uses PetitFS by default. But it's also possible to use a more feature rich FatFs (uses more resources)

## Changelog

### 2023-10-11
- integration of multistream

### 2023-05-21

- updated to version 1.8 (#29)

### 2023-05-10

- initial implementation
