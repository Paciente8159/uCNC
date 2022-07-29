/*
	Name: mcu_virtual.c
	Description: Simulates and MCU that runs on a Windows PC. This is mainly used to test/simulate µCNC.
		Besides all the functions declared in the mcu.h it also implements the code responsible
		for handling:
			interpolator.h
				void itp_step_isr();
				void itp_step_reset_isr();
			serial.h
				void serial_rx_isr(char c);
				char serial_tx_isr();
			trigger_control.h
				void dio_limits_isr(uint8_t limits);
				void io_controls_isr(uint8_t controls);

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"
#if (MCU == MCU_VIRTUAL_WIN)
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // Winsock Library
#include <windows.h>

#ifndef WIN_INTERFACE
#define WIN_INTERFACE 0
#endif

#define _str(x) #x
#define str(x) _str(x)

// uncomment to use sockets
#if (WIN_INTERFACE == 1)
#define USESOCKETS
#ifdef USESOCKETS
#define DEFAULT_BUFLEN 127
#ifndef SOCKET_PORT
#define SOCKET_PORT 34000
#endif
#endif
#elif (WIN_INTERFACE == 0)
// uncomment to use serial port
#define USESERIAL
#ifdef USESERIAL
#ifndef WIN_COM_NAME
#define WIN_COM_NAME COM1
#endif
#endif
#elif (WIN_INTERFACE == 2)
#define USECONSOLE
#endif

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUDRATE
#define BAUDRATE 115200
#endif

#ifndef COM_BUFFER_SIZE
#define COM_BUFFER_SIZE 50
#endif

extern void mod_input_change_hook(void);
MCU_IO_CALLBACK void mcu_inputs_changed_cb(void)
{
#ifdef ENABLE_IO_MODULES
	mod_input_change_hook();
#endif
}

/*timers*/
int start_timer(int, void (*)(void));
void stop_timer(void);
void startCycleCounter(void);
unsigned long stopCycleCounter(void);
unsigned long getCPUFreq(void);
unsigned long getTickCounter(void);

HANDLE win_timer;
void (*timer_func_handler_pntr)(void);
unsigned long perf_start;

VOID CALLBACK timer_sig_handler(PVOID, BOOLEAN);

int start_timer(int mSec, void (*timer_func_handler)(void))
{
	timer_func_handler_pntr = timer_func_handler;

	if (CreateTimerQueueTimer(&win_timer, NULL, (WAITORTIMERCALLBACK)timer_sig_handler, NULL, mSec, mSec, WT_EXECUTEINTIMERTHREAD) == 0)
	{
		printf("\nCreateTimerQueueTimer() error\n");
		return (1);
	}

	return (0);
}

VOID CALLBACK timer_sig_handler(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	timer_func_handler_pntr();
}

void stop_timer(void)
{
	DeleteTimerQueueTimer(NULL, win_timer, NULL);
	CloseHandle(win_timer);
}

void startCycleCounter(void)
{
	if (getCPUFreq() == 0)
	{
		return;
	}

	perf_start = getTickCounter();
}

unsigned long stopCycleCounter(void)
{
	return (getTickCounter() - perf_start);
}

unsigned long getCPUFreq(void)
{
	LARGE_INTEGER perf_counter;

	if (!QueryPerformanceFrequency(&perf_counter))
	{
		printf("QueryPerformanceFrequency failed!\n");
		return 0;
	}

	return perf_counter.QuadPart;
}

unsigned long getTickCounter(void)
{
	LARGE_INTEGER perf_counter;
	QueryPerformanceCounter(&perf_counter);
	return perf_counter.QuadPart;
}

/**
 * IO simulation and handling for external app
 * */
typedef struct virtual_map_t
{
	uint32_t special_outputs;
	uint32_t outputs;
	uint8_t pwm[16];
	uint8_t servos[6];
	uint32_t special_inputs;
	uint32_t inputs;
	uint8_t analog[16];
} VIRTUAL_MAP;

static volatile VIRTUAL_MAP virtualmap;

