#ifndef REPORT_H
#define REPORT_H

#include <stdint.h>
#define REPORT_BUFFER_SIZE 128

void report_error(uint8_t code);
void report_msg(const char* s);
void report_formatmsg(const char* format,...);

#endif
