/*
	Name: telnet.h
	Description: Implements a simple Telnet Server based on BSD/POSIX Sockets for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-08-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef TELNET_H
#define TELNET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../module.h"
#include "socket.h"
#include <stdlib.h>
#include <stdint.h>

	DECL_MODULE(telnet_server);

#ifdef __cplusplus
}
#endif

#endif
