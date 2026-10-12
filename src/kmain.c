
/**
 * kmain is the main process created by the entry point. It is basically
 * a main function for the kernel, but running as a process as well.
 */

#include "kprint.h"
#include "process.h"
#include "scheduler.h"

process A() {
    kputs("hello from process A");
    return OK;
}

process B() {
    kputs("hello from process B");
    return OK;
}

process C() {
    for (int32 i = 0; i < 5; i++) {
        kputs("C");
        yield();
    }
    return OK;
}

process D() {
    for (int32 i = 0; i < 5; i++) {
        kputs("D");
        yield();
    }
    return OK;
}

process E() {
    for (int32 i = 0; i < 5; i++) {
        kputs("E");
        yield();
    }
    return OK;
}

process kmain() {
    kputs("----------------");
    kputs(" Welcome to RIO ");
    kputs("----------------");

    resume(create(A, 20, "A", 0));
    resume(create(B, 10, "B", 0));
    resume(create(C, 5, "C", 0));
    resume(create(D, 5, "D", 0));
    resume(create(E, 2, "E", 0));

    return OK;
}
