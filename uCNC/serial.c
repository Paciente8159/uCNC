/*
	Name: serial.c
	Description: Serial communication basic read/write functions uCNC.
	
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 30/12/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "grbl_interface.h"
#include "settings.h"
#include "mcudefs.h"
#include "mcu.h"
#include "dio_control.h"
#include "cnc.h"
#include "serial.h"
#include "utils.h"

#define RX_BUFFER_SIZE 128
#define TX_BUFFER_SIZE 128


static char serial_rx_buffer[RX_BUFFER_SIZE];
volatile static uint8_t serial_rx_count;
volatile static uint8_t serial_rx_read;
volatile static uint8_t serial_rx_write;

static char serial_tx_buffer[TX_BUFFER_SIZE];
volatile static uint8_t serial_tx_read;
volatile static uint8_t serial_tx_write;
volatile static uint8_t serial_tx_count;

void serial_init()
{
	serial_rx_write = 0;
	serial_rx_read = 0;
	serial_rx_count = 0;
	
	serial_tx_read = 0;
	serial_tx_write = 0;
	serial_tx_count = 0;
	
	//resets buffers
	memset(&serial_rx_buffer, 0, sizeof(serial_rx_buffer));
	memset(&serial_tx_buffer, 0, sizeof(serial_tx_buffer));
}

bool serial_rx_is_empty()
{
	return (!serial_rx_count);
}

bool serial_tx_is_empty()
{
	return (!serial_tx_count);
}

char serial_getc()
{
	#ifdef ECHO_CMD
	static bool echo = false;
	#endif
	char c = '\0';
	
	if(!serial_rx_count)
	{
		return c;
	}
	
	#ifdef ECHO_CMD
	if(!echo)
	{
		echo = true;
		serial_print_str(MSG_ECHO);
	}	
    #endif
	
	c = serial_rx_buffer[serial_rx_read];
	switch(c)
	{
		case '\n':
			serial_rx_count--;
			#ifdef ECHO_CMD
			echo = false;
		    serial_print_str(__romstr__("]\r\n"));
		    #endif
			break;
		default:
			#ifdef ECHO_CMD
			serial_putc(c);
			#endif
			break;				
	}
	
	serial_rx_read++;
	if(serial_rx_read == RX_BUFFER_SIZE)
	{
		serial_rx_read = 0;
	}
	
	return c;
}

char serial_peek()
{
	return ((serial_rx_count != 0) ? serial_rx_buffer[serial_rx_read] : 0);
}

void serial_inject_cmd(const char* __s)
{
	char c = rom_strptr(__s++);
	do{
		serial_rx_buffer[serial_rx_write] = c;
		serial_rx_write++;
		if(serial_rx_write == RX_BUFFER_SIZE)
		{
			serial_rx_write = 0;
		}
		c = rom_strptr(__s++);
	} while(c != 0);
	serial_rx_count++;
}

void serial_discard_cmd()
{
	if(serial_rx_count != 0)
	{
		while(serial_getc() != '\n');
	}
}

void serial_putc(char c)
{
	while((serial_tx_write == serial_tx_read) && (serial_tx_count != 0))
	{
		mcu_start_send();
		cnc_doevents();	
	} //while buffer is full 
	
	serial_tx_buffer[serial_tx_write] = c;
	serial_tx_write++;
	if(c == '\n')
	{
		serial_tx_count++;
		mcu_start_send();
	}

	if(serial_tx_write == TX_BUFFER_SIZE)
	{
		serial_tx_write = 0;
	}
}

void serial_print_str(const char* __s)
{
	char c = rom_strptr(__s++);
	do
	{
		serial_putc(c);
		c = rom_strptr(__s++);
	} while(c != 0);
}

void serial_print_int(uint16_t num)
{
	char buffer[6];
	sprintf(buffer, MSG_INT, num);
	
	uint8_t i = 0;
	do
	{
		serial_putc(buffer[i]);
		i++;
	} while(buffer[i] != 0);	

}

void serial_print_flt(float num)
{
	char buffer[12];
	if(!g_settings.report_inches)
	{
		sprintf(buffer, MSG_FLT, num);
	}
	else
	{
		num *= MM_INCH_MULT;
		sprintf(buffer, MSG_FLT_IMPERIAL, num);
	}
	
	
	uint8_t i = 0;
	do
	{
		serial_putc(buffer[i]);
		i++;
	} while(buffer[i] != 0);
	
}

void serial_print_intarr(uint16_t* arr, int count)
{
	do
	{
		serial_print_int(*arr++);
		count--;
		if(count)
		{
			serial_putc(',');
		}
			
	}while(count);
}

void serial_print_fltarr(float* arr, int count)
{
	do
	{
		serial_print_flt(*arr++);
		count--;
		if(count)
		{
			serial_putc(',');
		}
			
	}while(count);

}

void serial_flush()
{
	while(serial_tx_count)
	{
		mcu_start_send();
		cnc_doevents();
	}
}

//ISR

void serial_rx_isr(char c)
{
	static uint8_t comment_count = 0;
	
	if((c > 0x21) && (c < 0x7B))
	{
		switch(c)
		{
			case RT_CMD_REPORT:
				cnc_exec_rt_command(RT_CMD_REPORT);
				return;
			case '(':
				comment_count++;
				return;
			case ')':
				comment_count--;
				return;
			default:
				if(!comment_count)
				{
					serial_rx_buffer[serial_rx_write] = c;
					serial_rx_write++;
				}
				break;
		}
	}
	else
	{
		switch(c)
		{
			case '\r':
				c = '\n';//replaces CR with LF
			case '\n':
				serial_rx_buffer[serial_rx_write] = c;
				serial_rx_write++;
				serial_rx_count++;
				comment_count = 0;
				break;
			default:
				cnc_exec_rt_command((uint8_t)c);
				return;
		}
	}
	
	if(serial_rx_write == RX_BUFFER_SIZE)
	{
		serial_rx_write = 0;
	}
}

char serial_tx_isr()
{
	if(serial_tx_count == 0)
	{
		return 0;
	}
	
	char c = serial_tx_buffer[serial_tx_read];
	
	if(c == '\n')
	{
		serial_tx_count--;
	}

	if(++serial_tx_read == TX_BUFFER_SIZE)
	{
		serial_tx_read = 0;
	}
	
	return c;
}

