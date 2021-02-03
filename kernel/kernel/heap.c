#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

static void *(*curr_kmalloc_func)(uint32_t sz, int align, uint32_t *phys);

void *kmalloc_internal(uint32_t sz, int align, uint32_t *phys) {
	//printf("in kmalloc_internal\n");
        return (*curr_kmalloc_func)(sz, align, phys);
}

void *kmalloc_a(uint32_t sz) { // page aligned.
	return kmalloc_internal(sz, true, NULL);
}
void *kmalloc_p(uint32_t sz, uint32_t *phys) { // returns a physical address.
	return kmalloc_internal(sz, false, phys);
}
void *kmalloc_ap(uint32_t sz, uint32_t *phys) { // page aligned and returns a physical address.
	return kmalloc_internal(sz, true, phys);
}
void *kmalloc(uint32_t sz) { // vanilla (normal).
	//printf("in kmalloc\n");
	return kmalloc_internal(sz, false, NULL);
}

void set_kmalloc_function(void *(*func_p)(uint32_t , int , uint32_t *)) {
	curr_kmalloc_func = func_p;
}
