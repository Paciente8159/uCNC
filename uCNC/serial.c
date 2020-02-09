/*
	Name: serial.c
	Description: Serial communication basic read/write functions µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 30/12/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "grbl_interface.h"
#include "settings.h"
#include "mcudefs.h"
#include "mcu.h"
#include "io_control.h"
#include "cnc.h"
#include "serial.h"
#include "utils.h"

#include <math.h>

static unsigned char serial_rx_buffer[RX_BUFFER_SIZE];
volatile static uint8_t serial_rx_count;
static uint8_t serial_rx_read;
volatile static uint8_t serial_rx_write;

static unsigned char serial_tx_buffer[TX_BUFFER_SIZE];
volatile static uint8_t serial_tx_read;
static uint8_t serial_tx_write;
volatile static uint8_t serial_tx_count;

static uint8_t serial_read_select;
static uint16_t serial_read_index;

//static void serial_rx_clear();

void serial_init()
{
#ifdef FORCE_GLOBALS_TO_0
    serial_rx_write = 0;
    serial_rx_read = 0;
    serial_rx_count = 0;

    serial_tx_read = 0;
    serial_tx_write = 0;
    serial_tx_count = 0;

    //resets buffers
    memset(&serial_rx_buffer, 0, sizeof(serial_rx_buffer));
    memset(&serial_tx_buffer, 0, sizeof(serial_tx_buffer));
#endif
}

bool serial_rx_is_empty()
{
    switch(serial_read_select)
    {
        case SERIAL_UART:
            return (!serial_rx_count);
        case SERIAL_N0:
        case SERIAL_N1:
            return false;
    }

    return true;
}

bool serial_tx_is_empty()
{
    return (!serial_tx_count);
}

unsigned char serial_getc()
{
    unsigned char c;

    switch(serial_read_select)
    {
        case SERIAL_UART:
            if(!serial_rx_count)
            {
                return EOL;
            }

            do
            {
                c = serial_rx_buffer[serial_rx_read];
                if(++serial_rx_read == RX_BUFFER_SIZE)
                {
                    serial_rx_read = 0;
                }
                switch(c)
                {
                    case EOL: //EOL
                        serial_rx_count--;
                        return EOL;
                    case ' ':
                    case '\t'://eats white chars
                        break;
                    default:
                        if(c >= 'a' && c <= 'z') //serial only returns upper case letters
                        {
                            c -= 32;
                        }
                        #ifdef ECHO_CMD
                        serial_putc(c);
                        #endif
                        return c;
                }
            } while (serial_rx_count);
            break;
        case SERIAL_N0:
        case SERIAL_N1:
            c = mcu_eeprom_getc(serial_read_index++);
            if(c)
            {
                serial_putc(c);
            }
            else
            {
                serial_putc(':');
                serial_read_select = SERIAL_UART; // resets the serial select
            }
            return c;
    }

    return EOL;
}

void serial_ungetc()
{
	if(--serial_rx_read==0xFF)
    {
        serial_rx_read = RX_BUFFER_SIZE - 1;
    }
    
    if(serial_rx_buffer[serial_rx_read] == EOL)
    {
    	serial_rx_count++; //recoverd command
	}
}

void serial_select(uint8_t source)
{
    serial_read_select = source;
    switch(serial_read_select)
    {
        case SERIAL_N0:
            serial_putc('>');
            serial_read_index = STARTUP_BLOCK0_ADDRESS_OFFSET;
            break;
        case SERIAL_N1:
            serial_putc('>');
            serial_read_index = STARTUP_BLOCK1_ADDRESS_OFFSET;
                break;
    }
}

unsigned char serial_peek()
{
    unsigned char c;
    switch(serial_read_select)
    {
        case SERIAL_UART:
            while (serial_rx_count)
            {
                c = serial_rx_buffer[serial_rx_read];
                switch(c)
                {
                    case ' ':
                    case '\t'://eats white chars
                        if(++serial_rx_read == RX_BUFFER_SIZE)
                        {
                            serial_rx_read = 0;
                        }
                        break;
                    default:
                        if(c >= 'a' && c <= 'z') //serial only returns upper case letters
                        {
                            c -= 32;
                        }
                        return c;
                }
            }
            break;
        case SERIAL_N0:
        case SERIAL_N1:
            return mcu_eeprom_getc(serial_read_index);
    }
    return EOL;
}

void serial_inject_cmd(const unsigned char* __s)
{
    unsigned char c = rom_strptr(__s++);
    do
    {
        serial_rx_isr(c);
        c = rom_strptr(__s++);
    }
    while(c != 0);
}

void serial_putc(unsigned char c)
{
    while((serial_tx_write == serial_tx_read) && (serial_tx_count != 0))
    {
        mcu_start_send(); //starts async send and loops while buffer full
        if(!cnc_doevents()) //on any alarm abort
        {
            return;
        }
    } //while buffer is full

    serial_tx_buffer[serial_tx_write] = c;
    serial_tx_write++;
    if(c == '\n' || c == '\r')
    {
        serial_tx_count++;
        mcu_start_send();
    }

    if(serial_tx_write == TX_BUFFER_SIZE)
    {
        serial_tx_write = 0;
    }
}

void serial_print_str(const unsigned char* __s)
{
    unsigned char c = rom_strptr(__s++);
    do
    {
        serial_putc(c);
        c = rom_strptr(__s++);
    }
    while(c != 0);
}

void serial_print_int(uint16_t num)
{
    if (num == 0)
    {
        serial_putc('0');
        return;
    }

    unsigned char buffer[6];
    uint8_t i = 0;

    while (num > 0)
    {
        uint8_t digit = num % 10;
        num = ((((uint32_t)num * (UINT16_MAX/10))>>16) + ((digit!=0) ? 0 : 1)); //same has divide by 10 but faster
        buffer[i++] = digit;
        /*buffer[i++] = num % 10;
        num /= 10;*/
    }

    do
    {
        i--;
        serial_putc('0' + buffer[i]);
    }
    while(i);
}
#ifdef GCODE_PROCESS_LINE_NUMBERS
void serial_print_long(uint32_t num)
{
    if (num == 0)
    {
        serial_putc('0');
        return;
    }

    unsigned char buffer[11];
    uint8_t i = 0;

    while (num > 0)
    {
        uint8_t digit = num % 10;
        num = (uint32_t)truncf((float)num * 0.1f);
        buffer[i++] = digit;
        /*buffer[i++] = num % 10;
        num /= 10;*/
    }

    do
    {
        i--;
        serial_putc('0' + buffer[i]);
    }
    while(i);
}
#endif
/*
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

}*/

