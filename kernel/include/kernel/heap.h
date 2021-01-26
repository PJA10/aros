#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

uint32_t kmalloc_internal(uint32_t sz, int align, uint32_t *phys);
uint32_t kmalloc_a(uint32_t sz);  // page aligned.
uint32_t kmalloc_p(uint32_t sz, uint32_t *phys); // returns a physical address.
uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys); // page aligned and returns a physical address.
uint32_t kmalloc(uint32_t sz); // vanilla (normal).
void set_kmalloc_function(uint32_t (*func_p)(uint32_t , int , uint32_t *));

#endif
