#ifndef KPRINT_H
#define KPRINT_H

#include <stdarg.h>
#include "ktypes.h"

#define UART_ADDR ((unsigned char *) 0x10000000) 

status_code kputc(char c);

status_code kprint(const char *str);

status_code kputs(const char *str);

void kvprintf(const char *format, va_list arg);

void kprintf(const char *format, ...);

#endif
