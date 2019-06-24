#include <libc/string.h>

char strcmp(const char* a, const char* b) {
	size_t s_a = strlen(a);

	for(size_t i=0; i<s_a; i++) if(a[i] != b[i]) return 1;

	return 0;
}
