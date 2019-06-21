#include <tty.h>
#include <stdio.h>

void writes(const char* data) {
	for(size_t i=0; i<strlen(data); i++) writec(data[i]);
}

void kernel_main(void) {
	terminal_initialize();
	writes("Hello, kernel World!\nAnother line bites the dust.\n");
}
