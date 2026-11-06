#include <rio.h>

syscall panic(const char *msg) {
    intmask mask = interrupt_save();

    kputs("<---- KERNEL PANIC ---->");
    if (msg != NULL) {
        kprintf(">> Description: %s\n", msg);
    }

    while (true);

    interrupt_restore(mask);
    return OK;
}

