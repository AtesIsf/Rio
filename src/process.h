#ifndef PROCESS_H
#define PROCESS_H

#include "ktypes.h"
#include "hardwaredef.h"

// constants

#ifndef N_PROCS
#define N_PROCS (64)
#endif

typedef status_code process;
typedef status_code syscall;

// process states
#define PROC_CURR   (0) // current running proc
#define PROC_READY  (1) // in ready list
#define PROC_SLEEP  (2) // sleeping
#define PROC_WAIT   (3) // waiting on a semaphore
#define PROC_EMPTY  (4) // unused entry
#define PROC_SUSP   (5) // suspended process

// Process table

// id = index
struct ProcessTableEntry {
    struct Context ctx;
    byte *stack_base;
    const char *name;
    int32 priority;
    byte state;
};

extern struct ProcessTableEntry g_process_table[N_PROCS];

// function prototypes

void process_table_init();

int32 get_valid_pid();

process null_process();

// macros

#define validpid(x) (x < N_PROCS && x >= 0)

#endif
