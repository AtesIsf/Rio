
/**
 * Entry point for the RIO kernel
 */

#include "create.h"
#include "kmain.h"
#include "process.h"
#include "schedqueue.h"
#include "scheduler.h"

/**
 * Entry point for Rio, initializes necessary structs and calls the
 * main process. Also serves as the NULL process afterwards.
 */

void __kentry(void) {
    process_table_init();
    sched_queue_init();

    // the main process will have priority 64
    resume(create(kmain, 64, "kmain", 0));
    yield();

    // this basically also becomes the NULL process
    while (true);
}
