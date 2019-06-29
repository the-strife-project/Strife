#include <libc/stdio.h>
#include <kernel/drivers/keyboard/keyboard.h>

char* readLine() {
	keyboard_resume(1);
	char* ret = keyboard_getBuffer();
	while(ret[keyboard_getBuffered()-1] != '\n') {}

	ret[keyboard_getBuffered()-1] = 0;
	keyboard_pause();
	return ret;
}
