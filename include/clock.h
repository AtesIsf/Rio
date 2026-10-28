#ifndef CLOCK_H
#define CLOCK_H

// QEMU clock is 10 MHz, so a 20ms quantum is 200000 ticks
#define QUANTUM (200000)

void clock_init();

#endif