void ioserver(void *args)
{
	HANDLE hPipe;
	TCHAR chBuf[sizeof(VIRTUAL_MAP)];
	BOOL fSuccess = FALSE;
	DWORD cbRead, cbToWrite, cbWritten, dwMode;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\ucncio");

	// Try to open a named pipe; wait for it, if necessary.

	while (1)
	{
		BOOL fConnected = FALSE;

		hPipe = CreateNamedPipe(
			lpszPipename,				// pipe name
			PIPE_ACCESS_DUPLEX,			// read/write access
			PIPE_TYPE_MESSAGE |			// message type pipe
				PIPE_READMODE_MESSAGE | // message-read mode
				PIPE_WAIT,				// blocking mode
			PIPE_UNLIMITED_INSTANCES,	// max. instances
			sizeof(VIRTUAL_MAP),		// output buffer size
			sizeof(VIRTUAL_MAP),		// input buffer size
			0,							// client time-out
			NULL);						// no template file

		if (hPipe == INVALID_HANDLE_VALUE)
		{
			printf("CreateNamedPipe failed, GLE=%d.\n", GetLastError());
			return;
		}

		// Wait for the client to connect; if it succeeds,
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED.

		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (fConnected)
		{
			// Send a message to the pipe server.

			cbToWrite = sizeof(VIRTUAL_MAP);
			char lpvMessage[sizeof(VIRTUAL_MAP)];
			do
			{
				memcpy(lpvMessage, &virtualmap, sizeof(VIRTUAL_MAP));

				fSuccess = WriteFile(
					hPipe,		// pipe handle
					lpvMessage, // message
					cbToWrite,	// message length
					&cbWritten, // bytes written
					NULL);		// not overlapped

				if (!fSuccess)
				{
					printf("WriteFile to pipe failed. GLE=%d\n", GetLastError());
					break;
				}

				// Read from the pipe.

				fSuccess = ReadFile(
					hPipe,		// pipe handle
					lpvMessage, // buffer to receive reply
					cbToWrite,	// size of buffer
					&cbRead,	// number of bytes read
					NULL);		// not overlapped

				if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
					break;

				VIRTUAL_MAP *ptr = &lpvMessage;
				if (virtualmap.special_inputs != ptr->special_inputs)
				{
					uint32_t diff = virtualmap.special_inputs ^ ptr->special_inputs;
					virtualmap.special_inputs = ptr->special_inputs;

					if (diff & 0x1FFUL)
						mcu_limits_changed_cb();
					if (diff & 0x200UL)
						mcu_probe_changed_cb();
					if (diff & 0x3C00UL)
						mcu_controls_changed_cb();
				}
				if (virtualmap.inputs != ptr->inputs)
				{
					virtualmap.inputs = ptr->inputs;
					mcu_inputs_changed_cb();
				}
				memcpy(virtualmap.analog, ptr->analog, 16);

			} while (fSuccess); // repeat loop if ERROR_MORE_DATA

			if (!fSuccess)
			{
				printf("ReadFile from pipe failed. GLE=%d\n", GetLastError());
			}
		}

		CloseHandle(hPipe);
	}

	return;
}

/**
 * Comunications can be done via sockets or serial port
 * */
HANDLE rxReady, rxThread;
HANDLE txReady, txThread;
HANDLE bufferMutex;
char com_buffer[256];
char mcu_tx_buffer[256];
volatile bool mcu_tx_empty;
volatile bool mcu_tx_enabled;

#ifdef USESOCKETS

SOCKET ClientSocket = INVALID_SOCKET;

