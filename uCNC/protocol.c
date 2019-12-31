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
#include "trigger_control.h"
#include "cnc.h"
#include "ringbuffer.h"

#define CMD_BUFFER_SIZE 128
#define RESP_BUFFER_SIZE 128
static char protocol_cmd_buffer[CMD_BUFFER_SIZE];
static char protocol_resp_buffer[RESP_BUFFER_SIZE];
//static bool protocol_send_busy;
static bool protocol_cmd_available;
static uint8_t protocol_cmd_count;
volatile static uint8_t read_index;
volatile static uint8_t write_index;
static uint8_t protocol_append_index;

void protocol_read_char_isr(uint8_t c)
{
	#ifdef MCU_VIRTUAL
	static uint8_t limit;
	#endif
	switch(c)
	{
		//simulates limits
		#ifdef MCU_VIRTUAL
		case '/':
			limit^=LIMIT_X_MASK;
			tc_limits_isr(limit);
			break;
		#endif
		#ifdef MCU_VIRTUAL
		case '*':
			limit^=LIMIT_Y_MASK;
			tc_limits_isr(limit);
			break;
		#endif
		#ifdef MCU_VIRTUAL
		case 174:
			limit^=LIMIT_Z_MASK;
			tc_limits_isr(limit);
			break;
		#endif
		
		#ifdef MCU_VIRTUAL
		case '\\':
		#endif
		case 0x18:
			cnc_exec_rt_command(RT_CMD_RESET);
			break;
		#ifdef MCU_VIRTUAL
		case '<':
		#endif
		case 0x84:
			cnc_exec_rt_command(RT_CMD_SAFETY_DOOR);
			break;
		case 0x85:
			cnc_exec_rt_command(RT_CMD_JOG_CANCEL);
			break;
		case '?':
			cnc_exec_rt_command(RT_CMD_REPORT);
			break;
		case '~':
			//cycle start
			cnc_exec_rt_command(RT_CMD_CYCLE_START);
			break;
		case '!':
			//feed hold
			cnc_exec_rt_command(RT_CMD_FEED_HOLD);
			break;
		case ' ':
		case '\t':
		case '\v':
			//eats white chars
			break;
		case '\r':
		case '\n':
			if(!protocol_cmd_available) //blocks all chars after cmd is made available
			{
				protocol_cmd_buffer[write_index] = '\0'; //ensures end of command marker
				protocol_cmd_available = true; //flags command available
			}
			break;
		default:
			if(!protocol_cmd_available && c > 22 && c < 126)
			{
				protocol_cmd_buffer[write_index++] = c;
			}
			//protocol_cmd_buffer[write_index++] = c;
			break;
	}
	
}

void protocol_init()
{
	read_index = 0;
	write_index = 0;
	protocol_cmd_count = 0;
	protocol_cmd_available = false;
	//protocol_send_busy = false;
	
	//resets buffers
	memset(&protocol_cmd_buffer, 0, sizeof(protocol_cmd_buffer));
	memset(&protocol_resp_buffer, 0, sizeof(protocol_resp_buffer));
}

bool protocol_received_cmd()
{
	return protocol_cmd_available;
}

bool protocol_sent_resp()
{
	return mcu_is_txready();
}

void protocol_clear()
{
	write_index = 0; //resets index
	read_index = 0; //resets read index
	protocol_cmd_available = false; //flags command available
}

char protocol_getc()
{
	char c = '\0';
	
	if(!protocol_cmd_available)
	{
		return c;
	}
	
	c = protocol_cmd_buffer[read_index++];
	if(c == '\0') //EOL marker (discard rest of buffer)
	{
		write_index = 0;
		read_index = 0;
		protocol_cmd_available = false;
	}
	
	return c;
}

/*char* protocol_get_bufferptr()
{
	return &protocol_cmd_buffer[read_index];
}*/

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

void protocol_appendf(const char* __fmt, ...)
{
	while(!mcu_is_txready());
	
	//writes the formated progmem string to RAM and then print it to the buffer with the parameters
	char buffer[RESP_BUFFER_SIZE];
	char* newfmt = rom_strncpy((char*)&buffer, __fmt, RESP_BUFFER_SIZE);
	va_list __ap;
 	va_start(__ap,__fmt);
 	vsprintf((char*)&protocol_resp_buffer[protocol_append_index], newfmt,__ap);
 	va_end(__ap);
	protocol_append_index = strlen((const char*)&protocol_resp_buffer);
}

void protocol_append(const char* __s)
{
	while(!mcu_is_txready());
	
	char *s = rom_strncpy((char*)&protocol_resp_buffer[protocol_append_index], __s, RESP_BUFFER_SIZE - protocol_append_index);
	protocol_append_index = strlen((const char*)&protocol_resp_buffer);
}

void protocol_puts(const char* __s)
{
	while(!mcu_is_txready());
	
	char *s = rom_strncpy((char*)&protocol_resp_buffer[protocol_append_index], __s, RESP_BUFFER_SIZE - protocol_append_index);
	protocol_append_index = 0;
	//transmit async
	
	mcu_puts(protocol_resp_buffer);
}

void protocol_printf(const char* __fmt, ...)
{
	while(!mcu_is_txready());
	
	//writes the formated progmem string to RAM and then print it to the buffer with the parameters
	char buffer[RESP_BUFFER_SIZE];
	char* newfmt = rom_strncpy((char*)&buffer, __fmt, RESP_BUFFER_SIZE);
	va_list __ap;
 	va_start(__ap,__fmt);
 	vsprintf((char*)&protocol_resp_buffer[protocol_append_index], newfmt,__ap);
 	va_end(__ap);
	protocol_append_index = 0;
	//transmit async
 	mcu_puts(protocol_resp_buffer);
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
	write_index = 0; //resets index
	read_index = 0; //resets read index
}
#endif

