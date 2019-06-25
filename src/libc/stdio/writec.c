#include <libc/stdio.h>
#include <kernel/drivers/TTY/TTY.h>

void writec(char c) {
	switch(c) {
		case '\n':
			terminal_goDown();
			break;
		default:
			terminal_writec(c);
	}
}