DWORD WINAPI socketserver(LPVOID lpParam)
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[256];
	int recvbuflen = 256;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, str(SOCKET_PORT), &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %d\n", (int)WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", (int)WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", (int)WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", (int)WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(ListenSocket);
	memset(recvbuf, 0, sizeof(recvbuf));
	SetEvent(rxReady);
	// Receive until the peer shuts down the connection
	do
	{

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			recvbuflen = strlen(recvbuf);
			for (int i = 0; i < recvbuflen; i++)
			{
				mcu_com_rx_cb(recvbuf[i]);
			}
			memset(recvbuf, 0, sizeof(recvbuf));
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else
		{
			printf("recv failed with error: %d\n", (int)WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", (int)WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}

void socketclient(void)
{
	DWORD dwWaitResult;
	char buffer[256];
	int iSendResult;
	int iResult;

	while (1)
	{
		dwWaitResult = WaitForSingleObject(
			txReady,   // event handle
			INFINITE); // indefinite wait

		switch (dwWaitResult)
		{
		// Event object was signaled
		case WAIT_OBJECT_0:
			dwWaitResult = WaitForSingleObject(
				bufferMutex, // handle to mutex
				INFINITE);	 // no time-out interval

			switch (dwWaitResult)
			{
			// The thread got ownership of the mutex
			case WAIT_OBJECT_0:
				iResult = strlen(com_buffer);

				iSendResult = send(ClientSocket, com_buffer, iResult, 0);
				if (iSendResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", (int)WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
				}
				memset(com_buffer, 0, 256);
				ReleaseMutex(bufferMutex);
				break;

			// The thread got ownership of an abandoned mutex
			case WAIT_ABANDONED:
				printf("Wait error (%d)\n", (int)GetLastError());
				return;
			}

			break;

		// An error occurred
		default:
			printf("Socket client thread error (%d)\n", (int)GetLastError());
			return;
		}
	}
}

#endif

#ifdef USESERIAL
volatile HANDLE hComm = NULL;
unsigned char ComPortName[] = "\\\\.\\" str(WIN_COM_NAME);
unsigned char ComParams[] = "baud=" str(BAUDRATE) " parity=N data=8 stop=1";

DWORD WINAPI virtualserialserver(LPVOID lpParam)
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
	hComm = CreateFileA(
		ComPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hComm == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error\n");
		return 1;
	}
	else
		fprintf(stderr, "OK\n");

	// Set COM port timeout settings
	if (!SetCommTimeouts(hComm, &timeouts))
	{
		fprintf(stderr, "Error setting timeouts\n");
		CloseHandle(hComm);
		return 1;
	}

	PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	dwStoredFlags = EV_RXCHAR;
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
		}

		SetEvent(rxReady);
		// Check on overlapped operation.
		// if (fWaitingOnStat)
		// {
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
				fprintf(stderr, "Error %d in COM event", (int)GetLastError());
				break;
			}
			else
			{
				// Status event is stored in the event flag
				// specified in the original WaitCommEvent call.
				// Deal with the status event as appropriate.
				char recvbuf[256];
				int recvbuflen = 256;
				DWORD dwRead = 0;
				switch (dwCommEvent)
				{
				case EV_RXCHAR:
					do
					{
						if (ReadFile(hComm, &recvbuf, recvbuflen, &dwRead, &osStatus))
						{
							// Read a byte and process it.
							recvbuflen = strlen(recvbuf);
							recvbuflen = (dwRead > recvbuflen) ? recvbuflen : dwRead;
							for (int i = 0; i < recvbuflen; i++)
							{
								putchar(recvbuf[i]);
								mcu_com_rx_cb(recvbuf[i]);
							}
							memset(recvbuf, 0, sizeof(recvbuf));
						}
						else
						{
							// An error occurred when calling the ReadFile function.
							fprintf(stderr, "Error %d reading COM", (int)GetLastError());
							break;
						}

					} while (dwRead);
					break;
				default:
					break;
				}
			}

			// Set fWaitingOnStat flag to indicate that a new
			// WaitCommEvent is to be issued.
			fWaitingOnStat = FALSE;
			break;
		}
		// }
	}

	CloseHandle(osStatus.hEvent);
}

void virtualserialclient(void)
{
	DWORD dwWaitResult;
	OVERLAPPED osWrite = {0};
	char buffer[256];
	int iSendResult;
	int iResult;

	DWORD dNoOfBytesWritten;

	while (1)
	{
		dwWaitResult = WaitForSingleObject(
			txReady,   // event handle
			INFINITE); // indefinite wait
		osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		switch (dwWaitResult)
		{
		// Event object was signaled
		case WAIT_OBJECT_0:
			dwWaitResult = WaitForSingleObject(
				bufferMutex, // handle to mutex
				INFINITE);	 // no time-out interval

			switch (dwWaitResult)
			{
			// The thread got ownership of the mutex
			case WAIT_OBJECT_0:
				iResult = strlen(com_buffer);
				if (!WriteFile(hComm, com_buffer, iResult, &dNoOfBytesWritten, &osWrite))
				{
					if (GetLastError() != ERROR_IO_PENDING)
					{
						// WriteFile failed, but isn't delayed. Report error and abort.
						fprintf(stderr, "Error %d in Writing to Serial Port", (int)GetLastError());
					}
				}
				break;

			// The thread got ownership of an abandoned mutex
			case WAIT_ABANDONED:
				printf("Wait error (%d)\n", (int)GetLastError());
				return;
			}

			break;

		// An error occurred
		default:
			printf("Serial client thread error (%d)\n", (int)GetLastError());
			return;
		}

		CloseHandle(osWrite.hEvent);
		memset(com_buffer, 0, 256);
		ReleaseMutex(bufferMutex);
	}
}

