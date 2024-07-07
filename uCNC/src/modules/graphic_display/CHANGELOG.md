# About graphic_display for µCNC

This module adds graphic display support for µCNC.

## Changelog

### 2024-07-02

- added u8g2 library abstration layer. This allows replacing the display library to make use of different types of displays/drivers while reusing the same interface.(#55)
- it's more easy do add extra drivers and display types using some utility macros to declare the display initialization call.(#55)

### 2024-04-27

- added display beep on press

### 2024-04-25

- increased SPI speed
- fixed SPI data pin calls using u8g2 lib functions

### 2024-03-30

- fixed get parser modes call

### 2024-02-13

- updated for v010808

### 2023-10-21
- popup with fixed width

### 2023-10-20
- modified event hooks used to prevent loop inception that caused some menus to become irresponsive (#39)

### 2023-10-11
- integration of multistream

### 2023-09-23
- added multiple main loop calls to prevent planner starving and inconsistent motion (#33)

### 2023-09-22
- redesigned alarm screen (#32)

### 2023-09-20
- added long press (5s) on button to do soft reset (#31)
- added alarm screen popup (#31)


### 2023-09-17
- fixed line separator drawing for 5-axis or more
- increased encoder sensitivity (soft polling)

### 2023-09-10
- fixed I2C graphic display IO call missing argument

### 2023-05-21

- updated to version 1.8 (#29)

### 2023-05-08

- slight change to idle screen elements position (#25)
- header render prints RAM string (#25)
- modified startup code (#25)

### 2023-05-03

- initial release with support for [RepRap Discount Full Graphic Smart Controller](https://reprap.org/wiki/RepRapDiscount_Full_Graphic_Smart_Controller)
