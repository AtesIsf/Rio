#include "check.h"

int32 g_pass = 0;
int32 g_fail = 0;

// QEMU finisher: exit status is value >> 16, so failure needs bit 16 set
#define FINISHER ((volatile uint32 *) 0x100000)
#define FINISH_PASS (0x5555)
#define FINISH_FAIL ((1 << 16) | 0x3333)

process kmain(void) {
    kputs("============================");
    kputs("    RIO KERNEL TEST SUITE   ");
    kputs("============================");

    kputs("\n[+] Subsystem Unit Tests:");

    test_lobby();
    test_interrupt();
    test_process();
    test_scheduler();
    test_semaphore();
    test_timed();
    test_memory();
    test_kprint(); // last: may crash the kernel

    kputs("\n============================");
    kputs("    TEST RESULT SUMMARY    ");
    kputs("============================");
    kprintf("  Verified Invariants:\t%d\n", g_pass);
    kprintf("  Defects Exposed:\t%d\n", g_fail);
    kputs("============================");

    *FINISHER = g_fail ? FINISH_FAIL : FINISH_PASS;
    return OK;
}
