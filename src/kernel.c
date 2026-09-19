
/**
 * Entry point for the RIO kernel
 */

#include "kprint.h"

void kmain(void) {
    kputs("----------------");
    kputs(" Welcome to RIO");
    kputs("----------------");

    kputs("Entered S-mode");

	return;
}
