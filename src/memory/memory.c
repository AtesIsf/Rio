#include <rio.h>

// exported from the linker, first byte after the kernel, 16 byte aligned
extern byte kernel_end;

static uint64 n_pages; // number of pages in the RAM
static byte *pages_start; // where the pages start from

/* static inline bitmap helper functions */

static inline void set_page_allocated(uint64 page_index) {
    if (page_index >= n_pages) {
        return;
    }
    pages_start[page_index / 8] |= (1U << (page_index % 8));
}

static inline void clear_page_allocated(uint64 page_index) {
    if (page_index >= n_pages) {
        return;
    }
    pages_start[page_index / 8] &= ~(1U << (page_index % 8));
}

static inline bool is_page_allocated(uint64 page_index) {
    if (page_index >= n_pages) {
        return true;
    }
    return (pages_start[page_index / 8] & (1U << (page_index % 8))) != 0;
}

/* exposed functions */

/**
 * Initializes the kernel memory subsystem and sets up the necessary
 * metadata.
 */

void mem_init() {
    pages_start = (byte *) PAGE_ROUND_UP((uint64) &kernel_end);
    n_pages = (RAM_END - (uint64) pages_start) / PAGE_SIZE;

    // calculate #necessary bytes (round up n_pages)
    uint64 bytes_required = ((n_pages + 7) & (~7)) / (sizeof(byte) * 8);

    // calculate the necessary amount of metadata pages
    uint64 pages_needed = PAGE_ROUND_UP(bytes_required) / PAGE_SIZE;

    // zero out the metadata pages so all pages initially start as free (0)
    for (uint64 i = 0; i < PAGE_ROUND_UP(bytes_required); i++) {
        pages_start[i] = 0;
    }

    // sets those # of metadata pages from the start to be allocated
    for (uint64 i = 0; i < pages_needed; i++) {
        set_page_allocated(i);
    }
}

/**
 * Finds and returns "n" contagious pages from the physical address space.
 * Returns OK on success and ERR on error. Writes the start address of the
 * found "n" pages into "base". Stack calculation is the caller's
 * responsibility.
 */

syscall kalloc(byte **base, int32 n) {
    intmask mask = interrupt_save();

    if (base == NULL || n <= 0) {
        interrupt_restore(mask);
        return ERR;
    }

    int32 start_index = -1;
    int32 len = 0;
    for (uint64 i = 0; i < n_pages; i++) {
        if (is_page_allocated(i) == true) {
            start_index = -1;
            len = 0;
            continue;
        }
        // unallocated page
        start_index = start_index == -1 ? (int32) i : start_index;
        if (++len == n) {
            break;
        }
    }

    status_code result = OK;
    if (start_index == -1) {
        result = ERR;
        base = NULL;
    } else {
        result = OK;
        *base = pages_start + PAGE_SIZE * start_index;
    }

    interrupt_restore(mask);
    return result;
}

/**
 * Frees "n" physical pages starting from the given base pointer.
 * Returns OK on success and ERR on error. If a wrong "n" is passed, the
 * RAM will be corrupted!
 */

syscall kfree(byte *base, int32 n) {
    intmask mask = interrupt_save();

    if (base == NULL || n <= 0) {
        interrupt_restore(mask);
        return ERR;
    }

    // first, check the given pages are allocated => would prevent some issues
    uint64 start_index = (base - pages_start) / PAGE_SIZE; 
    for (uint64 i = start_index; i < start_index + n; i++) {
        if (is_page_allocated(i) == false) {
            interrupt_restore(mask);
            return ERR;
        }
    }

    // actual free
    for (uint64 i = start_index; i < start_index + n; i++) {
        clear_page_allocated(i);
    }

    interrupt_restore(mask);
    return ERR;
}

