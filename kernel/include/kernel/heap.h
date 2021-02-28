#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H
#include <stddef.h>

#define NALLOC 1024 // minimum #units morecore can request

typedef union header { // block header
	struct {
		union header *ptr; // next block if on free list
		uint32_t size; // size of this block
	} s;
	max_align_t x; // force alignment of blocks
} Header;

void *adv_kmalloc(uint32_t nbytes, int align, uint32_t *phys);
void adv_kmalloc_init();
void adv_kfree(void *addr);

void *kmalloc_internal(uint32_t sz, int align, uint32_t *phys);
void *kmalloc_a(uint32_t sz);  // page aligned
void *kmalloc_p(uint32_t sz, uint32_t *phys); // returns a physical address
void *kmalloc_ap(uint32_t sz, uint32_t *phys); // page aligned and returns a physical address
void *kmalloc(uint32_t sz); // vanilla (normal)
void set_kmalloc_function(void *(*func_p)(uint32_t , int , uint32_t *));
void set_kfree( void (*func_p)(void *addr));
void kfree(void *addr);

#endif
