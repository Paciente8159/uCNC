#include "../../../cnc.h"
#if (MCU == MCU_VIRTUAL_WIN)

#include <pthread.h>

// /**
//  * GENERIC
//  * **/
// static void port_init(win_port_t *port)
// {
//     DWORD rxThreadID;
//     DWORD txThreadID;
//     DWORD dwWaitResult;

//     port->io.tx.bufferMutex = CreateMutex(
//         NULL,  // default security attributes
//         FALSE, // initially not owned
//         NULL); // unnamed mutex

//     if (port->io.tx.bufferMutex == NULL)
//     {
//         printf("CreateMutex error: %lu\n", GetLastError());
//         return;
//     }

//     port->io.tx.txReady = CreateEvent(
//         NULL,               // default security attributes
//         FALSE,              // manual-reset event
//         FALSE,              // initial state is nonsignaled
//         TEXT("SocketReady") // object name
//     );

//     if (port->io.tx.txReady == NULL)
//     {
//         printf("CreateEvent failed (%lu)\n", GetLastError());
//         return;
//     }

//     port->io.rx.rxReady = CreateEvent(
//         NULL,               // default security attributes
//         FALSE,              // manual-reset event
//         FALSE,              // initial state is nonsignaled
//         TEXT("SocketReady") // object name
//     );

//     if (port->io.rx.rxReady == NULL)
//     {
//         printf("CreateEvent failed (%lu)\n", GetLastError());
//         return;
//     }

//     port->rxThread = CreateThread(
//         NULL,             // default security attributes
//         0,                // use default stack size
//         port->rxCallback, // thread function name
//         &(port->io),      // argument to thread function
//         0,                // use default creation flags
//         &rxThreadID);     // returns the thread identifier

//     if (port->rxThread == NULL)
//     {
//         printf("CreateThread failed (%lu)\n", GetLastError());
//         return;
//     }

//     dwWaitResult = WaitForSingleObject(
//         port->io.rx.rxReady, // event handle
//         INFINITE);           // indefinite wait

//     switch (dwWaitResult)
//     {
//     // Event object was signaled
//     case WAIT_OBJECT_0:
//         printf("Connection started\n");
//         port->txThread = CreateThread(
//             NULL,             // default security attributes
//             0,                // use default stack size
//             port->txCallback, // thread function name
//             &(port->io),      // argument to thread function
//             0,                // use default creation flags
//             &txThreadID);     // returns the thread identifier

//         if (port->txThread == NULL)
//         {
//             printf("CreateThread failed (%lu)\n", GetLastError());
//             return;
//         }
//         break;

//     // An error occurred
//     default:
//         printf("Wait error (%lu)\n", GetLastError());
//         return;
//     }
// }

// void port_write(win_port_t *port, char *buff, int len)
// {
//     DWORD dwWaitResult;
//     dwWaitResult = WaitForSingleObject(
//         port->io.tx.bufferMutex, // handle to mutex
//         INFINITE);               // no time-out interval
//     int l = port->io.tx.len;
//     port->io.tx.len += len;
//     switch (dwWaitResult)
//     {
//     // The thread got ownership of the mutex
//     case WAIT_OBJECT_0:
//         memcpy((char *)&port->io.tx.buffer[l], buff, len);
//         ReleaseMutex(port->io.tx.bufferMutex);
//         break;

//     // The thread got ownership of an abandoned mutex
//     case WAIT_ABANDONED:
//         printf("Wait error (%lu)\n", GetLastError());
//         return;
//     }

//     SetEvent(port->io.tx.txReady);
// }

// /**
//  * CONSOLE
//  * **/
// DWORD WINAPI consoleserver(LPVOID lpParam)
// {
//     win_port_io_t *io = (win_port_io_t *)lpParam;
//     char recvbuf[256];

//     memset(recvbuf, 0, 256);
//     SetEvent(io->rx.rxReady);
//     sleep(1);
//     // Receive until the peer shuts down the connection
//     do
//     {
//         unsigned char c = getch();
//         putc(c, stderr);
//         if (c == '\r')
//         {
//             putc('\n', stderr);
//         }
//         switch (c)
//         {
//         default:
//             if (io->rx.rxHandler)
//             {
//                 io->rx.rxHandler(c);
//             }
//             break;
//         }

//     } while (1);

//     return -1;
// }

