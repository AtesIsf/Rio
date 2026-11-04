#include <rio.h>

int32 g_tests_passed = 0;
int32 g_defects_found = 0;

extern process test_lobby(void);
extern process test_kprint(void);
extern process test_interrupt(void);
extern process test_memory(void);
extern process test_process(void);
extern process test_scheduler(void);
extern process test_semaphore(void);
extern process test_timed(void);
extern process test_integration(void);

static void run_suite(void *fn, const char *name) {
    pid p = create(fn, 70, name, 0);
    if (validpid(p)) {
        resume(p);
    } else {
        kprint("FAIL: could not spawn test process ");
        kputs(name);
    }
}

process kmain(void) {
    kputs("============================");
    kputs("    RIO KERNEL TEST SUITE   ");
    kputs("============================");

    kputs("\n[+] Subsystem Unit Test Processes:");
    run_suite(test_lobby, "test_lobby");
    run_suite(test_kprint, "test_kprint");
    run_suite(test_interrupt, "test_interrupt");
    run_suite(test_memory, "test_memory");
    run_suite(test_process, "test_process");
    run_suite(test_scheduler, "test_scheduler");
    run_suite(test_semaphore, "test_semaphore");
    run_suite(test_timed, "test_timed");

    kputs("\n[+] Cross-Subsystem Integration Test Process:");
    run_suite(test_integration, "test_integration");

    kputs("\n============================");
    kputs("    TEST RESULT SUMMARY    ");
    kputs("============================");
    kprintf("  Verified Invariants:\t%d\n", g_tests_passed);
    kprintf("  Defects Exposed:\t%d\n", g_defects_found);
    kputs("============================");

    // QEMU test finisher poweroff (success)
    *(volatile uint32 *)0x100000 = 0x5555;
    return OK;
}
