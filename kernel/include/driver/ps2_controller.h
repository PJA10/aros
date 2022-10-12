#ifndef _DRIVER_PS2_CONTROLLER_H
#define _DRIVER_PS2_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include <common.h>

extern bool ps2_keyboard__is_init;

return_code_t ps2_controller__read_data(uint8_t *out_data);
return_code_t ps2_controller__send_data(uint8_t value);
return_code_t ps2_controller__init();

#endif
