/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2010, 2011, 2012 Teunis van Beelen
*
* teuniz@gmail.com
*
***************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation version 2 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
***************************************************************************
*
* This version of GPL is at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*
***************************************************************************
*/
#include <stdio.h>
#include "mcumap_virtual.h"
#include "virtualserial.h"

#ifdef __linux__

#else

HANDLE win_serial = NULL;
unsigned char ComPortName[] = COMPORT;
unsigned char ComParams[] = "baud=115200 parity=N data=8 stop=1";

int virtualserial_open(void)
{
	DCB dcbSerialParams = {0};
	COMMTIMEOUTS timeouts = {0};
	win_serial = NULL;
	fprintf(stderr, "Opening serial port %s...", ComPortName);
    win_serial = CreateFileA(ComPortName
                , GENERIC_READ|GENERIC_WRITE, 0, NULL,
                OPEN_EXISTING, 0, NULL );
    if (win_serial == INVALID_HANDLE_VALUE)
    {
            fprintf(stderr, "Error\n");
            return 1;
    }
    else fprintf(stderr, "OK\n");
	
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	
	if (!BuildCommDCBA(ComParams, &dcbSerialParams))
	{
        fprintf(stderr, "Error setting device parameters\n");
        CloseHandle(win_serial);
        return 1;
    }
    /*if (!GetCommState(win_serial, &dcbSerialParams))
    {
        fprintf(stderr, "Error getting device state\n");
        CloseHandle(win_serial);
        return 1;
    }
     
    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if(!SetCommState(win_serial, &dcbSerialParams))
    {
        fprintf(stderr, "Error setting device parameters\n");
        CloseHandle(win_serial);
        return 1;
    }*/
    
    // Set COM port timeout settings
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutConstant = 1;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.WriteTotalTimeoutConstant = 1;
    timeouts.WriteTotalTimeoutMultiplier = 1;
    if(!SetCommTimeouts(win_serial, &timeouts))
    {
        fprintf(stderr, "Error setting timeouts\n");
        CloseHandle(win_serial);
        return 1;
    }
    
    PurgeComm(win_serial, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
    
    return 0;
}

char serial_buffer[127];
int serial_charcount = 0;
int serial_charindex = 0;

unsigned char virtualserial_getc(void)
{
	DWORD dwEventMask;
	unsigned char  TempChar;
	DWORD NoBytesRead;

	if(serial_charcount)
	{
		serial_charcount--;
		serial_charindex++;
		return (unsigned char)serial_buffer[serial_charindex];
	}

	if(ReadFile(win_serial, serial_buffer, sizeof(serial_buffer), &NoBytesRead, NULL))
	{
		serial_charcount = NoBytesRead;
		if(serial_charcount != 0)
		{
			//fprintf(stderr, "[Recieved]:%s", serial_buffer);
			serial_charcount--;
			serial_charindex = 0;
			return (unsigned char)serial_buffer[0];
		}
		PurgeComm(win_serial, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
	}
	
	return 0;
}

void virtualserial_putc(unsigned char c)
{
	DWORD  dNoOfBytesWritten = 0;          // No of bytes written to the port
	DWORD dwRes;
	
	if (!WriteFile(win_serial, &c, 1, &dNoOfBytesWritten,  NULL))
		fprintf(stderr, "Error %d in Writing to Serial Port",GetLastError());
	fputc(c,stderr);
	PurgeComm(win_serial, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
		
}

void virtualserial_puts(const unsigned char* __str)
{
	unsigned char   lpBuffer[127];		       // lpBuffer should be  char or byte array, otherwise write wil fail
	strcpy(lpBuffer, __str);
	DWORD  dNoOFBytestoWrite;              // No of bytes to write into the port
	DWORD  dNoOfBytesWritten = 0;          // No of bytes written to the port	
	dNoOFBytestoWrite = strlen(__str); // Calculating the no of bytes to write into the port

	if (!WriteFile(win_serial, lpBuffer, dNoOFBytestoWrite, &dNoOfBytesWritten, NULL))
		fprintf(stderr, "Error %d in Writing to Serial Port",GetLastError());
	fputs(__str, stderr);
	PurgeComm(win_serial, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
}

int virtualserial_close(void)
{
	// Close serial port
    fprintf(stderr, "Closing serial port COM3...");
    if (CloseHandle(win_serial) == 0)
    {
        fprintf(stderr, "Error\n");
        return 1;
    }
    fprintf(stderr, "OK\n");
    return 0;
}

#endif

