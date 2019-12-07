/*
	Name: protocol.c - implementation of a grbl compatible send-response protocol
	Copyright: 2019 João Martins
	Author: João Martins
	Date: Nov/2019
	Description: uCNC is a free cnc controller software designed to be flexible and
	portable to several	microcontrollers/architectures.
	uCNC is a FREE SOFTWARE under the terms of the GPLv3 (see <http://www.gnu.org/licenses/>).
*/

#include "config.h"
#include "protocol.h"
#include "mcu.h"
#include "cnc.h"
#include "ringbuffer.h"

#define CMD_BUFFER_SIZE 128
#define RESP_BUFFER_SIZE 64
static char protocol_cmd_buffer[CMD_BUFFER_SIZE];
static char protocol_resp_buffer[RESP_BUFFER_SIZE];
static bool protocol_send_busy;
static bool protocol_cmd_available;
static uint8_t protocol_cmd_count;
volatile static uint8_t read_index;
volatile static uint8_t write_index;

void protocol_read_char_isr(volatile char c)
{
	switch(c)
	{
		case 0x18:
			//imediatly calls killing process
			g_cnc_state.halt = true; //flags all isr to stop
			g_cnc_state.rt_cmd |= RT_CMD_RESET;
			break;
		case '?':
			g_cnc_state.rt_cmd |= RT_CMD_REPORT;
			break;
		case '~':
			//cycle start
			g_cnc_state.rt_cmd |= RT_CMD_CYCLE_START;
			break;
		case '!':
			//feed hold
			g_cnc_state.rt_cmd |= RT_CMD_FEED_HOLD;
			break;
		default:
			protocol_cmd_buffer[write_index] = c;
			write_index++;
			if(c == '\r')
			{
				protocol_cmd_buffer[write_index] = '\0'; //ensures end of command marker
				protocol_cmd_available = true; //flags command available
				write_index = 0; //resets index
				read_index = 0; //resets read index
			}
			break;
	}
	
}


void protocol_write_char_isr()
{
	static uint8_t index = 1;
	char c = protocol_resp_buffer[index++];
	
	if(c == '\0') //reached end of response
	{
		protocol_send_busy = false; //flags can send new response
		index = 1; //resets index
		return;
	}

	if(c == '\n')
	{
		protocol_resp_buffer[index] = '\0'; //ensures end of response marker
	}
	
	mcu_putc(c);
}

void protocol_init()
{
	read_index = 0;
	write_index = 0;
	protocol_cmd_count = 0;
	protocol_cmd_available = false;
	protocol_send_busy = false;
	
	//resets buffers
	memset(&protocol_cmd_buffer, 0, sizeof(protocol_cmd_buffer));
	memset(&protocol_resp_buffer, 0, sizeof(protocol_resp_buffer));
	
	//attaches ISR to functions
	mcu_attachOnReadChar(protocol_read_char_isr);
	mcu_attachOnSentChar(protocol_write_char_isr);
}

bool protocol_received_cmd()
{
	return protocol_cmd_available;
}

void protocol_clear()
{
	protocol_cmd_available = false; //flags command available
	write_index = 0; //resets index
	read_index = 0; //resets read index
}

char protocol_getc()
{
	char c = '\0';
	
	if(!protocol_cmd_available)
	{
		return c;
	}
	
	c = protocol_cmd_buffer[read_index];
	read_index++;
	if(c == '\r' || c == '\n') //end of buffer allow incomming 
	{
		protocol_cmd_available = false;
		read_index = 0;
	}
	
	return c;
}

char protocol_peek()
{
	char c = '\0';
	
	if(!protocol_cmd_available)
	{
		return c;
	}
	
	c = protocol_cmd_buffer[read_index];
	return c;
}

void protocol_puts(const char* __s)
{
	while(protocol_send_busy);
	
	protocol_send_busy  = true;
	char *s = rom_strncpy((char*)&protocol_resp_buffer, __s, RESP_BUFFER_SIZE);
	//send first char
	//the rest will be sent async

	mcu_putc(protocol_resp_buffer[0]);
}

void protocol_printf(const char* __fmt, ...)
{
	while(protocol_send_busy);
	
	protocol_send_busy  = true;
	//writes the formated progmem string to RAM and then print it to the buffer with the parameters
	char buffer[RESP_BUFFER_SIZE];
	char* newfmt = rom_strncpy((char*)&buffer, __fmt, RESP_BUFFER_SIZE);
	va_list __ap;
 	va_start(__ap,__fmt);
 	vsprintf((char*)&protocol_resp_buffer, newfmt,__ap);
 	va_end(__ap);
 	//send first char
	//the rest will be sent async
 	mcu_putc(protocol_resp_buffer[0]);
}

#ifdef __DEBUG__
void protocol_inject_cmd(const char* __fmt, ...)
{
	char buffer[CMD_BUFFER_SIZE];
	char* newfmt = rom_strncpy((char*)&buffer, __fmt, CMD_BUFFER_SIZE);
	va_list __ap;
 	va_start(__ap,__fmt);
 	vsprintf((char*)&protocol_cmd_buffer, newfmt,__ap);
 	va_end(__ap);
 	//flag cmd recieved
	protocol_cmd_available = true; //flags command available
	read_index = 0; //resets read index
}
#endif

