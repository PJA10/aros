#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/serial.h>
#include <kernel/idt.h>
#include <kernel/pic.h>

/*
 * TODO: change to a proper GDT init
 * add double fault handler that dumps the contents of the registers
 *
 */


void kernel_main(void) {
	terminal_initialize();
	init_serial();
	idt_init();
	printf("idt_init finished\n");
	//asm("int $0");
	//printf("after int $0\n");
	asm("mov $0, %edx");
	asm("div %edx");
	printf("kernel finished - hlting\n");
	for(;;) {
		asm("hlt");
	}
}

