#ifndef _KERNEL_SERIAL_H
#define _KERNEL_SERIAL_H

void init_serial();
char read_serial();
void write_serial(char a);
void str_write_serial(char *str);


#endif
