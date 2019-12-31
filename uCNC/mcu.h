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

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "mcudefs.h"
#include "mcumap.h"

#ifndef __romstr__
	#define __romstr__
#endif

#ifndef __rom__
	#define __rom__
#endif

#ifndef rom_strcpy
	#define rom_strcpy strcpy
#endif

#ifndef rom_strncpy
	#define rom_strncpy strncpy
#endif

#ifndef rom_memcpy
	#define rom_memcpy memcpy
#endif

#ifndef rom_read_byte
	#define rom_read_byte
#endif

typedef void (*ISRVOID)();
typedef void (*ISRPORTCHANGE)(volatile uint8_t);
typedef void (*ISRCOMRX)(volatile char);

void mcu_init();

//IO functions    
//Inputs  
//returns the value of the input pins
uint16_t mcu_getInputs();
//returns the value of the critical input pins
uint8_t mcu_get_controls();
uint8_t mcu_get_limits();
uint8_t mcu_get_probe();

//outputs
//sets all step pins
void mcu_setSteps(uint8_t value);
//sets all dir pins
void mcu_setDirs(uint8_t value);
//sets all digital outputs pins
void mcu_setOutputs(uint16_t value);

void mcu_set_pwm(uint8_t pwm, uint8_t value);

//Communication functions
void mcu_putc(char c);
void mcu_puts(const char* __str);
bool mcu_is_txready();
char mcu_getc();

//RealTime
//enables all interrupts on the mcu. Must be called to enable all IRS functions
void mcu_enableInterrupts();
//disables all ISR functions
void mcu_disableInterrupts();

//convert step rate to clock cycles
void mcu_freq2clocks(float frequency, uint16_t* ticks, uint8_t* prescaller);
//starts a constant rate pulse at a given frequency. This triggers to ISR handles with an offset of MIN_PULSE_WIDTH useconds
void mcu_startStepISR(uint16_t ticks, uint8_t prescaller);
//modifies the pulse frequency
void mcu_changeStepISR(uint16_t ticks, uint8_t prescaller);
//stops the pulse 
void mcu_step_isrstop();

void mcu_delay_ms(uint16_t miliseconds);

uint8_t mcu_eeprom_getc(uint16_t address);
uint8_t mcu_eeprom_putc(uint16_t address, uint8_t value);
/*
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

///void mcu_loadDummyPayload(const char* __fmt, ...);
#endif*/


#endif
