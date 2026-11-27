#include <rio.h>

static byte *kernel_page_table;

void vm_init() {
    kernel_page_table = kernel_vm_init();

    return; // TODO: REMOVE THIS AFTER kernel_vm_init() is done

    sfence_vma(); // wait previous writes to finish
    write_satp(INIT_SATP(kernel_page_table));
    sfence_vma(); // flush TLB
}

byte *kernel_vm_init() {
    return nullptr;
}

