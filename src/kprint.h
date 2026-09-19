#ifndef KPRINT_H
#define KPRINT_H

#include "ktypes.h"

#define UART_ADDR ((unsigned char *) 0x10000000) 

status_code kputc(char c);

status_code kputs(const char *str);

#endif
