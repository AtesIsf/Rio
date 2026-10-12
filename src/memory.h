#ifndef MEMORY_H
#define MEMORY_H

#include "ktypes.h"
#include "process.h"

// Each memory block is 4KiB, including state and padding
#define BLOCK_SIZE (4096 - 16)
#define N_BLOCKS (N_PROCS * 2)

#define MEM_FREE (0)
#define MEM_ALLOC (1)

struct MemBlock {
    byte state;
    byte padding[15]; // keeps data 16-byte aligned
    byte data[BLOCK_SIZE];
};

extern struct MemBlock g_memlist[N_BLOCKS];

byte *get_stack();

byte *get_heap();

status_code free_stack(byte *addr);

status_code free_heap(byte *addr);

#endif
