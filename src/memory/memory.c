#include <rio.h>

// exported from the linker, first address after the kernel, 16 byte aligned
extern byte *kernel_end;

/**
 * Initializes the kernel memory subsystem and sets up the necessary
 * metadata.
 */

void mem_init() {
    byte *mem_start = (byte *) PAGE_ROUND_UP((uint64) kernel_end);
    uint64 n_pages = (RAM_END - (uint64) mem_start) / PAGE_SIZE;

    // calculate #necessary bits

    // calculate the necessary amount of metadata pages

    // sets those # of metadata pages from the start to be allocated
}

syscall get_stack(byte *top, int32 n) {
    return ERR;
}

syscall get_heap(byte *top, int32 n) {
    return ERR;
}

syscall free_stack(byte *top, int32 n) {
    return ERR;
}

syscall free_heap(byte *top, int32 n) {
    return ERR;
}

