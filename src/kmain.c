
/**
 * kmain is the main process created by the entry point. It is basically
 * a main function for the kernel, but running as a process as well.
 */

#include "rio.h"

process A() {
    while (true) {
        kputc('A');
    }
    return OK;
}

process B() {
    while (true) {
        kputc('B');
    }
    return OK;
}

process kmain() {
    kputs("----------------");
    kputs(" Welcome to RIO ");
    kputs("----------------");

    resume(create(A, 10, "A", 0));
    resume(create(B, 10, "B", 0));

    return OK;
}
