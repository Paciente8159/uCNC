#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "structures.h"
#include <stdlib.h>
#include <stdint.h>

void protocol_sync();
void protocol_send_packet();
uint8_t protocol_get_packet(CMD_PACKET* packet);

#endif
