/**
 * SPI C implementation
 */

#include "../../../cnc.h"
#if (MCU == MCU_ESP32)
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_ipc.h"
#include "driver/uart.h"
#include "driver/timer.h"
#include "soc/i2s_struct.h"
#ifdef MCU_HAS_I2C
#include "driver/i2c.h"
#endif
#ifdef MCU_HAS_SPI
#include "hal/spi_types.h"
#include "driver/spi_master.h"
SemaphoreHandle_t spi_access_mutex = NULL;
bool spi_dma_enabled = false;
#ifndef SPI_DMA_BUFFER_SIZE
#define SPI_DMA_BUFFER_SIZE 1024
#endif
#endif
#ifdef MCU_HAS_SPI2
#include "hal/spi_types.h"
#include "driver/spi_master.h"
bool spi2_dma_enabled = false;
#ifndef SPI2_DMA_BUFFER_SIZE
#define SPI2_DMA_BUFFER_SIZE 1024
#endif
#endif
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#if (defined(MCU_HAS_SPI) && !defined(USE_ARDUINO_SPI_LIBRARY))

static spi_device_handle_t mcu_spi_handle = NULL;

#ifndef mcu_spi_xmit
uint8_t mcu_spi_xmit(uint8_t data)
{
	uint8_t rxdata = 0xFF;
	spi_transaction_t spi_trans = {0};
	spi_trans.length = 8; // Number of bits NOT number of bytes.
	spi_trans.tx_buffer = &data;
	spi_trans.rx_buffer = &rxdata;

	spi_device_transmit(mcu_spi_handle, &spi_trans);

	return rxdata;
}
#endif

#ifndef mcu_spi_config
void mcu_spi_config(spi_config_t config, uint32_t frequency)
{
	mcu_spi_start(config, frequency);
	mcu_spi_stop();
}
#endif

#ifndef mcu_spi_start
void mcu_spi_start(spi_config_t config, uint32_t frequency)
{
	if (mcu_spi_handle)
	{
		spi_device_acquire_bus(mcu_spi_handle, portMAX_DELAY);
		spi_device_release_bus(mcu_spi_handle);
		spi_bus_remove_device(mcu_spi_handle);
		mcu_spi_handle = NULL;
		spi_bus_free(SPI_PORT);
	}

	spi_bus_config_t spiconf = {
			.miso_io_num = SPI_SDI_BIT,
			.mosi_io_num = SPI_SDO_BIT,
			.sclk_io_num = SPI_CLK_BIT,
			.quadwp_io_num = -1,
			.quadhd_io_num = -1,
			.data4_io_num = -1,
			.data5_io_num = -1,
			.data6_io_num = -1,
			.data7_io_num = -1,
			.max_transfer_sz = (config.enable_dma) ? SPI_DMA_BUFFER_SIZE : SOC_SPI_MAXIMUM_BUFFER_SIZE,
			.flags = 0,
			.intr_flags = 0};
	// Initialize the SPI bus
	spi_bus_initialize(SPI_PORT, &spiconf, (config.enable_dma) ? SPI_DMA_CH_AUTO : SPI_DMA_DISABLED);

	spi_device_interface_config_t mcu_spi_conf = {0};
	mcu_spi_conf.mode = config.mode;
	mcu_spi_conf.clock_speed_hz = frequency;
	mcu_spi_conf.spics_io_num = -1;
	mcu_spi_conf.queue_size = 1;
	if (config.enable_dma)
	{
		mcu_spi_conf.queue_size = 1;
		mcu_spi_conf.pre_cb = NULL;
		mcu_spi_conf.pre_cb = NULL;
	}

	spi_dma_enabled = config.enable_dma;
	spi_bus_add_device(SPI_PORT, &mcu_spi_conf, &mcu_spi_handle);
	spi_device_acquire_bus(mcu_spi_handle, portMAX_DELAY);
}
#endif

