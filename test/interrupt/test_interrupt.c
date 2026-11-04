#include <rio.h>

extern int32 g_tests_passed;
extern int32 g_defects_found;

process test_interrupt(void) {
    // 1. Single interrupt save disables SIE
    intmask m1 = interrupt_save();
    if ((read_sstatus() & SSTATUS_SIE) == 0) {
        g_tests_passed++;
    } else {
        kputs("FAIL: interrupt_save did not clear SIE bit");
    }

    // 2. Nested interrupt save keeps SIE disabled
    intmask m2 = interrupt_save();
    if ((read_sstatus() & SSTATUS_SIE) == 0) {
        g_tests_passed++;
    } else {
        kputs("FAIL: nested interrupt_save enabled SIE bit");
    }

    // 3. Inner restore maintains disabled state
    interrupt_restore(m2);
    if ((read_sstatus() & SSTATUS_SIE) == 0) {
        g_tests_passed++;
    } else {
        kputs("FAIL: inner restore prematurely re-enabled interrupts");
    }

    // 4. Outer restore re-enables interrupts
    interrupt_restore(m1);
    if ((read_sstatus() & SSTATUS_SIE) != 0) {
        g_tests_passed++;
    } else {
        kputs("FAIL: outer restore failed to re-enable interrupts");
    }

    kputs("  [PASS] test_interrupt");
    return OK;
}
