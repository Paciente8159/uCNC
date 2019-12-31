/*
	Name: protocol.h - implementation of a grbl compatible send-response protocol
	Copyright: 2019 João Martins
	Author: João Martins
	Date: Nov/2019
	Description: uCNC is a free cnc controller software designed to be flexible and
	portable to several	microcontrollers/architectures.
	uCNC is a FREE SOFTWARE under the terms of the GPLv3 (see <http://www.gnu.org/licenses/>).
*/

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

void protocol_init();
bool protocol_received_cmd();
bool protocol_sent_resp();
char protocol_getc();
char protocol_peek();
//char* protocol_get_bufferptr();
void protocol_clear();
void protocol_appendf(const char* __fmt, ...);
void protocol_append(const char* __s);
void protocol_puts(const char* __s);
void protocol_printf(const char* __fmt, ...);
void protocol_read_char_isr(uint8_t c);
#ifdef __DEBUG__
void protocol_inject_cmd(const char* __fmt, ...);
#endif

#endif
