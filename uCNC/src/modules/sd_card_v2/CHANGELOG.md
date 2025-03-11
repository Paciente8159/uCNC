# About SD Card v2 for µCNC

SD card allows to add an SD card read/write support to µCNC.
The SD card can be navigated using grbl commands similar to shell commands
For best performance the hardware SPI should be used.
This version uses FatFs. PetitFs might be implemented in the future

## Changelog

### 2024-10-09

- prevent settings reloading on card mount/unmount
- allow settings reload on cnc reset

### 2024-10-09

- modifications to make module compatible with new settings safety features

### 2024-10-03

- fixed file open/create if file does not exist (#77)

### 2024-07-04

- modified softspi interface to match improved softspi core module (#56)

### 2023-04-27

- fixed module version checking

### 2023-04-11

- initial implementation (#53)
