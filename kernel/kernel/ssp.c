#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define STACK_CHK_GUARD 0xe2dee396
// TODO: randomize the guard value

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

__attribute__((noreturn)) void __stack_chk_fail(void) {
	printf("stack overflow detected \n");
	abort();
}
