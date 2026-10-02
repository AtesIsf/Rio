#include "process.h"

/**
 * process_table_init() must be called on startup!
 */

struct ProcessTableEntry g_process_table[N_PROCS];

void process_table_init() {
    // maybe can remove this line later
    g_process_table[0].state = PROC_READY;
    g_process_table[0].name = "NULL";

    // skip 0 -> null process
    for (int32 i = 1; i < N_PROCS; i++) {
        g_process_table[i].state = PROC_EMPTY;
        g_process_table[i].name = "";
    }
}

process null_process() {
    while (true);
    return OK;
}

