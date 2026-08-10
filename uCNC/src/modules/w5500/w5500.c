/*
	This is and example of how to add a custom task extension to µCNC
	Extension tasks can be added simply by adding an event listener to the core main loop
*/

#include "../../cnc.h"
#include "../softspi.h"
#include <stdint.h>
#include <stdbool.h>
#include "../net/socket.h"
#include "driver/wizchip_conf.h"
#include "wiznet5500_socket_device.h"

#define W5500_SW_SPI 0
#define W5500_HW_SPI 1
#define W5500_HW_SPI2 2

#ifndef W5500_INTERFACE
#define W5500_INTERFACE W5500_HW_SPI
#endif

#ifndef W5500_SPI_FREQ
#define W5500_SPI_FREQ 1000000
#endif

#if (W5500_INTERFACE == W5500_SW_SPI)
#ifndef W5500_SPI_CLK
#define W5500_SPI_CLK DOUT30
#endif
#ifndef W5500_SPI_SDO
#define W5500_SPI_SDO DOUT29
#endif
#ifndef W5500_SPI_SDI
#define W5500_SPI_SDI DIN29
#endif
#ifndef W5500_SPI_CS
#define W5500_SPI_CS SPI_CS
#endif
SOFTSPI(w5500_spi, W5500_SPI_FREQ, 0, W5500_SPI_SDO, W5500_SPI_SDI, W5500_SPI_CLK);
#elif (W5500_INTERFACE == W5500_HW_SPI)
#ifndef W5500_SPI_CS
#define W5500_SPI_CS SPI_CS
#endif
#ifndef W5500_SPI_DMA
#define W5500_SPI_DMA true
#endif
HARDSPI(w5500_spi, W5500_SPI_FREQ, 0, mcu_spi_port);
#elif (W5500_INTERFACE == W5500_HW_SPI2)
#ifndef W5500_SPI_CS
#define W5500_SPI_CS SPI2_CS
#endif
#ifndef W5500_SPI_DMA
#define W5500_SPI_DMA true
#endif
HARDSPI(w5500_spi, W5500_SPI_FREQ, 0, mcu_spi2_port);
#endif

#define W5500_SPI_PORT (&w5500_spi)

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
#endif

/**
 * @brief	Task extensions depend on the ENABLE_MAIN_LOOP_MODULES option
 * 			Check if this option is defined or not
 */
#ifdef ENABLE_MAIN_LOOP_MODULES

/**
 * @brief	Check if your current module is up to date with the current core version of module
 */
#if (UCNC_MODULE_VERSION < 11680 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

/**
 * @brief Create a function execute your custom task. All event functions are declared as uint8_t <function>(void* args, bool* handle)
 *
 * @param args		is a pointer to a set of arguments to be passed to the event handler. In the case of the cnc_dotasks event it's a NULL pointer.
 * @return bool 	a boolean that tells the handler if the event should continue to propagate through additional listeners or is handled by the current listener an should stop propagation
 */

#define W5500_UNINITIALIZED 0
#define W5500_INITIALIZED 1
#define W5500_REGISTERED 2
static uint8_t w5500_state = W5500_UNINITIALIZED;

bool w5500_dotasks(void *args)
{

	uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
	uint8_t tmp = 0;

#ifndef USE_STATIC_IP
	wiz_NetInfo w5500_info = {.mac = {0x00, 0x08, 0xDC, 0x44, 0x55, 0x66}, .dhcp = 2};
#else
	ipv4_address_t ip = {STATIC_IP_IP};
	ipv4_address_t sn = {STATIC_IP_SUB};
	ipv4_address_t gw = {STATIC_IP_GW};
	wiz_NetInfo w5500_info = {.mac = {0x00, 0x08, 0xDC, 0x44, 0x55, 0x66}, .dhcp = 1};
	memcpy(&w5500_info.ip, &ip.ip, sizeof(ipv4_address_t));
	memcpy(&w5500_info.sn, &sn.ip, sizeof(ipv4_address_t));
	memcpy(&w5500_info.gw, &gw.ip, sizeof(ipv4_address_t));
#endif

	switch (w5500_state)
	{
	case W5500_UNINITIALIZED:
		if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
		{
			w5500_state = W5500_INITIALIZED;
		}
		__FALL_THROUGH__;
	case W5500_INITIALIZED:
		ctlwizchip(CW_GET_PHYLINK, (void *)&tmp);
		if (tmp == PHY_LINK_OFF)
		{
			break;
		}
		ctlnetwork(CN_SET_NETINFO, (void *)&w5500_info);
		socket_register_device(&wiznet5500_socket_device);
		w5500_state = W5500_REGISTERED;
		__FALL_THROUGH__;
	case W5500_REGISTERED:
		ctlwizchip(CW_GET_PHYLINK, (void *)&tmp);
		if (tmp == PHY_LINK_OFF)
		{
			w5500_state = W5500_INITIALIZED;
		}
		break;
	}

	// you must return EVENT_CONTINUE to enable other tasks to run or return EVENT_HANDLED to terminate the event handling within this callback
	return EVENT_CONTINUE;
}

