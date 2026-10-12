
/**
 * kmain is the main process created by the entry point. It is basically
 * a main function for the kernel, but running as a process as well.
 */

#include "kprint.h"
#include "memory.h"
#include "process.h"

process test_proc() {
    byte *data = get_heap();

    int32 *num = chef_malloc(data, sizeof(int32));
    byte *ptr = chef_malloc(data, 399);
    void *fail = chef_malloc(data, 4096);

    kprintf("%p %p %p\n", num, ptr, fail);

    struct MemBlock *block = (struct MemBlock *) (data - 16);
    kprintf("%p -- %p -- %d\n", block->data, block->curr_end, 
            (int32) (block->curr_end - block->data));

    free_heap(data);
    return OK;
}

process kmain() {
    kputs("----------------");
    kputs(" Welcome to RIO ");
    kputs("----------------");

    resume(create(test_proc, 10, "test", 0));

    return OK;
}
