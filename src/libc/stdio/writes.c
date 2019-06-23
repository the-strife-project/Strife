#include <libc/stdio.h>

void writes(const char* data) {
	for(size_t i=0; i<strlen(data); i++) writec(data[i]);
}
