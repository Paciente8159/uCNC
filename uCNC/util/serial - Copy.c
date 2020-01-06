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
#include "serial.h"

#ifdef __linux__

#else

HANDLE win_serial = NULL;
char ComPortName[] = "\\\\.\\COM3";
char ComParams[] = "baud=115200 parity=N data=8 stop=1";

int serial_open()
{
	DCB dcbSerialParams = {};
	COMMTIMEOUTS timeouts = {};
	win_serial = NULL;
	fprintf(stderr, "Opening serial port %s...", ComPortName);
    win_serial = CreateFile(ComPortName
                , GENERIC_READ|GENERIC_WRITE, 0, NULL,
                OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL );
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
    
    // Set COM port timeout settings
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
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

char serial_getc()
{
	DWORD dwEventMask;
	char  TempChar;
	DWORD dwRead; 
	OVERLAPPED osReader = {};
	BOOL fWaitingOnRead = FALSE;
	
	if(serial_charcount)
	{
		serial_charcount--;
		serial_charindex++;
		return serial_buffer[serial_charindex];
	}
	
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osReader.hEvent == NULL)
	{
		fprintf(stderr, "Error %d! in creating event", GetLastError());
		return 0;
	}
   	
	if (!fWaitingOnRead)
	{
		// Issue read operation.
		if (!ReadFile(win_serial, serial_buffer, sizeof(serial_buffer), &dwRead, NULL))
		{
			if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
			{
			}
			else
				fWaitingOnRead = TRUE;
		}
		else
		{    
			serial_charcount = dwRead;
			serial_charindex = 0;
			// read completed immediately
			//HandleASuccessfulRead(serial_buffer, dwRead);
		}
	}
	
	DWORD dwRes;
	if (fWaitingOnRead) {
	   dwRes = WaitForSingleObject(osReader.hEvent, 0);
	   switch(dwRes)
	   {
	      // Read completed.
	      case WAIT_OBJECT_0:
	          if (!GetOverlappedResult(win_serial, &osReader, &dwRead, FALSE))
	          {
	          }
	          else
	          {
				 }   
				 //HandleASuccessfulRead(serial_buffer, dwRead);
	
	          //  Reset flag so that another opertion can be issued.
	          fWaitingOnRead = FALSE;
	          break;
	
	      case WAIT_TIMEOUT:
	          // Operation isn't complete yet. fWaitingOnRead flag isn't
	          // changed since I'll loop back around, and I don't want
	          // to issue another read until the first one finishes.
	          //
	          // This is a good time to do some background work.
	          break;                       
	
	      default:
	          // Error in the WaitForSingleObject; abort.
	          // This indicates a problem with the OVERLAPPED structure's
	          // event handle.
	          break;
	   }
	}

	return 0;
}

void serial_putc(char c)
{
	OVERLAPPED osWrite = {0};
	DWORD dwWritten;
	BOOL fRes;

	// Create this writes OVERLAPPED structure hEvent.
	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osWrite.hEvent == NULL)
	  // Error creating overlapped event handle.
	  return FALSE;
	
	// Issue write.
	if (!WriteFile(win_serial, &c, 1, &dwWritten, &osWrite)) {
	  if (GetLastError() != ERROR_IO_PENDING) { 
	     // WriteFile failed, but it isn't delayed. Report error and abort.
	     fRes = FALSE;
	  }
	  else {
	     // Write is pending.
	     if (!GetOverlappedResult(win_serial, &osWrite, &dwWritten, TRUE))
	        fRes = FALSE;
	     else
	        // Write operation completed successfully.
	        fRes = TRUE;
	  }
	}
	else
	  // WriteFile completed immediately.
	  fRes = TRUE;
	
	CloseHandle(osWrite.hEvent);
	PurgeComm(win_serial, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
	return;	
}

void serial_puts(const char* __str)
{
	char   lpBuffer[127];		       // lpBuffer should be  char or byte array, otherwise write wil fail
	strcpy(lpBuffer, __str);
	DWORD  dNoOFBytestoWrite;              // No of bytes to write into the port
	DWORD  dNoOfBytesWritten = 0;          // No of bytes written to the port
	OVERLAPPED osWrite = {};	
	dNoOFBytestoWrite = strlen(__str); // Calculating the no of bytes to write into the port

	if (!WriteFile(win_serial, lpBuffer, dNoOFBytestoWrite, &dNoOfBytesWritten, &osWrite))
		fprintf(stderr, "Error %d in Writing to Serial Port",GetLastError());
		
	PurgeComm(win_serial, PURGE_TXABORT| PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
}

int serial_close()
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

