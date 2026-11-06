#ifndef MEMORY_H
#define MEMORY_H

#include <ktypes.h>
#include <process.h>

/**
 * The memory model is loosely based on both Xinu and xv6.
 *
 * The main highlight of this memory manager is that the caller can
 * specify number of requested pages (in 4KB).
 *
 * Main Idea: Sacrifice the first page to store metadata.
 * Place the MemMetadata struct there. Calculate the number of pages required
 * to store the said metadata (bitmap of used/free pages). Remember to keep
 * the data structure 16-byte aligned.
 *
 * The comment below is taken from xv6
 *
 * qemu -machine virt is set up like this,
 * based on qemu's hw/riscv/virt.c:
 *
 * 00001000 -- boot ROM, provided by qemu
 * 02000000 -- CLINT => Core Local Interrupt Controller
 * 0C000000 -- PLIC => Platform Local Interrupt Controller
 * 10000000 -- uart0
 * 10001000 -- virtio disk
 * 80000000 -- qemu's boot ROM loads the kernel here,
 *             then jumps here.
 * unused RAM after 80000000.
 */

// The kernel memory starts from here due to QEMU'S ROM
#define KERNEL_BASE (0x80000000L)

// QEMU defaults to 128 MB of RAM
#define RAM_END (KERNEL_BASE + 128 * 1024 * 1024)

#define PAGE_SIZE (4096)

/* helper macros */
#define PAGE_ROUND_UP(x)  (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_ROUND_DOWN(x) (((x)) & ~(PAGE_SIZE - 1))

/* functions */

void mem_init();

/* Memory Manager Endpoint Functions */

syscall get_stack(byte *top, int32 n);

syscall get_heap(byte *start, int32 n);

/* can I implement these without n? */

syscall free_stack(byte *top, int32 n);

syscall free_heap(byte *start, int32 n);

#endif