// DWORD WINAPI consoleclient(LPVOID lpParam)
// {
//     win_port_io_t *io = lpParam;
//     DWORD dwWaitResult;
//     int iResult = 0;

//     while (1)
//     {
//         dwWaitResult = WaitForSingleObject(
//             io->tx.txReady, // event handle
//             INFINITE);      // indefinite wait

//         switch (dwWaitResult)
//         {
//         // Event object was signaled
//         case WAIT_OBJECT_0:
//             dwWaitResult = WaitForSingleObject(
//                 io->tx.bufferMutex, // handle to mutex
//                 INFINITE);          // no time-out interval

//             switch (dwWaitResult)
//             {
//             // The thread got ownership of the mutex
//             case WAIT_OBJECT_0:
//                 printf("%s", ((char *)io->tx.buffer));
//                 io->tx.len = 0;
//                 memset((char *)io->tx.buffer, 0, 256);
//                 break;

//             // The thread got ownership of an abandoned mutex
//             case WAIT_ABANDONED:
//                 printf("Wait error (%lu)\n", GetLastError());
//                 return -1;
//             }

//             break;

//         // An error occurred
//         default:
//             printf("Serial client thread error (%lu)\n", GetLastError());
//             return -1;
//         }

//         memset((char *)io->tx.buffer, 0, 256);
//         ReleaseMutex(io->tx.bufferMutex);
//     }
//     return -1;
// }

// void console_init(win_port_t *port)
// {
//     port->rxCallback = &consoleserver;
//     port->txCallback = &consoleclient;
//     port_init(port);
// }

// /**
//  * SOCKET
//  * **/
// static DWORD WINAPI socketserver(LPVOID lpParam)
// {
//     win_port_io_t *io = lpParam;
//     WSADATA wsaData;
//     int iResult;

//     SOCKET ListenSocket = INVALID_SOCKET;

//     struct addrinfo *result = NULL;
//     struct addrinfo hints;

//     int iSendResult;

//     // Initialize Winsock
//     iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//     if (iResult != 0)
//     {
//         printf("WSAStartup failed with error: %d\n", iResult);
//         return -1;
//     }

//     ZeroMemory(&hints, sizeof(hints));
//     hints.ai_family = AF_INET;
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_protocol = IPPROTO_TCP;
//     hints.ai_flags = AI_PASSIVE;

//     // Resolve the server address and port
//     iResult = getaddrinfo(NULL, io->portname, &hints, &result);
//     if (iResult != 0)
//     {
//         printf("getaddrinfo failed with error: %d\n", iResult);
//         WSACleanup();
//         return -1;
//     }

//     // Create a SOCKET for connecting to server
//     ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
//     if (ListenSocket == INVALID_SOCKET)
//     {
//         printf("socket failed with error: %d\n", WSAGetLastError());
//         freeaddrinfo(result);
//         WSACleanup();
//         return -1;
//     }

//     if (!io->isclient)
//     {
//         // Setup the TCP listening socket
//         iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
//         if (iResult == SOCKET_ERROR)
//         {
//             printf("bind failed with error: %d\n", WSAGetLastError());
//             freeaddrinfo(result);
//             closesocket(ListenSocket);
//             WSACleanup();
//             return -1;
//         }
//     }
//     else
//     {
//         iResult = connect(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
//         if (iResult == SOCKET_ERROR)
//         {
//             freeaddrinfo(result);
//             closesocket(ListenSocket);
//             WSACleanup();
//             return -1;
//         }
//     }

//     freeaddrinfo(result);

//     iResult = listen(ListenSocket, SOMAXCONN);
//     if (iResult == SOCKET_ERROR)
//     {
//         printf("listen failed with error: %d\n", WSAGetLastError());
//         closesocket(ListenSocket);
//         WSACleanup();
//         return -1;
//     }

//     // Accept a client socket
//     io->tx.txHandle = malloc(sizeof(SOCKET));
//     (*((SOCKET *)io->tx.txHandle)) = accept(ListenSocket, NULL, NULL);
//     if ((*((SOCKET *)io->tx.txHandle)) == INVALID_SOCKET)
//     {
//         printf("accept failed with error: %d\n", WSAGetLastError());
//         closesocket(ListenSocket);
//         WSACleanup();
//         return -1;
//     }

//     // No longer need server socket
//     closesocket(ListenSocket);
//     memset(recvbuf, 0, sizeof(recvbuf));
//     SetEvent(io->rx.rxReady);
//     // Receive until the peer shuts down the connection
//     do
//     {

