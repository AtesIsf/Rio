#include <rio.h>

status_code test_memory(void) {
    int32 fails = 0;

    // 1. Stack allocation and free
    byte *s1 = get_stack();
    byte *s2 = get_stack();
    if (s1 == NULL || s2 == NULL || s1 == s2) {
        kputs("FAIL: get_stack failed to allocate distinct blocks");
        fails++;
    }
    if (free_stack(s1) != OK || free_stack(s2) != OK) {
        kputs("FAIL: free_stack failed on valid stacks");
        fails++;
    }
    if (free_stack(NULL) != ERR) {
        kputs("FAIL: free_stack(NULL) did not return ERR");
        fails++;
    }

    // 2. Heap allocation and free
    byte *h1 = get_heap();
    byte *h2 = get_heap();
    if (h1 == NULL || h2 == NULL || h1 == h2) {
        kputs("FAIL: get_heap failed to allocate distinct blocks");
        fails++;
    }
    if (free_heap(h1) != OK || free_heap(h2) != OK) {
        kputs("FAIL: free_heap failed on valid heaps");
        fails++;
    }
    if (free_heap(NULL) != ERR) {
        kputs("FAIL: free_heap(NULL) did not return ERR");
        fails++;
    }

    // 3. chef_malloc boundary and alignment checks
    byte *h = get_heap();
    byte *s = get_stack();

    if (chef_malloc(NULL, 16) != NULL) {
        kputs("FAIL: chef_malloc(NULL, 16) did not return NULL");
        fails++;
    }
    if (chef_malloc(h, 0) != NULL || chef_malloc(h, -8) != NULL) {
        kputs("FAIL: chef_malloc rejected non-positive sizes");
        fails++;
    }
    if (chef_malloc(s, 16) != NULL) {
        kputs("FAIL: chef_malloc on stack block did not return NULL");
        fails++;
    }

    void *p1 = chef_malloc(h, 5);
    void *p2 = chef_malloc(h, 9);
    if (p1 == NULL || p2 == NULL) {
        kputs("FAIL: chef_malloc failed on valid requests");
        fails++;
    } else {
        if (((uint64) p1 & 0x7) != 0 || ((uint64) p2 & 0x7) != 0) {
            kputs("FAIL: chef_malloc returned unaligned address");
            fails++;
        }
        if ((byte *) p2 < (byte *) p1 + 8) {
            kputs("FAIL: chef_malloc overlapping chunks");
            fails++;
        }
    }

    free_heap(h);
    free_stack(s);

    if (fails == 0) {
        kputs("PASS: test_memory");
        return OK;
    }
    return ERR;
}
