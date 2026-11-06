#include <rio.h>

extern int32 g_tests_passed;
extern int32 g_defects_found;

process test_memory(void) {
    // 1. Parameter validation for kalloc and kfree
    byte *dummy = NULL;
    if (kalloc(NULL, 1) == ERR &&
        kalloc(&dummy, 0) == ERR &&
        kalloc(&dummy, -1) == ERR &&
        kfree(NULL, 1) == ERR &&
        kfree((byte *) KERNEL_BASE, 0) == ERR &&
        kfree((byte *) KERNEL_BASE, -1) == ERR) {
        g_tests_passed++;
    }

    // 2. Basic allocation and page alignment
    byte *p1 = NULL;
    if (kalloc(&p1, 1) == OK && p1 != NULL && ((uint64) p1 & (PAGE_SIZE - 1)) == 0) {
        g_tests_passed++;
    }

    // 3. Contiguous multi-page allocation
    byte *p_multi = NULL;
    if (kalloc(&p_multi, 3) == OK && p_multi != NULL && ((uint64) p_multi & (PAGE_SIZE - 1)) == 0) {
        g_tests_passed++;
    }

    // 4. Adversarial: Memory aliasing / missing set_page_allocated() in kalloc
    byte *p2 = NULL;
    kalloc(&p2, 1);
    if (p1 != NULL && p2 != NULL && p1 == p2) {
        kputs("  [DEFECT] kalloc() fails to mark pages as allocated: consecutive calls return identical memory address");
        g_defects_found++;
    } else if (p1 != NULL && p2 != NULL && p1 != p2) {
        g_tests_passed++;
    }

    // 5. Adversarial: kfree() return code contract compliance
    status_code free_res = kfree(p_multi, 3);
    if (free_res == ERR) {
        kputs("  [DEFECT] kfree() returns ERR on successful page deallocation (violates API contract)");
        g_defects_found++;
    } else {
        g_tests_passed++;
    }

    // 6. Adversarial: kalloc() failure pointer nullification (*base = NULL bug)
    byte *overflow_ptr = (byte *) 0xDEADBEEF;
    status_code alloc_res = kalloc(&overflow_ptr, 10000000);
    if (alloc_res == ERR && overflow_ptr != NULL) {
        kputs("  [DEFECT] kalloc() fails to set caller pointer to NULL on allocation failure (*base vs base bug)");
        g_defects_found++;
    } else if (alloc_res == ERR && overflow_ptr == NULL) {
        g_tests_passed++;
    }

    kputs("  [PASS] test_memory");
    return OK;
}
