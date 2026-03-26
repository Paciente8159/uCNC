/**
 * SPI C implementation
 */

#include "../../../cnc.h"
#if (CONFIG_IDF_TARGET_ESP32)
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_ipc.h"
#ifdef MCU_HAS_SPI
#include "hal/spi_types.h"
#include "driver/spi_master.h"
SemaphoreHandle_t spi_access_lock = NULL;
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
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

static spi_device_handle_t spi_dev = NULL;
static spi_config_t last_cfg;
static uint32_t last_freq = 0;
static bool spi_initialized = false;

void mcu_spi_config(spi_config_t config, uint32_t frequency)
{
    if (!spi_initialized)
    {
        spi_bus_config_t buscfg = {
            .mosi_io_num = SPI_SDO_BIT,
            .miso_io_num = SPI_SDI_BIT,
            .sclk_io_num = SPI_CLK_BIT,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 4096
        };

        esp_err_t err = spi_bus_initialize(SPI_INSTANCE, &buscfg,
                                           config.enable_dma ? SPI_DMA_CH_AUTO : 0);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "spi_bus_initialize failed: %d", err);
            return;
        }

        spi_initialized = true;
    }

    // Remove previous device if exists
    if (spi_dev)
    {
        spi_bus_remove_device(spi_dev);
        spi_dev = NULL;
    }

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = (int)frequency,
        .mode = config.mode,
        .spics_io_num = -1,   // user controls CS externally
        .queue_size = 1,
        .flags = 0
    };

    esp_err_t err = spi_bus_add_device(SPI_INSTANCE, &devcfg, &spi_dev);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %d", err);
        return;
    }

    last_cfg = config;
    last_freq = frequency;
}

void mcu_spi_start(spi_config_t config, uint32_t frequency)
{
    mcu_spi_config(config, frequency);
}

void mcu_spi_stop(void)
{
}

uint8_t mcu_spi_xmit(uint8_t data)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    uint8_t rx = 0;

    t.length = 8;
    t.tx_buffer = &data;
    t.rx_buffer = &rx;

    esp_err_t err = spi_device_transmit(spi_dev, &t);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "spi_device_transmit failed: %d", err);
        return 0xFF;
    }

    return rx;
}

bool mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
    static bool busy = false;
    static spi_transaction_t t;

    if (!busy)
    {
        memset(&t, 0, sizeof(t));

        t.length = len * 8;

        if (out == NULL)
        {
            static uint8_t ff_buf[256];
            memset(ff_buf, 0xFF, sizeof(ff_buf));
            t.tx_buffer = ff_buf;
        }
        else
        {
            t.tx_buffer = out;
        }

        t.rx_buffer = in;

        esp_err_t err = spi_device_queue_trans(spi_dev, &t, 0);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "queue failed: %d", err);
            return false;
        }

        busy = true;
        return true; // still running
    }

    // Check if finished
    spi_transaction_t *ret;
    esp_err_t err = spi_device_get_trans_result(spi_dev, &ret, 0);

    if (err == ESP_ERR_TIMEOUT)
    {
        return true; // still running
    }

    busy = false;
    return false; // finished
}

spi_port_t __attribute__((used)) mcu_spi_port = {
    .isbusy = false,
    .start = mcu_spi_start,
    .xmit = mcu_spi_xmit,
    .bulk_xmit = mcu_spi_bulk_transfer,
    .stop = mcu_spi_stop
};

#endif

#if (defined(MCU_HAS_SPI2) && !defined(USE_ARDUINO_SPI_LIBRARY))

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

static spi_device_handle_t spi2_dev = NULL;
static spi_config_t spi2_last_cfg;
static uint32_t spi2_last_freq = 0;
static bool spi2_initialized = false;

void mcu_spi2_config(spi_config_t config, uint32_t frequency)
{
    if (!spi2_initialized)
    {
        spi_bus_config_t buscfg = {
            .mosi_io_num = SPI2_SDO_BIT,
            .miso_io_num = SPI2_SDI_BIT,
            .sclk_io_num = SPI2_CLK_BIT,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 4096
        };

        esp_err_t err = spi_bus_initialize(SPI2_INSTANCE, &buscfg,
                                           config.enable_dma ? SPI_DMA_CH_AUTO : 0);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "spi_bus_initialize failed: %d", err);
            return;
        }

        spi2_initialized = true;
    }

    // Remove previous device if exists
    if (spi2_dev)
    {
        spi_bus_remove_device(spi2_dev);
        spi2_dev = NULL;
    }

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = (int)frequency,
        .mode = config.mode,
        .spics_io_num = -1,   // user controls CS externally
        .queue_size = 1,
        .flags = 0
    };

    esp_err_t err = spi_bus_add_device(SPI2_INSTANCE, &devcfg, &spi2_dev);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %d", err);
        return;
    }

    spi2_last_cfg = config;
    spi2_last_freq = frequency;
}

void mcu_spi2_start(spi_config_t config, uint32_t frequency)
{
    mcu_spi2_config(config, frequency);
}

void mcu_spi2_stop(void)
{
}

uint8_t mcu_spi2_xmit(uint8_t data)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    uint8_t rx = 0;

    t.length = 8;
    t.tx_buffer = &data;
    t.rx_buffer = &rx;

    esp_err_t err = spi_device_transmit(spi2_dev, &t);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "spi_device_transmit failed: %d", err);
        return 0xFF;
    }

    return rx;
}

bool mcu_spi2_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
    static bool busy = false;
    static spi_transaction_t t;

    if (!busy)
    {
        memset(&t, 0, sizeof(t));

        t.length = len * 8;

        if (out == NULL)
        {
            static uint8_t ff_buf[256];
            memset(ff_buf, 0xFF, sizeof(ff_buf));
            t.tx_buffer = ff_buf;
        }
        else
        {
            t.tx_buffer = out;
        }

        t.rx_buffer = in;

        esp_err_t err = spi_device_queue_trans(spi2_dev, &t, 0);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "queue failed: %d", err);
            return false;
        }

        busy = true;
        return true; // still running
    }

    // Check if finished
    spi_transaction_t *ret;
    esp_err_t err = spi_device_get_trans_result(spi2_dev, &ret, 0);

    if (err == ESP_ERR_TIMEOUT)
    {
        return true; // still running
    }

    busy = false;
    return false; // finished
}

// -----------------------------
// PORT STRUCT
// -----------------------------
spi_port_t __attribute__((used)) mcu_spi2_port = {
    .isbusy = false,
    .start = mcu_spi2_start,
    .xmit = mcu_spi2_xmit,
    .bulk_xmit = mcu_spi2_bulk_transfer,
    .stop = mcu_spi2_stop
};

#endif

#endif