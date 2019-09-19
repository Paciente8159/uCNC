#include <string.h>
#include "config.h"
#include "cnc.h"
//#include "board.h"
#include "structures.h"
#include "protocol.h"
#include "motion.h"


volatile uint16_t g_cnc_stepdir_pulse;
CMD_PACKET g_cnc_combuffer[COM_BUFFER_SIZE];
uint8_t g_cnc_combuffer_head;
uint8_t g_cnc_combuffer_tail;
CNC_STATE g_cnc_state;


void cnc_setup()
{
	//setup the board.
	//sets pins defaults states, configures comunications
	board_setup();
	//resets all step/dir pins to their default state
	board_setStepDirs(STEPDIR_INVERT_MASK);
	//resets all outputs pins to their default state
	board_setOutputs(OUTPUT_INVERT_MASK);
	
	board_startPulse(100);
	//board_attachOnPulse(cnc_execPulse);
	//board_attachOnPulseReset(cnc_execPulseReset);
	
	protocol_sync();
}

void cnc_execMainLoop()
{
	CMD_PACKET command;
	
	memset(&command, 0, sizeof(CMD_PACKET));
	
	while(1)
	{
		/*//dummy
		uint16_t steps[3] = {10, 50, 20};
		uint16_t stepcounter = 50;
		CNC_REPORT report;
		board_comGetPacket((uint8_t*)&command, sizeof(CNC_COMMAND));
		uint8_t crc = command.crc;
		command.crc = 0;
		if(crc != crc7(0, (uint8_t*)&command, sizeof(CNC_COMMAND)))
		{
			report.report_code = REPORT_CODE_BADCRC; //bad command crc
			report.crc = crc7(0,(uint8_t*)&report, 1);
			board_comSendPacket((uint8_t*)&report, sizeof(CNC_REPORT));
		}*/
		
		if(protocol_get_packet(&command)) //if a new command was received process it
		{
			switch(command.commandType)
			{
				case 1:
					//abort command
					//this causes ucnc to abort everything and exit the main loop.
					//it will cause the controller to do a full reset
					g_cnc_state.alarm_state |= ALARM_CMD_ABORT;
					return;
				case 2:
					//send a homing command
					//this will send a series off motion commands to the 
					
					break;
				case 3:
					//normal move
					//dummy
					//motion_rt_linear(steps, &stepcounter, 0, 100);
					break;
				case 20:
					//set outputs
					break;
				case 21:
					//set outputs on input condition
					break;
				case 31:
					//wait for input condition
				case 32:
					//wait for input condition with timeout
					break;
				case 255:
					//emergency abort
					break;
			}
		}
	}
}

/*
void CNCController::loadDummy()
{
  MACHINE_COMMAND command;
  command.commandType=1;
  command.motion.joint_steps[0]=2;
  command.motion.total_steps=4;
  if(!_cmdBuffer.isFull())
  {
    _cmdBuffer.enqueue(&command);
  }
}

void CNCController::nonRealTimeLoop()
{
    MACHINE_COMMAND command;
    MACHINE_STATE state;
    unsigned char bitindex = 7;

    //loops for commands
    if(_machine._board->comGetPacket(_serialBuffer, sizeof(MACHINE_COMMAND)) != 0)
    {
        buffer2MachineCommand(_serialBuffer, &command);

        switch(command.commandType)
        {
            case 0x01:
                _cmdBuffer.enqueue(&command);
                break;
            case 0x02:
                // _machine.getInputs();
                // _machine.getAnalogInputs();
                // _machine.getState(&state);
                // machineReport2Buffer(&state, _serialBuffer);
                // Board.comSendPacket(_serialBuffer, sizeof(MACHINE_REPORT));
                break;
            case 0x80:
                _abort = true;
                break;
            default:
                //bad command
                break;
        }
    }
}

bool CNCController::getKillState()
{
	return _abort;
}*/

void cnc_execPulseReset()
{
    static uint8_t tick_counter = 0;
    static uint16_t remaining_steps = 0;
    static uint16_t joint_counter[5];
    static uint16_t error;
    static CMD_PACKET* runCommand;
    static uint8_t criticalMask = 0xFF;
    uint8_t i = TOTAL_STEPPERS;
    uint8_t critical;

	//resets all step/dir pins to their default state
	board_setStepDirs(STEPDIR_INVERT_MASK);
	
	//checks critical inputs state
	critical = board_getCriticalInputs() | g_cnc_state.alarm_state;
	
    //on emergency stop or abort command kill everything
    if(critical != 0)
    {
    	if(critical & 0x01)
		{
			board_setStepDirs(0);
		}
    	return;
	}
	/*
	
    tick_counter--;
    if (tick_counter == 0)
    {
    	if(!runCommand)
    		return;
    	//reset tick counter
    	tick_counter = 1;//runCommand->motion.ticks_per_step;
        //execute step on all joints
        uint8_t stepbits = 0;
        while (i--)
        {
            joint_counter[i] += runCommand->motion.joint_steps[i];
            if (joint_counter[i] > runCommand->motion.total_steps)
            {
                joint_counter[i] -= runCommand->motion.total_steps;
                stepbits |= (1 << i);
            }
        }
        
        //set step pulses and propagates to the board 
        _machine.setJoints(stepbits);
        _machine.writeOutputs();
        
        //reset tick counter
        //tick_counter = runCommand->motion.ticks_per_step;
        remaining_steps--;
        if(remaining_steps==0)
        {
            _cmdBuffer.dequeue();
            runCommand = NULL;
        }

        //reset step pins to their default state
        _machine.setJoints(0);
        _machine.writeOutputs();
        if (remaining_steps!=0)
        {
        	return;
		}
    }
    
    //check if is running motion command. If not tries to load motion command from buffer
    //no running command (check if there is a command to execute)
    if (_cmdBuffer.isEmpty())
    {
        return;
    }

    runCommand = _cmdBuffer.unsafePeek();
    criticalMask = runCommand->motion.criticalInputMask;
    //load bresenham algorithm values
    remaining_steps = runCommand->motion.total_steps;
    tick_counter = 1;//runCommand->motion.ticks_per_step;
    error = (runCommand->motion.total_steps >> 1);
    while (i--)
    {
        joint_counter[i] = error;
    }
    _machine.setOutputs(runCommand->motion.digitalOutputs);
    _machine.setDir(runCommand->motion.joint_dir);
    _machine.writeOutputs();*/
}

void cnc_execPulse()
{
	board_setStepDirs(g_cnc_stepdir_pulse);
}
/*
bool CNCController::bufferEmpty()
{
    return _cmdBuffer.isEmpty();
}

void CNCController::buffer2MachineCommand(char *str, MACHINE_COMMAND *command)
{
    memcpy(command, str, sizeof(MACHINE_COMMAND));
}

void CNCController::machineReport2Buffer(MACHINE_STATE *state, char *str)
{
    memcpy(str, state, sizeof(MACHINE_STATE));
}*/