#endif

#ifdef USECONSOLE

DWORD WINAPI virtualconsoleserver(LPVOID lpParam)
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[256];
	int recvbuflen = 256;

	memset(recvbuf, 0, sizeof(recvbuf));
	SetEvent(rxReady);
	// Receive until the peer shuts down the connection
	do
	{
		unsigned char c = getchar();
		mcu_com_rx_cb(c);

	} while (1);

	return 0;
}

void virtualconsoleclient(void)
{
	DWORD dwWaitResult;
	int iResult;

	while (1)
	{
		dwWaitResult = WaitForSingleObject(
			txReady,   // event handle
			INFINITE); // indefinite wait

		switch (dwWaitResult)
		{
		// Event object was signaled
		case WAIT_OBJECT_0:
			dwWaitResult = WaitForSingleObject(
				bufferMutex, // handle to mutex
				INFINITE);	 // no time-out interval

			switch (dwWaitResult)
			{
			// The thread got ownership of the mutex
			case WAIT_OBJECT_0:
				iResult = strlen(com_buffer);
				/*for (int k = 0; k < iResult; k++)
				{
					putchar(com_buffer[k]);
				}*/
				break;

			// The thread got ownership of an abandoned mutex
			case WAIT_ABANDONED:
				printf("Wait error (%d)\n", (int)GetLastError());
				return;
			}

			break;

		// An error occurred
		default:
			printf("Serial client thread error (%d)\n", (int)GetLastError());
			return 0;
		}

		memset(com_buffer, 0, 256);
		ReleaseMutex(bufferMutex);
	}
}
#endif

void com_init(void)
{
	DWORD rxThreadID;
	DWORD txThreadID;
	DWORD dwWaitResult;

	PVOID txCallback;
	PVOID rxCallback;

#ifdef USESOCKETS
	rxCallback = &socketserver;
	txCallback = &socketclient;
#elif defined(USESERIAL)
	rxCallback = &virtualserialserver;
	txCallback = &virtualserialclient;
#elif defined(USECONSOLE)
	rxCallback = &virtualconsoleserver;
	txCallback = &virtualconsoleclient;
#endif

	bufferMutex = CreateMutex(
		NULL,  // default security attributes
		FALSE, // initially not owned
		NULL); // unnamed mutex

	if (bufferMutex == NULL)
	{
		printf("CreateMutex error: %d\n", (int)GetLastError());
		return;
	}

	rxReady = CreateEvent(
		NULL,				// default security attributes
		FALSE,				// manual-reset event
		FALSE,				// initial state is nonsignaled
		TEXT("SocketReady") // object name
	);

	if (rxReady == NULL)
	{
		printf("CreateEvent failed (%d)\n", GetLastError());
		return;
	}

	txReady = CreateEvent(
		NULL,				// default security attributes
		FALSE,				// manual-reset event
		FALSE,				// initial state is nonsignaled
		TEXT("SocketReady") // object name
	);

	if (txReady == NULL)
	{
		printf("CreateEvent failed (%d)\n", GetLastError());
		return;
	}

	rxThread = CreateThread(
		NULL,		  // default security attributes
		0,			  // use default stack size
		rxCallback,	  // thread function name
		NULL,		  // argument to thread function
		0,			  // use default creation flags
		&rxThreadID); // returns the thread identifier

	if (rxThread == NULL)
	{
		printf("CreateThread failed (%d)\n", GetLastError());
		return;
	}

	dwWaitResult = WaitForSingleObject(
		rxReady,   // event handle
		INFINITE); // indefinite wait

	switch (dwWaitResult)
	{
	// Event object was signaled
	case WAIT_OBJECT_0:
		printf("Connection started\n");
		txThread = CreateThread(
			NULL,		  // default security attributes
			0,			  // use default stack size
			txCallback,	  // thread function name
			NULL,		  // argument to thread function
			0,			  // use default creation flags
			&txThreadID); // returns the thread identifier

		if (txThread == NULL)
		{
			printf("CreateThread failed (%d)\n", (int)GetLastError());
			return;
		}
		break;

	// An error occurred
	default:
		printf("Wait error (%d)\n", (int)GetLastError());
		return 0;
	}
}

