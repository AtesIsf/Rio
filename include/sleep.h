#ifndef SLEEP_H
#define SLEEP_H

#include "process.h"

syscall sleep(uint64 ms);

syscall unsleep(pid id);

syscall sleep_abort(pid id);

#endif
