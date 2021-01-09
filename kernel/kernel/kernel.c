#include <stdio.h>

#include <driver/tty.h>
#include <driver/serial.h>

/*
 * TODO: fix the fact that array out of range doesn't get caught
 * Memory Managment - Physical Memory Allocators & Virtual Memory Management
 * Heap
 * Stack Smash Protector
 * Testing and Unit Testing
 * Running the os on real hardware
 * Device Management
 * Hardware Abstraction
 * arch independent isr
 * interrupt driven io / io using DMA
 * higher half kernel - permissions
 * Timers - maybe PIT first?
 *
 * keep in mind Code Structuring + Portability
 */


void kernel_main(void) {
	terminal_initialize();
	init_serial();
	printf("kernel main start\n");

	printf("kernel main finished, hlting\n");
	for(;;) {
		asm("hlt");
	}
}

