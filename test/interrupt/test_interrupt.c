#include "check.h"

static bool sie(void) {
    return (read_sstatus() & SSTATUS_SIE) != 0;
}

static void nesting(void) {
    intmask outer = interrupt_save();
    bool off1 = !sie();
    intmask inner = interrupt_save();
    bool off2 = !sie();
    interrupt_restore(inner);
    bool off3 = !sie();
    interrupt_restore(outer);
    bool on = sie();

    CHECK("sie_save_restore_nesting", off1 && off2 && off3 && on);
}

// Every early-return path must give back the SIE state it found.
static bool errors_keep(bool want) {
    sem s = -1;
    bool ok = true;

#define STEP(call) do { ok &= ((call) == ERR) && (sie() == want); } while (0)
    STEP(kalloc(NULL, 1));
    STEP(kfree(NULL, 1));
    STEP(create(NULL, 10, 1, 1, "x", 0));
    STEP(resume(-1));
    STEP(kill(-1));
    STEP(scheduler_add(-1));
    STEP(scheduler_remove(-1));
    STEP(sem_create(-1, &s));
    STEP(sem_wait(-1));
    STEP(sem_release(-1));
    STEP(sem_free(-1));
    STEP(sem_abort(-1, 0));
    STEP(sleep_abort(-1));
    STEP(unsleep(-1));
#undef STEP
    return ok;
}

static void errors_with_sie_on(void) {
    bool start_on = sie();
    bool ok = errors_keep(true);
    CHECK("syscall_errors_keep_sie_on", start_on && ok);
}

static void errors_with_sie_off(void) {
    intmask mask = interrupt_save();
    bool ok = errors_keep(false);
    interrupt_restore(mask);
    CHECK("syscall_errors_keep_sie_off", ok);
}

void test_interrupt(void) {
    nesting();
    errors_with_sie_on();
    errors_with_sie_off();
}
