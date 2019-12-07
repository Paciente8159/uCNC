/*
	Name: report.c - implementation of a grbl compatible send-response protocol
	Copyright: 2019 João Martins
	Author: João Martins
	Date: Nov/2019
	Description: uCNC is a free cnc controller software designed to be flexible and
	portable to several	microcontrollers/architectures.
	uCNC is a FREE SOFTWARE under the terms of the GPLv3 (see <http://www.gnu.org/licenses/>).
*/

#include "config.h"
#include "mcu.h"
#include "report.h"
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

char g_report_buffer[REPORT_BUFFER_SIZE];

void report_error(uint8_t code)
{
    
}

void report_msg(const char* s)
{
    printf(s);
}

void report_formatmsg(const char* format,...)
{
	char *buffer = (char*)&g_report_buffer;
	va_list argList;
    va_start(argList, format);
    memset(&g_report_buffer,0,REPORT_BUFFER_SIZE*sizeof(char));
    vsprintf(buffer, format, argList);
    va_end(argList);
    printf(buffer);
}

