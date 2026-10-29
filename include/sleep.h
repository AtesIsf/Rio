#ifndef SLEEP_H
#define SLEEP_H

#include "process.h"

syscall sleep(uint64 ms);

syscall unsleep(pid id);

#endif
