#include <arch-i386/idt.h>
#include <arch-i386/gdt.h>

void kernel_init() {
	extern int kernel_main();
	gdt_init();
	idt_init();
	kernel_main();
}
