#include "check.h"

static void rejects_nullptr(void) {
    // len 0: a missing check with len >= 1 would store to VA 0 and hang
    CHECK("kmemset_rejects_nullptr", kmemset(nullptr, 0xA5, 0) == ERR);
}

static void len_zero_writes_nothing(void) {
    byte b = 0x5A;
    status_code r = kmemset(&b, 0xA5, 0);

    CHECK("kmemset_len_zero_writes_nothing", r == OK && b == 0x5A);
}

static void fills_exact_range(void) {
    byte *p = kalloc(1);
    bool ok = p != nullptr;
    if (ok) {
        ok &= kmemset(p, 0x5A, PAGE_SIZE) == OK;
        ok &= kmemset(p + 1, 0xA5, PAGE_SIZE - 2) == OK;
        ok &= p[0] == 0x5A && p[PAGE_SIZE - 1] == 0x5A;
        for (int32 i = 1; i < PAGE_SIZE - 1; i++) {
            ok &= p[i] == 0xA5;
        }
    }
    kfree(p, 1);

    CHECK("kmemset_fills_exact_range", ok);
}

void test_klib(void) {
    rejects_nullptr();
    len_zero_writes_nothing();
    fills_exact_range();
}
