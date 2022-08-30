# Recommended pinouts
These are the default (recommended) pinouts assignments for the general purpose IO pins. These are not mandatory. They act more like a guide line to the default wiring inside µCNC configurations <-> modules <-> tools.
This wiring can be completely modified by the user.

| **Generic**    | **Recommended assignment (not mandatory)**    |
| ------   | ------    |
| DOUT0   | SPINDLE_DIR or SPINDLE_FWD or SPINDLE_POWER_RELAY    |
| DOUT1   | SPINDLE_REV   |
| DOUT2   | COOLANT_FLOOD    |
| DOUT3   | COOLANT_MIST    |
| DOUT4   |     |
| DOUT5   |     |
| DOUT6   |     |
| DOUT7   |     |
| DOUT8   |     |
| DOUT9   |     |
| DOUT10   |     |
| DOUT11   | STEPPER_DIGIPOT_CS    |
| DOUT12   | STEPPER0_MSTEP1    |
| DOUT13   | STEPPER1_MSTEP1    |
| DOUT14   | STEPPER2_MSTEP1    |
| DOUT15   | STEPPER3_MSTEP1    |
| DOUT16   | STEPPER4_MSTEP1    |
| DOUT17   | STEPPER5_MSTEP1    |
| DOUT18   | STEPPER6_MSTEP1    |
| DOUT19   | STEPPER7_MSTEP1    |
| DOUT20   | STEPPER0_UART_TX or STEPPER0_SPI_CS or STEPPER0_MSTEP0    |
| DOUT21   | STEPPER1_UART_TX or STEPPER1_SPI_CS or STEPPER1_MSTEP0    |
| DOUT22   | STEPPER2_UART_TX or STEPPER2_SPI_CS or STEPPER2_MSTEP0    |
| DOUT23   | STEPPER3_UART_TX or STEPPER3_SPI_CS or STEPPER3_MSTEP0    |
| DOUT24   | STEPPER4_UART_TX or STEPPER4_SPI_CS or STEPPER4_MSTEP0    |
| DOUT25   | STEPPER5_UART_TX or STEPPER5_SPI_CS or STEPPER5_MSTEP0    |
| DOUT26   | STEPPER6_UART_TX or STEPPER6_SPI_CS or STEPPER6_MSTEP0    |
| DOUT27   | STEPPER7_UART_TX or STEPPER7_SPI_CS or STEPPER7_MSTEP0    |
| DOUT28   | SOFT_UART_TX    |
| DOUT29   | SOFT_SPI_SDO or TMC_SPI_SDO or STEPPER_DIGIPOT_SDO    |
| DOUT30   | SOFT_SPI_CLK or TMC_SPI_CLK or STEPPER_DIGIPOT_CLK    |
| DOUT31   | ACTIVITY_LED    |
| DIN0   | ENC0_PULSE    |
| DIN1   | ENC1_PULSE    |
| DIN2   | ENC2_PULSE    |
| DIN3   | ENC3_PULSE    |
| DIN4   | ENC4_PULSE    |
| DIN5   | ENC5_PULSE    |
| DIN6   | ENC6_PULSE    |
| DIN7   | ENC7_PULSE    |
| DIN8   | ENC0_DIR    |
| DIN9   | ENC1_DIR    |
| DIN10   | ENC2_DIR    |
| DIN11   | ENC3_DIR    |
| DIN12   | ENC4_DIR    |
| DIN13   | ENC5_DIR    |
| DIN14   | ENC6_DIR    |
| DIN15   | ENC7_DIR    |
| DIN16   |     |
| DIN17   |     |
| DIN18   |     |
| DIN19   |     |
| DIN20   | STEPPER0_UART_RX    |
| DIN21   | STEPPER1_UART_RX    |
| DIN22   | STEPPER2_UART_RX    |
| DIN23   | STEPPER3_UART_RX    |
| DIN24   | STEPPER4_UART_RX    |
| DIN25   | STEPPER5_UART_RX    |
| DIN26   | STEPPER6_UART_RX    |
| DIN27   | STEPPER7_UART_RX    |
| DIN28   | SOFT_UART_RX    |
| DIN29   | SOFT_SPI_SDI or TMC_SPI_SDI or STEPPER_DIGIPOT_SDI    |
| DIN30   | SOFT_I2C_SCL    |
| DIN31   | SOFT_I2C_SDA    |
