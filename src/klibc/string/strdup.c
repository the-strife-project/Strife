#include <klibc/stdlib.h>

char* strdup(const char* orig) {
	size_t s_orig = strlen(orig);
	char* ret = jmalloc(s_orig+1);
	strcpy(ret, orig);
	return ret;
}
