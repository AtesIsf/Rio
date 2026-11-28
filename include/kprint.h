#ifndef KPRINT_H
#define KPRINT_H

#include <ktypes.h>

#include <stdarg.h>

status_code kputc(char c);

status_code kprint(const char *str);

status_code kputs(const char *str);

void kvprintf(const char *format, va_list arg);

void kprintf(const char *format, ...);

#endif
