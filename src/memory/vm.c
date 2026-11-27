#include <rio.h>

static byte *kernel_page_table;

void vm_init() {
    kernel_page_table = kernel_vm_init();
}

byte *kernel_vm_init() {
    return nullptr;
}

