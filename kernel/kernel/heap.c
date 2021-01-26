#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

uint32_t (*curr_kmalloc_func)(uint32_t sz, int align, uint32_t *phys);

uint32_t kmalloc_internal(uint32_t sz, int align, uint32_t *phys) {
	//printf("in kmalloc_internal\n");
        return (*curr_kmalloc_func)(sz, align, phys);
}

uint32_t kmalloc_a(uint32_t sz) { // page aligned.
	return kmalloc_internal(sz, true, NULL);
}
uint32_t kmalloc_p(uint32_t sz, uint32_t *phys) { // returns a physical address.
	return kmalloc_internal(sz, false, phys);
}
uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys) { // page aligned and returns a physical address.
	return kmalloc_internal(sz, true, phys);
}
uint32_t kmalloc(uint32_t sz) { // vanilla (normal).
	//printf("in kmalloc\n");
	return kmalloc_internal(sz, false, NULL);
}

void set_kmalloc_function(uint32_t (*func_p)(uint32_t , int , uint32_t *)) {
	curr_kmalloc_func = func_p;
}
