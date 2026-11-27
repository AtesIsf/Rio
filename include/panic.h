#ifndef PANIC_H
#define PANIC_H

#include <process.h>

[[noreturn]]
void panic(const char *);

#endif
