#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/serial.h>
#include <kernel/idt.h>
#include <kernel/pic.h>

void kernel_main(void) {
	terminal_initialize();
	init_serial();
	idt_init();
	printf("idt_init finished\n");
	asm("int $0x27");
	printf("kernel finished - hlting\n");
	for(;;) {
		asm("hlt");
	}
}

