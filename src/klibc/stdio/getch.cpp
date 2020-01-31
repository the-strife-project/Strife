#include <klibc/stdio.h>
#include <kernel/drivers/keyboard/keyboard.h>

char getch() {
	keyboard_resume(0);
	// TODO: Make this use keyboard's Silent Mode.
	while(!(keyboard_getBuffer().size())) {}

	keyboard_pause();
	return keyboard_getBuffer().get();
}
