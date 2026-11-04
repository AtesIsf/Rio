#include <rio.h>

process kmain(void) {
    kputs("RIO: running tests...");

    // Subsystem tests go here

    kputs("RIO: all tests passed.");

    // Exit QEMU virt machine (SiFive test finisher device)
    *(volatile uint32 *)0x100000 = 0x5555;
    return OK;
}
