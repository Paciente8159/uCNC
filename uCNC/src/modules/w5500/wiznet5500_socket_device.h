/*
 * Name: wiznet5500_socket_device.h
 * Description: WIZnet W5500 backend declaration for µCNC socket_device_t.
 */

#ifndef UCNC_WIZNET5500_SOCKET_DEVICE_H
#define UCNC_WIZNET5500_SOCKET_DEVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../net/utils/socket_device.h"

/*
 * ioLibrary_Driver hardware socket ownership.
 *
 * By default the backend may use all W5500 hardware sockets. If DHCP, DNS,
 * MQTT, or another ioLibrary component uses fixed socket numbers, define
 * UCNC_W5500_SOCKET_MASK so µCNC owns only a disjoint subset.
 *
 * Example: reserve sockets 6 and 7 for other code:
 *   -DUCNC_W5500_SOCKET_MASK=0x3F
 */
#ifndef UCNC_W5500_SOCKET_MASK
#define UCNC_W5500_SOCKET_MASK 0xFFUL
#endif

/*
 * Maximum bytes delivered in one socket_device_events_t::data callback.
 * Define this to SOCKET_MAX_DATA_SIZE in the project configuration if that
 * symbol is not visible when this backend is compiled.
 */
#ifndef UCNC_W5500_RX_CHUNK_SIZE
#ifdef SOCKET_MAX_DATA_SIZE
#define UCNC_W5500_RX_CHUNK_SIZE SOCKET_MAX_DATA_SIZE
#else
#define UCNC_W5500_RX_CHUNK_SIZE 1024U
#endif
#endif

extern socket_device_t wiznet5500_socket_device;

#ifdef __cplusplus
}
#endif

#endif
