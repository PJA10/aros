#include <arch-i386/idt.h>
#include <arch-i386/gdt.h>
#include <arch-i386/paging.h>
#include <stdio.h>

void kernel_init() {
	extern int kernel_main();
	//paging_init();
	gdt_init();
	idt_init();
	kernel_main();
}
