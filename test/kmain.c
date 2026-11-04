#include <rio.h>

extern status_code test_lobby(void);
extern status_code test_kprint(void);
extern status_code test_memory(void);
extern status_code test_process(void);
extern status_code test_scheduler(void);
extern status_code test_semaphore(void);
extern status_code test_timed(void);
extern status_code test_integration(void);

process kmain(void) {
    kputs("================================");
    kputs("      RIO KERNEL TEST SUITE     ");
    kputs("================================");

    int32 failed_suites = 0;

    kputs("\n[+] Subsystem Unit Tests:");
    if (test_lobby() != OK) failed_suites++;
    if (test_kprint() != OK) failed_suites++;
    if (test_memory() != OK) failed_suites++;
    if (test_process() != OK) failed_suites++;
    if (test_scheduler() != OK) failed_suites++;
    if (test_semaphore() != OK) failed_suites++;
    if (test_timed() != OK) failed_suites++;

    kputs("\n[+] Cross-Subsystem Integration Tests:");
    if (test_integration() != OK) failed_suites++;

    kputs("\n================================");
    if (failed_suites == 0) {
        kputs("  ALL TESTS PASSED");
        kputs("================================");
        // Clean QEMU exit (pass)
        *(volatile uint32 *)0x100000 = 0x5555;
    } else {
        kputs("  SOME TESTS FAILED");
        kputs("================================");
        // Clean QEMU exit (fail code 1)
        *(volatile uint32 *)0x100000 = (1 << 16) | 0x3333;
    }

    return OK;
}
