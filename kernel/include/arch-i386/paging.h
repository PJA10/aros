#ifndef ARCH_I386_PAGING_H
#define ARCH_I386_PAGING_H

#include <stdint.h>

#define KERNEL_PHYS_ADDRESS(addr) ((void *)((uint32_t) addr - KERNEL_VIRTUAL_BASE))

void paging_init();
void *vmm_allocate_page(uint32_t addr, uint16_t flags);

#endif
