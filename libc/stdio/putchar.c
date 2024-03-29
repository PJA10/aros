#include <stdio.h>

#if defined(__is_libk)
#include <driver/tty.h>
#include <driver/serial.h>
#endif

int putchar(int ic) {
#if defined(__is_libk)
	char c = (char) ic;
	terminal_write(&c, sizeof(c));
	write_serial(c); // debug - print to serial port
#else
	// TODO: Implement stdio and the write system call.
#endif
	return ic;
}
