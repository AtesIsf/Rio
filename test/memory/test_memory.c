#include <rio.h>

extern int32 g_tests_passed;
extern int32 g_defects_found;

extern byte kernel_end;

process test_memory(void) {
    // ------------------------------------------------------------------------
    // Test 1: Parameter validation for kalloc and kfree
    // ------------------------------------------------------------------------
    byte *dummy = NULL;
    if (kalloc(NULL, 1) == ERR &&
        kalloc(&dummy, 0) == ERR &&
        kalloc(&dummy, -1) == ERR &&
        kfree(NULL, 1) == ERR &&
        kfree((byte *) KERNEL_BASE, 0) == ERR &&
        kfree((byte *) KERNEL_BASE, -1) == ERR) {
        g_tests_passed++;
    }

    // ------------------------------------------------------------------------
    // Test 2: Basic single-page allocation and 4 KiB alignment
    // ------------------------------------------------------------------------
    byte *p1 = NULL;
    if (kalloc(&p1, 1) == OK && p1 != NULL && ((uint64) p1 & (PAGE_SIZE - 1)) == 0) {
        g_tests_passed++;
    }

    // ------------------------------------------------------------------------
    // Test 3: Contiguous multi-page allocation and 4 KiB alignment
    // ------------------------------------------------------------------------
    byte *p_multi = NULL;
    if (kalloc(&p_multi, 3) == OK && p_multi != NULL && ((uint64) p_multi & (PAGE_SIZE - 1)) == 0) {
        g_tests_passed++;
    }

    // ------------------------------------------------------------------------
    // Test 4: Consecutive allocation non-aliasing validation
    // ------------------------------------------------------------------------
    byte *p2 = NULL;
    kalloc(&p2, 1);
    if (p1 != NULL && p2 != NULL && p1 == p2) {
        kputs("  [DEFECT] kalloc() fails to mark pages as allocated: consecutive calls return identical memory address");
        g_defects_found++;
    } else if (p1 != NULL && p2 != NULL && p1 != p2) {
        g_tests_passed++;
    }

    // ------------------------------------------------------------------------
    // Test 5: Baseline valid multi-page deallocation
    // ------------------------------------------------------------------------
    status_code free_res = kfree(p_multi, 3);
    if (free_res == ERR) {
        kputs("  [DEFECT] kfree() returns ERR on successful page deallocation (violates API contract)");
        g_defects_found++;
    } else {
        g_tests_passed++;
    }

    // ------------------------------------------------------------------------
    // Test 6: Incomplete run / loop termination defect on oversized kalloc
    // ------------------------------------------------------------------------
    // When requested contiguous pages exceed available memory, kalloc must fail
    // and assign *base = NULL. If start_index != -1 at loop termination without
    // satisfying len == n, kalloc may incorrectly return OK, or ERR without nullifying *base.
    byte *large_ptr = (byte *) 0xDEADBEEF;
    status_code alloc_large_res = kalloc(&large_ptr, 10000000);
    if (alloc_large_res == OK) {
        kputs("  [DEFECT] kalloc() succeeds on oversized contiguous request (loop termination bug when len < n)");
        g_defects_found++;
        kfree(large_ptr, 10000000);
    } else if (alloc_large_res == ERR && large_ptr != NULL) {
        kputs("  [DEFECT] kalloc() fails to set caller pointer to NULL on allocation failure (*base vs base bug)");
        g_defects_found++;
    } else if (alloc_large_res == ERR && large_ptr == NULL) {
        g_tests_passed++;
    }

    // Check with 0x7FFFFFFF (INT32_MAX) boundary
    byte *max_ptr = (byte *) 0xCAFEBABE;
    status_code alloc_max_res = kalloc(&max_ptr, 0x7FFFFFFF);
    if (alloc_max_res == OK) {
        kputs("  [DEFECT] kalloc() succeeds on INT32_MAX contiguous request");
        g_defects_found++;
        kfree(max_ptr, 0x7FFFFFFF);
    } else if (alloc_max_res == ERR && max_ptr != NULL) {
        kputs("  [DEFECT] kalloc() fails to set caller pointer to NULL on INT32_MAX failure");
        g_defects_found++;
    } else if (alloc_max_res == ERR && max_ptr == NULL) {
        g_tests_passed++;
    }

    // ------------------------------------------------------------------------
    // Test 7: Adversarial - Kernel metadata deallocation protection
    // ------------------------------------------------------------------------
    // Deallocating bitmap/metadata pages at pages_start must be rejected with ERR.
    byte *metadata_page = (byte *) PAGE_ROUND_UP((uint64) &kernel_end);
    status_code free_meta_res = kfree(metadata_page, 1);
    if (free_meta_res == OK) {
        kputs("  [DEFECT] kfree() permits deallocation of kernel allocator metadata/bitmap pages");
        g_defects_found++;
        // Re-protect metadata page in bitmap so future allocations don't corrupt it
        // The bitmap's first byte maps the first 8 pages; bit 0 must be set.
        metadata_page[0] |= 1U;
    } else {
        g_tests_passed++;
    }

    // ------------------------------------------------------------------------
    // Test 8: Adversarial - Out-of-bounds deallocation via sentinel return
    // ------------------------------------------------------------------------
    // Freeing memory before pages_start or at/beyond physical RAM (RAM_END) must fail with ERR.
    // If is_page_allocated(page_index >= n_pages) returns true, kfree accepts OOB addresses.
    status_code free_ram_end = kfree((byte *) RAM_END, 1);
    status_code free_ram_oob = kfree((byte *) (RAM_END + 0x1000000), 1);
    status_code free_before_start = kfree((byte *) KERNEL_BASE, 1);

    if (free_ram_end == OK || free_ram_oob == OK || free_before_start == OK) {
        kputs("  [DEFECT] kfree() allows out-of-bounds page deallocation (is_page_allocated sentinel bug)");
        g_defects_found++;
    } else {
        g_tests_passed++;
    }

    // ------------------------------------------------------------------------
    // Test 9: Adversarial - Unaligned pointer deallocation
    // ------------------------------------------------------------------------
    // Freeing an unaligned pointer (e.g. p1 + 16 or p1 + 1) must fail with ERR
    // rather than silently integer-truncating to a valid page boundary.
    if (p1 != NULL) {
        status_code free_unaligned1 = kfree(p1 + 16, 1);
        status_code free_unaligned2 = kfree(p1 + 1, 1);
        if (free_unaligned1 == OK || free_unaligned2 == OK) {
            kputs("  [DEFECT] kfree() accepts unaligned pointer, silently truncating page offset");
            g_defects_found++;
        } else {
            g_tests_passed++;
        }
    }

    // ------------------------------------------------------------------------
    // Test 10: Adversarial - Double free and partial overlap validation
    // ------------------------------------------------------------------------
    // Freeing an already freed page must fail with ERR.
    byte *p_df = NULL;
    if (kalloc(&p_df, 1) == OK && p_df != NULL) {
        status_code first_free = kfree(p_df, 1);
        status_code second_free = kfree(p_df, 1);
        if (first_free == OK && second_free == OK) {
            kputs("  [DEFECT] kfree() allows double-free on previously deallocated page");
            g_defects_found++;
        } else if (first_free == OK && second_free == ERR) {
            g_tests_passed++;
        }
    }

    // ------------------------------------------------------------------------
    // Test 11: Adversarial - Allocation exhaustion & boundary fit
    // ------------------------------------------------------------------------
    // Test allocation exhaustion and verify kalloc gracefully returns ERR
    // without crashing or corrupting memory markers.
    #define EXHAUSTION_BLOCK_SIZE 512
    #define MAX_EXHAUSTION_SLOTS 128
    byte *blocks[MAX_EXHAUSTION_SLOTS];
    int32 blocks_allocated = 0;

    for (int32 i = 0; i < MAX_EXHAUSTION_SLOTS; i++) {
        byte *b = NULL;
        if (kalloc(&b, EXHAUSTION_BLOCK_SIZE) == OK && b != NULL) {
            blocks[blocks_allocated++] = b;
        } else {
            break;
        }
    }

    // At exhaustion or near capacity, verify requesting more pages than available cleanly fails
    byte *exhaust_fail_ptr = (byte *) 0x12345678;
    status_code exhaust_res = kalloc(&exhaust_fail_ptr, EXHAUSTION_BLOCK_SIZE);
    if (exhaust_res == OK) {
        blocks[blocks_allocated++] = exhaust_fail_ptr;
    } else {
        if (exhaust_fail_ptr != NULL) {
            kputs("  [DEFECT] kalloc() exhaustion fails without setting caller pointer to NULL");
            g_defects_found++;
        } else {
            g_tests_passed++;
        }
    }

    // Clean up allocated blocks
    for (int32 i = 0; i < blocks_allocated; i++) {
        kfree(blocks[i], EXHAUSTION_BLOCK_SIZE);
    }

    // Clean up p1 and p2 from earlier tests
    if (p1 != NULL) {
        kfree(p1, 1);
    }
    if (p2 != NULL) {
        kfree(p2, 1);
    }

    kputs("  [PASS] test_memory");
    return OK;
}

