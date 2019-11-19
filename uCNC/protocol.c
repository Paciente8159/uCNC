#include "config.h"
#include "protocol.h"
#include "mcu.h"

#define CMD_BUFFER_SIZE 128
#define RESP_BUFFER_SIZE 64
char protocol_cmd_buffer[CMD_BUFFER_SIZE];
char protocol_resp_buffer[RESP_BUFFER_SIZE];
bool protocol_send_busy;
bool protocol_cmd_available;
uint8_t protocol_cmd_count;

void protocol_read_char(volatile char c)
{
	static uint8_t index = 0;
	protocol_cmd_buffer[index] = c;
	index++;
	if(c == '\r')
	{
		protocol_cmd_available = true;
		index = 0;
	}
}


void protocol_write_char()
{
	static uint8_t index = 1;
	char c = protocol_resp_buffer[index++];
	mcu_putc(c);
	if(c == '\r')
	{
		mcu_putc('\n');
		protocol_send_busy = false;
		index = 1;
		mcu_detachOnSentChar();
	}
}

void protocol_init()
{
	protocol_cmd_count = 0;
	protocol_cmd_available = false;
	protocol_send_busy = false;
	
	//resets buffers
	memset(&protocol_cmd_buffer, 0, sizeof(protocol_cmd_buffer));
	memset(&protocol_resp_buffer, 0, sizeof(protocol_resp_buffer));
	
	//attaches ISR to functions
	mcu_attachOnReadChar(protocol_read_char);
}

bool protocol_hasCommand()
{
	return protocol_cmd_available;
}

char protocol_getChar()
{
	static uint8_t index = 0;
	char c = '\0';
	
	if(!protocol_cmd_available)
	{
		return c;
	}
	
	c = protocol_cmd_buffer[index];
	index++;
	if(c == '\r') //end of buffer allow incomming 
	{
		protocol_cmd_available = false;
		index = 0;
	}
	
	return c;
}

void protocol_send(const char* __s)
{
	while(protocol_send_busy);
	
	protocol_send_busy  = true;
	char *s = mcu_strcpyProgMem((char*)&protocol_resp_buffer, __s);
	//send first char
	//the rest will be sent async
	mcu_attachOnSentChar(protocol_write_char);
	mcu_putc(protocol_resp_buffer[0]);
}

void protocol_sendFormat(const char* __fmt, ...)
{
	while(protocol_send_busy);
	
	protocol_send_busy  = true;
	//writes the formated progmem string to RAM and then print it to the buffer with the parameters
	char buffer[RESP_BUFFER_SIZE];
	char* newfmt = mcu_strcpyProgMem((char*)&buffer, __fmt);
	va_list __ap;
 	va_start(__ap,__fmt);
 	vsprintf((char*)&protocol_resp_buffer, newfmt,__ap);
 	va_end(__ap);
 	//send first char
	//the rest will be sent async
	mcu_attachOnSentChar(protocol_write_char);
 	mcu_putc(protocol_resp_buffer[0]);
}