//         iResult = recv((*((SOCKET *)io->tx.txHandle)), recvbuf, recvbuflen, 0);
//         if (iResult > 0)
//         {
//             recvbuflen = strlen(recvbuf);
//             for (int i = 0; i < recvbuflen; i++)
//             {
//                 if (io->rx.rxHandler)
//                     io->rx.rxHandler(recvbuf[i]);
//             }
//             memset(recvbuf, 0, sizeof(recvbuf));
//         }
//         else if (iResult == 0)
//             printf("Connection closing...\n");
//         else
//         {
//             printf("recv failed with error: %d\n", WSAGetLastError());
//             closesocket((*((SOCKET *)io->tx.txHandle)));
//             WSACleanup();
//             return -1;
//         }

//     } while (iResult > 0);

//     // shutdown the connection since we're done
//     iResult = shutdown((*((SOCKET *)io->tx.txHandle)), SD_SEND);
//     if (iResult == SOCKET_ERROR)
//     {
//         printf("shutdown failed with error: %d\n", WSAGetLastError());
//         closesocket((*((SOCKET *)io->tx.txHandle)));
//         WSACleanup();
//         return -1;
//     }

//     // cleanup
//     closesocket((*((SOCKET *)io->tx.txHandle)));
//     WSACleanup();

//     return -1;
// }

// static DWORD WINAPI socketclient(LPVOID lpParam)
// {
//     win_port_io_t *io = lpParam;
//     DWORD dwWaitResult;
//     char buffer[256];
//     int iSendResult;
//     int iResult;

//     while (1)
//     {
//         dwWaitResult = WaitForSingleObject(
//             io->tx.txReady, // event handle
//             INFINITE);      // indefinite wait

//         switch (dwWaitResult)
//         {
//         // Event object was signaled
//         case WAIT_OBJECT_0:
//             dwWaitResult = WaitForSingleObject(
//                 io->tx.bufferMutex, // handle to mutex
//                 INFINITE);          // no time-out interval

//             switch (dwWaitResult)
//             {
//             // The thread got ownership of the mutex
//             case WAIT_OBJECT_0:
//                 iResult = strlen((char *)io->tx.buffer);

//                 iSendResult = send((*((SOCKET *)io->tx.txHandle)), io->tx.buffer, iResult, 0);
//                 if (iSendResult == SOCKET_ERROR)
//                 {
//                     printf("send failed with error: %d\n", WSAGetLastError());
//                     closesocket((*((SOCKET *)io->tx.txHandle)));
//                     WSACleanup();
//                 }
//                 memset(io->tx.buffer, 0, 256);
//                 ReleaseMutex(io->tx.bufferMutex);
//                 break;

//             // The thread got ownership of an abandoned mutex
//             case WAIT_ABANDONED:
//                 printf("Wait error (%lu)\n", GetLastError());
//                 return -1;
//             }

//             break;

//         // An error occurred
//         default:
//             printf("Socket client thread error (%lu)\n", GetLastError());
//             return -1;
//         }
//     }

//     return -1;
// }

// void socket_init(win_port_t *port)
// {
//     port->rxCallback = socketserver;
//     port->txCallback = socketclient;
//     port_init(port);
// }

// int socket_read(win_port_t *port, uint8_t *buffer, int len)
// {
//     if (!port->connected)
//     {
//         return 0;
//     }
//     // If nothing has been read, or that an error was detected return 0
//     return 0;
// }

// int socket_write(win_port_t *port, uint8_t *buffer, int len)
// {
//     if (!port->connected)
//     {
//         return 0;
//     }
// }

// int socket_available(win_port_t *port)
// {
//     if (!port->connected)
//     {
//         return 0;
//     }
//     SOCKET *client = port->client;
//     client->
// }

// void* socketinit(void* p){
//     win_port_t *port = (win_port_t *)p;

//     WSADATA wsaData;
//     int iResult;

//     SOCKET ListenSocket = INVALID_SOCKET;

//     struct addrinfo *result = NULL;
//     struct addrinfo hints;

//     int iSendResult;

//     // Initialize Winsock
//     iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//     if (iResult != 0)
//     {
//         printf("WSAStartup failed with error: %d\n", iResult);
//         return -1;
//     }

//     ZeroMemory(&hints, sizeof(hints));
//     hints.ai_family = AF_INET;
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_protocol = IPPROTO_TCP;
//     hints.ai_flags = AI_PASSIVE;