void com_send(char *buff, int len)
{
	DWORD dwWaitResult;
	dwWaitResult = WaitForSingleObject(
		bufferMutex, // handle to mutex
		INFINITE);	 // no time-out interval
	mcu_tx_empty = false;
	switch (dwWaitResult)
	{
	// The thread got ownership of the mutex
	case WAIT_OBJECT_0:
		memcpy(com_buffer, buff, len);
		ReleaseMutex(bufferMutex);
		mcu_tx_empty = true;
		break;

	// The thread got ownership of an abandoned mutex
	case WAIT_ABANDONED:
		printf("Wait error (%d)\n", (int)GetLastError());
		return;
	}

	SetEvent(txReady);
}

// UART communication
uint8_t g_mcu_combuffer[COM_BUFFER_SIZE];
uint8_t g_mcu_bufferhead;
uint8_t g_mcu_buffertail;
uint8_t g_mcu_buffercount;

uint32_t _previnputs = 0;

volatile bool global_isr_enabled = false;
volatile unsigned long isr_flags = 0;

#define ISR_PULSE 1
#define ISR_PULSERESET 2
#define ISR_COMRX 4
#define ISR_COMTX 8
#define ISR_INPUT 16

volatile unsigned long g_cpu_freq = 0;
volatile unsigned long pulse_interval = 0;
volatile unsigned long resetpulse_interval = 0;
volatile unsigned long pulse_counter = 0;
volatile unsigned long *pulse_counter_ptr;
volatile unsigned long integrator_counter = 0;
volatile bool pulse_enabled = false;
volatile bool send_char = false;
volatile unsigned char uart_char;

uint8_t pwms[16];

pthread_t thread_io;
pthread_t thread_id;
pthread_t thread_idout;
pthread_t thread_timer_id;
pthread_t thread_step_id;

void mcu_rx_isr(unsigned char c)
{
	if (c)
	{
		mcu_com_rx_cb(c);
	}
}

void mcu_tx_isr(void)
{
	mcu_tx_empty = true;
}

// emulates uart RX
//  void *comsimul(void)
//  {
//  #ifdef USECOM
//  mcu_tx_empty = true;
//  virtualserial_init(&mcu_tx_isr, &mcu_rx_isr);
//  #else
//  	for (;;)
//  	{
//  		unsigned char c = getch();
//  		if (c != 0)
//  		{
//  			uart_char = c;
//  			while (!serial_rx_is_empty())
//  			{
//  			}
//  			serial_rx_isr(c);
//  			if (c == '\n' | c == '\r')
//  			{
//  			}
//  		}
//  	}
//  #endif
//  }

// emulates uart TX
// void *comoutsimul(void)
//{
//	for (;;)
//	{
//		if (mcu_tx_enabled)
//		{
//			mcu_tx_enabled = false;
//			serial_tx_isr();
//		}
//	}
// }

// simulates internal clock (1Kz limited by windows timer)
volatile static uint32_t mcu_runtime = 0;

