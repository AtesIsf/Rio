#ifndef CREATE_H
#define CREATE_H

#include "process.h"

syscall create(void *fn, int32 priority, const char *name, int32 n_args, ...);

#endif
