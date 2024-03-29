#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <driver/tty.h>
#include <driver/serial.h>
#include <driver/ata.h>

#include <kernel/mm.h>
#include <kernel/fat.h>


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
 * lint - static code analysis
 *
 * keep in mind Code Structuring + Portability
 */

static void fat_demo();

void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
	terminal_initialize();
	init_serial();
	ata_init(0);
	mm_init(mbd, magic);
	fat_init();
	printf("kernel main start\n");
	printf("kernel main finished, hlting\n");
	for(;;) {
		asm("hlt");
	}
}