#ifndef VM_H
#define VM_H

#include <ktypes.h>

/* RIO uses SV-39 paging -- bottom 39 bits are used for virtual addresses */
/* RISC-V also has other types of paging for bigger pages such as SV-48 */
/* Not suprisingly, RISC-V has a TLB */

/* TODO: write a page fault handler later */

#ifndef PAGE_SIZE // should be defined in hardwaredef.h
#define PAGE_SIZE (4096)
#endif

#define MAX_VIRT_ADDR (1L << (9 + 9 + 9 + 12 - 1))

/* PTE flags */
enum pte_flag : uint64 {
    PTE_V = 1UL << 0, // valid
    PTE_R = 1UL << 1, // readable
    PTE_W = 1UL << 2, // writeable
    PTE_X = 1UL << 3, // CPU views the content as executable instructions
    PTE_U = 1UL << 4  // Can user space instructions access this?
};

void vm_init();

void kernel_vm_init();

[[nodiscard]]
byte *pte_walk(byte *page_table_root, byte *virt_addr, bool alloc);

[[nodiscard]]
status_code map_pages(byte *page_table_root, byte *src, byte *dest,
                      uint64 n_pages, uint64 flags);

/* macros */

// indexes the given level for the given virt_addr
#define VPN_INDEX(level, virt_addr) \
   ((((uint64) (virt_addr)) >> (12 + 9 * (level))) & 0x1FF) 

#define PTE2PA(pte) (((pte) >> 10) << 12)
#define PA2PTE(pa) ((((uint64)pa) >> 12) << 10)

#define PTE_FLAGS(pte) ((pte) & 0x3FF)

#endif
