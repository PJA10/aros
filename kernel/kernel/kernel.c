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


void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
	terminal_initialize();
	init_serial();
	printf("kernel main start\n");
	mm_init(mbd, magic);
	ata_init(0);
	printf("---------------------------------------------\n");
	fat_init();
	printf("------\n");
	//fat_demo();
	printf("kernel main finished, hlting\n");
	for(;;) {
		asm("hlt");
	}
}

void fat_demo() {
	char str[20];
	for (size_t i = 0; i < 100; i++)
	{
		itoa(i, str, 10);
		char file_name[20] = "/";
		strcpy(file_name + 1, str);
		strcpy(file_name + 1 + strlen(str), ".TXT");
		fat_file_t *file = fat_open(file_name);
		if (file == NULL) {
			printf("error in opening\n");
		}
		else {
			char c[1024];
			while(file->eof != 1) {
				if(fat_read(file, c, 1024) == -1) {
					printf("error in reading\n");
				}
				printf("%s", c);
			}
			printf("\n");
			fat_close(file);
		}
	}
    fat_file_t *aviv_file = fat_open("/DIR/AVIV.TXT");
	if (aviv_file == NULL) {
		printf("error in opening\n");
	}
	else {
		char c[1024];
		while(aviv_file->eof != 1) {
			if(fat_read(aviv_file, c, 1024) == -1) {
				printf("error in reading\n");
			}
			printf("%s", c);
		}
		printf("\n");
		fat_close(aviv_file);
	}
}