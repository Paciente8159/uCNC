# About graphic_display for µCNC

This module adds graphic display support for µCNC.

## Changelog

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

### 2023-05-08

- slight change to idle screen elements position (#25)
- header render prints RAM string (#25)
- modified startup code (#25)

### 2023-05-03

- initial release with support for [RepRap Discount Full Graphic Smart Controller](https://reprap.org/wiki/RepRapDiscount_Full_Graphic_Smart_Controller)
