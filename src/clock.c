#include "rio.h"

void clock_init() {
    // the permission setup in xv6 here is done in boot.s
    // define first interrupt
    write_stimecmp(read_time() + QUANTUM);
}
