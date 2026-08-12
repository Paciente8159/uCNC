/*
 * C99 WIZnet W5100/W5200/W5500 register and Ethernet implementation.
 * Derived from the register protocol and socket-buffer handling used by the
 * Arduino Ethernet library, without Arduino or C++ dependencies.
 */

#ifndef WIZNET_ETHERNET_H
#define WIZNET_ETHERNET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../../cnc.h"
#include "../softspi.h"
#include "../net/utils/socket_device.h"

#ifndef WIZDGB
#define WIZDGB(...) ((void)0)
#endif

#ifndef WIZNET_STARTUP_DELAY_MS
#define WIZNET_STARTUP_DELAY_MS 560U
#endif

#ifndef WIZNET_COMMAND_POLL_LIMIT
#define WIZNET_COMMAND_POLL_LIMIT 100U
#endif

#ifndef WIZNET_STABLE_READ_LIMIT
#define WIZNET_STABLE_READ_LIMIT 4U
#endif

#define WIZNET_MAX_HW_SOCKETS 8U
#define WIZNET_SOCKET_BUFFER_SIZE 2048U

#define WIZNET_SW_SPI 0
#define WIZNET_HW_SPI 1
#define WIZNET_HW_SPI2 2

void wiznet_set_mac(const uint8_t *mac);
/* This implementation uses static IPv4 configuration; it does not contain a
 * DHCP client. Call wiznet_config() before wiznet_init(). */
void wiznet_config(ipv4_address_t ip, ipv4_address_t sn, ipv4_address_t gw);
ipv4_address_t wiznet_get_ip(void);
void wiznet_init(softspi_port_t * spiport);

/* No wiznet_dotasks() call is required. socket_server_dotasks() invokes the
 * registered backend's bounded service callback. */

bool wiznet_is_ready(void);

/* Internal hardware API shared with wiznet_lwip.c. */
uint8_t wiznet_hw_socket_count(void);
uint8_t wiznet_hw_socket_status(uint8_t socket_number);
uint8_t wiznet_hw_socket_interrupt(uint8_t socket_number);
void wiznet_hw_socket_clear_interrupt(uint8_t socket_number, uint8_t mask);
bool wiznet_hw_socket_open_tcp_server(uint8_t socket_number, uint16_t port);
void wiznet_hw_socket_close(uint8_t socket_number);
int wiznet_hw_socket_send(uint8_t socket_number, const uint8_t *data,
                          size_t length);
int wiznet_hw_socket_receive(uint8_t socket_number, uint8_t *data,
                             size_t capacity);

bool wiznet_socket_backend_register(void);

#ifdef __cplusplus
}
#endif

#endif
