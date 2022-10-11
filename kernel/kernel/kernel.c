#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/io.h>

#include <driver/tty.h>
#include <driver/serial.h>
#include <driver/ata.h>
#include <driver/pit.h>
#include <driver/ps2_controller.h>
#include <driver/ps2_keyboard.h>

#include <arch-i386/gdt.h>

#include <kernel/mm.h>
#include <kernel/fat.h>
#include <kernel/thread.h>


/*
 * TODO: fix the fact that array out of range doesn't get caught
 * Testing and Unit Testing?
 * Running the os on real hardware
 * Hardware Abstraction - Device Management?
 * arch independent isr
 * interrupt driven io / io using DMA
 * improve format string printing
 * improve kernel heap - tree instead of linked list
 * randomize Stack Smash Protector
 * lint - static code analysis
 *
 * keep in mind Code Structuring + Portability
 */


void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
	terminal_initialize();
	init_serial();
	mm_init(mbd, magic);
	ata_init(0);
	fat_init();
	pit_init();
	
	printf("kernel main start ticks: %q\n", get_nanoseconds_since_boot());
	init_multitasking();
	if (ps2_controller__init() == -1) {
		printf("ps2_controller__init failed!\n");
	}
	else {
		ps2_keyboard__init();
	}

	// TCB *second_task = new_kernel_thread(thread_task, "second");
	// new_kernel_thread(thread_task, "third");
	// printf("starting switch\n");
	// lock_scheduler();
	// schedule();
	// unlock_scheduler();

	// //thread_task();
	// int count = 0;
	// while (1) {
    //     printf("current_task_TCB->pid: %d - %s - time used: %q count: %d\n", current_task_TCB->pid, current_task_TCB->thread_name, current_task_TCB->time_used, count);
	// 	printf("time now is: %q\n", get_nanoseconds_since_boot());
	// 	if (count == 10) {
	// 		printf("terminate_task!!!!!!!!!!!!!!!!!! now is: %q\n", get_nanoseconds_since_boot());
	// 		terminate_task();
	// 	}
		
    //     // lock_scheduler();
    //     // schedule();
    //     // unlock_scheduler();
	// 	//sleep(1);
	// 	count++;
		
    // }

	// printf("kernel main finished, hlting  ticks: %q\n", get_nanoseconds_since_boot());
	for(;;) {
		asm("hlt");
	}
}

