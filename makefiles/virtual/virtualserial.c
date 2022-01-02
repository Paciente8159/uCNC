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
#include "../../uCNC/src/hal/mcus/virtual/mcumap_virtual.h"
#include "virtualserial.h"

#ifdef __linux__

#else
#include <windows.h>

HANDLE hComm = NULL;
unsigned char ComPortName[] = COMPORT;
unsigned char ComParams[] = "baud=115200 parity=N data=8 stop=1";

int virtualserial_open(send_char_callback a, read_char_callback b)
{
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {MAXDWORD, 0, 0, 0, 0};
    hComm = NULL;
    DWORD dwRes;
    DWORD dwCommEvent;
    DWORD dwStoredFlags;
    DWORD dwOvRes;
    BOOL fWaitingOnStat = FALSE;
    OVERLAPPED osStatus = {0};

    fprintf(stderr, "Opening serial port %s...", ComPortName);
    hComm = CreateFileA(ComPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (hComm == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Error\n");
        return 1;
    }
    else
        fprintf(stderr, "OK\n");

    // dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    // if (!BuildCommDCBA(ComParams, &dcbSerialParams))
    // {
    //     fprintf(stderr, "Error setting device parameters\n");
    //     CloseHandle(hComm);
    //     return 1;
    // }

    // if (!GetCommState(hComm, &dcbSerialParams))
    // {
    //     fprintf(stderr, "Error getting device state\n");
    //     CloseHandle(hComm);
    //     return 1;
    // }

    // dcbSerialParams.BaudRate = CBR_115200;
    // dcbSerialParams.ByteSize = 8;
    // dcbSerialParams.StopBits = ONESTOPBIT;
    // dcbSerialParams.Parity = NOPARITY;
    // if (!SetCommState(hComm, &dcbSerialParams))
    // {
    //     fprintf(stderr, "Error setting device parameters\n");
    //     CloseHandle(hComm);
    //     return 1;
    // }

    // Set COM port timeout settings
    if (!SetCommTimeouts(hComm, &timeouts))
    {
        fprintf(stderr, "Error setting timeouts\n");
        CloseHandle(hComm);
        return 1;
    }

    PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    dwStoredFlags = EV_RXCHAR | EV_TXEMPTY;
    if (!SetCommMask(hComm, dwStoredFlags))
    {
        fprintf(stderr, "Error setting COM mask\n");
        CloseHandle(hComm);
        return 1;
    }

    osStatus.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osStatus.hEvent == NULL)
    {
        fprintf(stderr, "Error setting COM event\n");
        CloseHandle(hComm);
        return 1;
    }

    for (;;)
    {
        // Issue a status event check if one hasn't been issued already.
        if (!fWaitingOnStat)
        {
            if (!WaitCommEvent(hComm, &dwCommEvent, &osStatus))
            {
                if (GetLastError() == ERROR_IO_PENDING)
                    fWaitingOnStat = TRUE;
                // The original text is: bWaitingOnStatusHandle = TRUE;
                else
                    // Error in WaitCommEvent function; abort.
                    break;
            }
            /*else  
	            // The WaitCommEvent function executed successfully.  
	            // Properly handle status events.  
	            ReportStatusEvent(dwCommEvent);   */
        }

        // Check on overlapped operation.
        if (fWaitingOnStat)
        {
            // Wait a little while for an event to occur.
            dwRes = WaitForSingleObject(osStatus.hEvent, INFINITE);
            switch (dwRes)
            {
            // Event occurred.
            case WAIT_OBJECT_0:
                if (!GetOverlappedResult(hComm, &osStatus, &dwOvRes, FALSE))
                {
                    // An error occurred in the overlapped operation;
                    // call GetLastError to find out what it was
                    // and abort if it is fatal.
                    fprintf(stderr, "Error %d in COM event", GetLastError());
                    break;
                }
                else
                {
                    // Status event is stored in the event flag
                    // specified in the original WaitCommEvent call.
                    // Deal with the status event as appropriate.
                    DWORD dwRead = 0;
                    char chRead;
                    switch (dwCommEvent)
                    {
                    case EV_RXCHAR:
                        do
                        {
                            dwRead = 0;
                            if (ReadFile(hComm, &chRead, 1, &dwRead, &osStatus))
                            {
                                // Read a byte and process it.
                                if (b)
                                {
                                    b(chRead);
                                }
                            }
                            else
                            {
                                // An error occurred when calling the ReadFile function.
                                fprintf(stderr, "Error %d reading COM", GetLastError());
                                break;
                            }

                        } while (dwRead);
                        break;
                    case EV_TXEMPTY:
                        if (a)
                        {
                            a();
                        }
                        /*if (WriteFile(hComm, &chRead, 1, &dwRead, &osStatus))
                            {
                                // Read a byte and process it.
                            }
                            else
                            {
                                // An error occurred when calling the ReadFile function.
                                break;
                            }*/

                        break;
                    }
                }

                // Set fWaitingOnStat flag to indicate that a new
                // WaitCommEvent is to be issued.
                fWaitingOnStat = FALSE;
                break;
                /*default:
                // Error in the WaitForSingleObject; abort
                // This indicates a problem with the OVERLAPPED structure's
                // event handle.
                fprintf(stderr, "Abort %d in COM event", GetLastError());
                CloseHandle(osStatus.hEvent);
                return 0;*/
            }
        }
    }

    CloseHandle(osStatus.hEvent);
}

