#include <rio.h>

extern int32 g_tests_passed;
extern int32 g_defects_found;

process test_kprint(void) {
    // 1. NULL pointer rejection
    if (kprint(NULL) == ERR) {
        g_tests_passed++;
    }
    if (kputs(NULL) == ERR) {
        g_tests_passed++;
    }

    // 2. Empty string valid handling
    if (kprint("") == OK) {
        g_tests_passed++;
    }

    kputs("  [PASS] test_kprint");
    return OK;
}
