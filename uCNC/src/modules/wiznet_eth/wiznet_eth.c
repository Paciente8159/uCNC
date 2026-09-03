/*
	This is and example of how to add a custom task extension to µCNC
	Extension tasks can be added simply by adding an event listener to the core main loop
*/

#include "../../cnc.h"
#include "../softspi.h"
#include <stdint.h>
#include <stdbool.h>
#include "../net/socket.h"
#include "wiznet_ethernet.h"

#if (UCNC_MODULE_VERSION < 11680 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#define WIZNET_SW_SPI 0
#define WIZNET_HW_SPI 1
#define WIZNET_HW_SPI2 2

#ifndef WIZNET_INTERFACE
#define WIZNET_INTERFACE WIZNET_HW_SPI
#endif

#ifndef WIZNET_SPI_FREQ
#define WIZNET_SPI_FREQ 1000000
#endif

#if (WIZNET_INTERFACE == WIZNET_SW_SPI)
#ifndef WIZNET_SPI_CLK
#define WIZNET_SPI_CLK DOUT30
#endif
#ifndef WIZNET_SPI_SDO
#define WIZNET_SPI_SDO DOUT29
#endif
#ifndef WIZNET_SPI_SDI
#define WIZNET_SPI_SDI DIN29
#endif
SOFTSPI(wiznet_spi, WIZNET_SPI_FREQ, 0, WIZNET_SPI_SDO, WIZNET_SPI_SDI, WIZNET_SPI_CLK);
#elif (WIZNET_INTERFACE == WIZNET_HW_SPI)
HARDSPI(wiznet_spi, WIZNET_SPI_FREQ, 0, mcu_spi_port);
#elif (WIZNET_INTERFACE == WIZNET_HW_SPI2)
HARDSPI(wiznet_spi, WIZNET_SPI_FREQ, 0, mcu_spi2_port);
#endif

#define WIZNET_SPI_PORT (&wiznet_spi)

#ifdef USE_STATIC_IP
#ifndef STATIC_IP_IP
// 192.168.1.200
#define STATIC_IP_IP 3355551936
#endif
#ifndef STATIC_IP_GW
// 192.168.1.1
#define STATIC_IP_GW 16885952
#endif
#ifndef STATIC_IP_SUB
// 255.255.255.0
#define STATIC_IP_SUB 16777215
#endif
#else
#error "Wiznet module requires static IP configuration"
#endif

#ifdef ENABLE_PARSER_MODULES

bool wiznet_custom_grbl_cmd(void *args)
{
	grbl_cmd_args_t *cmd_params = (grbl_cmd_args_t *)args;
	char arg[64];
	// uint8_t has_arg = (cmd_params->next_char == '=');
	memset(arg, 0, sizeof(arg));

	if (!strcmp((const char *)cmd_params->cmd, "IP"))
	{
		ipv4_address_t ip = {0};
		if (wiznet_is_ready())
		{
			ip.ip = wiznet_get_ip().ip;
		}

		proto_info("IP>%hu.%hu.%hu.%hu", ip.octets[0], ip.octets[1], ip.octets[2], ip.octets[3]);

		*(cmd_params->error) = STATUS_OK;
		return EVENT_HANDLED;
	}
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(grbl_cmd, wiznet_custom_grbl_cmd);

#endif

// override the network device
void mcu_network_init()
{
	ipv4_address_t ip = {STATIC_IP_IP};
	ipv4_address_t sn = {STATIC_IP_SUB};
	ipv4_address_t gw = {STATIC_IP_GW};
	uint8_t mac[6] = {0x00, 0x08, 0xDC, 0x44, 0x55, 0x66};
	wiznet_set_mac(mac);
	wiznet_config(ip, sn, gw);
	WIZNET_SPI_PORT->spifreq = WIZNET_SPI_FREQ;
	wiznet_init(WIZNET_SPI_PORT);
	DBGMSG("Wiznet init\r\n");
}

/**
 * @brief 	Declarates a new module and adds the event listeners.
 * 			Again this should check the if the appropriate module option is enabled
 * 			To add this module you just neet to call LOAD_MODULE(mycustom_task_module); from inside the core code
 */
DECL_MODULE(wiznet_eth)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(grbl_cmd, wiznet_custom_grbl_cmd);
#else
#warning "Parser extensions are not enabled. Wiznet Grbl commands will not work."
#endif
}
