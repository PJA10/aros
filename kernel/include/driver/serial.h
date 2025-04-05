#ifndef _DRIVER_SERIAL_H
#define _DRIVER_SERIAL_H

#include <common.h>

return_code_t init_serial();
char read_serial();
void write_serial(char a);
void str_write_serial(char *str);


#endif
