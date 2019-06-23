#include <libc/stdio.h>
#include <kernel/drivers/tty.h>

void writec(char c) {
	switch(c) {
		case '\n':
			terminal_goDown();
			break;
		default:
			terminal_writec(c);
	}
}
