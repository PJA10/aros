#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/serial.h>

void kernel_main(void) {
	terminal_initialize();
	init_serial();
	str_write_serial("debug: boot - OK\n");
	for(int i = 0; i < 40; i++) {
		printf("%d Base 10- %x Base 16\n", i,i);
	}
	printf("debug: terminal scrolling  - OK\n");
	printf("itamar is a noob\n");
	str_write_serial("debug: serial debug - OK\n");
}

