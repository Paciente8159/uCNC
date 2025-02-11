/*
	Name: esp8266_spi.c
	Description: Implements SPI for ESP8266.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 04-02-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_ESP8266)

#include <ets_sys.h>
#include <esp8266_peri.h>

#ifdef MCU_HAS_SPI

#ifndef mcu_spi_bulk_transfer
bool mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	static uint16_t remaining = 0;
	static bool waiting = false;
	uint16_t aligned_len = remaining;

	if (!aligned_len)
	{
		aligned_len = (len & 0xFFFC);
	}

	while (aligned_len)
	{
		volatile uint32_t *spififo = &SPI1W0;
		uint16_t chunck = MIN(aligned_len, 64);
		if (!waiting)
		{
			while (SPI1CMD & SPIBUSY)
				;
			if (out)
			{
				memcpy((void *)spififo, out, chunck);
				out += chunck;
			}
			else
			{
				memset((void *)spififo, 0xFF, chunck);
			}
			SPI1CMD |= SPIBUSY;
			waiting = true;
		}

		if (SPI1CMD & SPIBUSY)
		{
			return true;
		}

		waiting = false;

		if (in)
		{
			memcpy(in, (void *)spififo, chunck);
			in += chunck;
		}
		aligned_len -= chunck;
		remaining = aligned_len;
	}

	len -= aligned_len;
	while (len)
	{
		uint8_t c = 0xFF;
		if (out)
		{
			c = mcu_spi_xmit(*out++);
		}
		if (in)
		{
			*in++ = c;
		}
		len--;
	}

	remaining = 0;
	return false;
}
#endif

#ifndef mcu_spi_xmit
uint8_t mcu_spi_xmit(uint8_t data)
{
	while (SPI1CMD & SPIBUSY)
		;
	SPI1W0 = data;
	SPI1CMD |= SPIBUSY;
	while (SPI1CMD & SPIBUSY)
		;
	return (uint8_t)(SPI1W0 & 0xff);
}
#endif

#ifndef mcu_spi_config
#define ESP8266_SPI_FREQ(PRE, CNT) (ESP8266_CLOCK / ((PRE + 1) * (CNT + 1)))
#define ESP8266_SPI_CLK(PRE, CNT) ((PRE & SPICLKDIVPRE) << SPICLKDIVPRE_S) | ((CNT & SPICLKCN) << SPICLKCN_S) | (((CNT ? (((CNT + 1) >> 1) - 1) : 0) & SPICLKCH) << SPICLKCH_S) | ((CNT & SPICLKCL) & SPICLKCL_S)
void mcu_spi_config(spi_config_t config, uint32_t frequency)
{
#if SPI_PORT == 0
	IOSWAP |= (1 << IOSWAP2CS);
	// SPI0E3 |= 0x1; This is in the MP3_DECODER example, but makes the WD kick in here.
	SPI1E3 |= 0x3;
	mcu_config_af(SPI_CS, FUNCTION_1);
#else
	// IO
	mcu_config_af(SPI_CLK, SPECIAL);
	mcu_config_af(SPI_SDO, SPECIAL);
	mcu_config_af(SPI_SDI, SPECIAL);
#endif

	// MSB
	SPI1C = 0;
	SPI1C1 = 0;
	SPI1C2 = 0;
	SPI1U2 = 0;
	SPI1U = SPIUMOSI | SPIUDUPLEX | SPIUSSE;
	// 8bit
	SPI1U1 = (7 << SPILMOSI) | (7 << SPILMISO);
	SPI1S = 0;

#if SPI_PORT == 0
	SPI1P &= ~SPIPCS2DIS;
	SPI1P |= SPIPCS1DIS | SPIPCS0DIS;
	SPI1U |= (SPIUCSSETUP | SPIUCSHOLD);
#endif

	// Mode
	uint8_t cpha = config.mode & 0x01;
	if (config.mode & 0x10)
	{
		cpha ^= 1;
		SPI1P |= (1UL << 29);
	}
	else
	{
		SPI1P &= ~(1UL << 29);
	}

	if (cpha)
	{
		SPI1U |= (SPIUSME);
	}
	else
	{
		SPI1U &= ~(SPIUSME);
	}

	// Freq
	if (frequency > ESP8266_CLOCK)
	{
		GPMUX |= (1 << 9);
		return;
	}

	GPMUX &= ~(1 << 9);

	uint16_t pre_div = 0;
	uint8_t count = 0;
	while (frequency < ESP8266_SPI_FREQ(pre_div, SPICLKCN))
	{
		pre_div++;
		if (pre_div >= SPICLKDIVPRE)
		{
			break;
		}
	}

	while (frequency < ESP8266_SPI_FREQ(pre_div, count))
	{
		count++;
		if (count >= SPICLKCN)
		{
			break;
		}
	}

	SPI1CLK = ESP8266_SPI_CLK(pre_div, count);
}
#endif

extern spi_port_t mcu_spi_port;
#define MCU_SPI (&mcu_spi_port)
#else
#define MCU_SPI NULL
#endif

#ifdef MCU_HAS_SPI2
#ifndef mcu_spi2_xmit
uint8_t mcu_spi2_xmit(uint8_t data)
{
	while (SPI1CMD & SPIBUSY)
		;
	SPI1W0 = data;
	SPI1CMD |= SPIBUSY;
	while (SPI1CMD & SPIBUSY)
		;
	return (uint8_t)(SPI1W0 & 0xff);
}
#endif

#ifndef mcu_spi2_bulk_transfer
bool mcu_spi2_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	static uint16_t remaining = 0;
	static bool waiting = false;
	uint16_t aligned_len = remaining;

	if (!aligned_len)
	{
		aligned_len = (len & 0xFFFC);
	}

	while (aligned_len)
	{
		volatile uint32_t *spififo = &SPI1W0;
		uint16_t chunck = MIN(aligned_len, 64);
		if (!waiting)
		{
			while (SPI1CMD & SPIBUSY)
				;
			if (out)
			{
				memcpy((void *)spififo, out, chunck);
				out += chunck;
			}
			else
			{
				memset((void *)spififo, 0xFF, chunck);
			}
			SPI1CMD |= SPIBUSY;
			waiting = true;
		}

		if (SPI1CMD & SPIBUSY)
		{
			return true;
		}

		waiting = false;

		if (in)
		{
			memcpy(in, (void *)spififo, chunck);
			in += chunck;
		}
		aligned_len -= chunck;
		remaining = aligned_len;
	}

	len -= aligned_len;
	while (len)
	{
		uint8_t c = 0xFF;
		if (out)
		{
			c = mcu_spi2_xmit(*out++);
		}
		if (in)
		{
			*in++ = c;
		}
		len--;
	}

	remaining = 0;
	return false;
}
#endif

#ifndef mcu_spi2_config
void mcu_spi2_config(spi_config_t config, uint32_t frequency)
{
#if SPI_PORT == 0
	IOSWAP |= (1 << IOSWAP2CS);
	// SPI0E3 |= 0x1; This is in the MP3_DECODER example, but makes the WD kick in here.
	SPI1E3 |= 0x3;
	mcu_config_af(SPI2_CS, FUNCTION_1);
#else
	// IO
	mcu_config_af(SPI2_CLK, SPECIAL);
	mcu_config_af(SPI2_SDO, SPECIAL);
	mcu_config_af(SPI2_SDI, SPECIAL);
#endif

	// MSB
	SPI1C = 0;
	SPI1C1 = 0;
	SPI1C2 = 0;
	SPI1U2 = 0;
	SPI1U = SPIUMOSI | SPIUMISO | SPIUDUPLEX | SPIUSSE;
	// 8bit
	SPI1U1 = (7 << SPILMOSI) | (7 << SPILMISO);
	SPI1S = 0;

#if SPI2_PORT == 0
	SPI1P &= ~SPIPCS2DIS;
	SPI1P |= SPIPCS1DIS | SPIPCS0DIS;
	SPI1U |= (SPIUCSSETUP | SPIUCSHOLD);
#endif

	// Mode
	uint8_t cpha = config.mode & 0x01;
	if (config.mode & 0x10)
	{
		cpha ^= 1;
		SPI1P |= (1UL << 29);
	}
	else
	{
		SPI1P &= ~(1UL << 29);
	}

	if (cpha)
	{
		SPI1U |= (SPIUSME);
	}
	else
	{
		SPI1U &= ~(SPIUSME);
	}

	// Freq
	if (frequency > ESP8266_CLOCK)
	{
		GPMUX |= (1 << 9);
		return;
	}

	GPMUX &= ~(1 << 9);

	uint16_t pre_div = 0;
	uint8_t count = 0;
	while (frequency < ESP8266_SPI_FREQ(pre_div, SPICLKCN))
	{
		pre_div++;
		if (pre_div >= SPICLKDIVPRE)
		{
			break;
		}
	}

	while (frequency < ESP8266_SPI_FREQ(pre_div, count))
	{
		count++;
		if (count >= SPICLKCN)
		{
			break;
		}
	}

	SPI1CLK = ESP8266_SPI_CLK(pre_div, count);
}
#endif

extern spi_port_t mcu_spi2_port;
#define MCU_SPI2 (&mcu_spi2_port)
#else
#define MCU_SPI2 NULL
#endif

void mcu_spi_init(void)
{
#ifdef MCU_HAS_SPI
	spi_config_t spi_conf = {0};
	mcu_spi_config(spi_conf, SPI_FREQ);
#endif
#ifdef MCU_HAS_SPI2
	spi_config_t spi2_conf = {0};
	mcu_spi_config(spi2_conf, SPI2_FREQ);
#endif
}

#endif