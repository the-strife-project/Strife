#include <klibc/stdio.h>
#include <klibc/stdlib.h>
#include <kernel/drivers/keyboard/keyboard.h>

char* readLine() {
	keyboard_resume(1);
	char* buff = keyboard_getBuffer();
	while(1) {
		uint8_t br = 0;
		for(int i=0; i<keyboard_getBuffered(); i++) {
			if(buff[i] == '\n') {
				br = 1;
				break;
			}
		}
		if(br) break;
	}
	buff[keyboard_getBuffered()] = 0;

	char* ret = (char*)jmalloc(strlen(buff));
	size_t i=0;
	for(size_t j=0; j<strlen(buff); j++) {
		if(buff[j] != '\n') ret[i++] = buff[j];
	}
	ret[i] = 0;

	keyboard_pause();
	return ret;
}
