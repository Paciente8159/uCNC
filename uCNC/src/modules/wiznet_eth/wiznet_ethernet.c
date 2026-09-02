/*
 * C99 WIZnet W5100/W5200/W5500 register and Ethernet implementation.
 * Derived from the register protocol and socket-buffer handling used by the
 * Arduino Ethernet library, without Arduino or C++ dependencies.
 */
#include "wiznet_ethernet.h"

#include <string.h>

#ifndef WIZNET_CS
#define WIZNET_CS DOUT40
#endif

#ifndef WIZNET_CS
#error "WIZNET_CS must name the chip-select output pin"
#endif

enum {
    WIZ_CHIP_NONE = 0,
    WIZ_CHIP_W5100 = 51,
    WIZ_CHIP_W5200 = 52,
    WIZ_CHIP_W5500 = 55
};

enum {
    WIZ_MR = 0x0000,
    WIZ_GAR = 0x0001,
    WIZ_SUBR = 0x0005,
    WIZ_SHAR = 0x0009,
    WIZ_SIPR = 0x000F,
    WIZ_RMSR = 0x001A,
    WIZ_TMSR = 0x001B,
    WIZ_VERSIONR_W5200 = 0x001F,
    WIZ_PHY_STATUS_W5200 = 0x0035,
    WIZ_PHY_CONFIG_W5500 = 0x002E,
    WIZ_VERSIONR_W5500 = 0x0039
};

enum {
    WIZ_SN_MR = 0x0000,
    WIZ_SN_CR = 0x0001,
    WIZ_SN_IR = 0x0002,
    WIZ_SN_SR = 0x0003,
    WIZ_SN_PORT = 0x0004,
    WIZ_SN_RX_SIZE = 0x001E,
    WIZ_SN_TX_SIZE = 0x001F,
    WIZ_SN_TX_FSR = 0x0020,
    WIZ_SN_TX_WR = 0x0024,
    WIZ_SN_RX_RSR = 0x0026,
    WIZ_SN_RX_RD = 0x0028
};

enum {
    WIZ_SOCK_OPEN = 0x01,
    WIZ_SOCK_LISTEN = 0x02,
    WIZ_SOCK_CLOSE = 0x10,
    WIZ_SOCK_SEND = 0x20,
    WIZ_SOCK_RECV = 0x40
};

enum {
    WIZ_SNSR_CLOSED = 0x00,
    WIZ_SNSR_INIT = 0x13
};

static uint8_t wiz_chip;
static uint8_t wiz_socket_count;
static uint16_t wiz_channel_base;
static uint8_t configured_mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
static ipv4_address_t configured_ip;
static ipv4_address_t configured_subnet;
static ipv4_address_t configured_gateway;
static bool static_configuration_set;
static bool hardware_ready;
static softspi_port_t *WIZNET_SPI;

static void wiz_delay_ms(uint16_t milliseconds)
{
    while (milliseconds-- != 0U) {
        mcu_delay_us(1000U);
    }
}

static void wiz_select(void)
{
    io_clear_output(WIZNET_CS);
}

static void wiz_deselect(void)
{
    io_set_output(WIZNET_CS);
}

static void wiz_spi_begin(void)
{
    softspi_start(WIZNET_SPI);
}

static void wiz_spi_end(void)
{
    softspi_stop(WIZNET_SPI);
}

static uint8_t wiz_spi_transfer(uint8_t value)
{
    return softspi_xmit(WIZNET_SPI, value);
}

