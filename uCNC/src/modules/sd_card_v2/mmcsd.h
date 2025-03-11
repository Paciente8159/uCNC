/*
	Name: mmcsd.h
	Description: SD card module for µCNC.
	This adds SD card support via SPI

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 08-09-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MMCSD_H
#define MMCSD_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

	enum MMCSD_CARD_TYPE
	{
		NOCARD = 0,
		MMCv3 = 1,
		SDv1 = 2,
		SDv2 = 4,
	};

	typedef struct
	{
		uint8_t detected : 1;
		uint8_t initialized : 1;
		uint8_t writeprotected : 1;
		uint8_t is_highdensity : 1;
		uint8_t card_type : 3;
		uint64_t size;
		uint32_t sectors;
	} mmcsd_card_t;

	enum MMCSD_RESP1_BIT
	{
		R1_IDLE = 0,
		R1_ERASE_RESET = 1,
		R1_ILLEGAL_CMD = 2,
		R1_CRC_ERROR = 3,
		R1_ERASE_SEQ_ERROR = 4,
		R1_ADDRESS_ERROR = 5,
		R1_PARAM_ERROR = 6,
	};

	#define R1(x) (1<<x)

#ifdef __cplusplus
}
#endif

#endif