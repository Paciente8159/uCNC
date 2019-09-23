#ifndef ERROR_H
#define ERROR_H

#define OK 0

#define GCODE_BAD_NUMBER_FORMAT 1 		//an invalid number was found
#define GCODE_UNKNOWN_GCOMMAND 2		//an unknowned G command was found
#define GCODE_UNKNOWN_MCOMMAND 3		//an unknowned G command was found
#define GCODE_UNSUPORTED_COMMAND 4		//a valid (but unsuported command was found)
#define GCODE_MODAL_GROUP_VIOLATION 5	//two commands of the same modal group in the same line
#define GCODE_INVALID_WORD 6			//an invalid word char was in the command line
#define GCODE_WORD_REPEATED 7			//a word letter was repeated in the same line
#define GCODE_WORD_VALUE_ERROR 8		//the value is not valid for a given word
#define GCODE_MISSING_COMMAND 9			//no G or M command in the line
#define GCODE_INVALID_LINE_NUMBER 10	//the line number is invalid
#define GCODE_INVALID_COMMENT 11		//the comment is invalid (not closed)
#define GCODE_VALUE_NOT_INTEGER 12		//an integer value was expected but the value was float
#define GCODE_VALUE_IS_NEGATIVE 13		//a negative (invalid) word value was entered
#define GCODE_UNDEFINED_AXIS 14			//a motion command without a valid axis was issued

#endif
