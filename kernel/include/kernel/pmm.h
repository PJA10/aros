#ifndef _KERNEL_PMM__H
#define _KERNEL_PMM_H

#include <kernel/multiboot.h>

#include <stdint.h>

#define PAGE_SIZE 4096 // 4kib

extern uint32_t _KERNEL_VIRTUAL_BASE;
extern uint32_t _EARLY_HEAP_MAXSIZE;
extern uint32_t _KERNEL_END;
extern uint32_t _KERNEL_START;

#define KERNEL_VIRTUAL_BASE ((uint32_t) &_KERNEL_VIRTUAL_BASE)
#define EARLY_HEAP_MAXSIZE  ((uint32_t) &_EARLY_HEAP_MAXSIZE)
#define KERNEL_START        ((uint32_t) &_KERNEL_START)
#define KERNEL_END          ((uint32_t) &_KERNEL_END)
#define KERNEL_HEAP_START   (KERNEL_END)
#define KERNEL_HEAP_END     (KERNEL_HEAP_START + EARLY_HEAP_MAXSIZE)

#define VIRTUAL_ADDRESS(addr) ((void *)((uint32_t) addr + KERNEL_VIRTUAL_BASE))


void pmm_init(multiboot_info_t* mbd, unsigned int magic);
uint32_t pmm_allocate_frame();
void free_frame(uint32_t frame);




#endif