//     // Resolve the server address and port
//     iResult = getaddrinfo(NULL, port->portname, &hints, &result);
//     if (iResult != 0)
//     {
//         printf("getaddrinfo failed with error: %d\n", iResult);
//         WSACleanup();
//         return -1;
//     }

//     // Create a SOCKET for connecting to server
//     ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
//     if (ListenSocket == INVALID_SOCKET)
//     {
//         printf("socket failed with error: %d\n", WSAGetLastError());
//         freeaddrinfo(result);
//         WSACleanup();
//         return -1;
//     }

//     if (!port->isclient)
//     {
//         // Setup the TCP listening socket
//         iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
//         if (iResult == SOCKET_ERROR)
//         {
//             printf("bind failed with error: %d\n", WSAGetLastError());
//             freeaddrinfo(result);
//             closesocket(ListenSocket);
//             WSACleanup();
//             return -1;
//         }
//     }
//     else
//     {
//         iResult = connect(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
//         if (iResult == SOCKET_ERROR)
//         {
//             freeaddrinfo(result);
//             closesocket(ListenSocket);
//             WSACleanup();
//             return -1;
//         }
//     }

//     freeaddrinfo(result);

//     iResult = listen(ListenSocket, SOMAXCONN);
//     if (iResult == SOCKET_ERROR)
//     {
//         printf("listen failed with error: %d\n", WSAGetLastError());
//         closesocket(ListenSocket);
//         WSACleanup();
//         return -1;
//     }

//     // Accept a client socket
//     port->client = malloc(sizeof(SOCKET));
//     (*((SOCKET *)port->client)) = accept(ListenSocket, NULL, NULL);
//     if ((*((SOCKET *)port->client)) == INVALID_SOCKET)
//     {
//         printf("accept failed with error: %d\n", WSAGetLastError());
//         closesocket(ListenSocket);
//         WSACleanup();
//         return -1;
//     }

//     // No longer need server socket
//     closesocket(ListenSocket);
//     port->connected = true;
// }

// void socket_init(win_port_t *port)
// {
//     // We're not yet connected
//     port->connected = false;

//     pthread_t id;
//     pthread_create(&id, NULL, &socketinit, port);
// }

/**
 * UART PORT
 * **/

int uart_read(win_port_t *port, uint8_t *buffer, size_t len)
{
    // Number of bytes we'll have read
    DWORD bytesRead;
    // Number of bytes we'll really ask to read
    unsigned int toRead;
    COMSTAT status;

    // Use the ClearCommError function to get status info on the Serial port
    ClearCommError(port->handle, NULL, &status);

    // Check if there is something to read
    if (status.cbInQue > 0)
    {
        // If there is we check if there is enough data to read the required number
        // of characters, if not we'll read only the available characters to prevent
        // locking of the application.
        if (status.cbInQue > len)
        {
            toRead = len;
        }
        else
        {
            toRead = status.cbInQue;
        }

        // Try to read the require number of chars, and return the number of read bytes on success
        if (ReadFile(port->handle, buffer, toRead, &bytesRead, NULL))
        {
            return bytesRead;
        }
    }

    // If nothing has been read, or that an error was detected return 0
    return 0;
}

int uart_write(win_port_t *port, uint8_t *buffer, size_t len)
{
    DWORD bytesSend;

    // Try to write the buffer on the Serial port
    if (!WriteFile(port->handle, (void *)buffer, len, &bytesSend, 0))
    {
        // In case it don't work get comm error and return false
        ClearCommError(port->handle, NULL, NULL);
        return 0;
    }

    return bytesSend;
}

int uart_available(win_port_t *port)
{
    COMSTAT status;

    // Use the ClearCommError function to get status info on the Serial port
    ClearCommError(port->handle, NULL, &status);
    return status.cbInQue;
}