/**
 * @brief 	Create an event listener object an attach our custom code parser handler.
 * 			in this case we are adding a listener to the 'cnc_dotasks' EVENT
 *
 */
CREATE_EVENT_LISTENER(cnc_dotasks, w5500_dotasks);

#endif

bool w5500_custom_grbl_cmd(void *args)
{
	grbl_cmd_args_t *cmd_params = (grbl_cmd_args_t *)args;
	char arg[64];
	// uint8_t has_arg = (cmd_params->next_char == '=');
	memset(arg, 0, sizeof(arg));

	if (!strcmp((const char *)&(cmd_params->cmd)[4], "IP"))
	{
		if (w5500_state > W5500_INITIALIZED)
		{
			wiz_NetInfo w5500_info;
			ctlnetwork(CN_GET_NETINFO, (void *)&w5500_info);
			proto_info("IP>%hu.%hu.%hu.%hu", w5500_info.ip[0], w5500_info.ip[1], w5500_info.ip[2], w5500_info.ip[3]);
		}
		else
		{
			proto_info("W5500 is off");
		}

		*(cmd_params->error) = STATUS_OK;
		return EVENT_HANDLED;
	}
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(grbl_cmd, w5500_custom_grbl_cmd);

void w5500_sel(void)
{
	io_clear_output(W5500_SPI_CS);
}

void w5500_desel(void)
{
	io_clear_output(W5500_SPI_CS);
}

uint8_t w5500_read(void)
{
	return softspi_xmit(W5500_SPI_PORT, 0xFF);
}

void w5500_write(uint8_t wb)
{
	softspi_xmit(W5500_SPI_PORT, wb);
}

void w5500_recv(uint8_t *pBuf, uint16_t len)
{
	softspi_start(W5500_SPI_PORT);
	softspi_bulk_xmit(W5500_SPI_PORT, NULL, pBuf, len);
	softspi_stop(W5500_SPI_PORT);
}

void w5500_send(uint8_t *pBuf, uint16_t len)
{
	softspi_start(W5500_SPI_PORT);
	softspi_bulk_xmit(W5500_SPI_PORT, pBuf, NULL, len);
	softspi_stop(W5500_SPI_PORT);
}

// disable MCU network interface if available
void mcu_network_init()
{
}

/**
 * Wiznet SPI interface functions callbacks
 */

/**
 * @brief 	Declarates a new module and adds the event listeners.
 * 			Again this should check the if the appropriate module option is enabled
 * 			To add this module you just neet to call LOAD_MODULE(mycustom_task_module); from inside the core code
 */
DECL_MODULE(w5500)
{
	w5500_desel();
	spi_config_t conf = {0};
	softspi_config(W5500_SPI_PORT, conf, W5500_SPI_FREQ);
	reg_wizchip_cs_cbfunc(w5500_sel, w5500_desel);
	reg_wizchip_spi_cbfunc(w5500_read, w5500_write);
	reg_wizchip_spiburst_cbfunc(w5500_recv, w5500_send);
#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
	ADD_EVENT_LISTENER(grbl_cmd, w5500_custom_grbl_cmd);
#endif
#ifdef ENABLE_MAIN_LOOP_MODULES
	// Makes the event handler 'mycustom_task' listen to the event 'cnc_dotasks'
	ADD_EVENT_LISTENER(cnc_dotasks, w5500_dotasks);
#else
// just a warning in case you disabled the MAIN_LOOP option on build
#warning "Main loop extensions are not enabled. Your module will not work."
#endif
}
