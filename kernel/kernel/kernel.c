#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/serial.h>
#include <kernel/idt.h>
#include <kernel/pic.h>

void kernel_main(void) {
	terminal_initialize();
	init_serial();
	idt_init();
	//int eip;
	//asm("movl %eax, (%esp)
	//     movl %)
	//printf("%p\n");
	printf("idt_init finished\n");
	IRQ_clear_mask((unsigned char)1);
	IRQ_set_mask((unsigned char)0);
	asm("int $33");
	asm("int $0x2c");
	printf("after int $0x2c\n");
	printf("kernel finished - hlting\n");
	for(;;) {
		asm("hlt");
	}
}