void *stepsimul(void *args)
{
	static uint16_t tick_counter = 0;
	static uint16_t timer_counter = 0;
	unsigned long lasttime = getTickCounter();
	unsigned long acumm = 0;
	while (1)
	{

		unsigned long time = getTickCounter();
		unsigned long elapsed = time - lasttime;
		acumm += elapsed;
		elapsed *= F_CPU;
		elapsed /= g_cpu_freq;
		elapsed = (elapsed < 100) ? elapsed : 100;

		while (acumm > (F_CPU / 1000))
		{
			acumm -= (F_CPU / 1000);
			mcu_runtime++;
		}

		while (elapsed--)
		{
			if (pulse_interval && resetpulse_interval && pulse_enabled)
			{
				tick_counter++;
			}
			else
			{
				tick_counter = 0;
				break;
			}

			if (tick_counter == pulse_interval)
			{
				isr_flags |= ISR_PULSE; // flags step isr
			}

			if (tick_counter >= resetpulse_interval)
			{
				isr_flags |= ISR_PULSERESET; // flags step isr
				tick_counter = 0;
			}

			if (global_isr_enabled)
			{
				bool isr = global_isr_enabled;
				global_isr_enabled = false;

				if (isr_flags & ISR_INPUT)
				{
					// serial_rx_isr(uart_char);
					mcu_limits_changed_cb();
					mcu_controls_changed_cb();
					isr_flags &= ~ISR_INPUT;
				}

				if (pulse_enabled)
				{
					if (isr_flags & ISR_PULSE)
					{
						mcu_step_cb();
						isr_flags &= ~ISR_PULSE;
					}

					if (isr_flags & ISR_PULSERESET)
					{
						mcu_step_reset_cb();
						isr_flags &= ~ISR_PULSERESET;
					}
				}

				global_isr_enabled = isr;
			}

			lasttime = time;
		}
	}
}

void ticksimul(void)
{

	static VIRTUAL_MAP initials = {0};

	if (global_isr_enabled)
	{

		// FILE *infile = fopen("inputs.txt", "r");
		// char inputs[255];

		// if (infile != NULL) //checks input file
		// {
		// 	fscanf(infile, "%lX", &(virtualmap.inputs));
		// 	fclose(infile);

		// 	uint32_t diff = virtualmap.inputs ^ initials.inputs;
		// 	initials.inputs = virtualmap.inputs;

		// 	if (diff)
		// 	{
		// 		isr_flags |= ISR_INPUT; //flags input isr
		// 	}
		// }

		mcu_runtime++;
		mcu_disable_global_isr();
		mcu_rtc_cb(mcu_runtime);
		mcu_enable_global_isr();
	}
}

uint32_t mcu_millis()
{
	return mcu_runtime;
}

void mcu_init(void)
{
	virtualmap.special_outputs = 0;
	virtualmap.special_inputs = 0;
	virtualmap.inputs = 0;
	virtualmap.outputs = 0;
	com_init();
	send_char = false;
	//	FILE *infile = fopen("inputs.txt", "r");
	//	if (infile != NULL)
	//	{
	//		fscanf(infile, "%lX", &(virtualmap.inputs));
	//		fclose(infile);
	//	}
	//	else
	//	{
	//		infile = fopen("inputs.txt", "w+");
	//		if (infile != NULL)
	//		{
	//			fprintf(infile, "%lX", virtualmap.inputs);
	//			fflush(infile);
	//			fclose(infile);
	//		}
	//		else
	//		{
	//			printf("Failed to open input file");
	//		}
	//	}
	g_cpu_freq = getCPUFreq();
	start_timer(1, &ticksimul);
	//#ifdef USECONSOLE
	//	pthread_create(&thread_idout, NULL, &comoutsimul, NULL);
	//#endif
	pthread_create(&thread_step_id, NULL, &stepsimul, NULL);
	pthread_create(&thread_io, NULL, &ioserver, NULL);
	mcu_tx_enabled = false;
	g_mcu_buffercount = 0;
	pulse_counter_ptr = &pulse_counter;
	mcu_tx_empty = true;
	mcu_enable_global_isr();
}

// IO functions
void mcu_enable_probe_isr(void)
{
}
void mcu_disable_probe_isr(void)
{
}

uint8_t mcu_get_pin_offset(uint8_t pin)
{
	if (pin >= STEP0 && pin <= STEP7_EN)
	{
		return pin;
	}
	else if (pin >= DOUT0 && pin <= DOUT31)
	{
		return pin - DOUT0;
	}
	if (pin >= LIMIT_X && pin <= CS_RES)
	{
		return pin - LIMIT_X;
	}
	else if (pin >= DIN0 && pin <= DIN31)
	{
		return pin - DIN0;
	}

	return -1;
}

uint8_t mcu_get_input(uint8_t pin)
{
	uint8_t offset = mcu_get_pin_offset(pin);
	if (offset > 31)
	{
		return 0;
	}
	if (pin >= DIN0)
	{
		return (virtualmap.inputs & (1 << offset)) ? 1 : 0;
	}
	else
	{
		return (virtualmap.special_inputs & (1 << offset)) ? 1 : 0;
	}

	return 0;
}