void serial_print_flt(float num)
{
    if(g_settings.report_inches)
    {
        num *= MM_INCH_MULT;
    }

    if(num < 0)
    {
        serial_putc('-');
        num = -num;
    }

    uint16_t digits = (uint16_t)floorf(num);
    serial_print_int(digits);
    serial_putc('.');
    num -= digits;

    num *= 1000;
    digits = (uint16_t)roundf(num);

    if(g_settings.report_inches)
    {
        if(digits<10000)
        {
            serial_putc('0');
        }

        if(digits<1000)
        {
            serial_putc('0');
        }
    }

    if(digits<100)
    {
        serial_putc('0');
    }

    if(digits<10)
    {
        serial_putc('0');
    }

    serial_print_int(digits);
}

void serial_print_intarr(uint16_t* arr, uint8_t count)
{
    do
    {
        serial_print_int(*arr++);
        count--;
        if(count)
        {
            serial_putc(',');
        }

    }
    while(count);
}

void serial_print_fltarr(float* arr, uint8_t count)
{
    do
    {
        serial_print_flt(*arr++);
        count--;
        if(count)
        {
            serial_putc(',');
        }

    }
    while(count);

}

void serial_flush()
{
    while(serial_tx_count)
    {
        mcu_start_send();
        if(!cnc_doevents())
        {
            return;
        }
    }
}

//ISR
//New char handle strategy
//All ascii will be sent to buffer and processed later (including comments)
void serial_rx_isr(unsigned char c)
{
    uint8_t write;
    if(c < ((unsigned char)'~')) //ascii (except CMD_CODE_CYCLE_START and DEL)
    {
        switch(c)
        {
            case CMD_CODE_RESET:
                serial_rx_clear(); //dumps all unexecuted commands
            case CMD_CODE_FEED_HOLD:
            case CMD_CODE_REPORT:
                cnc_call_rt_command((uint8_t)c);
                return;
            case '\r':
            case '\n':
                c = EOL;//replaces CR and LF with EOL and continues
			case EOL:
                serial_rx_count++; //continues
            default:
                write = serial_rx_write;
                serial_rx_buffer[write] = c;
			    if(++write == RX_BUFFER_SIZE)
			    {
			        write = 0;
			    }
                //writes the overflow char ahead
                serial_rx_buffer[write] = OVF;
                serial_rx_write = write;
                break;
        }
    }
    else //extended ascii (plus CMD_CODE_CYCLE_START and DEL)
    {
        cnc_call_rt_command((uint8_t)c);
    }
}

void serial_tx_isr()
{
    if(!serial_tx_count)
    {
        return;
    }
    uint8_t read = serial_tx_read;
    unsigned char c = serial_tx_buffer[read];
    COM_OUTREG = c;
    if(c == '\n' || c == '\r')
    {
        if(!--serial_tx_count)
        {
            mcu_stop_send();
        }
    }
    if(++read == TX_BUFFER_SIZE)
    {
        read = 0;
    }
    serial_tx_read = read;
}

void serial_rx_clear()
{
    serial_rx_write = 0;
    serial_rx_read = 0;
    serial_rx_count = 0;
    serial_rx_buffer[0] = EOL;
    /*serial_tx_write = 0;
    serial_tx_read = 0;
    serial_tx_count = 0;*/
}
