#ifndef CLOCK_H
#define CLOCK_H

// QEMU clock is 10 MHz, so a 20ms quantum is 200000 ticks
#define TICK_MS (10000)
#define QUANTUM (20) // in ms

void clock_init();

#endif
