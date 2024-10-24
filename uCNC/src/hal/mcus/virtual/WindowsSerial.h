#if (MCU == MCU_VIRTUAL_WIN)
#ifndef ARDUINO

#ifndef SERIALCLASS_H_INCLUDED
#define SERIALCLASS_H_INCLUDED

#define ARDUINO_WAIT_TIME 2000

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <Windows.h>

class WindowsSerial
{
    private:
        //Serial comm handler
        HANDLE hSerial;
        //Connection status
        bool connected;
        //Get various information about the connection
        COMSTAT status;
        //Keep track of last error
        DWORD errors;

    public:
        //Initialize Serial communication with the given COM port
        WindowsSerial(const char *portName);
        //Close the connection
        ~WindowsSerial();
        //Read data in a buffer, if nbChar is greater than the
        //maximum number of bytes available, it will return only the
        //bytes available. The function return -1 when nothing could
        //be read, the number of bytes actually read.
        int ReadData(char *buffer, unsigned int nbChar);
        //Writes data from a buffer through the Serial connection
        //return true on success.
        bool WriteData(const uint8_t *buffer, unsigned int nbChar);
        //Check if we are actually connected
        bool IsConnected();


};

#endif // SERIALCLASS_H_INCLUDED
#endif
#endif