char serial_buffer[256];
volatile int serial_buffer_head = 0;
volatile int serial_buffer_tail = 0;
volatile int serial_charcount = 0;

unsigned char virtualserial_getc(void)
{
    DWORD dwEventMask;
    unsigned char TempChar;
    DWORD NoBytesRead;
    char internalbuffer[128];

    if (ReadFile(hComm, internalbuffer, sizeof(internalbuffer), &NoBytesRead, NULL))
    {
        if (NoBytesRead)
        {
            int readchars = NoBytesRead;
            if (readchars + serial_buffer_head <= 256)
            {
                memcpy(&serial_buffer[serial_buffer_head], internalbuffer, readchars);
                serial_buffer_head += readchars;
            }
            else
            {
                int chunck = 256 - serial_buffer_head;
                memcpy(&serial_buffer[serial_buffer_head], internalbuffer, chunck);
                serial_buffer_head = 0;
                memcpy(&serial_buffer[serial_buffer_head], &internalbuffer[chunck], readchars - chunck);
            }

            serial_charcount += readchars;
        }
        //PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
    }

    if (serial_charcount)
    {
        unsigned char c = (unsigned char)serial_buffer[serial_buffer_tail];
        serial_buffer_tail++;
        if (serial_buffer_tail == 256)
        {
            serial_buffer_tail = 0;
        }
        serial_charcount--;
        return c;
    }

    return 0;
}

char serial_tx_buffer[127];

void virtualserial_putc(unsigned char c)
{
    DWORD dNoOfBytesWritten = 0; // No of bytes written to the port
    OVERLAPPED osWrite = {0};
    DWORD dwRes;

    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL)
    {
        fprintf(stderr, "Error %d in creating Writing Event to Serial Port", GetLastError());
        return;
    }
    // error creating overlapped event handle

    if (!WriteFile(hComm, &c, 1, &dNoOfBytesWritten, &osWrite))
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            // WriteFile failed, but isn't delayed. Report error and abort.
            fprintf(stderr, "Error %d in Writing to Serial Port", GetLastError());
        }
        else
        {
            // Write is pending.
            dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);
            switch (dwRes)
            {
            // OVERLAPPED structure's event has been signaled.
            case WAIT_OBJECT_0:
                if (!GetOverlappedResult(hComm, &osWrite, &dNoOfBytesWritten, FALSE))
                    fprintf(stderr, "Error %d in Writing to Serial Port", GetLastError());
                else
                    // Write operation completed successfully.
                    break;

            default:
                // An error has occurred in WaitForSingleObject.
                // This usually indicates a problem with the
                // OVERLAPPED structure's event handle.
                fprintf(stderr, "Error %d in Writing to Serial Port", GetLastError());
                break;
            }
        }

        CloseHandle(osWrite.hEvent);
    }
}

/*void virtualserial_putc(unsigned char c)
{
    static uint8_t index = 0;

    serial_tx_buffer[index] = c;
    index++;
    serial_tx_buffer[index] = 0;
    if (c == '\n')
    {
        virtualserial_puts(&serial_tx_buffer);
        index = 0;
    }
}*/

void virtualserial_puts(const unsigned char *__str)
{
    unsigned char lpBuffer[127]; // lpBuffer should be  char or byte array, otherwise write wil fail
    strcpy(lpBuffer, __str);
    DWORD dNoOFBytestoWrite;           // No of bytes to write into the port
    DWORD dNoOfBytesWritten = 0;       // No of bytes written to the port
    dNoOFBytestoWrite = strlen(__str); // Calculating the no of bytes to write into the port
    OVERLAPPED osWrite = {0};
    DWORD dwRes;

    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL)
    {
        fprintf(stderr, "Error %d in creating Writing Event to Serial Port", GetLastError());
        return;
    }
    // error creating overlapped event handle

    if (!WriteFile(hComm, lpBuffer, dNoOFBytestoWrite, &dNoOfBytesWritten, &osWrite))
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            // WriteFile failed, but isn't delayed. Report error and abort.
            fprintf(stderr, "Error %d in Writing to Serial Port", GetLastError());
        }
        else
        {
            // Write is pending.
            dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);
            switch (dwRes)
            {
            // OVERLAPPED structure's event has been signaled.
            case WAIT_OBJECT_0:
                if (!GetOverlappedResult(hComm, &osWrite, &dNoOfBytesWritten, FALSE))
                    fprintf(stderr, "Error %d in Writing to Serial Port", GetLastError());
                else
                    // Write operation completed successfully.
                    break;

            default:
                // An error has occurred in WaitForSingleObject.
                // This usually indicates a problem with the
                // OVERLAPPED structure's event handle.
                fprintf(stderr, "Error %d in Writing to Serial Port", GetLastError());
                break;
            }
        }

        CloseHandle(osWrite.hEvent);
    }

    //fputs(__str, stderr);
    //PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
}

int virtualserial_close(void)
{
    // Close serial port
    fprintf(stderr, "Closing serial port COM3...");
    if (CloseHandle(hComm) == 0)
    {
        fprintf(stderr, "Error\n");
        return 1;
    }
    fprintf(stderr, "OK\n");
    return 0;
}

#endif