void uart_init(win_port_t *port)
{
    // We're not yet connected
    port->connected = false;

    // Try to connect to the given port throuh CreateFile
    port->handle = CreateFile(port->portname,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);

    // Check if the connection was successfull
    if (port->handle == INVALID_HANDLE_VALUE)
    {
        // If not success full display an Error
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {

            // Print Error if neccessary
            printf("ERROR: Handle was not attached. Reason: %s not available.\n", port->portname);
        }
        else
        {
            printf("ERROR!!!");
        }
    }
    else
    {
        // If connected we try to set the comm parameters
        DCB dcbSerialParams = {0};

        // Try to get the current
        if (!GetCommState(port->handle, &dcbSerialParams))
        {
            // If impossible, show an error
            printf("failed to get current serial parameters!");
        }
        else
        {
            // Define serial connection parameters for the arduino board
            dcbSerialParams.BaudRate = CBR_9600;
            dcbSerialParams.ByteSize = 8;
            dcbSerialParams.StopBits = ONESTOPBIT;
            dcbSerialParams.Parity = NOPARITY;
            // Setting the DTR to Control_Enable ensures that the Arduino is properly
            // reset upon establishing a connection
            dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

            // Set the parameters and check for their proper application
            if (!SetCommState(port->handle, &dcbSerialParams))
            {
                printf("ALERT: Could not set Serial Port parameters");
            }
            else
            {
                // If everything went fine we're connected
                port->connected = true;
                // Flush any remaining characters in the buffers
                PurgeComm(port->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
                port->available_cb = uart_available;
                port->read_cb = uart_read;
                port->write_cb = uart_write;
                // We wait 2s as the arduino board will be reseting
                Sleep(2000);
            }
        }
    }
}

/**
 * NAMED PIPE
 * **/

int namedpipe_read(win_port_t *port, uint8_t *buffer, size_t len)
{
    DWORD bytesRead;
    if (port->connected && ReadFile(port->handle, buffer, len, &bytesRead, NULL))
    {
        return bytesRead;
    }

    return 0;
}

int namedpipe_write(win_port_t *port, uint8_t *buffer, size_t len)
{
    DWORD bytesSend;
    if (port->connected && WriteFile(port->handle, (void *)buffer, len, &bytesSend, 0))
    {
        return bytesSend;
    }

    return 0;
}

int namedpipe_available(win_port_t *port)
{
    DWORD bytesAvail = 0;
    if (!PeekNamedPipe(port->handle, NULL, 0, NULL, &bytesAvail, NULL))
    {
        return 0;
    }
    return bytesAvail;
}

void *pipeinit(void *p)
{
    win_port_t *port = (win_port_t *)p;
    // Try to connect to the given port throuh CreateFile
    if (!port->isclient)
    {
        port->handle = CreateNamedPipe(port->portname, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 1024, 1024, 0, NULL);
        if (port->handle == INVALID_HANDLE_VALUE)
        {
            // error
            printf("CreateNamedPipe failed: %d\n", GetLastError());
            return NULL;
        }

        if (!ConnectNamedPipe(port->handle, NULL))
        {
            printf("ConnectNamedPipe failed: %d\n", GetLastError());
            CloseHandle(port->handle);
            return NULL;
        }
    }
    else
    {
        HANDLE hPipe;
        BOOL bSuccess;
        OVERLAPPED overlapped = {0};

        // Open the named pipe
        port->handle = CreateFile(port->portname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
        if (port->handle == INVALID_HANDLE_VALUE)
        {
            // error
            printf("CreateFile failed: %d\n", GetLastError());
            CloseHandle(port->handle);
            return NULL;
        }
    }

    port->available_cb = namedpipe_available;
    port->read_cb = namedpipe_read;
    port->write_cb = namedpipe_write;
    port->connected = true;
    return NULL;
}

void namedpipe_init(win_port_t *port)
{
    // We're not yet connected
    port->connected = false;
    pthread_t id;
    pthread_create(&id, NULL, &pipeinit, port);
}

/**
 * console
 * **/

int console_read(win_port_t *port, uint8_t *buffer, size_t len)
{
    int count = 0;
    while (kbhit() && len--)
    {
        char c = (uint8_t)(getch() & 0xFF);
        if (c == '\r')
        {
            putchar('\n');
        }
        putchar(c);
        buffer[count++] = (uint8_t)c;
    }

    return count;
}

int console_write(win_port_t *port, uint8_t *buffer, size_t len)
{
    char tmp[1024];
    memset(tmp, 0, 1024);
    memcpy(tmp, buffer, len);
    printf("%s", tmp);
    return strlen(tmp);
}

int console_available(win_port_t *port)
{
    return (kbhit() ? 1 : 0);
}

void console_init(win_port_t *port)
{
    port->available_cb = console_available;
    port->read_cb = console_read;
    port->write_cb = console_write;
    port->connected = true;
}

#endif