/**
 * gets the value of a digital output pin
 * can be defined either as a function or a macro call
 * */
uint8_t mcu_get_output(uint8_t pin)
{
	uint8_t offset = mcu_get_pin_offset(pin);
	if (offset > 31)
	{
		return 0;
	}

	if (pin >= DOUT0)
	{
		return (virtualmap.outputs & (1 << offset)) ? 1 : 0;
	}
	else
	{
		return (virtualmap.special_outputs & (1 << offset)) ? 1 : 0;
	}

	return 0;
}

/**
 * sets the value of a digital output pin to logical 1
 * can be defined either as a function or a macro call
 * */
void mcu_set_output(uint8_t pin)
{
	uint8_t offset = mcu_get_pin_offset(pin);
	if (offset > 31)
	{
		return;
	}

	if (pin >= DOUT0)
	{
		virtualmap.outputs |= (1UL << offset);
	}
	else
	{
		virtualmap.special_outputs |= (1UL << offset);
	}
}

/**
 * sets the value of a digital output pin to logical 0
 * can be defined either as a function or a macro call
 * */
void mcu_clear_output(uint8_t pin)
{
	uint8_t offset = mcu_get_pin_offset(pin);
	if (offset > 31)
	{
		return;
	}

	if (pin >= DOUT0)
	{
		virtualmap.outputs &= ~(1UL << offset);
	}
	else
	{
		virtualmap.special_outputs &= ~(1UL << offset);
	}
}

/**
 * toggles the value of a digital output pin
 * can be defined either as a function or a macro call
 * */
void mcu_toggle_output(uint8_t pin)
{
	uint8_t offset = mcu_get_pin_offset(pin);
	if (offset > 31)
	{
		return;
	}

	if (pin >= DOUT0)
	{
		virtualmap.outputs ^= (1UL << offset);
	}
	else
	{
		virtualmap.special_outputs ^= (1UL << offset);
	}
}

uint8_t mcu_get_analog(uint8_t channel)
{
	channel -= ANALOG0;
	return virtualmap.analog[channel];
}

// Outputs
void mcu_set_pwm(uint8_t pwm, uint8_t value)
{
	pwm -= PWM0;
	virtualmap.pwm[pwm] = value;
}

uint8_t mcu_get_pwm(uint8_t pwm)
{
	pwm -= PWM0;
	return virtualmap.pwm[pwm];
}

void mcu_set_servo(uint8_t servo, uint8_t value)
{
	servo -= SERVO0;
	virtualmap.servos[value] = value;
}

uint8_t mcu_get_servo(uint8_t servo)
{
	servo -= SERVO0;
	return virtualmap.servos[servo];
}

// Communication functions
// sends a packet
void mcu_enable_tx_isr(void)
{
#ifndef USECONSOLE
	mcu_com_tx_cb();
#endif
	mcu_tx_enabled = true;
}

void mcu_disable_tx_isr(void)
{
	mcu_tx_enabled = false;
}

bool mcu_tx_ready(void)
{
	return mcu_tx_empty;
}

void mcu_putc(char c)
{
	static int buff_index = 0;
	if (c != 0)
	{
		//		while (!mcu_tx_empty)
		//			;
		//		mcu_tx_empty = false;

		mcu_tx_buffer[buff_index++] = c;
		if (c == '\n')
		{
			mcu_tx_buffer[buff_index++] = 0;
			com_send(mcu_tx_buffer, strlen(mcu_tx_buffer));
			buff_index = 0;
		}
		putchar(c);
	}
	mcu_tx_empty = true;
	mcu_tx_enabled = true;
}

char mcu_getc(void)
{
	char c = 0;
	if (g_mcu_buffertail != g_mcu_bufferhead)
	{
		c = g_mcu_combuffer[g_mcu_buffertail];
		if (++g_mcu_buffertail == COM_BUFFER_SIZE)
		{
			g_mcu_buffertail = 0;
		}

		if (c == '\n')
		{
			g_mcu_buffercount--;
		}
	}

	return c;
}

