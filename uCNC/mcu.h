/*H**********************************************************************
* FILENAME :        fmcompres.c             DESIGN REF: FMCM00
*
* DESCRIPTION :
*       Board basic functions for the mcu controller.
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

#ifndef MCU_H
#define MCU_H

#include "mcudefs.h"

#ifndef PSTR
	#define PSTR(str) str
#endif

#include <stdint.h>

typedef void (*ISRVOID)();
typedef void (*ISRPINCHANGE)(volatile uint32_t*);
typedef void (*ISRCOMRX)(volatile char);

void mcu_init();

//IO functions    
//Inputs  
//returns the value of the input pins
uint16_t mcu_getInputs();
//returns the value of the critical input pins
uint8_t mcu_getControls();
uint8_t mcu_getLimits();
uint8_t mcu_getProbe();
//attaches a function handle to the input pin changed ISR
void mcu_attachOnInputChange(ISRPINCHANGE handler);
//detaches the input pin changed ISR
void mcu_detachOnInputChange();

//outputs
//sets all step pins
void mcu_setSteps(uint8_t value);
//sets all dir pins
void mcu_setDirs(uint8_t value);
//sets all digital outputs pins
void mcu_setOutputs(uint16_t value);

//Communication functions
void mcu_putc(char c);
char mcu_getc();
char mcu_peek();
void mcu_bufferClear();
void mcu_attachOnReadChar(ISRCOMRX handler);
void mcu_detachOnReadChar();
void mcu_attachOnSentChar(ISRVOID handler);
void mcu_detachOnSentChar();

//RealTime
//enables all interrupts on the mcu. Must be called to enable all IRS functions
void mcu_enableInterrupts();
//disables all ISR functions
void mcu_disableInterrupts();

//
void mcu_freq2clocks(float frequency, uint16_t* ticks, uint8_t* prescaller);
//starts a constant rate pulse at a given frequency. This triggers to ISR handles with an offset of MIN_PULSE_WIDTH useconds
void mcu_startStepISR(uint16_t ticks, uint8_t prescaller);
//modifies the pulse frequency
void mcu_changeStepISR(uint16_t ticks, uint8_t prescaller);
//stops the pulse 
void mcu_stopStepISR();
//attaches a function handle to the pulse ISR
void mcu_attachOnStep(ISRVOID handler);
void mcu_detachOnStep();
//attaches a function handle to the reset pulse ISR. This is fired MIN_PULSE_WIDTH useconds after pulse ISR
void mcu_attachOnStepReset(ISRVOID handler);
void mcu_detachOnStepReset();
/*
//starts a constant rate integrator at a given frequency.
void mcu_startIntegrator();
//stops the pulse 
void mcu_stopIntegrator();
//suspends the integrator
void mcu_pauseIntegrator();
//resumes the integrator
void mcu_resumeIntegrator();
//attaches a function handle to the integrator ISR
void mcu_attachOnIntegrator(ISRTIMER handler);
void mcu_detachOnIntegrator();
*/
char* mcu_strcpyProgMem(char* __s, const char* __fmt);
void mcu_printfp(const char* __fmt, ...);
uint8_t mcu_readProgMemByte(uint8_t* src);
uint8_t mcu_eeprom_getc(uint16_t address);
uint8_t mcu_eeprom_putc(uint16_t address, uint8_t value);

//measure performance
#ifdef __DEBUG__
typedef struct {
	uint16_t pulseCounter;
	uint16_t resetPulseCounter;
	uint16_t integratorCounter;
	uint16_t pinChangeCounter;
} PERFORMANCE_METER;

extern volatile PERFORMANCE_METER mcu_performacecounters;
void mcu_startTickCounter();
uint32_t mcu_getCycles();
uint32_t mcu_getElapsedCycles(uint32_t cycle_ref);
//void uint32_t tickcount = mcu_getCycles();
//uint16_t mcu_stopPerfCounter();

void mcu_loadDummyPayload(const char* __fmt, ...);
#endif


#endif
