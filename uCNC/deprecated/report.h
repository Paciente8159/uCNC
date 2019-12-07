/*
	Name: report.h - implementation of a grbl compatible send-response protocol
	Copyright: 2019 João Martins
	Author: João Martins
	Date: Nov/2019
	Description: uCNC is a free cnc controller software designed to be flexible and
	portable to several	microcontrollers/architectures.
	uCNC is a FREE SOFTWARE under the terms of the GPLv3 (see <http://www.gnu.org/licenses/>).
*/

#ifndef REPORT_H
#define REPORT_H

#include <stdint.h>
#define REPORT_BUFFER_SIZE 128

void report_error(uint8_t code);
void report_msg(const char* s);
void report_formatmsg(const char* format,...);

#endif
