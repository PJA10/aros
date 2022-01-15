#ifndef _DRIVER_PIT_H
#define _DRIVER_PIT_H

#include <stdint.h>

/**
 * return the number of nano seconds since boot/init of timer.
 */
uint64_t get_nanoseconds();
void pit_init();
void pit_interrupt();

#endif
