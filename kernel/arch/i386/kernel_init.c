#include <arch-i386/idt.h>
#include <arch-i386/gdt.h>
#include <arch-i386/paging.h>
#include <kernel/multiboot.h>

void kernel_init(multiboot_info_t* mbd, unsigned int magic) {
	extern int kernel_main();
	gdt_init();
	idt_init();
	kernel_main(mbd,magic);
}
