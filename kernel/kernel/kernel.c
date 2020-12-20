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
	asm("int $33");
}

