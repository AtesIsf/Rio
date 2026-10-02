
/**
 * Entry point for the RIO kernel
 */

#include "kprint.h"
#include "schedqueue.h"
#include "process.h"

void kmain(void) {
    kputs("----------------");
    kputs(" Welcome to RIO");
    kputs("----------------");

    kputs("Entered S-mode");

    process_table_init();
    kputs("Initialized process table...");

    sched_queue_init();
    kputs("Initialized scheduler queue...");

	return;
}
