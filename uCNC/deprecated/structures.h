#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdlib.h>
#include <stdint.h>

typedef struct {
	uint8_t criticalInputMask;
    uint8_t dirs;
    uint16_t steps[6];
    uint16_t total_steps;
    uint16_t feedrate;
} LINEAR_MOTION;

typedef struct {
	uint16_t inputs;
	uint16_t inputs_mask;
    uint16_t outputs;
    uint16_t outputs_mask;
    uint8_t analog_in[3];
    uint8_t analog_out[3];
    uint32_t event_timeout;
} IO_ACTION;

typedef union {
	LINEAR_MOTION lin;
	IO_ACTION io;
} COMMAND_DATA;

typedef struct {
    uint8_t commandType;
    COMMAND_DATA data;
    uint8_t crc;
} CMD_PACKET;

#define REPORT_CODE_BADCRC 255

typedef struct {
	uint8_t report_code;
	uint8_t system_state;
    uint32_t steppos[5];
    uint8_t critical_inputs;
    uint8_t analog_inputs[5];
    uint16_t digital_inputs;
    uint8_t crc;
} CNC_REPORT;

#define ALARM_CMD_ABORT 1

//holds the current machine states
typedef struct {
	uint8_t alarm_state;
	uint8_t system_state;
	uint8_t dirs; 			//joints direction bits
    uint32_t step_pos[5]; 		//joints actual positions in steps
    uint8_t critical_inputs; 	//critical inputs bits
    uint8_t analog_inputs[3]; 	//analog inputs values
    uint8_t analog_outputs[3]; 	//pwm outputs values
    uint16_t digital_inputs;		//digital input pin bits
    uint16_t digital_outputs;	//digital outputs pin bits
} CNC_STATE;

typedef struct {
	uint16_t stepdirs;
	uint16_t next_freq;
} MOTION_COMMAND;

uint8_t crc7 (uint8_t  crc, uint8_t *pc, int len);

#endif
