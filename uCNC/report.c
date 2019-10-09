#include "config.h"
#include "board.h"
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

