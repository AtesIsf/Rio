#include <rio.h>

status_code test_kprint(void) {
    int32 fails = 0;

    // 1. NULL pointer rejection
    if (kprint(NULL) != ERR) {
        kputs("FAIL: kprint(NULL) did not return ERR");
        fails++;
    }

    if (kputs(NULL) != ERR) {
        kputs("FAIL: kputs(NULL) did not return ERR");
        fails++;
    }

    // 2. Valid empty and normal string output
    if (kprint("") != OK) {
        kputs("FAIL: kprint(\"\") did not return OK");
        fails++;
    }

    if (fails == 0) {
        kputs("PASS: test_kprint");
        return OK;
    }
    return ERR;
}
