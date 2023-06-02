#include "cnc.h"

#ifdef CRC_WITHOUT_LOOKUP_TABLE
uint8_t crc7(uint8_t c, uint8_t crc)
{
	uint8_t i = 8;
	crc ^= c;
	for (;;)
	{
		if (crc & 0x80)
		{
			crc ^= 0x89;
		}
		if (!--i)
		{
			return crc;
		}
		crc <<= 1;
	}
}
#endif