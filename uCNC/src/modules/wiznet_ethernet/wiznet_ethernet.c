/*
    This is and example of how to add a custom task extension to µCNC
    Extension tasks can be added simply by adding an event listener to the core main loop
*/

#include "../../cnc.h"
#include "../softspi.h"
#include <stdint.h>
#include <stdbool.h>
#include "socket.h"

#if (UCNC_MODULE_VERSION < 11500 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#define WZ_SW_SPI 0
#define WZ_HW_SPI 1
#define WZ_HW_SPI2 2

#ifndef WZ_SPI
#define WZ_SPI WZ_HW_SPI
#endif

#ifndef WZ_SPI_CS
#define WZ_SPI_CS DOUT40
#endif

#ifndef WZ_SPI_RST
#define WZ_SPI_RST UNDEF_PIN
#endif

#if (WZ_SPI == WZ_SW_SPI)
#ifndef WZ_SPI_CLK
#define WZ_SPI_CLK DOUT30
#endif
#ifndef WZ_SPI_DO
#define WZ_SPI_DO DOUT29
#endif
#ifndef WZ_SPI_DI
#define WZ_SPI_DI DIN29
#endif
SOFTSPI(wz_spi, 1000000, 0, WZ_SPI_DO, WZ_SPI_DI, WZ_SPI_CLK);
#elif (WZ_SPI == WZ_HW_SPI)
HARDSPI(wz_spi, 1000000, 0, mcu_spi_port);
#elif (WZ_SPI == WZ_HW_SPI2)
HARDSPI(wz_spi, 1000000, 0, mcu_spi2_port);
#else
#error "Wiznet incorrect SPI port definition"
#endif

static uint8_t wz_spi_rd(void)
{
    return softspi_xmit(&wz_spi, 0xFF);
}

static void wz_spi_wr(uint8_t c)
{
    softspi_xmit(&wz_spi, c);
}

static void wz_spi_burst_rd(uint8_t *buffer, uint16_t len)
{
    softspi_bulk_xmit(&wz_spi, NULL, buffer, len);
}

static void wz_spi_burst_wr(uint8_t *buffer, uint16_t len)
{
    softspi_bulk_xmit(&wz_spi, buffer, buffer, len);
}

static void wz_cs_sel(void)
{
    io_clear_output(WZ_SPI_CS);
}

static void wz_cs_desel(void)
{
    io_set_output(WZ_SPI_CS);
}

/**
 * @brief 	Declarates a new module and adds the event listeners.
 * 			Again this should check the if the appropriate module option is enabled
 * 			To add this module you just neet to call LOAD_MODULE(mycustom_task_module); from inside the core code
 */

#ifdef ENABLE_SOCKETS
void mcu_sockets_init(void)
{
    uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};

#if ASSERT_PIN(WZ_SPI_RST)
    io_clear_output(WZ_SPI_RST);
    cnc_delay_ms(5000);
    io_set_output(WZ_SPI_RST);
    cnc_delay_ms(10000);
#endif

    reg_wizchip_spi_cbfunc(wz_spi_rd, wz_spi_wr);
    // reg_wizchip_spiburst_cbfunc(wz_spi_burst_rd, wz_spi_burst_wr);
    reg_wizchip_cs_cbfunc(wz_cs_sel, wz_cs_desel);

    if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
    {
        proto_print("WIZNET link failed to init");
        return;
    }

    uint8_t link_status = PHY_LINK_OFF;
    if (ctlwizchip(CW_GET_PHYLINK, (void *)&link_status) == -1)
    {
        proto_print("WIZNET link is off");
        return;
    }

    wiz_NetInfo netInfo = {
        .mac = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
        .ip = {192, 168, 1, 78},
        .sn = {255, 255, 255, 0},
        .gw = {192, 168, 1, 254},
        .dns = {192, 168, 1, 254},
#if USE_DHCP
        .dhcp = NETINFO_DHCP
#else
        .dhcp = NETINFO_STATIC
#endif
    };

    if (ctlnetwork(CN_SET_NETINFO, (void *)&netInfo) == -1)
    {
        proto_print("WIZNET net config error");
        return;
    }
    
    mcu_telnet_init();
}
#endif