
/**
 * Entry point for the RIO kernel
 */

#include "create.h"
#include "kprint.h"
#include "schedqueue.h"
#include "process.h"
#include "scheduler.h"

process A() {
    for (int32 i = 0; i < 10; i++) {
        kputc('A');
        yield();
    }
    return OK;
}

process B() {
    for (int32 i = 0; i < 10; i++) {
        kputc('B');
        yield();
    }
    return OK;
}

void kmain(void) {
    kputs("----------------");
    kputs(" Welcome to RIO");
    kputs("----------------");

    kputs("Entered S-mode");

    process_table_init();
    kputs("Initialized process table...");

    sched_queue_init();
    kputs("Initialized scheduler queue...");

    resume(create(A, 10, "A", 0));
    resume(create(B, 10, "B", 0));

    yield();
	return;
}
