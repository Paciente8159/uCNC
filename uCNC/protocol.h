/*
	Name: Protocol.h
	Copyright: 
	Author: João Martins
	Date: 19/11/19 17:31
	Description: Implements a compatible command-response grbl protocol
*/


#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>

void protocol_init();
bool protocol_hasCommand();
char protocol_getChar();
void protocol_send(const char* __s);
void protocol_sendFormat(const char* __fmt, ...);

#endif
