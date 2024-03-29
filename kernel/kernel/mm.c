#include <kernel/mm.h>
#include <kernel/pmm.h>
#include <arch-i386/paging.h>
#include <stdio.h>

void mm_init(multiboot_info_t* mbd, unsigned int magic) {
	pmm_init(mbd, magic);
	paging_init();
	printf("memory management initialized\n");
}