static void wiz_write(uint16_t address, const uint8_t *data, uint16_t length)
{
    uint16_t i;

    if (length == 0U) {
        return;
    }

    wiz_spi_begin();
    if (wiz_chip == WIZ_CHIP_W5100) {
        for (i = 0U; i < length; ++i) {
            wiz_select();
            (void)wiz_spi_transfer(0xF0U);
            (void)wiz_spi_transfer((uint8_t)(address >> 8));
            (void)wiz_spi_transfer((uint8_t)address);
            (void)wiz_spi_transfer(data[i]);
            wiz_deselect();
            ++address;
        }
    } else if (wiz_chip == WIZ_CHIP_W5200) {
        wiz_select();
        (void)wiz_spi_transfer((uint8_t)(address >> 8));
        (void)wiz_spi_transfer((uint8_t)address);
        (void)wiz_spi_transfer((uint8_t)(((length >> 8) & 0x7FU) | 0x80U));
        (void)wiz_spi_transfer((uint8_t)length);
        for (i = 0U; i < length; ++i) {
            (void)wiz_spi_transfer(data[i]);
        }
        wiz_deselect();
    } else if (wiz_chip == WIZ_CHIP_W5500) {
        uint8_t control;

        wiz_select();
        if (address < 0x0100U) {
            control = 0x04U;
            (void)wiz_spi_transfer(0U);
        } else if (address < 0x8000U) {
            control = (uint8_t)(((address >> 3) & 0xE0U) | 0x0CU);
            (void)wiz_spi_transfer(0U);
        } else if (address < 0xC000U) {
            control = (uint8_t)(((address >> 6) & 0xE0U) | 0x14U);
            (void)wiz_spi_transfer((uint8_t)(address >> 8));
        } else {
            control = (uint8_t)(((address >> 6) & 0xE0U) | 0x1CU);
            (void)wiz_spi_transfer((uint8_t)(address >> 8));
        }
        (void)wiz_spi_transfer((uint8_t)address);
        (void)wiz_spi_transfer(control);
        for (i = 0U; i < length; ++i) {
            (void)wiz_spi_transfer(data[i]);
        }
        wiz_deselect();
    }
    wiz_spi_end();
}

static void wiz_read(uint16_t address, uint8_t *data, uint16_t length)
{
    uint16_t i;

    if (length == 0U) {
        return;
    }

    wiz_spi_begin();
    if (wiz_chip == WIZ_CHIP_W5100) {
        for (i = 0U; i < length; ++i) {
            wiz_select();
            (void)wiz_spi_transfer(0x0FU);
            (void)wiz_spi_transfer((uint8_t)(address >> 8));
            (void)wiz_spi_transfer((uint8_t)address);
            data[i] = wiz_spi_transfer(0U);
            wiz_deselect();
            ++address;
        }
    } else if (wiz_chip == WIZ_CHIP_W5200) {
        wiz_select();
        (void)wiz_spi_transfer((uint8_t)(address >> 8));
        (void)wiz_spi_transfer((uint8_t)address);
        (void)wiz_spi_transfer((uint8_t)((length >> 8) & 0x7FU));
        (void)wiz_spi_transfer((uint8_t)length);
        for (i = 0U; i < length; ++i) {
            data[i] = wiz_spi_transfer(0U);
        }
        wiz_deselect();
    } else if (wiz_chip == WIZ_CHIP_W5500) {
        uint8_t control;

        wiz_select();
        if (address < 0x0100U) {
            control = 0x00U;
            (void)wiz_spi_transfer(0U);
        } else if (address < 0x8000U) {
            control = (uint8_t)(((address >> 3) & 0xE0U) | 0x08U);
            (void)wiz_spi_transfer(0U);
        } else if (address < 0xC000U) {
            control = (uint8_t)(((address >> 6) & 0xE0U) | 0x10U);
            (void)wiz_spi_transfer((uint8_t)(address >> 8));
        } else {
            control = (uint8_t)(((address >> 6) & 0xE0U) | 0x18U);
            (void)wiz_spi_transfer((uint8_t)(address >> 8));
        }
        (void)wiz_spi_transfer((uint8_t)address);
        (void)wiz_spi_transfer(control);
        for (i = 0U; i < length; ++i) {
            data[i] = wiz_spi_transfer(0U);
        }
        wiz_deselect();
    } else {
        memset(data, 0, length);
    }
    wiz_spi_end();
}

static uint8_t wiz_read8(uint16_t address)
{
    uint8_t value = 0U;
    wiz_read(address, &value, 1U);
    return value;
}

static void wiz_write8(uint16_t address, uint8_t value)
{
    wiz_write(address, &value, 1U);
}

