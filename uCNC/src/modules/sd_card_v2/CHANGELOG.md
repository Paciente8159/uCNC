# About SD Card v2 for µCNC

SD card allows to add an SD card read/write support to µCNC.
The SD card can be navigated using grbl commands similar to shell commands
For best performance the hardware SPI should be used.
This version uses FatFs. PetitFs might be implemented in the future

## Changelog

### 2023-07-04

- modified softspi interface to match improved softspi core module (#56)

### 2023-04-27

- fixed module version checking

### 2023-04-11

- initial implementation (#53)
