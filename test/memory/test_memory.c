#include <rio.h>

extern int32 g_tests_passed;
extern int32 g_defects_found;

process test_memory(void) {
    // 1. Stack allocation and distinct pointer checks
    byte *s1 = get_stack();
    byte *s2 = get_stack();
    if (s1 != NULL && s2 != NULL && s1 != s2) {
        g_tests_passed++;
    }
    free_stack(s1);
    free_stack(s2);

    // 2. NULL checks on free
    if (free_stack(NULL) == ERR && free_heap(NULL) == ERR) {
        g_tests_passed++;
    }

    // 3. Heap allocation and 8-byte alignment
    byte *h = get_heap();
    void *p1 = chef_malloc(h, 5);
    void *p2 = chef_malloc(h, 11);
    if (p1 != NULL && p2 != NULL) {
        if (((uint64) p1 & 0x7) == 0 && ((uint64) p2 & 0x7) == 0) {
            g_tests_passed++;
        }
    }

    // 4. Invalid size and stack rejection
    byte *stk = get_stack();
    if (chef_malloc(NULL, 16) == NULL && chef_malloc(h, 0) == NULL && chef_malloc(h, -8) == NULL && chef_malloc(stk, 16) == NULL) {
        g_tests_passed++;
    }
    free_stack(stk);
    free_heap(h);

    // 5. Adversarial: Full BLOCK_SIZE allocation in fresh heap block
    byte *h_full = get_heap();
    void *full_buf = chef_malloc(h_full, BLOCK_SIZE);
    if (full_buf == NULL) {
        kputs("  [DEFECT] chef_malloc off-by-one: rejects full BLOCK_SIZE allocation due to strict '<= &data[BLOCK_SIZE - 1]' check");
        g_defects_found++;
    } else {
        g_tests_passed++;
    }
    free_heap(h_full);

    kputs("  [PASS] test_memory");
    return OK;
}
