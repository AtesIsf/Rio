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
enum proc_state : uint8 {
    PROC_CURR  = 0, // current running proc
    PROC_READY = 1, // in ready list
    PROC_SLEEP = 2, // sleeping
    PROC_WAIT  = 3, // waiting on a semaphore
    PROC_EMPTY = 4, // unused entry
    PROC_SUSP  = 5, // suspended process
};

// Process table

// id = index
struct ProcessTableEntry {
    struct Context ctx;
    byte *stack_base; // not the top of the stack!
    byte *heap_start;
    const char *name;
    int32 priority;
    int32 n_stack_pages;
    int32 n_heap_pages;
    sem sem_id;
    enum proc_state state;
};

extern struct ProcessTableEntry g_process_table[N_PROCS];

// function prototypes

void process_table_init();

[[nodiscard]]
syscall create(void *fn, int32 priority, uint32 n_stack_pages,
                uint32 n_heap_pages, const char *name, int32 n_args, ...);

syscall resume(pid id);

syscall kill(pid id);

[[noreturn]]
void procend();

[[nodiscard]] 
int32 get_valid_pid();

// macros

#define validpid(x) (x < N_PROCS && x >= 0)

#endif
