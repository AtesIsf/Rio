#ifndef PROCESS_H
#define PROCESS_H

#include "ktypes.h"

// constants

#ifndef N_PROCS
#define N_PROCS (64)
#endif

typedef status_code process;
typedef status_code syscall;

// process states
#define PROC_CURR (0)
#define PROC_READY (1)
#define PROC_SLEEP (2)
#define PROC_WAIT (3)

// Process table

// macros

#define validpid(x) (x < N_PROCS && x >= 0)

#endif
