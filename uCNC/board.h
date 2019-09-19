/*H**********************************************************************
* FILENAME :        fmcompres.c             DESIGN REF: FMCM00
*
* DESCRIPTION :
*       Board basic functions for the board controller.
*
* PUBLIC FUNCTIONS :
*       int     FM_CompressFile( FileHandle )
*       int     FM_DecompressFile( FileHandle )
*
* NOTES :
*       These functions are a part of the FM suite;
*       See IMS FM0121 for detailed description.
*
*       Copyright A.N.Other Co. 1990, 1995.  All rights reserved.
*
* AUTHOR :    Arthur Other        START DATE :    16 Jan 99
*
* CHANGES :
*
* REF NO  VERSION DATE    WHO     DETAIL
* F21/33  A.03.04 22Jan99 JR      Function CalcHuffman corrected
*
*H*/

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

typedef void (*ISRTIMER)();
typedef void (*ISRPINCHANGE)(volatile uint32_t*);
typedef void (*ISRCOMRX)(volatile char*);

void board_setup();

//IO functions    
//Inputs  
//returns the value of the input pins
uint16_t board_getInputs();
//returns the value of the critical input pins
uint8_t board_getCriticalInputs();
//attaches a function handle to the input pin changed ISR
void board_attachOnInputChange(ISRPINCHANGE handler);
//detaches the input pin changed ISR
void board_detachOnInputChange();

//outputs
//sets all step and dir pins
void board_setStepDirs(uint16_t value);
//sets all digital outputs pins
void board_setOutputs(uint16_t value);

//Communication functions
//sends a packet
void board_comSendPacket(uint8_t *ptr, uint8_t length);
//receives a packet
uint8_t board_comGetPacket(uint8_t *ptr, uint8_t length);
int16_t board_comPeek();
void board_comClear();

//RealTime
//enables all interrupts on the board. Must be called to enable all IRS functions
void board_enableInterrupts();
//disables all ISR functions
void board_disableInterrupts();

//starts a constant rate pulse at a given frequency. This triggers to ISR handles with an offset of MIN_PULSE_WIDTH useconds
void board_startPulse(uint32_t frequency);
//stops the pulse 
void board_stopPulse();
//attaches a function handle to the pulse ISR
void board_attachOnPulse(ISRTIMER handler);
void board_detachOnPulse();
//attaches a function handle to the reset pulse ISR. This is fired MIN_PULSE_WIDTH useconds after pulse ISR
void board_attachOnPulseReset(ISRTIMER handler);
void board_detachOnPulseReset();

uint8_t board_readProMemByte(uint8_t* src);

//measure performance
#ifdef DEBUGMODE
typedef struct {
	uint16_t pulseCounter;
	uint16_t resetPulseCounter;
	uint16_t pinChangeCounter;
} PERFORMANCE_METER;

extern volatile PERFORMANCE_METER board_performacecounters;
void board_startPerfCounter();
uint16_t board_stopPerfCounter();
#endif


#endif
