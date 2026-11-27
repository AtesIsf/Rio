#include <rio.h>

void panic(const char *msg) {
    (void) interrupt_save(); // never restored: panic doesn't return

    kputs("<---- KERNEL PANIC ---->");
    if (msg != nullptr) {
        kprintf(">> Description: %s\n", msg);
    }

    while (true);
}

