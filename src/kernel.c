
/**
 * Entry point for the RIO kernel
 */

#include "kprint.h"
#include "schedqueue.h"

void kmain(void) {
    kputs("----------------");
    kputs(" Welcome to RIO");
    kputs("----------------");

    kputs("Entered S-mode");
    sched_queue_init();
    kputs("Initialized scheduler queue...");

	return;
}
