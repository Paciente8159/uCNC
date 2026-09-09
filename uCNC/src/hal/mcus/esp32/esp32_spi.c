/*
	Name: esp32_spic
	Description: Implements the SPI HAL for ESP32.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 26-03-2026

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"
#if ((CONFIG_IDF_TARGET_ESP32) && !defined(USE_ARDUINO_SPI_LIBRARY))

#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef SPI_DMA_CH_AUTO
#define SPI_DMA_CH_AUTO 1
#endif

#ifndef SPI_ASYNC_DMA_CHUNK_MAX
#define SPI_ASYNC_DMA_CHUNK_MAX 512U
#endif

#ifndef SPI_ASYNC_NODMA_CHUNK_MAX
#define SPI_ASYNC_NODMA_CHUNK_MAX 64U
#endif

typedef struct spi_ctx_
{
	spi_host_device_t host;
	int sclk_io;
	int miso_io;
	int mosi_io;

	bool bus_initialized;
	bool bus_dma_enabled;
	spi_device_handle_t dev;

	spi_config_t current_cfg;
	uint32_t current_freq;

	bool bulk_active;
	bool bulk_dma;
	bool bulk_inplace;
	bool bulk_queued;

	const uint8_t *bulk_out;
	uint8_t *bulk_in;
	uint16_t bulk_len;
	uint16_t bulk_pos;

	uint16_t bulk_chunk_len;
	uint8_t tx_stage[SPI_ASYNC_DMA_CHUNK_MAX];
	uint8_t rx_stage[SPI_ASYNC_DMA_CHUNK_MAX];
	uint8_t ff_stage[SPI_ASYNC_DMA_CHUNK_MAX];
	spi_transaction_t trans;
} spi_ctx_t;

#ifdef MCU_HAS_SPI
static spi_ctx_t spi_ctx = {
	.host = SPI_DEV_HOST,
	.sclk_io = SPI_CLK_BIT,
	.miso_io = SPI_SDI_BIT,
	.mosi_io = SPI_SDO_BIT,
};
#endif

#ifdef MCU_HAS_SPI2
static spi_ctx_t spi2_ctx = {
	.host = SPI2_DEV_HOST,
	.sclk_io = SPI2_CLK_BIT,
	.miso_io = SPI2_SDI_BIT,
	.mosi_io = SPI2_SDO_BIT,
};
#endif

static void spi_ctx_reset_bulk(spi_ctx_t *ctx)
{
	ctx->bulk_active = false;
	ctx->bulk_dma = false;
	ctx->bulk_inplace = false;
	ctx->bulk_queued = false;
	ctx->bulk_out = NULL;
	ctx->bulk_in = NULL;
	ctx->bulk_len = 0;
	ctx->bulk_pos = 0;
	ctx->bulk_chunk_len = 0;
	memset(&ctx->trans, 0, sizeof(ctx->trans));
}

static void spi_ctx_fill_ff(spi_ctx_t *ctx)
{
	static bool filled = false;
	if (!filled)
	{
		memset(ctx->ff_stage, 0xFF, sizeof(ctx->ff_stage));
		filled = true;
	}
}

static esp_err_t spi_ctx_deinit_device(spi_ctx_t *ctx)
{
	esp_err_t err = ESP_OK;

	if (ctx->dev)
	{
		err = spi_bus_remove_device(ctx->dev);
		ctx->dev = NULL;
	}

	return err;
}

static esp_err_t spi_ctx_deinit_bus(spi_ctx_t *ctx)
{
	esp_err_t err = ESP_OK;

	if (ctx->bus_initialized)
	{
		err = spi_bus_free(ctx->host);
		ctx->bus_initialized = false;
		ctx->bus_dma_enabled = false;
	}

	return err;
}

static esp_err_t spi_ctx_init_bus(spi_ctx_t *ctx, bool dma_enable)
{
	esp_err_t err;

	if (ctx->bus_initialized && (ctx->bus_dma_enabled == dma_enable))
	{
		return ESP_OK;
	}

	if (ctx->dev)
	{
		err = spi_ctx_deinit_device(ctx);
		if (err != ESP_OK)
		{
			return err;
		}
	}

	if (ctx->bus_initialized)
	{
		err = spi_ctx_deinit_bus(ctx);
		if (err != ESP_OK)
		{
			return err;
		}
	}

	spi_bus_config_t buscfg;
	memset(&buscfg, 0, sizeof(buscfg));
	buscfg.miso_io_num = ctx->miso_io;
	buscfg.mosi_io_num = ctx->mosi_io;
	buscfg.sclk_io_num = ctx->sclk_io;
	buscfg.quadwp_io_num = -1;
	buscfg.quadhd_io_num = -1;
	buscfg.max_transfer_sz = dma_enable ? SPI_ASYNC_DMA_CHUNK_MAX : SPI_ASYNC_NODMA_CHUNK_MAX;

	err = spi_bus_initialize(ctx->host, &buscfg, dma_enable ? SPI_DMA_CH_AUTO : 0);
	if (err != ESP_OK)
	{
		ctx->bus_initialized = false;
		ctx->bus_dma_enabled = false;
		return err;
	}

	ctx->bus_initialized = true;
	ctx->bus_dma_enabled = dma_enable;
	return ESP_OK;
}

static esp_err_t spi_ctx_apply_device(spi_ctx_t *ctx, spi_config_t config, uint32_t frequency)
{
	esp_err_t err;

	err = spi_ctx_init_bus(ctx, config.enable_dma ? true : false);
	if (err != ESP_OK)
	{
		return err;
	}

	if (ctx->dev &&
		(ctx->current_cfg.flags == config.flags) &&
		(ctx->current_freq == frequency))
	{
		return ESP_OK;
	}

	if (ctx->dev)
	{
		err = spi_ctx_deinit_device(ctx);
		if (err != ESP_OK)
		{
			return err;
		}
	}

	spi_device_interface_config_t devcfg;
	memset(&devcfg, 0, sizeof(devcfg));
	devcfg.mode = config.mode & 0x03U;
	devcfg.clock_speed_hz = (int)frequency;
	devcfg.spics_io_num = -1;
	devcfg.queue_size = 1;
	devcfg.flags = SPI_DEVICE_NO_DUMMY;

	err = spi_bus_add_device(ctx->host, &devcfg, &ctx->dev);
	if (err != ESP_OK)
	{
		ctx->dev = NULL;
		return err;
	}

	ctx->current_cfg = config;
	ctx->current_freq = frequency;
	spi_ctx_reset_bulk(ctx);
	return ESP_OK;
}

static uint8_t spi_ctx_xmit(spi_ctx_t *ctx, uint8_t data)
{
	if (!ctx->dev)
	{
		return 0xFF;
	}

	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
	t.length = 8;
	t.rxlength = 8;
	t.tx_data[0] = data;

	if (spi_device_polling_transmit(ctx->dev, &t) != ESP_OK)
	{
		return 0xFF;
	}

	return t.rx_data[0];
}

static uint16_t spi_ctx_prepare_chunk(spi_ctx_t *ctx)
{
	uint16_t remaining = (uint16_t)(ctx->bulk_len - ctx->bulk_pos);
	uint16_t max_chunk = ctx->bulk_dma ? (uint16_t)SPI_ASYNC_DMA_CHUNK_MAX : (uint16_t)SPI_ASYNC_NODMA_CHUNK_MAX;
	uint16_t chunk = (remaining > max_chunk) ? max_chunk : remaining;

	ctx->bulk_chunk_len = chunk;
	memset(&ctx->trans, 0, sizeof(ctx->trans));
	ctx->trans.length = ((size_t)chunk) * 8U;
	ctx->trans.rxlength = ((size_t)chunk) * 8U;

	if (ctx->bulk_out == NULL)
	{
		spi_ctx_fill_ff(ctx);
		ctx->trans.tx_buffer = ctx->ff_stage;
	}
	else if (ctx->bulk_inplace)
	{
		memcpy(ctx->tx_stage, ctx->bulk_out + ctx->bulk_pos, chunk);
		ctx->trans.tx_buffer = ctx->tx_stage;
	}
	else
	{
		ctx->trans.tx_buffer = ctx->bulk_out + ctx->bulk_pos;
	}

	if (ctx->bulk_in == NULL)
	{
		ctx->trans.rx_buffer = NULL;
	}
	else if (ctx->bulk_inplace)
	{
		ctx->trans.rx_buffer = ctx->rx_stage;
	}
	else
	{
		ctx->trans.rx_buffer = ctx->bulk_in + ctx->bulk_pos;
	}

	return chunk;
}

static bool spi_ctx_bulk_transfer(spi_ctx_t *ctx, const uint8_t *out, uint8_t *in, uint16_t len)
{
	if (!ctx->dev)
	{
		return false;
	}

	if (!ctx->bulk_active)
	{
		if (len == 0U)
		{
			return false;
		}

		ctx->bulk_active = true;
		ctx->bulk_dma = ctx->current_cfg.enable_dma ? true : false;
		ctx->bulk_inplace = ((out != NULL) && (in != NULL) && (out == in));
		ctx->bulk_queued = false;
		ctx->bulk_out = out;
		ctx->bulk_in = in;
		ctx->bulk_len = len;
		ctx->bulk_pos = 0U;
		ctx->bulk_chunk_len = 0U;
		memset(&ctx->trans, 0, sizeof(ctx->trans));
	}

	if (ctx->bulk_dma)
	{
		if (!ctx->bulk_queued)
		{
			(void)spi_ctx_prepare_chunk(ctx);
			if (spi_device_queue_trans(ctx->dev, &ctx->trans, 0) != ESP_OK)
			{
				spi_ctx_reset_bulk(ctx);
				return false;
			}
			ctx->bulk_queued = true;
			return true;
		}
		else
		{
			spi_transaction_t *ret_trans = NULL;
			esp_err_t err = spi_device_get_trans_result(ctx->dev, &ret_trans, 0);
			if (err == ESP_ERR_TIMEOUT)
			{
				return true;
			}
			if (err != ESP_OK)
			{
				spi_ctx_reset_bulk(ctx);
				return false;
			}

			if (ctx->bulk_inplace && (ctx->bulk_in != NULL) && (ctx->bulk_chunk_len != 0U))
			{
				memcpy(ctx->bulk_in + ctx->bulk_pos, ctx->rx_stage, ctx->bulk_chunk_len);
			}

			ctx->bulk_pos = (uint16_t)(ctx->bulk_pos + ctx->bulk_chunk_len);
			ctx->bulk_queued = false;

			if (ctx->bulk_pos >= ctx->bulk_len)
			{
				spi_ctx_reset_bulk(ctx);
				return false;
			}

			return true;
		}
	}
	else
	{
		uint16_t i;
		uint16_t chunk = spi_ctx_prepare_chunk(ctx);

		for (i = 0; i < chunk; i++)
		{
			uint8_t txb = 0xFF;
			uint8_t rxb;

			if (ctx->bulk_out != NULL)
			{
				txb = ctx->bulk_inplace ? ctx->tx_stage[i] : ctx->bulk_out[ctx->bulk_pos + i];
			}

			rxb = spi_ctx_xmit(ctx, txb);

			if (ctx->bulk_in != NULL)
			{
				if (ctx->bulk_inplace)
				{
					ctx->rx_stage[i] = rxb;
				}
				else
				{
					ctx->bulk_in[ctx->bulk_pos + i] = rxb;
				}
			}
		}

		if (ctx->bulk_inplace && (ctx->bulk_in != NULL))
		{
			memcpy(ctx->bulk_in + ctx->bulk_pos, ctx->rx_stage, chunk);
		}

		ctx->bulk_pos = (uint16_t)(ctx->bulk_pos + chunk);

		if (ctx->bulk_pos >= ctx->bulk_len)
		{
			spi_ctx_reset_bulk(ctx);
			return false;
		}

		return true;
	}
}

#ifdef MCU_HAS_SPI

void mcu_spi_config(spi_config_t config, uint32_t frequency)
{
	(void)spi_ctx_apply_device(&spi_ctx, config, frequency);
}

void mcu_spi_start(spi_config_t config, uint32_t frequency)
{
	while (mcu_spi_port.isbusy)
	{
		TASK_YIELD();
	}

	mcu_spi_port.isbusy = true;
	(void)spi_ctx_apply_device(&spi_ctx, config, frequency);
}

void mcu_spi_stop(void)
{
	if (spi_ctx.bulk_dma && spi_ctx.bulk_queued && spi_ctx.dev)
	{
		spi_transaction_t *ret_trans = NULL;
		while (spi_device_get_trans_result(spi_ctx.dev, &ret_trans, 0) == ESP_ERR_TIMEOUT)
		{
			TASK_YIELD();
		}

		if (spi_ctx.bulk_inplace && (spi_ctx.bulk_in != NULL) && (spi_ctx.bulk_chunk_len != 0U))
		{
			memcpy(spi_ctx.bulk_in + spi_ctx.bulk_pos, spi_ctx.rx_stage, spi_ctx.bulk_chunk_len);
		}

		spi_ctx.bulk_pos = (uint16_t)(spi_ctx.bulk_pos + spi_ctx.bulk_chunk_len);
	}

	spi_ctx_reset_bulk(&spi_ctx);
	mcu_spi_port.isbusy = false;
}

uint8_t mcu_spi_xmit(uint8_t data)
{
	return spi_ctx_xmit(&spi_ctx, data);
}

bool mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	return spi_ctx_bulk_transfer(&spi_ctx, out, in, len);
}

#endif

#ifdef MCU_HAS_SPI2

void mcu_spi2_config(spi_config_t config, uint32_t frequency)
{
	(void)spi_ctx_apply_device(&spi2_ctx, config, frequency);
}

void mcu_spi2_start(spi_config_t config, uint32_t frequency)
{
	while (mcu_spi2_port.isbusy)
	{
		TASK_YIELD();
	}

	mcu_spi2_port.isbusy = true;
	(void)spi_ctx_apply_device(&spi2_ctx, config, frequency);
}

void mcu_spi2_stop(void)
{
	if (spi2_ctx.bulk_dma && spi2_ctx.bulk_queued && spi2_ctx.dev)
	{
		spi_transaction_t *ret_trans = NULL;
		while (spi_device_get_trans_result(spi2_ctx.dev, &ret_trans, 0) == ESP_ERR_TIMEOUT)
		{
			TASK_YIELD();
		}

		if (spi2_ctx.bulk_inplace && (spi2_ctx.bulk_in != NULL) && (spi2_ctx.bulk_chunk_len != 0U))
		{
			memcpy(spi2_ctx.bulk_in + spi2_ctx.bulk_pos, spi2_ctx.rx_stage, spi2_ctx.bulk_chunk_len);
		}

		spi2_ctx.bulk_pos = (uint16_t)(spi2_ctx.bulk_pos + spi2_ctx.bulk_chunk_len);
	}

	spi_ctx_reset_bulk(&spi2_ctx);
	mcu_spi2_port.isbusy = false;
}

uint8_t mcu_spi2_xmit(uint8_t data)
{
	return spi_ctx_xmit(&spi2_ctx, data);
}

bool mcu_spi2_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	return spi_ctx_bulk_transfer(&spi2_ctx, out, in, len);
}

#endif

#endif