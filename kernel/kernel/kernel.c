#include <stdio.h>
#include <stdint.h>

#include <driver/tty.h>
#include <driver/serial.h>

#include <kernel/multiboot.h> //delete this in the future

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

void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
	terminal_initialize();
	init_serial();
	printf("kernel main start\n");
	void *pointer = (void *)mbd + 0xC0000000;
	mbd = (multiboot_info_t*) pointer;
	printf("mbd: %p\n", (void *)mbd);
	mmap_entry_t* entry = (mmap_entry_t*) (mbd->mmap_addr + 0xC0000000);
	while(entry < mbd->mmap_addr + mbd->mmap_length + 0xC0000000) {
		// do something with the entry
		printf("entry:\nbase addr: 0x%x%x, length: 0x%x%x, type: 0x%x\n\n", entry->base_addr_high, entry->base_addr_low,entry->length_high, entry->length_low, entry->type);
		entry = (mmap_entry_t*) ((unsigned int) entry + entry->size + sizeof(entry->size));
	}
	printf("kernel main finished, hlting\n");
	for(;;) {
		asm("hlt");
	}
}

