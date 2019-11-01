#include "config.h"
#include "protocol.h"
#include "mcu.h"

/*
	Synchronizes the packet protocol
	The synchronization fase is done in the following way
	
		1. uCNC keeps sending the 0x55 char until the host replies with 0xAA
		2. uCNC will send the number of sent sync chars (the number will always be different from 0x55 (sync char))
		3. The host must reply with the CRC7 value of that
		3. synchonization sis done and packet comunication can start
*/

void protocol_sync()
{
	uint8_t counter;
	uint8_t syncchar;
	do
	{
		syncchar= 0x55;
		while(mcu_comPeek()!=0xAA)
		{
			counter++;
			mcu_comSendPacket(&syncchar, 1);
		}
		
		//empties the buffer
		while(mcu_comPeek()>=0)
		{
			mcu_comGetPacket(&syncchar,1);
		}
		
		//sends counter
		mcu_comSendPacket(&counter, 1);
		
		//waits for reply
		while(mcu_comPeek()<0);
		mcu_comGetPacket(&syncchar, 1);
		
	} while(crc7(0,&counter, 1) != syncchar);
	
	//clears com buffer
	mcu_comClear();
}

/*
	Receives all command packets from host
	The packet is a PACKET_COMMAND as defined in the structures.h file
	After packet is received uCNC does:
		
		1. Check for crc7 errors
		2. Casts and routs the data packet to the correct parsing function
		3. Replies to the host with the crc result of the packet crc byte so the host confirms correct transmition
		
	This is non-blocking function. If no packet is available continues normal processing until it is called again.
*/

uint8_t protocol_get_packet(CMD_PACKET* packet)
{
	uint8_t result = 0;
	//if nothing available in the coms buffer exit and continue
	if(mcu_comPeek() < 0)
		return result;
		
	//if the packet was incomplete exit and continue
	if(mcu_comGetPacket((uint8_t*)packet, sizeof(CMD_PACKET))==0)
		return result;
		
	//check packet crc
	if(crc7(0, (uint8_t*)packet, sizeof(CMD_PACKET) - 1) == packet->crc)
	{
		result = crc7(0, &(packet->crc), 1);
		mcu_comSendPacket(&result, 1);
	}
	
	return result;
}
