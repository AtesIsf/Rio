/**
 * Functions to allocate the stack memory needed by low-level processes
 */

#include "memory.h"

struct MemBlock g_memlist[N_BLOCKS];

/**
 * Returns the data associated with the first free entry from the end. Returns
 * NULL if no suitable chunk is found.
 */

byte *get_stack() {
    byte *target = NULL;
    
    for (int32 i = N_BLOCKS - 1; i >= 0; i--) {
        if (g_memlist[i].state == MEM_ALLOC) {
            continue;
        }
        target = (byte *) g_memlist[i].data;
        target += BLOCK_SIZE; // we need to return the top, not base
        g_memlist[i].state = MEM_ALLOC;
        break;
    }
    return target;
}

/**
 * Returns the data associated with the first free entry from the start.
 * Returns NULL if no suitable chunk is found.
 */

byte *get_heap() {
    byte *target = NULL;
    
    for (int32 i = 0; i < N_BLOCKS; i++) {
        if (g_memlist[i].state == MEM_ALLOC) {
            continue;
        }
        target = (byte *) g_memlist[i].data;
        g_memlist[i].state = MEM_ALLOC;
        break;
    }
    return target;
}

/**
 * Frees the entry associated with the given data blob
 */

status_code free_stack(byte *addr) {
    if (addr == NULL) {
        return ERR;
    }
    // this should be the state field of the entry
    *(addr - BLOCK_SIZE - 16) = MEM_FREE;
    return OK;
}

/**
 * Exists for programmer convenience
 */
status_code free_heap(byte *addr) {
    if (addr == NULL) {
        return ERR;
    }
    return free_stack(addr + BLOCK_SIZE);
}