char mcu_peek(void)
{
	if (g_mcu_buffercount == 0)
		return 0;
	return g_mcu_combuffer[g_mcu_buffertail];
}

void mcu_bufferClear(void)
{
	memset(&g_mcu_combuffer, 0, sizeof(char) * COM_BUFFER_SIZE);
	g_mcu_buffertail = 0;
	g_mcu_bufferhead = 0;
}

// RealTime
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *tick_reps)
{
	if (frequency < F_STEP_MIN)
		frequency = F_STEP_MIN;
	if (frequency > F_STEP_MAX)
		frequency = F_STEP_MAX;

	*ticks = (uint16_t)floorf((F_CPU / frequency));
	*tick_reps = 1;
}

// enables all interrupts on the mcu. Must be called to enable all IRS functions
void mcu_enable_global_isr(void)
{
	global_isr_enabled = true;
}
// disables all ISR functions
void mcu_disable_global_isr(void)
{
	global_isr_enabled = false;
}

bool mcu_get_global_isr(void)
{
	return global_isr_enabled;
}

// starts a constant rate pulse at a given frequency. This triggers to ISR handles with an offset of MIN_PULSE_WIDTH useconds
void mcu_start_itp_isr(uint16_t clocks_speed, uint16_t prescaller)
{
	resetpulse_interval = clocks_speed;
	pulse_interval = resetpulse_interval >> 1;
	(*pulse_counter_ptr) = 0;
	pulse_enabled = true;
}

void mcu_change_itp_isr(uint16_t clocks_speed, uint16_t prescaller)
{
	pulse_enabled = false;
	resetpulse_interval = clocks_speed;
	pulse_interval = resetpulse_interval >> 1;
	(*pulse_counter_ptr) = 0;
	pulse_enabled = true;
}
// stops the pulse
void mcu_stop_itp_isr(void)
{
	pulse_enabled = false;
}

void mcu_printfp(const char *__fmt, ...)
{
	char buffer[50];
	char *newfmt = strcpy((char *)&buffer, __fmt);
	va_list __ap;
	va_start(__ap, __fmt);
	vprintf(newfmt, __ap);
	va_end(__ap);
}

void mcu_delay_us(uint8_t delay)
{
	unsigned long start = getTickCounter();
	double elapsed = 0;
	do
	{
		elapsed = ((double)(getTickCounter()) - (double)(start)) / (double)(getCPUFreq());
		elapsed *= 1000000;
	} while (elapsed < delay);
}

void mcu_loadDummyPayload(const char *__fmt, ...)
{
	char buffer[30];
	char payload[50];
	char *newfmt = strcpy((char *)&buffer, __fmt);
	va_list __ap;
	va_start(__ap, __fmt);
	vsprintf((char *)&payload, newfmt, __ap);
	va_end(__ap);
	g_mcu_bufferhead = strlen(payload);
	memset(&g_mcu_combuffer, 0, g_mcu_bufferhead);
	strcpy((char *)&g_mcu_combuffer, payload);
	g_mcu_buffertail = 0;
	g_mcu_buffercount++;
}

uint8_t mcu_eeprom_getc(uint16_t address)
{
	FILE *fp = fopen("virtualeeprom", "rb");
	uint8_t c = 0;

	if (fp != NULL)
	{
		if (!fseek(fp, address, SEEK_SET))
		{
			c = getc(fp);
			fclose(fp);
		}
	}

	return c;
}

void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	FILE *src = fopen("virtualeeprom", "rb+");

	if (!src)
	{
		FILE *dest = fopen("virtualeeprom", "wb");
		fclose(dest);
		src = fopen("virtualeeprom", "rb+");
	}

	/*for(int i = 0; i < address; i++)
	{
		getc(src);
	}*/

	fseek(src, address, SEEK_SET);
	putc((int)value, src);

	fflush(src);
	fclose(src);
}

void mcu_eeprom_flush()
{
}

void mcu_startPerfCounter(void)
{
	startCycleCounter();
}

uint16_t mcu_stopPerfCounter(void)
{
	return (uint16_t)stopCycleCounter();
}

void mcu_dotasks(void)
{
#ifdef ENABLE_SYNC_RX
	while (mcu_read_available())
	{
		unsigned char c = mcu_getc();
		serial_rx_isr(c);
	}
#endif
}

#endif