#ifndef mcu_spi_stop
void mcu_spi_stop(void)
{
	spi_device_release_bus(mcu_spi_handle);
	mcu_spi_handle = NULL;
}
#endif

#ifndef mcu_spi_bulk_transfer
// data buffer for normal or DMA
static FORCEINLINE bool mcu_spi_transaction(const uint8_t *out, uint8_t *in, uint16_t len)
{
	static void *o = NULL;
	static void *i = NULL;
	static bool is_running = false;
	static spi_transaction_t t = {0};

	// start a new transmition
	if (!is_running)
	{
		if (spi_dma_enabled)
		{
			// DMA requires 4byte aligned transfers
			uint16_t l = (((len >> 2) + ((len & 0x03) ? 1 : 0)) << 2);
			o = heap_caps_malloc(l, MALLOC_CAP_DMA);
			memcpy(o, out, len);
			if (in)
			{
				i = heap_caps_malloc(l, MALLOC_CAP_DMA);
				memset(i, 0x00, l);
			}
		}
		else
		{
			o = out;
			if (in)
			{
				i = in;
			}
		}

		t.length = len * 8; // Length in bits
		t.tx_buffer = o;
		t.rxlength = 0; // this deafults to length

		if (in)
		{
			t.rx_buffer = i;
		}

		spi_device_polling_start(mcu_spi_handle, &t, portMAX_DELAY);
		is_running = true;
	}
	else
	{
		// check transfer state
		if (spi_device_polling_end(mcu_spi_handle, 0) == ESP_OK)
		{
			if (spi_dma_enabled)
			{
				// copy back memory from DMA
				if (in)
				{
					memcpy(in, i, len);
					heap_caps_free(i);
				}
				heap_caps_free(o);
			}

			is_running = false;
			return false;
		}
	}

	return true;
}

bool mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	static uint16_t data_offset = 0;
	uint32_t offset = data_offset;
	uint16_t max_transfer_size = (spi_dma_enabled) ? SPI_DMA_BUFFER_SIZE : SOC_SPI_MAXIMUM_BUFFER_SIZE;

	uint8_t *i = NULL;
	if (in)
	{
		i = &in[offset];
	}

	if (!mcu_spi_transaction(&out[offset], i, MIN(max_transfer_size, (len - offset))))
	{
		offset += max_transfer_size;
		if (offset >= len)
		{
			data_offset = 0;
			return false;
		}

		data_offset = (uint16_t)offset;
	}

	return true;
}
#endif
#endif

#if (defined(MCU_HAS_SPI2) && !defined(USE_ARDUINO_SPI_LIBRARY))

static spi_device_handle_t mcu_spi2_handle = NULL;

#ifndef mcu_spi2_xmit
uint8_t mcu_spi2_xmit(uint8_t data)
{
	uint8_t rxdata = 0xFF;
	spi_transaction_t spi_trans = {0};
	spi_trans.length = 8; // Number of bits NOT number of bytes.
	spi_trans.tx_buffer = &data;
	spi_trans.rx_buffer = &rxdata;

	spi_device_transmit(mcu_spi2_handle, &spi_trans);

	return rxdata;
}
#endif

#ifndef mcu_spi2_config
void mcu_spi2_config(spi_config_t config, uint32_t frequency)
{
	mcu_spi2_start(config, frequency);
	mcu_spi2_stop();
}
#endif

