#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <driver/tty.h>
#include <driver/serial.h>

#include <kernel/mm.h>


/*
 * TODO: fix the fact that array out of range doesn't get caught
 * Testing and Unit Testing?
 * Running the os on real hardware
 * Hardware Abstraction - Device Management?
 * arch independent isr
 * interrupt driven io / io using DMA
 * improve format string printing
 * Timers - maybe PIT first?
 * improve kernel heap - tree instead of linked list
 * randomize Stack Smash Protector
 *
 * keep in mind Code Structuring + Portability
 */


void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
	terminal_initialize();
	init_serial();
	printf("kernel main start\n");
	mm_init(mbd, magic);
	ata_init(0);
	ata_init(1);
	printf("kernel main finished, hlting\n");
	for(;;) {
		asm("hlt");
	}
}

