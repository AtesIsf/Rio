/**
 * Functions to allocate the stack memory needed by low-level processes
 */

#include "rio.h"

struct MemBlock g_memlist[N_BLOCKS];

/**
 * Returns the data associated with the first free entry from the end. Returns
 * NULL if no suitable chunk is found.
 */

byte *get_stack() {
    byte *target = NULL;
    
    for (int32 i = N_BLOCKS - 1; i >= 0; i--) {
        if (g_memlist[i].state != MEM_FREE) {
            continue;
        }
        target = (byte *) g_memlist[i].data;
        target += BLOCK_SIZE; // we need to return the top, not base
        g_memlist[i].state = MEM_STACK;
        // this makes no sense for the stack but still
        g_memlist[i].curr_end = g_memlist[i].data;
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
        if (g_memlist[i].state != MEM_FREE) {
            continue;
        }
        target = (byte *) g_memlist[i].data;
        g_memlist[i].state = MEM_HEAP;
        g_memlist[i].curr_end = g_memlist[i].data;
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
    ((struct MemBlock *) (addr - BLOCK_SIZE - 16))->state = MEM_FREE;
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

/**
 * Given the data section pointer of a heap block obtained by the
 * low-level memory manager, allocates and returns a heap object of the
 * requested size. Returns NULL on failure.
 *
 * Causes undefined behaviour if the passed block is a stack block!
 */

void *chef_malloc(byte *data, int32 size) {
    if (data == NULL || size <= 0 
        || ((struct MemBlock *) (data - 16))->state != MEM_HEAP) {
        return NULL;
    }

    // round up to next multiple of 8
    size = (size + 7) & (-8);
    struct MemBlock *block = (struct MemBlock *) (data - 16);

    byte *result = NULL;
    if (block->curr_end + size <= &block->data[BLOCK_SIZE - 1]) {
        result = block->curr_end;
        block->curr_end += size;
    }

    return (void *) result;
}

