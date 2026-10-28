
/**
 * kmain is the main process created by the entry point. It is basically
 * a main function for the kernel, but running as a process as well.
 */

#include <rio.h>

process kmain() {
    kputs("----------------");
    kputs(" Welcome to RIO ");
    kputs("----------------");

    return OK;
}
