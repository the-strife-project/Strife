#include <libc/stdio.h>
#include <kernel/drivers/keyboard/keyboard.h>

char getch() {
	keyboard_resume(0);
	while(!keyboard_getBuffered()) {}

	keyboard_pause();
	return *keyboard_getBuffer();
}
