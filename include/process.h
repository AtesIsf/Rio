#ifndef PROCESS_H
#define PROCESS_H

#include <ktypes.h>
#include <hardwaredef.h>

// constants

#ifndef N_PROCS
#define N_PROCS (64)
#endif

// always set to the current process's id
extern pid curr_pid;

typedef status_code process;
typedef status_code syscall;

#define PROC_NOSEM (-1) // for empty sem_id in the process table

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
    int32 n_stack_pages;
    sem sem_id;
    byte state;
};

extern struct ProcessTableEntry g_process_table[N_PROCS];

// function prototypes

void process_table_init();

syscall create(void *fn, int32 priority, uint32 n_stack_pages,
                const char *name, int32 n_args, ...);

syscall resume(pid id);

syscall kill(pid id);

void procend();

int32 get_valid_pid();

// macros

#define validpid(x) (x < N_PROCS && x >= 0)

#endif
