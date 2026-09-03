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

/* Internal bounded-retry result for an incoherent volatile 16-bit register
 * sample.  It is not a transport failure and must be retried later. */
#define WIZNET_HW_RETRY (-2)

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
/*
 * Poll completion of the one outstanding SEND command without waiting.
 * Returns 1 when SEND_OK is consumed and another SEND may be issued, 0 while
 * still pending, WIZNET_HW_RETRY for a transient unstable register snapshot,
 * or -1 for a fatal native command error.  On W5200 this also applies the
 * vendor ioLibrary SEND_OK/TX-buffer workaround by reissuing SEND when the
 * hardware has not advanced the complete queued range.
 */
int wiznet_hw_socket_send_progress(uint8_t socket_number);
/* Stable snapshots of the hardware ring counters.  A negative result means
 * the multi-byte register did not stabilize during this bounded attempt; the
 * caller must defer and retry rather than treating it as zero. */
int wiznet_hw_socket_tx_free(uint8_t socket_number);
int wiznet_hw_socket_rx_available(uint8_t socket_number);
/* Read RX bytes without advancing Sn_RX_RD. The caller must commit exactly the
 * number of bytes it has successfully delivered. */
int wiznet_hw_socket_receive_peek(uint8_t socket_number, uint8_t *data,
                                  size_t capacity);
int wiznet_hw_socket_receive_commit(uint8_t socket_number, size_t length);
/* Compatibility helper: peek and immediately commit the returned bytes. */
int wiznet_hw_socket_receive(uint8_t socket_number, uint8_t *data,
                             size_t capacity);

bool wiznet_socket_backend_register(void);

#ifdef __cplusplus
}
#endif

#endif
