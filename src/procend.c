/**
 * procend(): the final cleanup function for processes
 */

#include "kprint.h"
#include "scheduler.h"

void procend() {
    // TODO: kill(get_currpid())
    g_process_table[curr_pid].state = PROC_EMPTY;
    
    kprintf("procend for pid: %d\n", curr_pid);
    yield();
}

