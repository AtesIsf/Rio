#include "vm.h"
#include <rio.h>

#include <stdckdint.h>

extern byte text_end; // provided by linker
static pte_t *kernel_page_table;

void vm_init() {
    kernel_vm_init();

    sfence_vma(); // wait previous writes to finish
    write_satp(INIT_SATP(kernel_page_table));
    sfence_vma(); // flush TLB
}

/**
 * Just like xv6, the kernel vm is directly mapped to physical memory
 */

void kernel_vm_init() {
    // this is the file-scoped static variable
    kernel_page_table = (pte_t *) kalloc(1);
    if (kernel_page_table == nullptr) {
        panic("panic: kernel page table kalloc error");
    }
    memset((byte *) kernel_page_table, 0, PAGE_SIZE);

    /* qemu virt devices */
    status_code result = OK;

    // UART
    result = map_pages(kernel_page_table, UART_ADDR, UART_ADDR,
                       1, PTE_R | PTE_W);
    if (result == ERR) {
        panic("kernel_vm_init: failed to map uart");
    }

    // virtio mmio disk
    result = map_pages(kernel_page_table, VIRTIO_ADDR, VIRTIO_ADDR,
                       1, PTE_R | PTE_W);
    if (result == ERR) {
        panic("kernel_vm_init: failed to map virtio mmio disk");
    }

    // plic
    result = map_pages(kernel_page_table, PLIC_ADDR, PLIC_ADDR,
                       0x400000 / PAGE_SIZE, PTE_R | PTE_W);
    if (result == ERR) {
        panic("kernel_vm_init: failed to map plic");
    }

    /* kernel things */

    // kernel text => executable & read
    result = map_pages(kernel_page_table, (byte *) KERNEL_BASE, 
           (byte *) KERNEL_BASE, ((uint64) &text_end - KERNEL_BASE) / PAGE_SIZE,
           PTE_R | PTE_X);
    if (result == ERR) {
        panic("kernel_vm_init: failed to map kernel text");
    }

    // kernel data & physical ram => read & write
    result = map_pages(kernel_page_table, (byte *) &text_end, 
           (byte *) &text_end, (RAM_END - (uint64) &text_end) / PAGE_SIZE,
           PTE_R | PTE_W);
    if (result == ERR) {
        panic("kernel_vm_init: failed to map kernel data & ram");
    }

    // TODO: trampoline??
    // TODO: proc_map_stacks()??
}

/**
 * Maps n_pages virtual pages starting from src to n_pages physical pages
 * starting from dest. The flags for that page should be passed in flags.
 * The root of the page table in question should be passed. If there is any
 * error, the function returns ERR. Returns OK on success.
 *
 * The src and dest must be PAGE_SIZE aligned
 */

status_code map_pages(pte_t *page_table_root, byte *src, byte *dest, 
                      uint64 n_pages, uint64 flags) {
    uint64 buf_for_ckd;
    if (page_table_root == nullptr || src == nullptr || dest == nullptr
        || (uint64) src % PAGE_SIZE != 0
        || (uint64) dest % PAGE_SIZE != 0
        || ckd_mul(&buf_for_ckd, n_pages, PAGE_SIZE)
        || ckd_add(&buf_for_ckd, buf_for_ckd, (uint64) usable_pages_start) ) {
        return ERR;
    }

    uint64 va = (uint64) src;
    uint64 pa = (uint64) dest;

    uint64 bytes;
    bool overflows = ckd_mul(&bytes, n_pages, PAGE_SIZE);
    if (overflows) return ERR;
    uint64 last = va + bytes;

    while (va != last) {
        pte_t *pte = pte_walk(page_table_root, (byte *) va, true);
        if (pte == nullptr) return ERR;

        if (*pte & PTE_V) {
            panic("map_pages: pte already in use");
        }

        *pte = PA2PTE(pa) | flags | PTE_V;
        
        // safe increments
        overflows = ckd_add(&bytes, va, PAGE_SIZE);
        if (overflows) return ERR;
        va = bytes;

        overflows = ckd_add(&bytes, pa, PAGE_SIZE);
        if (overflows) return ERR;
        pa = bytes;
    }

    return OK;
}

/**
 * Returns the PTE address of the given virt_addr. Return the address on
 * success and nullptr on error. Allocates necessary PTE's if alloc is true.
 *
 * This function basically mimics the RISC-V hardware in its search.
 */

pte_t *pte_walk(pte_t *page_table_root, byte *virt_addr, bool alloc) {
    if (page_table_root == nullptr || virt_addr == nullptr 
            || (uint64) virt_addr > MAX_VIRT_ADDR) {
        return nullptr;
    }

    for (int32 level = 2; level > 0; level--) {
        pte_t *pte = &page_table_root[VPN_INDEX(level, virt_addr)];
        if (*pte & PTE_V) { // basically if the pte valid
            page_table_root = (pte_t *) PTE2PA((uint64) *pte); //next lvl
        } else if (!alloc) {
            return nullptr;
        } else {
            page_table_root = (pte_t *) kalloc(1);
            if (page_table_root == nullptr) return nullptr;

            status_code result = memset((byte *) page_table_root, 0, PAGE_SIZE);
            if (result == ERR) return nullptr;
            *pte = PA2PTE(page_table_root) | PTE_V;
        }
    }

    // conceptually, this is just like a linked list walk and returning
    // curr's address at the end
    return &page_table_root[VPN_INDEX(0, virt_addr)];
}

