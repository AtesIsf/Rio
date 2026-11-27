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

/* PTE flags */
#define PTE_V (1UL << 0) // valid
#define PTE_R (1UL << 1) // readable
#define PTE_W (1UL << 2) // writeable
#define PTE_X (1UL << 3) // CPU views the content as executable instructions
#define PTE_U (1UL << 4) // Can user space instructions access this?

void vm_init();

[[nodiscard]]
byte *kernel_vm_init();

[[nodiscard]]
byte *find_pte(byte *page_table_root, byte *virt_addr);

[[nodiscard]]
status_code map(byte *page_table_root);

#endif
