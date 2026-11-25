#include "check.h"

extern byte kernel_end;

// same formula as mem_init: the first managed page holds the bitmap
#define META (PAGE_ROUND_UP((uint64) &kernel_end))

static bool overlaps(byte *a, int32 na, byte *b, int32 nb) {
    return a < b + nb * PAGE_SIZE && b < a + na * PAGE_SIZE;
}

static void bad_args(void) {
    byte *p = NULL;
    byte *a = NULL;
    kalloc(&a, 1);

    bool ok = kalloc(NULL, 1) == ERR;
    ok &= kalloc(&p, 0) == ERR;
    ok &= kalloc(&p, -1) == ERR;
    ok &= kfree(NULL, 1) == ERR;
    ok &= kfree(a, 0) == ERR;
    ok &= kfree(a, -1) == ERR;
    bool freed = kfree(a, 1) == OK;

    CHECK("mem_rejects_bad_args", ok && freed);
}

static void alloc_aligned_disjoint(void) {
    byte *a = NULL, *b = NULL, *c = NULL;
    status_code ra = kalloc(&a, 1);
    status_code rb = kalloc(&b, 3);
    status_code rc = kalloc(&c, 1);

    bool ok = ra == OK && rb == OK && rc == OK;
    byte *blk[3] = { a, b, c };
    int32 len[3] = { 1, 3, 1 };
    for (int32 i = 0; i < 3; i++) {
        ok &= (uint64) blk[i] % PAGE_SIZE == 0;
        ok &= (uint64) blk[i] >= META + PAGE_SIZE;
        ok &= (uint64) blk[i] + len[i] * PAGE_SIZE <= RAM_END;
        for (int32 j = i + 1; j < 3; j++) {
            ok &= !overlaps(blk[i], len[i], blk[j], len[j]);
        }
    }
    kfree(a, 1);
    kfree(b, 3);
    kfree(c, 1);

    CHECK("kalloc_aligned_in_ram_disjoint", ok);
}

static void double_free(void) {
    byte *a = NULL;
    kalloc(&a, 1);

    status_code f1 = kfree(a, 1);
    status_code f2 = kfree(a, 1);

    CHECK("kfree_double_free_rejected", f1 == OK && f2 == ERR);
}

static void partial_overlap(void) {
    byte *a = NULL;
    kalloc(&a, 2);

    status_code tail = kfree(a + PAGE_SIZE, 1);
    status_code both = kfree(a, 2);
    status_code head = kfree(a, 1);

    CHECK("kfree_partial_overlap_atomic",
          tail == OK && both == ERR && head == OK);
}

static void outside_ram(void) {
    status_code end = kfree((byte *) RAM_END, 1);
    status_code base = kfree((byte *) KERNEL_BASE, 1);

    CHECK("kfree_rejects_outside_ram", end == ERR && base == ERR);
}

static void metadata_page(void) {
    byte *q = NULL;
    intmask mask = interrupt_save();
    status_code r = kfree((byte *) META, 1);
    if (r == OK) {
        kalloc(&q, 1); // first-fit re-marks the metadata page; never use q
    }
    interrupt_restore(mask);

    CHECK("kfree_rejects_metadata_page", r == ERR);
}

static void oversize(void) {
    byte *p = NULL;
    status_code r = kalloc(&p, RAM_SIZE / PAGE_SIZE + 1);
    if (r == OK) {
        kfree(p, (RAM_END - (uint64) p) / PAGE_SIZE);
    }

    CHECK("kalloc_oversize_fails", r == ERR);
}

void test_memory(void) {
    bad_args();
    alloc_aligned_disjoint();
    double_free();
    partial_overlap();
    outside_ram();
    metadata_page();
    oversize();
}
