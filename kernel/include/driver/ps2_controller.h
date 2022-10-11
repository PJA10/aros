#ifndef _DRIVER_PS2_CONTROLLER_H
#define _DRIVER_PS2_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

extern bool ps2_controller__is_init;

uint8_t ps2_controller__read_data();
int ps2_controller__send_data(uint8_t value);
int ps2_controller__init();

#endif
