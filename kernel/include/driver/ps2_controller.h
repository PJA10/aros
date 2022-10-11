#ifndef _DRIVER_PS2_CONTROLLER_H
#define _DRIVER_PS2_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

extern bool is_ps2_controller_init;

uint8_t read_data();
int send_data(uint8_t value);
int ps2_controller_init();

#endif
