#ifndef _KERNEL_MM__H
#define _KERNEL_MM_H

#include <kernel/multiboot.h>

#include <stdint.h>

void mm_init(multiboot_info_t *mbd, unsigned int magic);

#endif
