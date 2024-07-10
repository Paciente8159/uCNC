#include "../../cnc.h"
#include "../softspi.h"
#include "../../modules/endpoint.h"
#include "../../modules/websocket.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef WIZNET_DRIVER
#define WIZNET_DRIVER W5500
#define _WIZCHIP_ WIZNET_DRIVER
#endif

#define WIZNET_HW_SPI 1
#define WIZNET_SW_SPI 2

#ifndef WIZNET_BUS
#define WIZNET_BUS WIZNET_HW_SPI
#endif

#ifndef WIZNET_CS
#define WIZNET_CS DOUT28
#endif

#if (WIZNET_BUS == WIZNET_HW_SPI)
#define WIZNET_SPI MCU_SPI
#endif

#if (UCNC_MODULE_VERSION < 10903 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

void wiznet_spi_config(void)
{
	io_config_output(WIZNET_CS);
	io_set_output(WIZNET_CS);
	softspi_config(WIZNET_SPI, 0, 14000000UL);
}

void wiznet_cs_select(void)
{
	io_clear_output(WIZNET_CS);
}

void wiznet_cs_deselect(void)
{
	io_set_output(WIZNET_CS);
}

uint8_t wiznet_spi_transmit(uint8_t c)
{
	return softspi_xmit(WIZNET_SPI, c);
}

void wiznet_spi_transmit_data(uint8_t *c, uint16_t len)
{
	while (len--)
	{
		*c = softspi_xmit(WIZNET_SPI, *c);
		c++;
	}
}

void wiznet_spi_start(void)
{
	softspi_start(WIZNET_SPI);
}

void wiznet_spi_stop(void)
{
	softspi_stop(WIZNET_SPI);
}