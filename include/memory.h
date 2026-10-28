#ifndef MEMORY_H
#define MEMORY_H

#include <ktypes.h>
#include <process.h>

// Each memory block is 4KiB, including state and padding
#define BLOCK_SIZE (4096 - 16)
#define N_BLOCKS (N_PROCS * 2)

#define MEM_FREE    (0)
#define MEM_STACK   (1)
#define MEM_HEAP    (2)

struct MemBlock {
    byte *curr_end;
    byte state;
    byte padding[7];
    byte data[BLOCK_SIZE];
};

extern struct MemBlock g_memlist[N_BLOCKS];

byte *get_stack();

byte *get_heap();

status_code free_stack(byte *addr);

status_code free_heap(byte *addr);

/**
 * These are heap manipulation functions. I call this the chef approach
 * because it conceptually looks like cutting onions.
 *
 * Advantages: simple, fast, and fits well with this memory system
 *
 * Disadvantage: no free, so could cause conceptual "fragmentation"
 */

void *chef_malloc(byte *data, int32 size);

/**
 * Could be implemented later, but I don't think its necessary
 * status_code chef_free(void *ptr);
 */

#endif