static uint16_t wiz_read16(uint16_t address)
{
    uint8_t bytes[2];
    wiz_read(address, bytes, 2U);
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static void wiz_write16(uint16_t address, uint16_t value)
{
    uint8_t bytes[2];
    bytes[0] = (uint8_t)(value >> 8);
    bytes[1] = (uint8_t)value;
    wiz_write(address, bytes, 2U);
}

static uint16_t wiz_socket_register(uint8_t socket_number, uint16_t offset)
{
    return (uint16_t)(wiz_channel_base +
                      ((uint16_t)socket_number * 0x0100U) + offset);
}

static uint16_t wiz_stable_read16(uint16_t address)
{
    uint16_t previous = wiz_read16(address);
    uint8_t attempt;

    for (attempt = 0U; attempt < WIZNET_STABLE_READ_LIMIT; ++attempt) {
        uint16_t current = wiz_read16(address);
        if (current == previous) {
            return current;
        }
        previous = current;
    }
    WIZDGB("WIZnet: unstable 16-bit register read at 0x%04X, using 0x%04X\n",
           (unsigned int)address, (unsigned int)previous);
    return previous;
}

static bool wiz_socket_command(uint8_t socket_number, uint8_t command)
{
    uint16_t address = wiz_socket_register(socket_number, WIZ_SN_CR);
    uint16_t attempt;

    wiz_write8(address, command);
    for (attempt = 0U; attempt < WIZNET_COMMAND_POLL_LIMIT; ++attempt) {
        if (wiz_read8(address) == 0U) {
            return true;
        }
    }
    WIZDGB("WIZnet: socket %u command 0x%X timed out, CR=0x%X\n",
           (unsigned int)socket_number, (unsigned int)command,
           (unsigned int)wiz_read8(address));
    return false;
}

static bool wiz_soft_reset(uint8_t chip)
{
    uint8_t attempt;

    wiz_chip = chip;
    WIZDGB("WIZnet: trying chip protocol W%u\n", (unsigned int)chip);
    wiz_write8(WIZ_MR, 0x80U);
    for (attempt = 0U; attempt < 20U; ++attempt) {
        if (wiz_read8(WIZ_MR) == 0U) {
            return true;
        }
        wiz_delay_ms(1U);
    }
    WIZDGB("WIZnet: W%u soft reset timed out, MR=0x%X\n",
           (unsigned int)chip, (unsigned int)wiz_read8(WIZ_MR));
    return false;
}

static bool wiz_detect_w5200(void)
{
    uint8_t version;

    if (!wiz_soft_reset(WIZ_CHIP_W5200)) {
        return false;
    }
    wiz_write8(WIZ_MR, 0x08U);
    if (wiz_read8(WIZ_MR) != 0x08U) {
        return false;
    }
    wiz_write8(WIZ_MR, 0x10U);
    if (wiz_read8(WIZ_MR) != 0x10U) {
        return false;
    }
    wiz_write8(WIZ_MR, 0U);
    version = wiz_read8(WIZ_VERSIONR_W5200);
    WIZDGB("WIZnet: W5200 probe VERSIONR=%u\n", (unsigned int)version);
    return (wiz_read8(WIZ_MR) == 0U) && (version == 3U);
}

static bool wiz_detect_w5500(void)
{
    uint8_t version;

    if (!wiz_soft_reset(WIZ_CHIP_W5500)) {
        return false;
    }
    wiz_write8(WIZ_MR, 0x08U);
    if (wiz_read8(WIZ_MR) != 0x08U) {
        return false;
    }
    wiz_write8(WIZ_MR, 0x10U);
    if (wiz_read8(WIZ_MR) != 0x10U) {
        return false;
    }
    wiz_write8(WIZ_MR, 0U);
    version = wiz_read8(WIZ_VERSIONR_W5500);
    WIZDGB("WIZnet: W5500 probe VERSIONR=%u\n", (unsigned int)version);
    return (wiz_read8(WIZ_MR) == 0U) && (version == 4U);
}

static bool wiz_detect_w5100(void)
{
    if (!wiz_soft_reset(WIZ_CHIP_W5100)) {
        return false;
    }
    wiz_write8(WIZ_MR, 0x10U);
    if (wiz_read8(WIZ_MR) != 0x10U) {
        return false;
    }
    wiz_write8(WIZ_MR, 0x12U);
    if (wiz_read8(WIZ_MR) != 0x12U) {
        return false;
    }
    wiz_write8(WIZ_MR, 0U);
    return wiz_read8(WIZ_MR) == 0U;
}

static uint16_t wiz_tx_base(uint8_t socket_number)
{
    uint16_t base = (wiz_chip == WIZ_CHIP_W5100) ? 0x4000U : 0x8000U;
    return (uint16_t)(base +
                      ((uint16_t)socket_number * WIZNET_SOCKET_BUFFER_SIZE));
}

static uint16_t wiz_rx_base(uint8_t socket_number)
{
    uint16_t base = (wiz_chip == WIZ_CHIP_W5100) ? 0x6000U : 0xC000U;
    return (uint16_t)(base +
                      ((uint16_t)socket_number * WIZNET_SOCKET_BUFFER_SIZE));
}

static void wiz_ring_write(uint8_t socket_number, uint16_t pointer,
                           const uint8_t *data, uint16_t length)
{
    uint16_t offset = (uint16_t)(pointer & (WIZNET_SOCKET_BUFFER_SIZE - 1U));
    uint16_t first = length;

    if ((uint16_t)(offset + length) > WIZNET_SOCKET_BUFFER_SIZE) {
        first = (uint16_t)(WIZNET_SOCKET_BUFFER_SIZE - offset);
    }
    wiz_write((uint16_t)(wiz_tx_base(socket_number) + offset), data, first);
    if (first < length) {
        wiz_write(wiz_tx_base(socket_number), data + first,
                  (uint16_t)(length - first));
    }
}

static void wiz_ring_read(uint8_t socket_number, uint16_t pointer,
                          uint8_t *data, uint16_t length)
{
    uint16_t offset = (uint16_t)(pointer & (WIZNET_SOCKET_BUFFER_SIZE - 1U));
    uint16_t first = length;

    if ((uint16_t)(offset + length) > WIZNET_SOCKET_BUFFER_SIZE) {
        first = (uint16_t)(WIZNET_SOCKET_BUFFER_SIZE - offset);
    }
    wiz_read((uint16_t)(wiz_rx_base(socket_number) + offset), data, first);
    if (first < length) {
        wiz_read(wiz_rx_base(socket_number), data + first,
                 (uint16_t)(length - first));
    }
}

void wiznet_set_mac(const uint8_t *mac)
{
    if (mac != NULL) {
        memcpy(configured_mac, mac, sizeof(configured_mac));
        WIZDGB("WIZnet: MAC set to %X:%X:%X:%X:%X:%X\n",
               (unsigned int)configured_mac[0],
               (unsigned int)configured_mac[1],
               (unsigned int)configured_mac[2],
               (unsigned int)configured_mac[3],
               (unsigned int)configured_mac[4],
               (unsigned int)configured_mac[5]);
        if (hardware_ready) {
            wiz_write(WIZ_SHAR, configured_mac, sizeof(configured_mac));
            WIZDGB("WIZnet: MAC register updated while running\n");
        }
    } else {
        WIZDGB("WIZnet: wiznet_set_mac called with NULL\n");
    }
}

void wiznet_config(ipv4_address_t ip, ipv4_address_t sn, ipv4_address_t gw)
{
    configured_ip = ip;
    configured_subnet = sn;
    configured_gateway = gw;
    static_configuration_set = true;

    WIZDGB("WIZnet: static IP %u.%u.%u.%u, mask %u.%u.%u.%u, gateway %u.%u.%u.%u\n",
           (unsigned int)ip.octets[0], (unsigned int)ip.octets[1],
           (unsigned int)ip.octets[2], (unsigned int)ip.octets[3],
           (unsigned int)sn.octets[0], (unsigned int)sn.octets[1],
           (unsigned int)sn.octets[2], (unsigned int)sn.octets[3],
           (unsigned int)gw.octets[0], (unsigned int)gw.octets[1],
           (unsigned int)gw.octets[2], (unsigned int)gw.octets[3]);

    if (hardware_ready) {
        wiz_write(WIZ_GAR, configured_gateway.octets, 4U);
        wiz_write(WIZ_SUBR, configured_subnet.octets, 4U);
        wiz_write(WIZ_SIPR, configured_ip.octets, 4U);
        WIZDGB("WIZnet: live network registers updated\n");
    }
}

ipv4_address_t wiznet_get_ip(void)
{
    ipv4_address_t ip = {.ip = 0U};
    if (hardware_ready) {
        wiz_read(WIZ_SIPR, ip.octets, 4U);
    } else if (static_configuration_set) {
        ip = configured_ip;
    }
    return ip;
}

bool wiznet_is_ready(void)
{
    return hardware_ready;
}

void wiznet_init(softspi_port_t *spiport)
{
    WIZNET_SPI = spiport;
    uint8_t socket_number;
    spi_config_t spi_config = {0};
    uint8_t readback_mac[6];
    uint8_t readback_ip[4];
    uint8_t readback_mask[4];
    uint8_t readback_gateway[4];
    bool backend_registered;

    hardware_ready = false;
    wiz_chip = WIZ_CHIP_NONE;
    wiz_socket_count = 0U;

    io_config_output(WIZNET_CS);
    wiz_deselect();

    WIZDGB("WIZnet: initialization started, frequency=%lu Hz\n", spiport->spifreq);

    spi_config.mode = 0U;
    if (WIZNET_SPI != NULL)
    {
        softspi_config(WIZNET_SPI, spi_config, spiport->spifreq);
    }
    else
    {
        WIZDGB("WIZnet: WIZNET_SPI is NULL; using the default hardware SPI path\n");
    }

    /* Accommodate shields whose reset supervisor can hold the chip reset for
     * up to approximately 560 ms after the MCU starts. */
    wiz_delay_ms((uint16_t)WIZNET_STARTUP_DELAY_MS);

    if (wiz_detect_w5200())
    {
        wiz_channel_base = 0x4000U;
        wiz_socket_count = 8U;
        WIZDGB("WIZnet: detected W5200, PHY=%s (PSTATUS=0x%X)\n",
               ((wiz_read8(WIZ_PHY_STATUS_W5200) & 0x20U) != 0U) ? "UP" : "DOWN",
               (unsigned int)wiz_read8(WIZ_PHY_STATUS_W5200));
    }
    else if (wiz_detect_w5500())
    {
        wiz_channel_base = 0x1000U;
        wiz_socket_count = 8U;
        WIZDGB("WIZnet: detected W5500, PHY=%s (PHYCFGR=0x%X)\n",
               ((wiz_read8(WIZ_PHY_CONFIG_W5500) & 0x01U) != 0U) ? "UP" : "DOWN",
               (unsigned int)wiz_read8(WIZ_PHY_CONFIG_W5500));
    }
    else if (wiz_detect_w5100())
    {
        wiz_channel_base = 0x0400U;
        wiz_socket_count = 4U;
        wiz_write8(WIZ_RMSR, 0x55U);
        wiz_write8(WIZ_TMSR, 0x55U);
        WIZDGB("WIZnet: detected W5100; PHY status is not readable\n");
    }
    else
    {
        wiz_chip = WIZ_CHIP_NONE;
        WIZDGB("WIZnet: ERROR - no supported W5100/W5200/W5500 detected\n");
        return;
    }

    WIZDGB("WIZnet: chip=%u, sockets=%u, channel base=0x%04X\n",
           (unsigned int)wiz_chip, (unsigned int)wiz_socket_count,
           (unsigned int)wiz_channel_base);

    if (wiz_chip != WIZ_CHIP_W5100)
    {
        for (socket_number = 0U; socket_number < WIZNET_MAX_HW_SOCKETS;
             ++socket_number)
        {
            wiz_write8(wiz_socket_register(socket_number, WIZ_SN_RX_SIZE), 2U);
            wiz_write8(wiz_socket_register(socket_number, WIZ_SN_TX_SIZE), 2U);
        }
        WIZDGB("WIZnet: configured %u RX/TX buffers of %u bytes\n",
               (unsigned int)WIZNET_MAX_HW_SOCKETS,
               (unsigned int)WIZNET_SOCKET_BUFFER_SIZE);
    }

    wiz_write(WIZ_SHAR, configured_mac, sizeof(configured_mac));
    if (static_configuration_set)
    {
        wiz_write(WIZ_GAR, configured_gateway.octets, 4U);
        wiz_write(WIZ_SUBR, configured_subnet.octets, 4U);
        wiz_write(WIZ_SIPR, configured_ip.octets, 4U);
    }
    else
    {
        /* DHCP is deliberately not hidden inside this low-level driver. */
        static const uint8_t zero[4] = {0U, 0U, 0U, 0U};
        wiz_write(WIZ_GAR, zero, 4U);
        wiz_write(WIZ_SUBR, zero, 4U);
        wiz_write(WIZ_SIPR, zero, 4U);
        WIZDGB("WIZnet: ERROR - no static configuration; DHCP is not implemented\n");
        return;
    }

    wiz_read(WIZ_SHAR, readback_mac, sizeof(readback_mac));
    wiz_read(WIZ_GAR, readback_gateway, sizeof(readback_gateway));
    wiz_read(WIZ_SUBR, readback_mask, sizeof(readback_mask));
    wiz_read(WIZ_SIPR, readback_ip, sizeof(readback_ip));
    WIZDGB("WIZnet: register readback MAC %X:%X:%X:%X:%X:%X\n",
           (unsigned int)readback_mac[0], (unsigned int)readback_mac[1],
           (unsigned int)readback_mac[2], (unsigned int)readback_mac[3],
           (unsigned int)readback_mac[4], (unsigned int)readback_mac[5]);
    WIZDGB("WIZnet: register readback IP %u.%u.%u.%u, mask %u.%u.%u.%u, gateway %u.%u.%u.%u\n",
           (unsigned int)readback_ip[0], (unsigned int)readback_ip[1],
           (unsigned int)readback_ip[2], (unsigned int)readback_ip[3],
           (unsigned int)readback_mask[0], (unsigned int)readback_mask[1],
           (unsigned int)readback_mask[2], (unsigned int)readback_mask[3],
           (unsigned int)readback_gateway[0],
           (unsigned int)readback_gateway[1],
           (unsigned int)readback_gateway[2],
           (unsigned int)readback_gateway[3]);
    if (memcmp(readback_mac, configured_mac, sizeof(readback_mac)) != 0 ||
        memcmp(readback_ip, configured_ip.octets, sizeof(readback_ip)) != 0 ||
        memcmp(readback_mask, configured_subnet.octets,
               sizeof(readback_mask)) != 0 ||
        memcmp(readback_gateway, configured_gateway.octets,
               sizeof(readback_gateway)) != 0)
    {
        WIZDGB("WIZnet: ERROR - network register readback mismatch\n");
    }

    hardware_ready = true;
    backend_registered = wiznet_socket_backend_register();
    if (!backend_registered)
    {
        hardware_ready = false;
        WIZDGB("WIZnet: ERROR - socket backend registration failed\n");
    }
    else
    {
        WIZDGB("WIZnet: initialization complete; socket backend registered\n");
    }
}

uint8_t wiznet_hw_socket_count(void)
{
    return wiz_socket_count;
}

uint8_t wiznet_hw_socket_status(uint8_t socket_number)
{
    if (!hardware_ready || socket_number >= wiz_socket_count) {
        return WIZ_SNSR_CLOSED;
    }
    return wiz_read8(wiz_socket_register(socket_number, WIZ_SN_SR));
}

uint8_t wiznet_hw_socket_interrupt(uint8_t socket_number)
{
    if (!hardware_ready || socket_number >= wiz_socket_count) {
        return 0U;
    }
    return wiz_read8(wiz_socket_register(socket_number, WIZ_SN_IR));
}

void wiznet_hw_socket_clear_interrupt(uint8_t socket_number, uint8_t mask)
{
    if (hardware_ready && socket_number < wiz_socket_count) {
        wiz_write8(wiz_socket_register(socket_number, WIZ_SN_IR), mask);
    }
}

void wiznet_hw_socket_close(uint8_t socket_number)
{
    if (hardware_ready && socket_number < wiz_socket_count) {
        WIZDGB("WIZnet: socket %u closing (status=0x%X)\n",
               (unsigned int)socket_number,
               (unsigned int)wiznet_hw_socket_status(socket_number));
        if (!wiz_socket_command(socket_number, WIZ_SOCK_CLOSE)) {
            WIZDGB("WIZnet: ERROR - socket %u CLOSE command failed\n",
                   (unsigned int)socket_number);
        }
        wiz_write8(wiz_socket_register(socket_number, WIZ_SN_IR), 0xFFU);
    }
}

bool wiznet_hw_socket_open_tcp_server(uint8_t socket_number, uint16_t port)
{
    if (!hardware_ready || socket_number >= wiz_socket_count || port == 0U) {
        WIZDGB("WIZnet: invalid TCP listener request socket=%u port=%u ready=%u\n",
               (unsigned int)socket_number, (unsigned int)port,
               hardware_ready ? 1U : 0U);
        return false;
    }

    WIZDGB("WIZnet: opening TCP listener on hardware socket %u, port %u\n",
           (unsigned int)socket_number, (unsigned int)port);
    wiznet_hw_socket_close(socket_number);
    /* TCP + No Delayed ACK, matching the Arduino Ethernet implementation. */
    wiz_write8(wiz_socket_register(socket_number, WIZ_SN_MR), 0x21U);
    wiz_write8(wiz_socket_register(socket_number, WIZ_SN_IR), 0xFFU);
    wiz_write16(wiz_socket_register(socket_number, WIZ_SN_PORT), port);
    if (!wiz_socket_command(socket_number, WIZ_SOCK_OPEN) ||
        wiznet_hw_socket_status(socket_number) != WIZ_SNSR_INIT) {
        WIZDGB("WIZnet: ERROR - socket %u failed to enter INIT, status=0x%X\n",
               (unsigned int)socket_number,
               (unsigned int)wiznet_hw_socket_status(socket_number));
        wiznet_hw_socket_close(socket_number);
        return false;
    }
    if (!wiz_socket_command(socket_number, WIZ_SOCK_LISTEN)) {
        WIZDGB("WIZnet: ERROR - socket %u LISTEN command failed\n",
               (unsigned int)socket_number);
        wiznet_hw_socket_close(socket_number);
        return false;
    }
    WIZDGB("WIZnet: socket %u listening, status=0x%X\n",
           (unsigned int)socket_number,
           (unsigned int)wiznet_hw_socket_status(socket_number));
    return true;
}

int wiznet_hw_socket_send(uint8_t socket_number, const uint8_t *data,
                          size_t length)
{
    uint16_t free_size;
    uint16_t pointer;
    uint16_t accepted;

    if (!hardware_ready || socket_number >= wiz_socket_count || data == NULL ||
        length == 0U) {
        return 0;
    }

    free_size = wiz_stable_read16(
        wiz_socket_register(socket_number, WIZ_SN_TX_FSR));
    if (free_size == 0U) {
        WIZDGB("WIZnet: socket %u TX would block (no free space)\n",
               (unsigned int)socket_number);
        return 0;
    }

    accepted = (length < free_size) ? (uint16_t)length : free_size;
    if (accepted > WIZNET_SOCKET_BUFFER_SIZE) {
        accepted = WIZNET_SOCKET_BUFFER_SIZE;
    }

    pointer = wiz_read16(wiz_socket_register(socket_number, WIZ_SN_TX_WR));
    wiz_ring_write(socket_number, pointer, data, accepted);
    wiz_write16(wiz_socket_register(socket_number, WIZ_SN_TX_WR),
                (uint16_t)(pointer + accepted));
    if (!wiz_socket_command(socket_number, WIZ_SOCK_SEND)) {
        WIZDGB("WIZnet: ERROR - socket %u SEND command failed\n",
               (unsigned int)socket_number);
        return -1;
    }
    WIZDGB("WIZnet: socket %u accepted %u/%lu TX bytes, free before=%u\n",
           (unsigned int)socket_number, (unsigned int)accepted,
           (unsigned long)length, (unsigned int)free_size);
    return (int)accepted;
}

uint16_t wiznet_hw_socket_available(uint8_t socket_number)
{
    if (!hardware_ready || socket_number >= wiz_socket_count) {
        return 0U;
    }
    return wiz_stable_read16(
        wiz_socket_register(socket_number, WIZ_SN_RX_RSR));
}

int wiznet_hw_socket_receive(uint8_t socket_number, uint8_t *data,
                             size_t capacity)
{
    uint16_t available;
    uint16_t pointer;
    uint16_t received;

    if (!hardware_ready || socket_number >= wiz_socket_count || data == NULL ||
        capacity == 0U) {
        return 0;
    }

    available = wiznet_hw_socket_available(socket_number);
    if (available == 0U) {
        return 0;
    }

    received = (capacity < available) ? (uint16_t)capacity : available;
    pointer = wiz_read16(wiz_socket_register(socket_number, WIZ_SN_RX_RD));
    wiz_ring_read(socket_number, pointer, data, received);
    wiz_write16(wiz_socket_register(socket_number, WIZ_SN_RX_RD),
                (uint16_t)(pointer + received));
    if (!wiz_socket_command(socket_number, WIZ_SOCK_RECV)) {
        WIZDGB("WIZnet: ERROR - socket %u RECV command failed\n",
               (unsigned int)socket_number);
        return -1;
    }
    WIZDGB("WIZnet: socket %u received %u bytes (%u were available)\n",
           (unsigned int)socket_number, (unsigned int)received,
           (unsigned int)available);
    return (int)received;
}