#ifndef mcu_spi2_start
void mcu_spi2_start(spi_config_t config, uint32_t frequency)
{
	if (mcu_spi2_handle)
	{
		spi_device_acquire_bus(mcu_spi2_handle, portMAX_DELAY);
		spi_device_release_bus(mcu_spi2_handle);
		spi_bus_remove_device(mcu_spi2_handle);
		mcu_spi2_handle = NULL;
		spi_bus_free(SPI2_PORT);
	}

	spi_bus_config_t spiconf = {
			.miso_io_num = SPI2_SDI_BIT,
			.mosi_io_num = SPI2_SDO_BIT,
			.sclk_io_num = SPI2_CLK_BIT,
			.quadwp_io_num = -1,
			.quadhd_io_num = -1,
			.data4_io_num = -1,
			.data5_io_num = -1,
			.data6_io_num = -1,
			.data7_io_num = -1,
			.max_transfer_sz = (config.enable_dma) ? SPI2_DMA_BUFFER_SIZE : SOC_SPI_MAXIMUM_BUFFER_SIZE,
			.flags = 0,
			.intr_flags = 0};
	// Initialize the SPI2 bus
	spi_bus_initialize(SPI2_PORT, &spiconf, (config.enable_dma) ? SPI_DMA_CH_AUTO : SPI_DMA_DISABLED);

	spi_device_interface_config_t mcu_spi_conf = {0};
	mcu_spi_conf.mode = config.mode;
	mcu_spi_conf.clock_speed_hz = frequency;
	mcu_spi_conf.spics_io_num = -1;
	mcu_spi_conf.queue_size = 1;
	if (config.enable_dma)
	{
		mcu_spi_conf.queue_size = 1;
		mcu_spi_conf.pre_cb = NULL;
		mcu_spi_conf.pre_cb = NULL;
	}

	spi2_dma_enabled = config.enable_dma;
	spi_bus_add_device(SPI2_PORT, &mcu_spi_conf, &mcu_spi2_handle);
	spi_device_acquire_bus(mcu_spi2_handle, portMAX_DELAY);
}
#endif

#ifndef mcu_spi2_stop
void mcu_spi2_stop(void)
{
	spi_device_release_bus(mcu_spi2_handle);
	mcu_spi2_handle = NULL;
}
#endif

#ifndef mcu_spi2_bulk_transfer
// data buffer for normal or DMA
static FORCEINLINE bool mcu_spi2_transaction(const uint8_t *out, uint8_t *in, uint16_t len)
{
	static void *o = NULL;
	static void *i = NULL;
	static bool is_running = false;
	static spi_transaction_t t = {0};

	// start a new transmition
	if (!is_running)
	{
		if (spi2_dma_enabled)
		{
			// DMA requires 4byte aligned transfers
			uint16_t l = (((len >> 2) + ((len & 0x03) ? 1 : 0)) << 2);
			o = heap_caps_malloc(l, MALLOC_CAP_DMA);
			memcpy(o, out, len);
			if (in)
			{
				i = heap_caps_malloc(l, MALLOC_CAP_DMA);
				memset(i, 0x00, l);
			}
		}
		else
		{
			o = out;
			if (in)
			{
				i = in;
			}
		}

		t.length = len * 8; // Length in bits
		t.tx_buffer = o;
		t.rxlength = 0; // this deafults to length

		if (in)
		{
			t.rx_buffer = i;
		}

		spi_device_polling_start(mcu_spi2_handle, &t, portMAX_DELAY);
		is_running = true;
	}
	else
	{
		// check transfer state
		if (spi_device_polling_end(mcu_spi2_handle, 0) == ESP_OK)
		{
			if (spi2_dma_enabled)
			{
				// copy back memory from DMA
				if (in)
				{
					memcpy(in, i, len);
					heap_caps_free(i);
				}
				heap_caps_free(o);
			}

			is_running = false;
			return false;
		}
	}

	return true;
}

bool mcu_spi2_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	static uint16_t data_offset = 0;
	uint32_t offset = data_offset;
	uint16_t max_transfer_size = (spi2_dma_enabled) ? SPI2_DMA_BUFFER_SIZE : SOC_SPI_MAXIMUM_BUFFER_SIZE;

	uint8_t *i = NULL;
	if (in)
	{
		i = &in[offset];
	}

	if (!mcu_spi2_transaction(&out[offset], i, MIN(max_transfer_size, (len - offset))))
	{
		offset += max_transfer_size;
		if (offset >= len)
		{
			data_offset = 0;
			return false;
		}

		data_offset = (uint16_t)offset;
	}

	return true;
}
#endif
#endif

#endif