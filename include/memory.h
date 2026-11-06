#ifndef MEMORY_H
#define MEMORY_H

#include <ktypes.h>
#include <process.h>

#define MAX_STACK_PAGES (16) // TODO: change this later

/**
 * Maybe make a big array of pages, then abstractly split them into 2 using
 * 2 pointers?
 *
 * Maybe huge array with a helper bitmap?
 *
 * Could I do a heap-allocator-like freelist-based system?
 */

syscall get_stack(byte *top, int32 n);

syscall get_heap(byte *start, int32 n);

/* can I implement these without n? */

syscall free_stack(byte *top, int32 n);

syscall free_heap(byte *start, int32 n);

#endif
