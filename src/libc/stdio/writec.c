#include <stdio.h>
#include <tty.h>

void writec(char c) {
	switch(c) {
		case '\n':
			terminal_goDown();
			break;
		default:
			terminal_writec(c);
	}
}
