#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

void *kmalloc_internal(uint32_t sz, int align, uint32_t *phys);
void *kmalloc_a(uint32_t sz);  // page aligned.
void *kmalloc_p(uint32_t sz, uint32_t *phys); // returns a physical address.
void *kmalloc_ap(uint32_t sz, uint32_t *phys); // page aligned and returns a physical address.
void *kmalloc(uint32_t sz); // vanilla (normal).
void set_kmalloc_function(void *(*func_p)(uint32_t , int , uint32_t *));

#endif
